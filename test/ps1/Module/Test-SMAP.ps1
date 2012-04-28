
$scope = 2
$path = [IO.Path]::GetTempFileName()
$name = (Split-Path -Leaf $path).Split('.')[0]

if (Test-Path $path)
{
    Remove-Item $path
}

New-H5Drive $name $path -RW -Force -Scope $scope -Core

cd "$($name):"

New-H5Attribute . GRingPointLongitude "GRingPointLongitude" string$("GRingPointLongitude".Length)
                    
New-H5Attribute . GRringPointLatitude "GRringPointLatitude" string$("GRringPointLatitude".Length)
                    
New-H5Attribute . GapStartDateTime "GapStartDateTime" string$("GapStartDateTime".Length)
                    
New-H5Attribute . GapStopDateTime "GapStopDateTime" string$("GapStopDateTime".Length)
                    
New-H5Attribute . Metadata_Conventions "Unidata Dataset Discovery v1.0, CF-1.5, ESDIS 1.0" string$("Unidata Dataset Discovery v1.0, CF-1.5, ESDIS 1.0".Length)
                    
New-H5Attribute . Metadata_Link "http://smap.nasa.gov/completeMetadata.xml" string$("http://smap.nasa.gov/completeMetadata.xml".Length)
                    
New-H5Attribute . NumberOfGaps "NumberOfGaps" string$("NumberOfGaps".Length)
                    
New-H5Attribute . PermanentWaterBodyThreshold "PermanentWaterBodyThreshold" string$("PermanentWaterBodyThreshold".Length)
                    
New-H5Attribute . RFIThreshold "RFIThreshold" string$("RFIThreshold".Length)
                    
New-H5Attribute . SizeMBECSDataGranule "SizeMBECSDataGranule" string$("SizeMBECSDataGranule".Length)
                    
New-H5Attribute . acknowledgment "acknowledgment" string$("acknowledgment".Length)
                    
New-H5Attribute . cdm_data_type "grid" string$("grid".Length)
                    
New-H5Attribute . comment "comment" string$("comment".Length)
                    
New-H5Attribute . id "GranuleName Short name > Long name" string$("GranuleName Short name > Long name".Length)
                    
New-H5Attribute . license ' '  string1
New-H5Attribute . processing_level "processing_level" string$("processing_level".Length)
                    
New-H5Attribute . standard_name_vocabulary "standard_name_vocabulary" string$("standard_name_vocabulary".Length)
                    
mkdir /LI_Lineage
cd /LI_Lineage
mkdir /LI_Lineage/LE_Source
cd /LI_Lineage/LE_Source
mkdir /LI_Lineage/LE_Source/1
cd /LI_Lineage/LE_Source/1
mkdir /LI_Lineage/LE_Source/2
cd /LI_Lineage/LE_Source/2
mkdir /acquisitionInformation_1
cd /acquisitionInformation_1
mkdir /acquisitionInformation_1/instrument_1
cd /acquisitionInformation_1/instrument_1
New-H5Attribute . description "Brief instrument description" string$("Brief instrument description".Length)
                    
New-H5Attribute . identifier "InstrumentShortName" string$("InstrumentShortName".Length)
                    
New-H5Attribute . mountedOn "UUID" string$("UUID".Length)
                    
