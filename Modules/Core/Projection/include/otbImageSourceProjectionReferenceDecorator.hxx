/*
 * Copyright (C) 2005-2026 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef otbImageSourceProjectionReferenceDecorator_hxx
#define otbImageSourceProjectionReferenceDecorator_hxx

#include "otbImageSourceProjectionReferenceDecorator.h"
#include "otbPositionHelpers.h"
#include "otbTupleIterator.h"
#include "otbImageMetadata.h"
#include "itkImageScanlineIterator.h"
#include "itkProgressReporter.h"
#include "otbMetaDataKey.h"
#include <cassert>

template <typename TImageProducer, typename TActual>
void
otb::details::ProjectionReferenceDecorator<TImageProducer, TActual>
::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();

  auto const& projection_ref = static_cast<TActual const*>(this)->GetOutputProjectionRef();

  // This may not be the most efficient, but there is no hook dedicated to fill the metadata...
  OutputImagePointer output  = this->GetOutput();
  assert(output);
  ImageMetadataBase  meta    = output->GetImageMetadata();
  meta.Add(MDGeom::ProjectionWKT, projection_ref);
  output->SetImageMetadata(std::move(meta));

  // Check the need for reprojecting image coordinates into WGS84
  static auto const& wgs84Srs = OGRSpatialReference::GetWGS84SRS();
  assert(wgs84Srs->GetAxisMappingStrategy() == OAMS_TRADITIONAL_GIS_ORDER);

  OGRSpatialReference outputSrs(projection_ref.c_str());
  // Make sure to use the same axis order in order to not mix coordinates orders with transforming
  outputSrs.SetAxisMappingStrategy(wgs84Srs->GetAxisMappingStrategy());
  if (!outputSrs.IsSame(wgs84Srs))
  {
    otbLogMacro(Info, << "Output projection is " << projection_ref << " => need to convert to WGS84");
    auto const nbThreads = this->GetNumberOfThreads();
    m_tlsCTs.reserve(nbThreads);
    for (auto i = 0u; i != nbThreads; ++i)
    {
#if 1
      m_tlsCTs.emplace_back(OGRCreateCoordinateTransformation(&outputSrs, wgs84Srs));
#else
      // Eventually when we can have a vector<CoordinateTransformation>
      m_tlsCTs.emplace_back(&outputSrs, wgs84Srs);
#endif
    }
  }
  else
  {
    otbLogMacro(Info, << "Output projection is " << projection_ref << " => NO need to convert to WGS84");
  }
}


template <typename TImageProducer, typename TActual>
template <typename TCommandOnLine, typename TCommandOnPixel, typename... TOtherIterators>
void
otb::details::ProjectionReferenceDecorator<TImageProducer, TActual>
::DoGenerateTileDataLineWise(
    TCommandOnLine               do_on_line,
    TCommandOnPixel              do_on_pixel,
    OutputImageRegionType const& outputRegionForThread,
    itk::ThreadIdType            threadId,
    TOtherIterators        &&... other_iterators)
{
  using OutputIterator = itk::ImageScanlineIterator<OutputImageType>;
  OutputIterator OutIt(this->GetOutput(), outputRegionForThread);

  auto iterators = wrap_iterators_in_tuple(OutIt, std::forward<TOtherIterators>(other_iterators)...);

  auto const& o_size      = outputRegionForThread.GetSize();
  auto const  o_nbcolumns = o_size[0];
  auto const  o_nblines   = o_size[1];

  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels() / o_nblines);

  std::vector<double> buffer_lonlats(o_nbcolumns * 2);  // <-- TODO: This could be cached in TLS
  auto*               poCT = m_tlsCTs.empty() ? nullptr : m_tlsCTs[threadId].get();

  iterators.GoToBegin();
  for (; !iterators.IsAtEnd(); iterators.NextLine())
  {
    iterators.GoToBeginOfLine();
    assert(OutIt.GetIndex() == iterators.template GetIndex<0>());

    // ---[ Compute lon/lat for all pixels of the current line
    lonlat_view_t lonlat_view(buffer_lonlats.data(), otb::narrow{}, o_nbcolumns);
    assert(lonlat_view.size() == std::ptrdiff_t(buffer_lonlats.size()));
    GeneratePhysicalPointCoordinatesForLine(lonlat_view, *this->GetOutput(), OutIt);

    if (poCT)
    {
      ConvertPhysicalPointCoordinatesToWGS84(lonlat_view, *poCT);
    }
    do_on_line(
        lonlat_view,
        OutIt,
        std::forward<TOtherIterators>(other_iterators)...
    );

    // ---[ Do the actual work on the line
    for (std::ptrdiff_t col_idx = 0; !iterators.IsAtEndOfLine(); ++col_idx, ++iterators)
    {
      assert(col_idx < std::ptrdiff_t(o_nbcolumns));
      auto lonlat_coord = ExtractLonLat(lonlat_view, col_idx);
      do_on_pixel(
          lonlat_coord,
          col_idx,
          OutIt,
          std::forward<TOtherIterators>(other_iterators)...
      );
    }
    progress.CompletedPixel();
  }
}

#endif  // otbImageSourceProjectionReferenceDecorator_hxx
