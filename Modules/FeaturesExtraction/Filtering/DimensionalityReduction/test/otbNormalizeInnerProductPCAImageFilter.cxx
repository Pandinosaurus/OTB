/*
 * Copyright (C) 2005-2024 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "otbNormalizeInnerProductPCAImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbVectorImage.h"

int otbNormalizeInnerProductPCAImageFilter(int itkNotUsed(argc), char* argv[])
{
  typedef double     PixelType;
  const unsigned int Dimension      = 2;
  const char*        inputFileName  = argv[1];
  const char*        outputFilename = argv[2];
  typedef otb::VectorImage<PixelType, Dimension> ImageType;
  typedef otb::ImageFileReader<ImageType> ReaderType;
  typedef otb::ImageFileWriter<ImageType> WriterType;
  typedef otb::NormalizeInnerProductPCAImageFilter<ImageType, ImageType> NormalizePCAFilterType;

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFileName);
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);

  NormalizePCAFilterType::Pointer normalizepcafilter = NormalizePCAFilterType::New();

  normalizepcafilter->SetInput(reader->GetOutput());
  writer->SetInput(normalizepcafilter->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;
}