New-H5Attribute . type "GCMD Instrument Keyword" string$("GCMD Instrument Keyword".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/instrument_1/citation_1
cd /acquisitionInformation_1/instrument_1/citation_1
New-H5Attribute . title "InstrumentShortName > InstrumentLongName" string$("InstrumentShortName > InstrumentLongName".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/instrument_1/citation_1/citedResponsibleParty_1
cd /acquisitionInformation_1/instrument_1/citation_1/citedResponsibleParty_1
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "author" string$("author".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/instrument_1/citation_1/citedResponsibleParty_1/onlineResource_1
cd /acquisitionInformation_1/instrument_1/citation_1/citedResponsibleParty_1/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/instrument_1/citation_1/date_1
cd /acquisitionInformation_1/instrument_1/citation_1/date_1
New-H5Attribute . date "2012-03-06" string$("2012-03-06".Length)
                    
New-H5Attribute . dateType "publication" string$("publication".Length)
                    
mkdir /acquisitionInformation_1/platform_1
cd /acquisitionInformation_1/platform_1
New-H5Attribute . description "PlatformType" string$("PlatformType".Length)
                    
New-H5Attribute . identifier "PlatformShortName > PlatformLongName" string$("PlatformShortName > PlatformLongName".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/platform_1/citation_1
cd /acquisitionInformation_1/platform_1/citation_1
New-H5Attribute . title "PlatformShortName > PlatformLongName" string$("PlatformShortName > PlatformLongName".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/platform_1/citation_1/citedResponsibleParty_1
cd /acquisitionInformation_1/platform_1/citation_1/citedResponsibleParty_1
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "author" string$("author".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/platform_1/citation_1/citedResponsibleParty_1/onlineResource_1
cd /acquisitionInformation_1/platform_1/citation_1/citedResponsibleParty_1/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /acquisitionInformation_1/platform_1/citation_1/date_1
cd /acquisitionInformation_1/platform_1/citation_1/date_1
New-H5Attribute . date "2012-03-06" string$("2012-03-06".Length)
                    
New-H5Attribute . dateType "publication" string$("publication".Length)
                    
mkdir /contact_1
cd /contact_1
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "pointOfContact" string$("pointOfContact".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /contact_1/onlineResource_1
cd /contact_1/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /contact_2
cd /contact_2
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "pointOfContact" string$("pointOfContact".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /contact_2/onlineResource_1
cd /contact_2/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1
cd /dataQualityInfo_1
New-H5Attribute . scope "dataset" string$("dataset".Length)
                    
mkdir /dataQualityInfo_1/lineage
cd /dataQualityInfo_1/lineage
mkdir /dataQualityInfo_1/lineage/processStep_1
cd /dataQualityInfo_1/lineage/processStep_1
New-H5Attribute . dateTime "ProductionDateTime" string$("ProductionDateTime".Length)
                    
New-H5Attribute . output "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . source "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/processStep_1/processingInformation_1
cd /dataQualityInfo_1/lineage/processStep_1/processingInformation_1
New-H5Attribute . identifier "SPSIdentifier" string$("SPSIdentifier".Length)
                    
mkdir /dataQualityInfo_1/lineage/processStep_1/processingInformation_1/algorithm_1
cd /dataQualityInfo_1/lineage/processStep_1/processingInformation_1/algorithm_1
New-H5Attribute . description "AlgorithmDescriptor" string$("AlgorithmDescriptor".Length)
                    
mkdir /dataQualityInfo_1/lineage/processStep_1/processingInformation_1/algorithm_1/citation_1
cd /dataQualityInfo_1/lineage/processStep_1/processingInformation_1/algorithm_1/citation_1
New-H5Attribute . edition "AlgorithmPackageVersionID" string$("AlgorithmPackageVersionID".Length)
                    
New-H5Attribute . identifier "AlgorithmPackageMaturityCode" string$("AlgorithmPackageMaturityCode".Length)
                    
New-H5Attribute . title "AlgorithmTitle" string$("AlgorithmTitle".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/processStep_1/processingInformation_1/algorithm_1/citation_1/date_1
cd /dataQualityInfo_1/lineage/processStep_1/processingInformation_1/algorithm_1/citation_1/date_1
New-H5Attribute . dateType "publication" string$("publication".Length)
                    
mkdir /dataQualityInfo_1/lineage/processStep_1/processor_1
cd /dataQualityInfo_1/lineage/processStep_1/processor_1
New-H5Attribute . organisationName "ProductionLocationCode" string$("ProductionLocationCode".Length)
                    
New-H5Attribute . role "processor" string$("processor".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_1
cd /dataQualityInfo_1/lineage/source_1
New-H5Attribute . description "Radar Level 1A Product Description" string$("Radar Level 1A Product Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_1/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_1/sourceCitation_1
New-H5Attribute . edition "Radar Level 1A Product Edition" string$("Radar Level 1A Product Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/RadarLevel1AProduct.h5" string$("http://smap.jpl.nasa.gov/RadarLevel1AProduct.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_1/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_1/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_2
cd /dataQualityInfo_1/lineage/source_2
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_2/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_2/sourceCitation_1
New-H5Attribute . edition "SMAP Epehemeris Data Product Edition" string$("SMAP Epehemeris Data Product Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/SMAPEpehemerisDataProduct.h5" string$("http://smap.jpl.nasa.gov/SMAPEpehemerisDataProduct.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_2/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_2/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_3
cd /dataQualityInfo_1/lineage/source_3
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_3/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_3/sourceCitation_1
New-H5Attribute . edition "SMAP Attitude Data Product Edition" string$("SMAP Attitude Data Product Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/SMAPAttitudeDataProduct.h5" string$("http://smap.jpl.nasa.gov/SMAPAttitudeDataProduct.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_3/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_3/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_4
cd /dataQualityInfo_1/lineage/source_4
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_4/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_4/sourceCitation_1
New-H5Attribute . edition "Input Antenna Pointing Data Product Edition" string$("Input Antenna Pointing Data Product Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/InputAntennaPointingDataProduct.h5" string$("http://smap.jpl.nasa.gov/InputAntennaPointingDataProduct.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_4/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_4/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_5
cd /dataQualityInfo_1/lineage/source_5
New-H5Attribute . description "Digital Elevation Model Product Description" string$("Digital Elevation Model Product Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_5/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_5/sourceCitation_1
New-H5Attribute . edition "Digital Elevation Model Data Product Edition" string$("Digital Elevation Model Data Product Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/DigitalElevationModelDataProduct.h5" string$("http://smap.jpl.nasa.gov/DigitalElevationModelDataProduct.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_5/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_5/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_6
cd /dataQualityInfo_1/lineage/source_6
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_6/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_6/sourceCitation_1
New-H5Attribute . edition "Input Product Configuration File Edition" string$("Input Product Configuration File Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/InputProductConfigurationFile.h5" string$("http://smap.jpl.nasa.gov/InputProductConfigurationFile.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_6/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_6/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_7
cd /dataQualityInfo_1/lineage/source_7
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_7/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_7/sourceCitation_1
New-H5Attribute . edition "Output Product Configuration File Edition" string$("Output Product Configuration File Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/OutputProductConfigurationFile.h5" string$("http://smap.jpl.nasa.gov/OutputProductConfigurationFile.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_7/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_7/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_8
cd /dataQualityInfo_1/lineage/source_8
New-H5Attribute . description "Metadata Configuration File Description" string$("Metadata Configuration File Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_8/sourceCitation_1
cd /dataQualityInfo_1/lineage/source_8/sourceCitation_1
New-H5Attribute . edition "Metadata Configuration File Edition" string$("Metadata Configuration File Edition".Length)
                    
New-H5Attribute . title "http://smap.jpl.nasa.gov/MetadataConfigurationFile.h5" string$("http://smap.jpl.nasa.gov/MetadataConfigurationFile.h5".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_1/lineage/source_8/sourceCitation_1/date_1
cd /dataQualityInfo_1/lineage/source_8/sourceCitation_1/date_1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /dataQualityInfo_2
cd /dataQualityInfo_2
New-H5Attribute . scope "dataset" string$("dataset".Length)
                    
mkdir /dataQualityInfo_2/lineage
cd /dataQualityInfo_2/lineage
mkdir /dataQualityInfo_2/lineage/processStep_1
cd /dataQualityInfo_2/lineage/processStep_1
New-H5Attribute . dateTime "ProductionDateTime" string$("ProductionDateTime".Length)
                    
New-H5Attribute . output "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . source "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/processStep_1/processor_1
cd /dataQualityInfo_2/lineage/processStep_1/processor_1
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "processor" string$("processor".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/processStep_1/processor_1/onlineResource_1
cd /dataQualityInfo_2/lineage/processStep_1/processor_1/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_1
cd /dataQualityInfo_2/lineage/source_1
New-H5Attribute . description "Radar Level 1A Product Description" string$("Radar Level 1A Product Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_1/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_1/sourceCitation_1
New-H5Attribute . edition "Radar Level 1A Product Version" string$("Radar Level 1A Product Version".Length)
                    
New-H5Attribute . title "Radar Level 1A Product Name" string$("Radar Level 1A Product Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_2
cd /dataQualityInfo_2/lineage/source_2
New-H5Attribute . description "SMAP Epehemeris Data Product Description" string$("SMAP Epehemeris Data Product Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_2/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_2/sourceCitation_1
New-H5Attribute . edition "SMAP Epehemeris Data Product Version" string$("SMAP Epehemeris Data Product Version".Length)
                    
New-H5Attribute . title "SMAP Epehemeris Data Product Name" string$("SMAP Epehemeris Data Product Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_3
cd /dataQualityInfo_2/lineage/source_3
New-H5Attribute . description "SMAP Attitude Data Product Description" string$("SMAP Attitude Data Product Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_3/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_3/sourceCitation_1
New-H5Attribute . edition "SMAP Attitude Data Product Version" string$("SMAP Attitude Data Product Version".Length)
                    
New-H5Attribute . title "SMAP Attitude Data Product Name" string$("SMAP Attitude Data Product Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_4
cd /dataQualityInfo_2/lineage/source_4
New-H5Attribute . description "Input Antenna Pointing Data Product Description" string$("Input Antenna Pointing Data Product Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_4/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_4/sourceCitation_1
New-H5Attribute . edition "Input Antenna Pointing Data Product Version" string$("Input Antenna Pointing Data Product Version".Length)
                    
New-H5Attribute . title "Input Antenna Pointing Data Product Name" string$("Input Antenna Pointing Data Product Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_5
cd /dataQualityInfo_2/lineage/source_5
New-H5Attribute . description "Digital Elevation Model Product Description" string$("Digital Elevation Model Product Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_5/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_5/sourceCitation_1
New-H5Attribute . edition "Digital Elevation Model Product Version" string$("Digital Elevation Model Product Version".Length)
                    
New-H5Attribute . title "Digital Elevation Model Product Name" string$("Digital Elevation Model Product Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_6
cd /dataQualityInfo_2/lineage/source_6
New-H5Attribute . description "Input Product Configuration File Description" string$("Input Product Configuration File Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_6/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_6/sourceCitation_1
New-H5Attribute . edition "Input Product Configuration File Version" string$("Input Product Configuration File Version".Length)
                    
New-H5Attribute . title "Input Product Configuration File Name" string$("Input Product Configuration File Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_7
cd /dataQualityInfo_2/lineage/source_7
New-H5Attribute . description "Output Product Configuration File Description" string$("Output Product Configuration File Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_7/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_7/sourceCitation_1
New-H5Attribute . edition "Output Product Configuration File Version" string$("Output Product Configuration File Version".Length)
                    
New-H5Attribute . title "Output Product Configuration File Name" string$("Output Product Configuration File Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_8
cd /dataQualityInfo_2/lineage/source_8
New-H5Attribute . description "Metadata Configuration File Description" string$("Metadata Configuration File Description".Length)
                    
New-H5Attribute . sourceStep "UUID,UUID,UUID" string$("UUID,UUID,UUID".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /dataQualityInfo_2/lineage/source_8/sourceCitation_1
cd /dataQualityInfo_2/lineage/source_8/sourceCitation_1
New-H5Attribute . edition "Metadata Configuration File Version" string$("Metadata Configuration File Version".Length)
                    
New-H5Attribute . title "Metadata Configuration File Name" string$("Metadata Configuration File Name".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_1
cd /identificationInfo_1
New-H5Attribute . summary "CollectionDescription]" string$("CollectionDescription]".Length)
                    
mkdir /identificationInfo_1/citation_1
cd /identificationInfo_1/citation_1
New-H5Attribute . identifier "/gmi:MI_Metadata/gmd:identificationInfo[1]/gmd:MD_DataIdentification[1]/gmd:citation[1]/gmd:CI_Citation[1]/gmd:identifier[1]/gmd:MD_Identifier[1]/gmd:code[1]/gco:CharacterString[1]" string$("/gmi:MI_Metadata/gmd:identificationInfo[1]/gmd:MD_DataIdentification[1]/gmd:citation[1]/gmd:CI_Citation[1]/gmd:identifier[1]/gmd:MD_Identifier[1]/gmd:code[1]/gco:CharacterString[1]".Length)
                    
New-H5Attribute . title "GranuleName" string$("GranuleName".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_1/citation_1/citedResponsibleParty_1
cd /identificationInfo_1/citation_1/citedResponsibleParty_1
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "originator" string$("originator".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_1/citation_1/citedResponsibleParty_1/onlineResource_1
cd /identificationInfo_1/citation_1/citedResponsibleParty_1/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_1/citation_1/date_1
cd /identificationInfo_1/citation_1/date_1
New-H5Attribute . date ' '  string1
New-H5Attribute . dateType "creation" string$("creation".Length)
                    
mkdir /identificationInfo_1/extent_1
cd /identificationInfo_1/extent_1
New-H5Attribute . beginPosition "RangeBeginningDateTRangeBeginningTime" string$("RangeBeginningDateTRangeBeginningTime".Length)
                    
New-H5Attribute . eastBoundLongitude "EastBoundingCoordinate" string$("EastBoundingCoordinate".Length)
                    
New-H5Attribute . endPosition "RangeEndingDateTRangeEndingTime" string$("RangeEndingDateTRangeEndingTime".Length)
                    
New-H5Attribute . geographicIdentifier "OrbitPathNumber" string$("OrbitPathNumber".Length)
                    
New-H5Attribute . northBoundLongitude "NorthBoundingCoordinate" string$("NorthBoundingCoordinate".Length)
                    
New-H5Attribute . southBoundLongitude "SouthBoundingCoordinate" string$("SouthBoundingCoordinate".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
New-H5Attribute . westBoundLongitude "WestBoundingCoordinate" string$("WestBoundingCoordinate".Length)
                    
mkdir /identificationInfo_1/pointOfContact_1
cd /identificationInfo_1/pointOfContact_1
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "pointOfContact" string$("pointOfContact".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_1/pointOfContact_1/onlineResource_1
cd /identificationInfo_1/pointOfContact_1/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2
cd /identificationInfo_2
New-H5Attribute . purpose "Scientific Research and Understanding" string$("Scientific Research and Understanding".Length)
                    
New-H5Attribute . summary "CollectionDescription" string$("CollectionDescription".Length)
                    
mkdir /identificationInfo_2/citation_1
cd /identificationInfo_2/citation_1
New-H5Attribute . edition "VersionID" string$("VersionID".Length)
                    
New-H5Attribute . identifier "ShortName DOI" string$("ShortName DOI".Length)
                    
New-H5Attribute . title "ShortName > LongName" string$("ShortName > LongName".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/citation_1/citedResponsibleParty_1
cd /identificationInfo_2/citation_1/citedResponsibleParty_1
New-H5Attribute . organisationName "NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA" string$("NASA/JPL/SMAP > Soil Moisture Active Passive, Jet Propulsion Laboratory, NASA".Length)
                    
New-H5Attribute . role "originator" string$("originator".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/citation_1/citedResponsibleParty_1/onlineResource_1
cd /identificationInfo_2/citation_1/citedResponsibleParty_1/onlineResource_1
New-H5Attribute . function "information" string$("information".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/citation_1/date_1
cd /identificationInfo_2/citation_1/date_1
New-H5Attribute . date ' '  string1
New-H5Attribute . dateType "publication" string$("publication".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_1
cd /identificationInfo_2/descriptiveKeywords_1
New-H5Attribute . keyword_1 "keyword_1" string$("keyword_1".Length)
                    
New-H5Attribute . keyword_2 "keyword_2" string$("keyword_2".Length)
                    
New-H5Attribute . keyword_3 "keyword_3" string$("keyword_3".Length)
                    
New-H5Attribute . type "theme" string$("theme".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_1/thesaurusName_1
cd /identificationInfo_2/descriptiveKeywords_1/thesaurusName_1
New-H5Attribute . title "title" string$("title".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_1/thesaurusName_1/date_1
cd /identificationInfo_2/descriptiveKeywords_1/thesaurusName_1/date_1
New-H5Attribute . date "date" string$("date".Length)
                    
New-H5Attribute . dateType "revision" string$("revision".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_2
cd /identificationInfo_2/descriptiveKeywords_2
New-H5Attribute . keyword "SMAP > Soil Moisture Active Passive" string$("SMAP > Soil Moisture Active Passive".Length)
                    
New-H5Attribute . type "project" string$("project".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_2/thesaurusName_1
cd /identificationInfo_2/descriptiveKeywords_2/thesaurusName_1
New-H5Attribute . title "GCMD Project Keywords" string$("GCMD Project Keywords".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_2/thesaurusName_1/date_1
cd /identificationInfo_2/descriptiveKeywords_2/thesaurusName_1/date_1
New-H5Attribute . date "date" string$("date".Length)
                    
New-H5Attribute . dateType "revision" string$("revision".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_3
cd /identificationInfo_2/descriptiveKeywords_3
New-H5Attribute . keyword "keyword_1" string$("keyword_1".Length)
                    
New-H5Attribute . type "dataCenter" string$("dataCenter".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_3/thesaurusName_1
cd /identificationInfo_2/descriptiveKeywords_3/thesaurusName_1
New-H5Attribute . title "title" string$("title".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_3/thesaurusName_1/date_1
cd /identificationInfo_2/descriptiveKeywords_3/thesaurusName_1/date_1
New-H5Attribute . date "date" string$("date".Length)
                    
New-H5Attribute . dateType "revision" string$("revision".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_4
cd /identificationInfo_2/descriptiveKeywords_4
New-H5Attribute . keyword "keyword_1" string$("keyword_1".Length)
                    
New-H5Attribute . type "place" string$("place".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_4/thesaurusName_1
cd /identificationInfo_2/descriptiveKeywords_4/thesaurusName_1
New-H5Attribute . title "title" string$("title".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_4/thesaurusName_1/date_1
cd /identificationInfo_2/descriptiveKeywords_4/thesaurusName_1/date_1
New-H5Attribute . date "date" string$("date".Length)
                    
New-H5Attribute . dateType "revision" string$("revision".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_5
cd /identificationInfo_2/descriptiveKeywords_5
New-H5Attribute . keyword "keyword_1" string$("keyword_1".Length)
                    
New-H5Attribute . type "instrument" string$("instrument".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_5/thesaurusName_1
cd /identificationInfo_2/descriptiveKeywords_5/thesaurusName_1
New-H5Attribute . title "title" string$("title".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_5/thesaurusName_1/date_1
cd /identificationInfo_2/descriptiveKeywords_5/thesaurusName_1/date_1
New-H5Attribute . date "date" string$("date".Length)
                    
New-H5Attribute . dateType "revision" string$("revision".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_6
cd /identificationInfo_2/descriptiveKeywords_6
New-H5Attribute . keyword "keyword_1" string$("keyword_1".Length)
                    
New-H5Attribute . type "platform" string$("platform".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_6/thesaurusName_1
cd /identificationInfo_2/descriptiveKeywords_6/thesaurusName_1
New-H5Attribute . title "title" string$("title".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_6/thesaurusName_1/date_1
cd /identificationInfo_2/descriptiveKeywords_6/thesaurusName_1/date_1
New-H5Attribute . date "date" string$("date".Length)
                    
New-H5Attribute . dateType "revision" string$("revision".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_7
cd /identificationInfo_2/descriptiveKeywords_7
New-H5Attribute . keyword "keyword_1" string$("keyword_1".Length)
                    
New-H5Attribute . type "dataResolution" string$("dataResolution".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_7/thesaurusName_1
cd /identificationInfo_2/descriptiveKeywords_7/thesaurusName_1
New-H5Attribute . title "title" string$("title".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/descriptiveKeywords_7/thesaurusName_1/date_1
cd /identificationInfo_2/descriptiveKeywords_7/thesaurusName_1/date_1
New-H5Attribute . date "date" string$("date".Length)
                    
New-H5Attribute . dateType "revision" string$("revision".Length)
                    
mkdir /identificationInfo_2/extent_1
cd /identificationInfo_2/extent_1
New-H5Attribute . beginPosition "seriesBeginningDate" string$("seriesBeginningDate".Length)
                    
New-H5Attribute . description "brief description of the extent (could be spatial keyword)" string$("brief description of the extent (could be spatial keyword)".Length)
                    
New-H5Attribute . eastBoundLongitude "EastBoundingCoordinate" string$("EastBoundingCoordinate".Length)
                    
New-H5Attribute . endPosition ' '  string1
New-H5Attribute . northBoundLatitude "NorthBoundingCoordinate" string$("NorthBoundingCoordinate".Length)
                    
New-H5Attribute . southBoundLatitude "SouthBoundingCoordinate" string$("SouthBoundingCoordinate".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
New-H5Attribute . westBoundLongitude "WestBoundingCoordinate" string$("WestBoundingCoordinate".Length)
                    
mkdir /identificationInfo_2/pointOfContact_1
cd /identificationInfo_2/pointOfContact_1
New-H5Attribute . organisationName "NASA > National Aeronautics and Space Administration" string$("NASA > National Aeronautics and Space Administration".Length)
                    
New-H5Attribute . role "pointOfContact" string$("pointOfContact".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /identificationInfo_2/pointOfContact_1/onlineResource_1
cd /identificationInfo_2/pointOfContact_1/onlineResource_1
New-H5Attribute . function "pointOfContact" string$("pointOfContact".Length)
                    
New-H5Attribute . linkage "http://smap.jpl.nasa.gov/" string$("http://smap.jpl.nasa.gov/".Length)
                    
New-H5Attribute . uuid "UUID" string$("UUID".Length)
                    
mkdir /orbitalInformation
cd /orbitalInformation
New-H5Attribute . argumentOfPerigee "555.555" string$("555.555".Length)
                    
New-H5Attribute . eccentricity "111.111" string$("111.111".Length)
                    
New-H5Attribute . inclination "222.222" string$("222.222".Length)
                    
New-H5Attribute . period "444.444" string$("444.444".Length)
                    
New-H5Attribute . semiMajorAxis "333.333" string$("333.333".Length)
                    
mkdir /processingInformation
cd /processingInformation
New-H5Attribute . RFIThreshold ' '  string1
New-H5Attribute . permanentWaterBodyThreshold "47.5" string$("47.5".Length)

c:

Remove-H5Drive $name -Scope $scope 
