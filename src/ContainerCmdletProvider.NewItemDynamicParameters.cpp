
#include "Provider.h"
#include "ProviderUtils.h"
#include "PSH5XException.h"

extern "C" {
#include "H5public.h"
}

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
    Object^ Provider::NewItemDynamicParameters(String^ path, String^ itemTypeName, Object^ newValue)
    {
        WriteVerbose(String::Format("HDF5Provider::NewItemDynamicParameters(Path = '{0}', ItemTypeName = '{1}')",
            path, itemTypeName));


        if (itemTypeName == nullptr || itemTypeName->Trim() == "") {
            throw gcnew PSH5XException("Item type must not be empty. Use -ItemType to specify!");
        }

        if (!ProviderUtils::ResolveItemType(itemTypeName, itemTypeName)) {
            throw gcnew PSH5XException("Invalid item type");
        }

        RuntimeDefinedParameterDictionary^ dict = gcnew RuntimeDefinedParameterDictionary();

        if (itemTypeName->ToUpper() == "DATASET")
        {

#pragma region Dataset

            // ElementType

            ParameterAttribute^ attr1 = gcnew ParameterAttribute();
            attr1->Mandatory = true;
            attr1->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramElementType = gcnew RuntimeDefinedParameter();
            paramElementType->Name = "ElementType";
            // can be String or Hashtable
            paramElementType->ParameterType = String::typeid;
            paramElementType->Attributes->Add(attr1);

            dict->Add("ElementType", paramElementType);

            // Dimensions

            ParameterAttribute^ attr2 = gcnew ParameterAttribute();
            attr2->Mandatory = false;
            attr2->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramDimensions = gcnew RuntimeDefinedParameter();
            paramDimensions->Name = "Dimensions";
            paramDimensions->ParameterType = array<hsize_t>::typeid;
            paramDimensions->Attributes->Add(attr2);

            dict->Add("Dimensions", paramDimensions);

            // MaxDimensions

            ParameterAttribute^ attr3 = gcnew ParameterAttribute();
            attr3->Mandatory = false;
            attr3->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramMaxDimensions = gcnew RuntimeDefinedParameter();
            paramMaxDimensions->Name = "MaxDimensions";
            paramMaxDimensions->ParameterType = array<long long>::typeid;
            paramMaxDimensions->Attributes->Add(attr3);

            dict->Add("MaxDimensions", paramMaxDimensions);

            // Chunked

            ParameterAttribute^ attr4 = gcnew ParameterAttribute();
            attr4->Mandatory = false;
            attr4->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramChunked = gcnew RuntimeDefinedParameter();
            paramChunked->Name = "Chunked";
            paramChunked->ParameterType = array<hsize_t>::typeid;
            paramChunked->Attributes->Add(attr4);

            dict->Add("Chunked", paramChunked);

            // Compact

            ParameterAttribute^ attr5 = gcnew ParameterAttribute();
            attr5->Mandatory = false;
            attr5->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramCompact = gcnew RuntimeDefinedParameter();
            paramCompact->Name = "Compact";
            paramCompact->ParameterType = SwitchParameter::typeid;
            paramCompact->Attributes->Add(attr5);

            dict->Add("Compact", paramCompact);

            // Gzip

            ParameterAttribute^ attr6 = gcnew ParameterAttribute();
            attr6->Mandatory = false;
            attr6->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramGzip = gcnew RuntimeDefinedParameter();
            paramGzip->Name = "Gzip";
            paramGzip->ParameterType = UInt32::typeid;
            paramGzip->Attributes->Add(attr6);

            dict->Add("Gzip", paramGzip);

			// Scalar

            ParameterAttribute^ attr7 = gcnew ParameterAttribute();
            attr7->Mandatory = false;
            attr7->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramScalar = gcnew RuntimeDefinedParameter();
            paramScalar->Name = "Scalar";
            paramScalar->ParameterType = SwitchParameter::typeid;
            paramScalar->Attributes->Add(attr7);

            dict->Add("Scalar", paramScalar);

			// Null

            ParameterAttribute^ attr8 = gcnew ParameterAttribute();
            attr8->Mandatory = false;
            attr8->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramNull = gcnew RuntimeDefinedParameter();
            paramNull->Name = "Null";
            paramNull->ParameterType = SwitchParameter::typeid;
            paramNull->Attributes->Add(attr8);

            dict->Add("Null", paramNull);

#pragma endregion

        }
        else if (itemTypeName->ToUpper() == "DATATYPE")
        {
            // Definition

            ParameterAttribute^ attr1 = gcnew ParameterAttribute();
            attr1->Mandatory = true;
            attr1->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramTypeDef = gcnew RuntimeDefinedParameter();
            paramTypeDef->Name = "Definition";
            paramTypeDef->ParameterType = String::typeid;
            paramTypeDef->Attributes->Add(attr1);

            dict->Add("Definition", paramTypeDef);
        }
        else if (itemTypeName->ToUpper() == "GROUP")
        {
#pragma region Group

			// Track link order

			ParameterAttribute^ attr1 = gcnew ParameterAttribute();
            attr1->Mandatory = false;
            attr1->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramTrackLinkOrder = gcnew RuntimeDefinedParameter();
            paramTrackLinkOrder->Name = "TrackLinkOrder";
            paramTrackLinkOrder->ParameterType = SwitchParameter::typeid;
            paramTrackLinkOrder->Attributes->Add(attr1);

            dict->Add("TrackLinkOrder", paramTrackLinkOrder);

			// Index link order

			ParameterAttribute^ attr2 = gcnew ParameterAttribute();
            attr2->Mandatory = false;
            attr2->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramIndexLinkOrder = gcnew RuntimeDefinedParameter();
            paramIndexLinkOrder->Name = "IndexLinkOrder";
            paramIndexLinkOrder->ParameterType = SwitchParameter::typeid;
            paramIndexLinkOrder->Attributes->Add(attr2);

            dict->Add("IndexLinkOrder", paramIndexLinkOrder);

			// Track attribute order

			ParameterAttribute^ attr3 = gcnew ParameterAttribute();
            attr3->Mandatory = false;
            attr3->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramTrackAttrOrder = gcnew RuntimeDefinedParameter();
            paramTrackAttrOrder->Name = "TrackAttributeOrder";
            paramTrackAttrOrder->ParameterType = SwitchParameter::typeid;
            paramTrackAttrOrder->Attributes->Add(attr3);

            dict->Add("TrackAttributeOrder", paramTrackAttrOrder);

			// Index attribute order

			ParameterAttribute^ attr4 = gcnew ParameterAttribute();
            attr4->Mandatory = false;
            attr4->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramIndexAttrOrder = gcnew RuntimeDefinedParameter();
            paramIndexAttrOrder->Name = "IndexAttributeOrder";
            paramIndexAttrOrder->ParameterType = SwitchParameter::typeid;
            paramIndexAttrOrder->Attributes->Add(attr4);

			dict->Add("IndexAttributeOrder", paramIndexAttrOrder);

#pragma endregion
        }
        else if (itemTypeName->ToUpper() == "IMAGE")
        {
#pragma region Image

            ParameterAttribute^ attr1 = gcnew ParameterAttribute();
            attr1->Mandatory = false;
            attr1->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramBits = gcnew RuntimeDefinedParameter();
            paramBits->Name = "Bits";
            paramBits->ParameterType = UInt32::typeid;
            paramBits->Attributes->Add(attr1);

            dict->Add("Bits", paramBits);

            ParameterAttribute^ attr2 = gcnew ParameterAttribute();
            attr2->Mandatory = false;
            attr2->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramWxH = gcnew RuntimeDefinedParameter();
            paramWxH->Name = "WxH";
            paramWxH->ParameterType = array<hsize_t,1>::typeid;
            paramWxH->Attributes->Add(attr2);

            dict->Add("WxH", paramWxH);

            ParameterAttribute^ attr3 = gcnew ParameterAttribute();
            attr3->Mandatory = false;
            attr3->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramInterlaceMode = gcnew RuntimeDefinedParameter();
            paramInterlaceMode->Name = "InterlaceMode";
            paramInterlaceMode->ParameterType = String::typeid;
            paramInterlaceMode->Attributes->Add(attr3);

            dict->Add("InterlaceMode", paramInterlaceMode);

#pragma endregion
        }
        else if (itemTypeName->ToUpper() == "PALETTE")
        {
            // Dimensions

            ParameterAttribute^ attr1 = gcnew ParameterAttribute();
            attr1->Mandatory = true;
            attr1->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramDimensions = gcnew RuntimeDefinedParameter();
            paramDimensions->Name = "Dimensions";
            paramDimensions->ParameterType = array<hsize_t>::typeid;
            paramDimensions->Attributes->Add(attr1);

            dict->Add("Dimensions", paramDimensions);
        }
        else if (itemTypeName->ToUpper() == "PACKETTABLE")
        {
#pragma region Packet Table

            // Packet type
            ParameterAttribute^ attr1 = gcnew ParameterAttribute();
            attr1->Mandatory = true;
            attr1->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramPacketType = gcnew RuntimeDefinedParameter();
            paramPacketType->Name = "PacketType";
            paramPacketType->ParameterType = Object::typeid;
            paramPacketType->Attributes->Add(attr1);

            dict->Add("PacketType", paramPacketType);

            ParameterAttribute^ attr4 = gcnew ParameterAttribute();
            attr4->Mandatory = true;
            attr4->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramChunkSize = gcnew RuntimeDefinedParameter();
            paramChunkSize->Name = "ChunkByteSize";
            paramChunkSize->ParameterType = hsize_t::typeid;
            paramChunkSize->Attributes->Add(attr4);

            dict->Add("ChunkByteSize", paramChunkSize);

            ParameterAttribute^ attr6 = gcnew ParameterAttribute();
            attr6->Mandatory = false;
            attr6->ValueFromPipeline = false;

            RuntimeDefinedParameter^ paramGzip = gcnew RuntimeDefinedParameter();
            paramGzip->Name = "Gzip";
            paramGzip->ParameterType = Int32::typeid;
            paramGzip->Attributes->Add(attr6);

            dict->Add("Gzip", paramGzip);

#pragma endregion
        }
        else if (itemTypeName->ToUpper() == "TABLE")
        {
        }

        return dict;
    }

}