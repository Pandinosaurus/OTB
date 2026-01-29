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

#ifndef otbImageSourceGeometryDecorator_hxx
#define otbImageSourceGeometryDecorator_hxx

#include "otbImageSourceGeometryDecorator.h"

template <typename TImageSource>
void
otb::ImageSourceGeometryDecorator<TImageSource>
::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();

  OutputImagePointer output = this->GetOutput();
  // ----[ Specify region parameters
  IndexType             start{0, 0};
  OutputImageRegionType largestPossibleRegion{start, m_OutputSize};

  output->SetLargestPossibleRegion(largestPossibleRegion);
  output->SetSignedSpacing(m_OutputSpacing);
  output->SetOrigin(m_OutputOrigin);
}


template <typename TImageSource>
void
otb::ImageSourceGeometryDecorator<TImageSource>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Output Spacing:" << m_OutputSpacing[0] << "," << m_OutputSpacing[1] << '\n';
  os << indent << "Output Origin: " << m_OutputOrigin[0]  << "," << m_OutputOrigin[1]  << '\n';
  os << indent << "Output Size:   " << m_OutputSize[0]    << "," << m_OutputSize[1]    << '\n';
}
#endif  // otbImageSourceGeometryDecorator_hxx
