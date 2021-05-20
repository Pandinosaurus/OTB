/*
 * Copyright (C) 2005-2020 Centre National d'Etudes Spatiales (CNES)
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


#include "otbSarImageMetadataInterface.h"
#include "otbRadarsat2ImageMetadataInterface.h"
#include "otbSARMetadata.h"

#include "otbMacro.h"
#include "itkMetaDataObject.h"
#include "otbImageKeywordlist.h"

// useful constants
#include <otbMath.h>
#include "otbXMLMetadataSupplier.h"
#include <boost/filesystem.hpp> 
#include "itksys/SystemTools.hxx"

namespace otb
{

Radarsat2ImageMetadataInterface::Radarsat2ImageMetadataInterface()
{
}

bool Radarsat2ImageMetadataInterface::CanRead() const
{
  std::string sensorID = GetSensorID();

  if (sensorID.find("RADARSAT-2") != std::string::npos)
  {
    return true;
  }
  else
    return false;
}

bool Radarsat2ImageMetadataInterface::HasCalibrationLookupDataFlag(const MetadataSupplierInterface &mds) const
{
  return mds.GetAs<bool>(true, "support_data.calibration_lookup_flag");
}

bool Radarsat2ImageMetadataInterface::CreateCalibrationLookupData(SARCalib& sarCalib,
                                                                  const ImageMetadata& imd,
                                                                  const MetadataSupplierInterface& mds,
                                                                  const bool geom) const
{

  if(SarImageMetadataInterface::CreateCalibrationLookupData(sarCalib, imd, mds, geom))
    return true;

  int offset = 0;

  std::ostringstream ossSigma;
  std::ostringstream ossBeta;
  std::ostringstream ossGamma;
  if(geom)
  {
    ossSigma << "referenceNoiseLevel[SigmaNought].gain";
    ossBeta << "referenceNoiseLevel[BetaNought].gain";
    ossGamma << "referenceNoiseLevel[GammaNought].gain";
  }
  else
  {
    ossSigma << "product.sourceAttributes.radarParameters.referenceNoiseLevel_"
             << mds.GetAttributId("product.sourceAttributes.radarParameters.referenceNoiseLevel_#.incidenceAngleCorrection",
                                  "Sigma Nought")
             << ".noiseLevelValues";
    ossBeta << "product.sourceAttributes.radarParameters.referenceNoiseLevel_"
            << mds.GetAttributId("product.sourceAttributes.radarParameters.referenceNoiseLevel_#.incidenceAngleCorrection",
                                 "Beta Nought")
            << ".noiseLevelValues";
    ossGamma << "product.sourceAttributes.radarParameters.referenceNoiseLevel_"
             << mds.GetAttributId("product.sourceAttributes.radarParameters.referenceNoiseLevel_#.incidenceAngleCorrection",
                                  "gamma")
             << ".noiseLevelValues";
  }

  Radarsat2CalibrationLookupData::Pointer sigmaSarLut = Radarsat2CalibrationLookupData::New();
  auto glist = mds.GetAsVector<float>(ossSigma.str());
  sigmaSarLut->InitParameters(SarCalibrationLookupData::SIGMA, offset, glist);
  sarCalib.calibrationLookupData[SarCalibrationLookupData::SIGMA] = std::move(sigmaSarLut);

  Radarsat2CalibrationLookupData::Pointer betaSarLut = Radarsat2CalibrationLookupData::New();
  glist = mds.GetAsVector<float>(ossSigma.str());
  betaSarLut->InitParameters(SarCalibrationLookupData::BETA, offset, glist);
  sarCalib.calibrationLookupData[SarCalibrationLookupData::BETA] = std::move(betaSarLut);

  Radarsat2CalibrationLookupData::Pointer gammaSarLut = Radarsat2CalibrationLookupData::New();
  glist = mds.GetAsVector<float>(ossSigma.str());
  gammaSarLut->InitParameters(SarCalibrationLookupData::GAMMA, offset, glist);
  sarCalib.calibrationLookupData[SarCalibrationLookupData::GAMMA] = std::move(gammaSarLut);

  Radarsat2CalibrationLookupData::Pointer dnSarLut = Radarsat2CalibrationLookupData::New();
  sarCalib.calibrationLookupData[SarCalibrationLookupData::DN] = std::move(dnSarLut);

  return true;
}

void Radarsat2ImageMetadataInterface::ParseDateTime(const char* key, std::vector<int>& dateFields) const
{
  if (dateFields.size() < 1)
  {
    // parse from keyword list
    if (!this->CanRead())
    {
      itkExceptionMacro(<< "Invalid Metadata, not a valid product");
    }

    const ImageKeywordlistType imageKeywordlist = this->GetImageKeywordlist();
    if (!imageKeywordlist.HasKey(key))
    {
      itkExceptionMacro(<< "no key named '" << key << "'");
    }

    std::string date_time_str = imageKeywordlist.GetMetadataByKey(key);
    date_time_str.resize(date_time_str.size() - 1);
    Utils::ConvertStringToVector(date_time_str, dateFields, key, "-T:.");
  }
}



int Radarsat2ImageMetadataInterface::GetYear() const
{
  int value = 0;
  ParseDateTime("support_data.image_date", m_AcquisitionDateFields);
  if (m_AcquisitionDateFields.size() > 0)
  {
    value = Utils::LexicalCast<int>(m_AcquisitionDateFields[0], "support_data.image_date(year)");
  }
  return value;
}

int Radarsat2ImageMetadataInterface::GetMonth() const
{
  int value = 0;
  ParseDateTime("support_data.image_date", m_AcquisitionDateFields);
  if (m_AcquisitionDateFields.size() > 1)
  {
    value = Utils::LexicalCast<int>(m_AcquisitionDateFields[1], "support_data.image_date(month)");
  }
  return value;
}

int Radarsat2ImageMetadataInterface::GetDay() const
{
  int value = 0;
  ParseDateTime("support_data.image_date", m_AcquisitionDateFields);
  if (m_AcquisitionDateFields.size() > 2)
  {
    value = Utils::LexicalCast<int>(m_AcquisitionDateFields[2], "support_data.image_date(day)");
  }
  return value;
}

int Radarsat2ImageMetadataInterface::GetHour() const
{
  int value = 0;
  ParseDateTime("support_data.image_date", m_AcquisitionDateFields);
  if (m_AcquisitionDateFields.size() > 3)
  {
    value = Utils::LexicalCast<int>(m_AcquisitionDateFields[3], "support_data.image_date(hour)");
  }
  return value;
}

int Radarsat2ImageMetadataInterface::GetMinute() const
{
  int value = 0;
  ParseDateTime("support_data.image_date", m_AcquisitionDateFields);
  if (m_AcquisitionDateFields.size() > 4)
  {
    value = Utils::LexicalCast<int>(m_AcquisitionDateFields[4], "support_data.image_date(minute)");
  }
  return value;
}

int Radarsat2ImageMetadataInterface::GetProductionYear() const
{
  int value = 0;
  ParseDateTime("support_data.date", m_ProductionDateFields);
  if (m_ProductionDateFields.size() > 0)
  {
    value = Utils::LexicalCast<int>(m_ProductionDateFields[0], "support_data.image_date(year)");
  }
  return value;
}

int Radarsat2ImageMetadataInterface::GetProductionMonth() const
{
  int value = 0;
  ParseDateTime("support_data.date", m_ProductionDateFields);
  if (m_ProductionDateFields.size() > 1)
  {
    value = Utils::LexicalCast<int>(m_ProductionDateFields[1], "support_data.image_date(production month)");
  }
  return value;
}

int Radarsat2ImageMetadataInterface::GetProductionDay() const
{
  int value = 0;
  ParseDateTime("support_data.date", m_ProductionDateFields);
  if (m_ProductionDateFields.size() > 2)
  {
    value = Utils::LexicalCast<int>(m_ProductionDateFields[2], "support_data.image_date(production day)");
  }
  return value;
}


double Radarsat2ImageMetadataInterface::GetPRF() const
{
  return 0;
}

double Radarsat2ImageMetadataInterface::GetRSF() const
{
  return 0;
}

double Radarsat2ImageMetadataInterface::GetRadarFrequency() const
{
  return 0;
}

double Radarsat2ImageMetadataInterface::GetCenterIncidenceAngle(const MetadataSupplierInterface &) const
{
  return 0;
}

Radarsat2ImageMetadataInterface::UIntVectorType Radarsat2ImageMetadataInterface::GetDefaultDisplay() const
{
  UIntVectorType rgb(3);
  rgb[0] = 0;
  rgb[1] = 0;
  rgb[2] = 0;
  return rgb;
}

void Radarsat2ImageMetadataInterface::ParseGdal(ImageMetadata & imd)
{
  // Product file
  std::string ProductFilePath =
      itksys::SystemTools::GetParentDirectory(m_MetadataSupplierInterface->GetResourceFile(""))
      + "/product.xml";
  if (!itksys::SystemTools::FileExists(ProductFilePath))
  {
    otbGenericExceptionMacro(MissingMetadataException,
           << "Cannot find the Radarsat2 product.xml file");
  }
  XMLMetadataSupplier ProductMS(ProductFilePath);

  // Polarization
  std::string ImageFilePath = itksys::SystemTools::GetFilenameName(m_MetadataSupplierInterface->GetResourceFile(""));
  imd.Add(MDStr::Polarization, ImageFilePath.substr(8, 2));

  imd.Add(MDTime::AcquisitionStartTime, ProductMS.GetAs<MetaData::Time>("product.sourceAttributes.rawDataStartTime"));
  imd.Add(MDStr::BeamMode, ProductMS.GetAs<std::string>("product.sourceAttributes.beamModeMnemonic"));
  imd.Add("FACILITY_IDENTIFIER", ProductMS.GetAs<std::string>("product.sourceAttributes.inputDatasetFacilityId"));
  imd.Add(MDNum::LineSpacing, ProductMS.GetAs<double>("product.imageAttributes.rasterAttributes.sampledLineSpacing"));
  imd.Add(MDNum::PixelSpacing, ProductMS.GetAs<double>("product.imageAttributes.rasterAttributes.sampledPixelSpacing"));
  imd.Add(MDStr::OrbitDirection, ProductMS.GetAs<std::string>("product.sourceAttributes.orbitAndAttitude.orbitInformation.passDirection"));
  imd.Add(MDStr::ProductType, ProductMS.GetAs<std::string>("product.imageGenerationParameters.generalProcessingInformation.productType"));
  imd.Add(MDStr::Instrument, ProductMS.GetAs<std::string>("product.sourceAttributes.satellite"));
  imd.Add(MDStr::SensorID, ProductMS.GetAs<std::string>("product.sourceAttributes.sensor"));
  imd.Add(MDStr::Mission, ProductMS.GetAs<std::string>("product.sourceAttributes.satellite"));
  imd.Add(MDNum::NumberOfLines, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfLines"));
  imd.Add(MDNum::NumberOfColumns, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfSamplesPerLine"));
  imd.Add(MDTime::ProductionDate, ProductMS.GetFirstAs<MetaData::Time>("product.imageGenerationParameters.generalProcessingInformation.processingTime"));
  imd.Add(MDNum::AverageSceneHeight, ProductMS.GetAs<double>("product.imageAttributes.geographicInformation.referenceEllipsoidParameters.geodeticTerrainHeight"));
  imd.Add(MDNum::RadarFrequency, this->GetRadarFrequency());
  imd.Add(MDNum::PRF, this->GetPRF());
  imd.Add(MDNum::RSF, this->GetRSF());
  imd.Add(MDNum::CenterIncidenceAngle, this->GetCenterIncidenceAngle(ProductMS));

  SARParam sarParam;
  imd.Add(MDGeom::SAR, sarParam);

  SARCalib sarCalib;
  LoadRadiometricCalibrationData(sarCalib, ProductMS, imd);
  CreateCalibrationLookupData(sarCalib, imd, ProductMS, false);
  imd.Add(MDGeom::SARCalib, sarCalib);
}

void Radarsat2ImageMetadataInterface::ParseGeom(ImageMetadata & imd)
{
  Fetch(MDTime::AcquisitionStartTime, imd, "support_data.image_date");
  Fetch(MDNum::LineSpacing, imd, "meters_per_pixel_y");
  Fetch(MDNum::PixelSpacing, imd, "meters_per_pixel_x");
  Fetch(MDStr::Instrument, imd, "sensor");
  imd.Add(MDStr::SensorID, "SAR");

  // Product file
  auto ProductFilePath = boost::filesystem::path(m_MetadataSupplierInterface->GetResourceFile("geom"));  // TODO C++ 2017 use std::filesystem

  if (!ProductFilePath.empty())
  {
    ProductFilePath = ProductFilePath.remove_filename() /= "product.xml";
    if(boost::filesystem::exists(ProductFilePath))
    {
      XMLMetadataSupplier ProductMS((ProductFilePath.remove_filename() /= "product.xml").string());
      imd.Add(MDStr::Mission, ProductMS.GetAs<std::string>("product.sourceAttributes.satellite"));
      imd.Add(MDNum::NumberOfLines, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfLines"));
      imd.Add(MDNum::NumberOfColumns, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfSamplesPerLine"));
      imd.Add(MDTime::ProductionDate,
          ProductMS.GetFirstAs<MetaData::Time>("product.imageGenerationParameters.generalProcessingInformation.processingTime"));
      imd.Add(MDNum::AverageSceneHeight,
          ProductMS.GetAs<double>("product.imageAttributes.geographicInformation.referenceEllipsoidParameters.geodeticTerrainHeight"));
      imd.Add(MDNum::RadarFrequency, this->GetRadarFrequency());
      imd.Add(MDNum::PRF, this->GetPRF());
      imd.Add(MDNum::RSF, this->GetRSF());
      imd.Add(MDNum::CenterIncidenceAngle, this->GetCenterIncidenceAngle(ProductMS));
      imd.Add(MDStr::BeamMode, ProductMS.GetAs<std::string>("product.sourceAttributes.beamModeMnemonic"));
      imd.Add("FACILITY_IDENTIFIER", ProductMS.GetAs<std::string>("product.sourceAttributes.inputDatasetFacilityId"));
      imd.Add(MDStr::OrbitDirection, ProductMS.GetAs<std::string>("product.sourceAttributes.orbitAndAttitude.orbitInformation.passDirection"));
      imd.Add(MDStr::ProductType, ProductMS.GetAs<std::string>("product.imageGenerationParameters.generalProcessingInformation.productType"));
    }
  }

  // Polarization
  auto polarization = m_MetadataSupplierInterface->GetResourceFile("image");
  if(!polarization.empty())
  {
    polarization = itksys::SystemTools::GetFilenameName(polarization);
    polarization = polarization.substr(polarization.rfind("_") + 1, 2);
    imd.Add(MDStr::Polarization, polarization);
  }

  // SAR Model
  SARParam sarParam;
  imd.Add(MDGeom::SAR, sarParam);
  SARCalib sarCalib;
  LoadRadiometricCalibrationData(sarCalib, *m_MetadataSupplierInterface, imd);
  CreateCalibrationLookupData(sarCalib, imd, *m_MetadataSupplierInterface, true);
  imd.Add(MDGeom::SARCalib, sarCalib);
}

void Radarsat2ImageMetadataInterface::Parse(ImageMetadata & imd)
{
  // Try to fetch the metadata from GEOM file
  if (m_MetadataSupplierInterface->GetAs<std::string>("", "sensor") == "RADARSAT-2")
    this->ParseGeom(imd);
  // Try to fetch the metadata from GDAL Metadata Supplier
  this->ParseGdal(imd);
}

} // end namespace otb
