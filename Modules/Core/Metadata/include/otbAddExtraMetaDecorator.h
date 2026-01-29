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

#ifndef otbAddExtraMetaDecorator_h
#define otbAddExtraMetaDecorator_h

#include "OTBMetadataExport.h"
#include "otbImageMetadata.h"
#include <cassert>
#include <itkSmartPointer.h>
#include <itkMacro.h>
#include <unordered_map>

namespace otb
{

/**
 * Helper decorator class that permits to inject extra image metadata without needing to alter other domain
 * specific filters.
 * \tparam TImageProducer  An ITK image source, or an image filter.
 *
 * \author Luc Hermitte
 * \copyright CS Group
 */
template <typename TImageProducer>
class OTBMetadata_EXPORT_TEMPLATE AddExtraMetaDecorator
: public TImageProducer
{
public:

  /**\name Standard class typedefs */
  //@{
  using Self                  = AddExtraMetaDecorator;
  using Superclass            = TImageProducer;
  using Pointer               = itk::SmartPointer<Self>;
  using ConstPointer          = itk::SmartPointer<Self const>;

  using OutputImageType       = typename TImageProducer::OutputImageType;

  using OutputImagePointer    = typename OutputImageType::Pointer;
  using SpacingType           = typename OutputImageType::SpacingType;
  using SizeType              = typename OutputImageType::SizeType;
  using PointType             = typename OutputImageType::PointType;
  using IndexType             = typename OutputImageType::IndexType;
  using PixelType             = typename OutputImageType::PixelType;
  using OutputImageRegionType = typename Superclass::OutputImageRegionType;
  //@}

  /**
   * Registers extra textual metadata.
   * @param[in] key metadata key
   * @param[in] value metadata value
   */
  void RegisterExtraMetadata(std::string key, std::string value)
  {
    m_extra_meta.emplace(std::move(key), std::move(value));
  }

protected:
  /** Run-time type information (and related methods). */
  itkTypeMacro(Self, Unused);

protected:
  /// Forward constructor
  using Superclass::Superclass;

  /**
   * Add the registered extra keys.
   */
  void GenerateOutputInformation() override
  {
    Superclass::GenerateOutputInformation();
    OutputImagePointer output  = this->GetOutput();
    assert(output);
    ImageMetadata      meta    = output->GetImageMetadata();
    for (auto const& kv : m_extra_meta)
    {
      meta.Add(kv.first, kv.second);
    }
    output->SetImageMetadata(std::move(meta));
  }

private:
  std::unordered_map<std::string, std::string> m_extra_meta;
};

}

#endif // otbAddExtraMetaDecorator_h
