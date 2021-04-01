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


void Radarsat2ImageMetadataInterface::CreateCalibrationLookupData(const short type)
{
  std::string lut = "SigmaNought";

  switch (type)
  {
  case SarCalibrationLookupData::BETA:
  {
    lut = "BetaNought";
  }
  break;

  case SarCalibrationLookupData::GAMMA:
  {
    lut = "GammaNought";
  }
  break;

  case SarCalibrationLookupData::DN:
  {
    lut = "DN";
  }
  break;

  case SarCalibrationLookupData::SIGMA:
  default:
  {
    lut = "SigmaNought";
  }
  break;
  }

  const ImageKeywordlistType imageKeywordlist = this->GetImageKeywordlist();
  const std::string          key              = "referenceNoiseLevel[" + lut + "].gain";

  Radarsat2CalibrationLookupData::GainListType glist;
  int                                          offset = 0;

  Utils::ConvertStringToVector(imageKeywordlist.GetMetadataByKey("referenceNoiseLevel[" + lut + "].gain"), glist, "referenceNoiseLevel[" + lut + "].gain");

  Utils::LexicalCast<int>(imageKeywordlist.GetMetadataByKey("referenceNoiseLevel[" + lut + "].offset"), "referenceNoiseLevel[" + lut + "].offset");

  Radarsat2CalibrationLookupData::Pointer sarLut;
  sarLut = Radarsat2CalibrationLookupData::New();
  sarLut->InitParameters(type, offset, glist);
  this->SetCalibrationLookupData(sarLut);
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

double Radarsat2ImageMetadataInterface::GetCenterIncidenceAngle() const
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

//"


void Radarsat2ImageMetadataInterface::ParseGdal(const MetadataSupplierInterface & mds)
{
  // Metadata read by GDAL
  Fetch(MDTime::AcquisitionStartTime, mds, "ACQUISITION_START_TIME");
  // Fetch(MDTime::AcquisitionStopTime, mds, "PROCESSING_TIME"); 
  Fetch(MDStr::BeamMode, mds, "BEAM_MODE");
  Fetch("FACILITY_IDENTIFIER", mds, "FACILITY_IDENTIFIER");
  Fetch(MDNum::LineSpacing, mds, "LINE_SPACING");
  Fetch(MDNum::PixelSpacing, mds, "PIXEL_SPACING");
  // Fetch(MDStr::Mode, mds, "MODE");
  Fetch(MDStr::OrbitDirection, mds, "ORBIT_DIRECTION");
  // Fetch(MDNum::OrbitNumber, mds, "ORBIT_NUMBER");
  Fetch(MDStr::ProductType, mds, "PRODUCT_TYPE");
  Fetch(MDStr::Instrument, mds, "SATELLITE_IDENTIFIER");
  Fetch(MDStr::SensorID, mds, "SENSOR_IDENTIFIER");
  

  // Product file
  std::string ProductFilePath = mds.GetResourceFile("product.xml");
  if (!ProductFilePath.empty())
    {
    XMLMetadataSupplier ProductMS(ProductFilePath);
    m_Imd.Add(MDStr::Mission, ProductMS.GetAs<std::string>("product.sourceAttributes.satellite"));

    m_Imd.Add(MDNum::NumberOfLines, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfLines"));
    m_Imd.Add(MDNum::NumberOfColumns, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfSamplesPerLine"));
    m_Imd.Add(MDTime::ProductionDate,
        ProductMS.GetFirstAs<MetaData::Time>("product.imageGenerationParameters.generalProcessingInformation.processingTime"));
    // m_Imd.Add(MDNum::RadarFrequency, ProductMS.GetAs<double>("product.sourceAttributes.radarParameters.radarCenterFrequency"));
    m_Imd.Add(MDNum::AverageSceneHeight, ProductMS.GetAs<double>("product.imageAttributes.geographicInformation.referenceEllipsoidParameters.geodeticTerrainHeight"));


    m_Imd.Add(MDNum::RadarFrequency, this->GetRadarFrequency());
    m_Imd.Add(MDNum::PRF, this->GetPRF());
    m_Imd.Add(MDNum::RSF, this->GetRSF());
    m_Imd.Add(MDNum::CenterIncidenceAngle, this->GetCenterIncidenceAngle());


    assert(mds.GetNbBands() == this->m_Imd.Bands.size());

    SARParam sarParam;
    LoadRadiometricCalibrationData(sarParam);
    for (int bandId = 0 ; bandId < mds.GetNbBands() ; ++bandId)
      {
      Fetch(MDStr::Polarization, mds, "POLARIMETRIC_INTERP", bandId);
      m_Imd.Bands[bandId].Add(MDGeom::SAR, sarParam);
      }
    }
}

void Radarsat2ImageMetadataInterface::ParseGeom(const MetadataSupplierInterface & mds)
{
  // Metadata read by GDAL
  Fetch(MDTime::AcquisitionStartTime, mds, "support_data.image_date");
  Fetch(MDNum::LineSpacing, mds, "meters_per_pixel_y");
  Fetch(MDNum::PixelSpacing, mds, "meters_per_pixel_x");
  Fetch(MDStr::Instrument, mds, "sensor");
  m_Imd.Add(MDStr::SensorID, "SAR");  

  // Product file
  auto ProductFilePath = boost::filesystem::path(mds.GetResourceFile());
  if (!ProductFilePath.empty())
  {
    XMLMetadataSupplier ProductMS((ProductFilePath.remove_filename() /= "product.xml").string());
    m_Imd.Add(MDStr::Mission, ProductMS.GetAs<std::string>("product.sourceAttributes.satellite"));
    m_Imd.Add(MDNum::NumberOfLines, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfLines"));
    m_Imd.Add(MDNum::NumberOfColumns, ProductMS.GetAs<int>("product.imageAttributes.rasterAttributes.numberOfSamplesPerLine"));
    m_Imd.Add(MDTime::ProductionDate,
	      ProductMS.GetFirstAs<MetaData::Time>("product.imageGenerationParameters.generalProcessingInformation.processingTime"));
    m_Imd.Add(MDNum::AverageSceneHeight,
	      ProductMS.GetAs<double>("product.imageAttributes.geographicInformation.referenceEllipsoidParameters.geodeticTerrainHeight"));
    m_Imd.Add(MDNum::RadarFrequency, this->GetRadarFrequency());
    m_Imd.Add(MDNum::PRF, this->GetPRF());
    m_Imd.Add(MDNum::RSF, this->GetRSF());
    m_Imd.Add(MDNum::CenterIncidenceAngle, this->GetCenterIncidenceAngle());
    m_Imd.Add(MDStr::BeamMode, ProductMS.GetAs<std::string>("product.sourceAttributes.beamModeMnemonic"));
    m_Imd.Add("FACILITY_IDENTIFIER", ProductMS.GetAs<std::string>("product.sourceAttributes.inputDatasetFacilityId"));
    m_Imd.Add(MDStr::OrbitDirection, ProductMS.GetAs<std::string>("product.sourceAttributes.orbitAndAttitude.orbitInformation.passDirection"));
    m_Imd.Add(MDStr::ProductType, ProductMS.GetAs<std::string>("product.imageGenerationParameters.generalProcessingInformation.productType"));

    auto polarizations = ProductMS.GetAsVector<std::string>("product.sourceAttributes.radarParameters.polarizations");
    assert(polarizations.size() == m_Imd.Bands.size());
    SARParam sarParam;
    LoadRadiometricCalibrationData(sarParam);
    for (int bandId = 0 ; bandId < polarizations.size() ; ++bandId)
    {
      m_Imd.Bands[bandId].Add(MDStr::Polarization, polarizations[bandId]);
      m_Imd.Bands[bandId].Add(MDGeom::SAR, sarParam);
    }
  }  
}

void Radarsat2ImageMetadataInterface::Parse(const MetadataSupplierInterface & mds)
{
  // Try to fetch the metadata from GDAL Metadata Supplier
  if (mds.GetAs<std::string>("", "SATELLITE_IDENTIFIER") == "RADARSAT-2")
    this->ParseGdal(mds);
  // Try to fetch the metadata from GEOM file
  else if (mds.GetAs<std::string>("", "sensor") == "RADARSAT-2")
    this->ParseGeom(mds);
  // Failed to fetch the metadata
  else
    otbGenericExceptionMacro(MissingMetadataException,
			     << "Not a Sentinel1 product");
}

} // end namespace otb
