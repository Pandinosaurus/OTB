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

#ifndef otbNoDataHelper_h
#define otbNoDataHelper_h
#include <vector>
#include <cassert>
#include "vnl/vnl_math.h"
#include <itkVariableLengthVector.h>
#include "OTBMetadataExport.h"
#include "vcl_legacy_aliases.h"

namespace itk
{
class MetaDataDictionary;
}

namespace otb
{
class ImageMetadataBase;
class ImageMetadata;

/**
 * Reads no data flag from the ImageMetadata to flags and values vectors.
 * \returns true upon success.
 */
bool OTBMetadata_EXPORT ReadNoDataFlags(const ImageMetadata & imd, std::vector<bool>& flags, std::vector<double>& values);

/**
 * Write no data flags to the ImageMetadata from flags and values vectors.
 * \returns true upon success.
 */
void OTBMetadata_EXPORT WriteNoDataFlags(const std::vector<bool>& flags, const std::vector<double>& values, ImageMetadata & imd);



/**
* Test if the pixel corresponds to a no data pixel according to a
* vector of no data flags, and a vector of no data values.
* \param[in] pixel The pixel to test
* \param[in] flags A vector of size > 1 containing a flag per band to
* indicate if a no data value is available for this band
* \param[in] values A vector of size > 1 corresponding to the no data
* value for each band. If flag is 0, the value will be ignored.
* \param[in] nanIsNoData If true, NaN values will be reported as no-data.
*
* \pre neither `flags`, nor `values` shal be empty
*/
template <typename T>
bool IsNoData(const T& pixel, const std::vector<bool>& flags, const std::vector<double>& values, bool nanIsNoData = false)
{
  assert(flags.size() > 0);
  assert(values.size() > 0);

  return ((nanIsNoData && vnl_math_isnan(pixel)) || (flags[0] && (pixel == values[0])));
}

/**
* Reads a pixel and change the no data value if it is found.
* No data value is changed either if the pixel value is NaN or if the pixel
* value equals the no data value and flag is true.
*
* \param[in] pixel The pixel to process \param flags A vector of size > 1
* containing a flag per band to indicate if a no data value is
* available for this band
* \param[in] values A vector of size > 1 corresponding to the current no data
* value for each band. If flag is 0, the value will be ignored.
* \param[in] newValues A vector of size > 1 corresponding to the new no data
* value for each band. If flag is 0, the value will be ignored
* \param[in] nanIsNoData If true, NaN values will be considered as no-data and changed as well.
*
* \pre neither `flags`, `values`, nor `newValues` shal be empty
*/
template <typename T>
T ChangeNoData(
    const T& pixel,
    const std::vector<bool>& flags,
    const std::vector<double>& values,
    const std::vector<double>& newValues,
    bool nanIsNoData = false)
{
  assert(flags.size() > 0);
  assert(values.size() > 0);
  assert(newValues.size() > 0);

  if (nanIsNoData && vnl_math_isnan(pixel))
  {
    return static_cast<T>(newValues[0]);
  }

  if (flags[0] && pixel == values[0])
  {
    return static_cast<T>(newValues[0]);
  }
  return pixel;
}


/**
 * Specialization of `IsNoData` function to handle `itk::VariableLengthVector`
 */
template <typename T>
bool IsNoData(const itk::VariableLengthVector<T>& pixel, const std::vector<bool>& flags, const std::vector<double>& values, bool nanIsNoData = false)
{
  assert(flags.size() >= pixel.Size());
  assert(values.size() >= pixel.Size());

  for (unsigned int i = 0; i < pixel.Size(); ++i)
  {
    if ((nanIsNoData && vnl_math_isnan(pixel[i])) || (flags[i] && (pixel[i] == values[i])))
    {
      return true;
    }
  }
  return false;
}


/**
 * Specialization of `ChangeNoData` function to handle `itk::VariableLengthVector`
 */
template <typename T>
itk::VariableLengthVector<T> ChangeNoData(const itk::VariableLengthVector<T>& pixel, const std::vector<bool>& flags, const std::vector<double>& values,
                                          const std::vector<double>& newValues, bool nanIsNoData = false)
{
  assert(flags.size() >= pixel.Size());
  assert(values.size() >= pixel.Size());
  assert(newValues.size() >= pixel.Size());

  itk::VariableLengthVector<T> outPixel(pixel.Size());

  for (unsigned int i = 0; i < pixel.Size(); ++i)
  {
    if ((nanIsNoData && vnl_math_isnan(pixel[i])) || (flags[i] && (pixel[i] == values[i])))
    {
      outPixel[i] = newValues[i];
    }
    else
    {
      outPixel[i] = pixel[i];
    }
  }

  return outPixel;
}

/**
 * Portable function to extract novalue data from metadata.
 * \return the nodata value found in the first band that has nodata
 * \return `NaN` when there is no meta data in the input image
 *
 * Unlike the `otb::ReadNoDataFlags`, this function considers all bands are supposed to return the
 * same metadata value. This matches the use cases related to S1Tiling support applications that
 * works on GeoTIFF images -- GeoTIFF images can't have different value for the nodata information
 * in its bands.
 */
double OTBMetadata_EXPORT ExtractNoDataValue(ImageMetadata const& meta);

/**
 * Extract NoData Value from metadata.
 * \see `ExtractNoDataValue(ImageMetadata const&)`
 */
template <typename TImageType>
double ExtractNoDataValueFromImage(TImageType const& image)
{
  auto const& meta = image.GetImageMetadata();
  return ExtractNoDataValue(meta);
}

/** Helper class to complete Band nodata values incrementally.
 * This object is meant to be used from images `GenerateOutputInformation()` functions.
 *
 * Band meta data will contain name and nodata value.
 */
class /*OTBMetadata_EXPORT*/ BandInformation
{
public:
  enum Bands { clear, append };

  /**
   * Constructor.
   * Initialize the band information filler object.
   *
   * \param[in,out] meta        Metadata to which the obj. shall be associated
   * \param[in] previous_bands  Tells whether all previous bands will be
   *                            removed or wheither we will append extra bands
   *                            to ones already declared.
   */
  BandInformation(ImageMetadata& meta, Bands previous_bands);

  /**
   * Add information about a new band.
   *
   * \param[in] name   Name for a new band
   * \param[in] nodata No-data value for the new band
   * \return a reference to the band metadata object currently added.
   * \warning adding another band can invalidate the returned reference
   */
  ImageMetadataBase& add_new(std::string name, double nodata);

private:

  ImageMetadata & m_meta;
};


} // End namespace otb

#endif
