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

#ifndef otbPositionHelpers_h
#define otbPositionHelpers_h

#include "OTBProjectionExport.h"
#include "otbRepack.h"
#include "otbMdSpan.h"
#include "itkImageScanlineConstIterator.h"

class OGRCoordinateTransformation;

namespace otb
{
using lonlat_view_t = otb::mdspan<double, 2, otb::dynamic_extent>;

/**
 * Produces the _Physical Point Coordinates_ for the iterable image input line.
 * \tparam ImageType Any type of image
 * \param[out] lonlats  Output 2D view where _Physical Point_ coordinates will be produced.
 *                      In WGS84 case, the first row would have contained longitudes, and the second
 *                      row would have contained latitudes for all points.
 * \param[in]  image    Image used for _index_ to _Physical Point_ translation
 * \param[in]  itInLine Input iterable line from which point indexes will be extracted.
 *
 * \post `lonlats` has 2 rows, each column corresponds to a pixel coordinate in
 *       its image spatial reference.
 * \throw None?
 * \note Data are organised as a _Struct of Arrays_
 */
template <typename ImageType>
void GeneratePhysicalPointCoordinatesForLine(
    lonlat_view_t                                lonlats,
    ImageType const&                             image,
    itk::ImageScanlineConstIterator< ImageType > itInLine
)
{
  for (std::ptrdiff_t idx = 0; !itInLine.IsAtEndOfLine(); ++itInLine, ++idx)
  {
    // assert(idx < line_size);

    // Transform index to Lat/Lon Point
    itk::Point<double, 2> demLonLatPoint = image.TransformIndexToPhysicalPoint(itInLine.GetIndex());
    lonlats(0, idx) = demLonLatPoint[0];
    lonlats(1, idx) = demLonLatPoint[1];
  }
}

/**
 * Makes sure _Physical Point_ coordinates are expressed in WGS84.
 * \param[in,out] lonlats 2D array of physical point coordinates that will be
 *                        converted into GWS84 (lon lat) spatial reference.
 * \param[in]     ct      OGR coordinate transformation instance
 *
 * \pre `lonlats` has 2 rows, each column corresponds to a pixel coordinate in
 *      its image spatial reference.
 * \pre The input physical points coordinates are not already in WGS84. The
 *      function is not expected to be called in that case.
 * \post `lonlats` first row will contain longitudes
 * \post `lonlats` second row will contain latitudes
 *
 * \throw None?
 * \note Data are organised as a _Struct of Arrays_
 */
OTBProjection_EXPORT
void ConvertPhysicalPointCoordinatesToWGS84(lonlat_view_t lonlats, OGRCoordinateTransformation& ct);

/**
 * Extracts {lon,lat} from a view.
 * This helper function gather longitude and latitude from the idx-th point in the 2d view.
 * \param[in] lonlat_view  View of lon, lat points
 * \param[in] idx  Index of the point to extract
 *
 * \return the {lon, lat} point
 * \throw None
 * \pre `idx < lonlat_view_t.extent(1)`
 */
inline
itk::Point<double, 2> ExtractLonLat(lonlat_view_t lonlat_view, std::ptrdiff_t idx)
{
  using Point2DType = itk::Point<double, 2>;
  return otb::repack<Point2DType>(
      lonlat_view(0, idx),
      lonlat_view(1, idx)
  );
}

}  // otb namespace


#endif  // otbPositionHelpers_h
