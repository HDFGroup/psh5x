
#include "DatatypeInfo.h"
#include "GroupInfo.h"
#include "HDF5Exception.h"
#include "LinkInfo.h"
#include "Provider.h"
#include "PSH5XException.h"

extern "C" {
#include "H5Lpublic.h"
#include "H5Opublic.h"
}

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Collections::ObjectModel;
using namespace System::Management::Automation;
using namespace System::Management::Automation::Provider;
using namespace System::Runtime::InteropServices;

namespace PSH5X
{
	ItemInfo^ Provider::passThru(hid_t loc_id, String^ h5path, bool& isContainer)
	{
		ItemInfo^ result = nullptr;
		isContainer = false;

		if (!ProviderUtils::IsValidH5Path(loc_id, h5path)) {
			throw gcnew PSH5XException("Invalid path found!");
		}

		char* h5path_str = NULL;
		H5L_info_t linfo;
		hid_t obj_id = -1;
		H5O_info_t oinfo;

		try
		{
			h5path_str =  (char*)(Marshal::StringToHGlobalAnsi(h5path)).ToPointer();

			if (H5Lget_info(loc_id, h5path_str, &linfo, H5P_DEFAULT) < 0) {
				throw gcnew HDF5Exception("H5Lget_info failed!");
			}

			switch (linfo.type)
			{
			case H5L_TYPE_HARD:
				{
#pragma region HDF5 hard link

					if ((obj_id = H5Oopen(loc_id, h5path_str, H5P_DEFAULT)) < 0) {
						throw gcnew HDF5Exception("H5Oopen failed!");
					}

					if (H5Oget_info(obj_id, &oinfo) >= 0)
					{
						switch (oinfo.type)
						{
						case H5O_TYPE_GROUP:
							{
								result = gcnew GroupInfo(obj_id);
								isContainer = true;
							}
							break;

						case H5O_TYPE_DATASET:
							{
								result = ProviderUtils::GetDatasetInfo(obj_id);
							}
							break;

						case H5O_TYPE_NAMED_DATATYPE:
							{
								result = gcnew DatatypeInfo(obj_id);
							}
							break;

						default:
							break;
						}
					}
					else {
						throw gcnew HDF5Exception("H5Oget_info failed!");
					}
#pragma endregion
				}
				break;

			case H5L_TYPE_SOFT:
			case H5L_TYPE_EXTERNAL:
				{
					result = gcnew LinkInfo(loc_id, h5path);
				}
				break;

			default:

				throw gcnew PSH5XException("Unable to determine the item type for this path!!!");
				break;
			}
		}
		finally
		{
			if (obj_id >= 0) {
				H5Oclose(obj_id);
			}
			if (h5path_str != NULL) {
				 Marshal::FreeHGlobal(IntPtr(h5path_str));
			}
		}

		return result;
	}
}
