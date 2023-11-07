
#include "Filter.h"

const wchar_t FilterName[] = L"FHacklan";
const wchar_t FriendlyName[] = L"Hacklan Network Monitor";
const wchar_t FilterGUID[] = L"{72EFAC82-649B-4B42-8DEC-F9B015EFC62F}";

int RegisterFilterDriver(PDRIVER_OBJECT Driver)
{
	NDIS_STATUS Status;

	FilterRegistered = FALSE;
	FilterHandle = NULL;
	//RtlZeroMemory(&miniports, sizeof(miniports_));
	RtlZeroMemory(&FilterDriverChar, sizeof(NDIS_FILTER_DRIVER_CHARACTERISTICS));
	FilterDriverChar.MajorNdisVersion = NDIS_FILTER_MINIMUM_MAJOR_VERSION;
	FilterDriverChar.MinorNdisVersion = 0;
	FilterDriverChar.MajorDriverVersion = 1;
	FilterDriverChar.MinorDriverVersion = 0;
	FilterDriverChar.Header.Type = NDIS_OBJECT_TYPE_FILTER_DRIVER_CHARACTERISTICS;
	FilterDriverChar.Header.Revision = NDIS_FILTER_CHARACTERISTICS_REVISION_1;
	FilterDriverChar.Header.Size = NDIS_SIZEOF_FILTER_DRIVER_CHARACTERISTICS_REVISION_1;
	FilterDriverChar.Flags = 0;
	RtlInitUnicodeString(&FilterDriverChar.FriendlyName, FriendlyName);
	RtlInitUnicodeString(&FilterDriverChar.ServiceName, FilterName);
	RtlInitUnicodeString(&FilterDriverChar.UniqueName, FilterGUID);

	FilterDriverChar.SetOptionsHandler = &FilterSetOptions;
	FilterDriverChar.SetFilterModuleOptionsHandler = &SetFilterModuleOptions;
	FilterDriverChar.AttachHandler = &FilterAttach;
	FilterDriverChar.DetachHandler = &FilterDetach;
	FilterDriverChar.RestartHandler = &FilterRestart;
	FilterDriverChar.PauseHandler = &FilterPause;
	FilterDriverChar.SendNetBufferListsHandler = &FilterSendNetBufferLists;
	FilterDriverChar.SendNetBufferListsCompleteHandler = &FilterSendNetBufferListsComplete;
	FilterDriverChar.CancelSendNetBufferListsHandler = &FilterCancelSendNetBufferLists;
	FilterDriverChar.ReceiveNetBufferListsHandler = &FilterReceiveNetBufferLists;
	FilterDriverChar.ReturnNetBufferListsHandler = &FilterReturnNetBufferLists;
	FilterDriverChar.OidRequestHandler = &FilterOidRequest;
	FilterDriverChar.OidRequestCompleteHandler = &FilterOidRequestComplete;
	FilterDriverChar.CancelOidRequestHandler = &FilterCancelOidRequest;
	FilterDriverChar.DevicePnPEventNotifyHandler = &FilterDevicePnpEventNotify;
	FilterDriverChar.NetPnPEventHandler =&FilterNetPnpEvent;
	FilterDriverChar.StatusHandler = &FilterStatus;
	FilterDriverChar.DirectOidRequestHandler = NULL; // &FilterDirectOidRequest;
	FilterDriverChar.DirectOidRequestCompleteHandler = NULL; // &FilterDirectOidRequestComplete;
	FilterDriverChar.CancelDirectOidRequestHandler = NULL; // &FilterCancelDirectOidRequest;

	Status = NdisFRegisterFilterDriver(Driver, (NDIS_HANDLE)Driver, &FilterDriverChar, &FilterHandle);
	if(Status == NDIS_STATUS_SUCCESS)
	{
		FilterRegistered = TRUE;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Filter Driver Register  success......");
		return 0;
	} else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Filter Driver Register  failed status %x......",Status);

	return 100;
}

NDIS_STATUS FilterSetOptions(IN NDIS_HANDLE NdisDriverHandle, IN NDIS_HANDLE DriverContext)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter Set Options execute......");
	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS SetFilterModuleOptions(IN  NDIS_HANDLE FilterModuleContext)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter SetFilterModuleOptions execute......");
	return NDIS_STATUS_SUCCESS;
}
NDIS_STATUS FilterAttach(IN NDIS_HANDLE NdisFilterHandle, IN NDIS_HANDLE FilterDriverContext, IN PNDIS_FILTER_ATTACH_PARAMETERS AttachParameters)
{
	int i;
	miniport* _Miniport;
	PLIST_ENTRY PLE;
	filterModuleHandle * fmodule;
	int finded;
	NDIS_STATUS status;
	ANSI_STRING AS;
	
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Miniport Name : %ws", AttachParameters->BaseMiniportInstanceName->Buffer);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter Physical Medium %d......", AttachParameters->MiniportPhysicalMediaType);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter Medium : %d", AttachParameters->MiniportMediaType);
	
	finded = 0;

	if (miniportsCount > 0)
	{
		PLE=(PLIST_ENTRY)MiniportsEntries.Flink;
		//_Miniport = (miniport*)CONTAINING_RECORD(PLE, miniport, ListE);

		//while (_Miniport != &MiniportsEntries)
		while(PLE!=&MiniportsEntries)
		{
			_Miniport = (miniport*)CONTAINING_RECORD(PLE, miniport, ListE);

			RtlUnicodeStringToAnsiString(&AS, AttachParameters->BaseMiniportInstanceName, TRUE);

			if (strcmp(_Miniport->MiniportName, AS.Buffer) == 0)
			{
				RtlFreeAnsiString(&AS);

				fmodule = (filterModuleHandle*)ExAllocatePool(NonPagedPool, sizeof(filterModuleHandle));

				memset(fmodule, 0, sizeof(filterModuleHandle));

				fmodule->MiniportHandle = NdisFilterHandle;
				memcpy(&fmodule->FAP, AttachParameters, sizeof(NDIS_FILTER_ATTACH_PARAMETERS));
				fmodule->MP = (PVOID)_Miniport;
				fmodule->State = FilterAttaching;
				fmodule->RecvPacket = 0;
				fmodule->SendPacket = 0;
				fmodule->PowerState = NdisDeviceStateUnspecified;
				RtlZeroMemory(&fmodule->FilterAttrib, sizeof(NDIS_FILTER_ATTRIBUTES));
				fmodule->FilterAttrib.Header.Type = NDIS_OBJECT_TYPE_FILTER_ATTRIBUTES;
				fmodule->FilterAttrib.Header.Revision = NDIS_FILTER_ATTRIBUTES_REVISION_1;
				fmodule->FilterAttrib.Header.Size = NDIS_SIZEOF_FILTER_ATTRIBUTES_REVISION_1;
				fmodule->OP_Mode = NULL;

				if (_Miniport->HandleCount == 0)
				{
					_Miniport->MainModule = NdisFilterHandle;
					memcpy(&_Miniport->Main_FAP, AttachParameters, sizeof(NDIS_FILTER_ATTACH_PARAMETERS));

					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Flter Main Module ............... ATTACHED...............");
				}
				
				_Miniport->HandleCount++;

				if (AttachParameters->MiniportPhysicalMediaType == NdisPhysicalMediumNative802_11 && AttachParameters->MiniportMediaType == NdisMediumNative802_11)
				{

					if (_Miniport->Main_FAP.MiniportPhysicalMediaType == NdisPhysicalMediumNative802_11 && _Miniport->Main_FAP.MiniportMediaType == NdisMedium802_3)
					{
						AcqSpinLock(&_Miniport->SpinLock_, FALSE);

						_Miniport->MainModule = NdisFilterHandle;
						memcpy(&_Miniport->Main_FAP, AttachParameters, sizeof(NDIS_FILTER_ATTACH_PARAMETERS));

						_Miniport->SendCount = 0;
						_Miniport->RecvCount = 0;
						_Miniport->SendLicz = 0;
						_Miniport->RecvLicz = 0;

						RelSpinLock(&_Miniport->SpinLock_, FALSE);

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Flter Main Module ............... REPLACED...............");
					}

				}

				InsertTailList(&_Miniport->ModeuleEntries, &fmodule->ListE);
				finded = 1;

				status = NdisFSetAttributes(NdisFilterHandle, (NDIS_HANDLE)fmodule, &fmodule->FilterAttrib);

				AcqSpinLock(&_Miniport->SpinLock_, FALSE);

				if (status == STATUS_SUCCESS)
					fmodule->State = FilterPaused;
				else fmodule->State = FilterDetached;
				
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Flter Module : %x -  Set Attributes...", fmodule->MiniportHandle);
				

				if (status == NDIS_STATUS_SUCCESS)
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Status Success");
				else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Status failed");
				

				RelSpinLock(&_Miniport->SpinLock_, FALSE);

				break;
			}
			RtlFreeAnsiString(&AS);
			PLE = _Miniport->ListE.Flink;
			//= (miniport*)_Miniport->ListE.Flink;
		}
	}
	if (finded == 0)
	{
		_Miniport = (miniport*)ExAllocatePool(NonPagedPool, sizeof(miniport));
		memset(_Miniport, 0, sizeof(miniport));
		//wcscpy(_Miniport->MiniportName, AttachParameters->BaseMiniportInstanceName->Buffer);
		RtlUnicodeStringToAnsiString(&AS, AttachParameters->BaseMiniportInstanceName, TRUE);
		if (AS.Length < 250)
			memcpy(_Miniport->MiniportName, AS.Buffer, AS.Length);
		RtlFreeAnsiString(&AS);
		_Miniport->SPacket = (EHeader*)ExAllocatePool(NonPagedPool, sizeof(EHeader) * PACKET_COUNT);
		_Miniport->RPacket = (EHeader*)ExAllocatePool(NonPagedPool, sizeof(EHeader) * PACKET_COUNT);
		_Miniport->SendCount = 0;
		_Miniport->RecvCount = 0;
		_Miniport->SendLicz = 0;
		_Miniport->RecvLicz = 0;
	//	_Miniport->ModeSet = FALSE;

		NdisAllocateSpinLock(&_Miniport->SpinLock_);

		fmodule = (filterModuleHandle*)ExAllocatePool(NonPagedPool, sizeof(filterModuleHandle));

		memset(fmodule, 0, sizeof(filterModuleHandle));

		fmodule->MiniportHandle = NdisFilterHandle;
		memcpy(&fmodule->FAP, AttachParameters, sizeof(NDIS_FILTER_ATTACH_PARAMETERS));
		fmodule->MP = (PVOID)_Miniport;
		fmodule->State = FilterAttaching;
		fmodule->RecvPacket = 0;
		fmodule->SendPacket = 0;
		fmodule->PowerState = NdisDeviceStateUnspecified;
		RtlZeroMemory(&fmodule->FilterAttrib, sizeof(NDIS_FILTER_ATTRIBUTES));
		fmodule->FilterAttrib.Header.Type = NDIS_OBJECT_TYPE_FILTER_ATTRIBUTES;
		fmodule->FilterAttrib.Header.Revision = NDIS_FILTER_ATTRIBUTES_REVISION_1;
		fmodule->FilterAttrib.Header.Size = NDIS_SIZEOF_FILTER_ATTRIBUTES_REVISION_1;
		fmodule->OP_Mode = NULL;

		InitializeListHead(&_Miniport->ModeuleEntries);

		_Miniport->HandleCount = 1;

		miniportsCount++;

		_Miniport->Index = miniportsCount;

		_Miniport->MainModule = NdisFilterHandle;
		memcpy(&_Miniport->Main_FAP, AttachParameters, sizeof(NDIS_FILTER_ATTACH_PARAMETERS));

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Flter Main Module ............... ATTACHED...............");

		InsertTailList(&_Miniport->ModeuleEntries, &fmodule->ListE);
		InsertTailList(&MiniportsEntries, &_Miniport->ListE);

		status = NdisFSetAttributes(NdisFilterHandle, (NDIS_HANDLE)fmodule, &fmodule->FilterAttrib);

		AcqSpinLock(&_Miniport->SpinLock_, FALSE);

		if (status == STATUS_SUCCESS)
			fmodule->State = FilterPaused;
		else fmodule->State = FilterDetached;


		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Flter Module : %x -  Set Attributes...", fmodule->MiniportHandle);


		if (status == NDIS_STATUS_SUCCESS)
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Status Success");
		else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Status failed");

		RelSpinLock(&_Miniport->SpinLock_, FALSE);
	}
	return NDIS_STATUS_SUCCESS;
}


void  FilterDetach(IN NDIS_HANDLE FilterModuleContext)
{
	filterModuleHandle* fmh;
	miniport* mp;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterDetach execute......");


	fmh = (filterModuleHandle*)FilterModuleContext;
	mp = (miniport*)fmh->MP;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter Module Name : %s",mp->MiniportName);

	RemoveEntryList(&fmh->ListE);
	
	if (fmh->OP_Mode != NULL)
		ExFreePool(fmh->OP_Mode);

	ExFreePool(fmh);

	mp->HandleCount--;

	if (mp->HandleCount == 0 && isUnload==TRUE)
	{
		RemoveEntryList(&mp->ListE);
		NdisFreeSpinLock(&mp->SpinLock_);

		ExFreePool(mp->SPacket);
		ExFreePool(mp->RPacket);
		ExFreePool(mp);

		if (miniportsCount > 0)
			miniportsCount--;

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Miniport relased ");
	}
}

NDIS_STATUS FilterRestart(IN NDIS_HANDLE FilterModuleContext, IN PNDIS_FILTER_RESTART_PARAMETERS RestartParameters)
{
	filterModuleHandle* fmh;
	miniport* MP;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterRestart execute......");

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	NdisAcquireSpinLock(&MP->SpinLock_);

	fmh->State = FilterRunning;
	fmh->PowerState = NdisDeviceStateD0;
	NdisReleaseSpinLock(&MP->SpinLock_);

	NdisFRestartComplete(fmh->MiniportHandle, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS FilterPause(IN NDIS_HANDLE FilterModuleContext, IN PNDIS_FILTER_PAUSE_PARAMETERS PauseParameters)
{
	filterModuleHandle *fmh;
	miniport* MP;

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	NdisAcquireSpinLock(&MP->SpinLock_);
	fmh->State = FilterPaused;
	NdisReleaseSpinLock(&MP->SpinLock_);

	NdisFPauseComplete(fmh->MiniportHandle);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterPause execute...... %s",((miniport*)fmh->MP)->MiniportName);


	return NDIS_STATUS_SUCCESS;
}

void FilterSendNetBufferLists(IN NDIS_HANDLE FilterModuleContext, IN PNET_BUFFER_LIST NetBufferList, IN NDIS_PORT_NUMBER PortNumber, IN ULONG SendFlags)
{
	filterModuleHandle* fmh;
	BOOLEAN isDispatch;
	PNET_BUFFER_LIST NBL;
	PNET_BUFFER  NB;
	PMDL mdl_ch;
	PVOID Buffer;
	int ByteCount;
	miniport* MP;
	ULONG SPacket;
	ULONG SFlags;
	int DataOffset;
	ULONG Flags_;
	ULONG NBLFlags;

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;
	NBL = NetBufferList;

	
	Flags_ = NET_BUFFER_LIST_FLAGS(NetBufferList);

//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "NetBufferList sending Flags- %x  NBLFlags- %x  Port Number- %d......... ",Flags_, NetBufferList->NblFlags,PortNumber);

/*
	if (Flags_ & NBL_FLAGS_PROTOCOL_RESERVED)
	{
		if (NetBufferList->ProtocolReserved[0] == NetBufferList)
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Internal NetBufferList is sending.........");
	}
*/
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterSendNetBufferLists NBl-%x  PoolHandle - %x", NetBufferList, NdisGetPoolFromNetBufferList(NetBufferList));

	isDispatch = NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendFlags);

	SPacket = 0;


	AcqSpinLock(&MP->SpinLock_, isDispatch);

	if (fmh->State != FilterRunning || fmh->PowerState!= NdisDeviceStateD0)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterSendNetBufferLists Paused-----");
		RelSpinLock(&MP->SpinLock_, isDispatch);
		while (NBL != NULL)
		{
			//NBL->Status = NDIS_STATUS_PAUSED;
			NET_BUFFER_LIST_STATUS(NBL) = NDIS_STATUS_PAUSED;
			NBL = NET_BUFFER_LIST_NEXT_NBL(NBL);
		}
		if (isDispatch)
			SFlags = NDIS_SEND_COMPLETE_FLAGS_DISPATCH_LEVEL; else SFlags = 0;
		NdisFSendNetBufferListsComplete(fmh->MiniportHandle, NetBufferList, SFlags);
	}
	else
	{
		RelSpinLock(&MP->SpinLock_, isDispatch);
	
		while (NBL != NULL)
		{
			SPacket++;

			if (MP->MainModule == fmh->MiniportHandle)
			{
				AcqSpinLock(&MP->SpinLock_, isDispatch);

				RtlZeroMemory(&MP->SPacket[MP->SendCount], sizeof(EHeader));

				NB = NET_BUFFER_LIST_FIRST_NB(NBL);

				while (NB != NULL)
				{

					mdl_ch = NET_BUFFER_FIRST_MDL(NB);

					while (mdl_ch != NULL)
					{
						Buffer = NULL;

						ByteCount = 0;

						NdisQueryMdl(mdl_ch, &Buffer, &ByteCount, NormalPagePriority)

					//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "RAMKA-%d---Handle : %d---- ByteCount %d", MP->SendCount,fmh->MiniportHandle, ByteCount);

						if (Buffer != NULL && ByteCount > 0 && (MP->SPacket[MP->SendCount].DataSize + ByteCount) <= 5000)
						{

							if (MP->SendCount > 0 || MP->SendCount < PACKET_COUNT)
							{
								DataOffset=MP->SPacket[MP->SendCount].DataSize;

								RtlCopyMemory(&MP->SPacket[MP->SendCount].NetworkData[DataOffset], Buffer, ByteCount);

								//MP->SPacket[MP->SendCount].DataSize = MP->SPacket[MP->SendCount].DataSize + ByteCount;

								MP->SPacket[MP->SendCount].DataSize += ByteCount;

							}

						}
						mdl_ch = mdl_ch->Next;
					}
					NB = NET_BUFFER_NEXT_NB(NB);
				}

				if (MP->SPacket[MP->SendCount].DataSize > 0)
				{

					MP->SPacket[MP->SendCount].MediumType = fmh->FAP.MiniportPhysicalMediaType;

					MP->SPacket[MP->SendCount].Medium = fmh->FAP.MiniportMediaType;

					MP->SPacket[MP->SendCount].MacAddressLength = MP->Main_FAP.MacAddressLength;

					memcpy(MP->SPacket[MP->SendCount].NetworkMiniportName, MP->MiniportName, 250);

					memcpy(MP->SPacket[MP->SendCount].CurrentMacAddress, MP->Main_FAP.CurrentMacAddress, MP->Main_FAP.MacAddressLength);

					if (MP->SendCount == PACKET_COUNT - 1) MP->SendCount = 0;
					else  MP->SendCount++;

					if (MP->SendLicz == PACKET_COUNT) MP->SendLicz = 1;
					else MP->SendLicz++;

				}

				RelSpinLock(&MP->SpinLock_, isDispatch);

			}
			
			NBL = NET_BUFFER_LIST_NEXT_NBL(NBL);
			
		}

		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterSendNetBufferLists SPacket = %d",fmh->SendPacket);

		AcqSpinLock(&MP->SpinLock_, isDispatch);

		fmh->SendPacket = fmh->SendPacket + SPacket;

		RelSpinLock(&MP->SpinLock_, isDispatch);

		NdisFSendNetBufferLists(fmh->MiniportHandle, NetBufferList, PortNumber, SendFlags);
	}
	


}

void FilterSendNetBufferListsComplete(IN NDIS_HANDLE FilterModuleContext, IN PNET_BUFFER_LIST NetBufferList, IN ULONG SendCompleteFlags)
{
	filterModuleHandle* fmh;
	PNET_BUFFER_LIST NBL;
	BOOLEAN isDispatch;
	ULONG SPacket;
	miniport* MP;
//	ULONG Flags_;

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	/*
	
	Flags_ = NET_BUFFER_LIST_FLAGS(NetBufferList);

	if (Flags_ & NBL_FLAGS_PROTOCOL_RESERVED)
	{
		if (NetBufferList->ProtocolReserved[0] == NetBufferList)
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Protocol Driver NetBufferList is sending.........");

		if (NetBufferList->ProtocolReserved[1] == NetBufferList)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Internal NetBufferList is sending.........");

			if (NetBufferList->Status == NDIS_STATUS_SUCCESS)
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Status Success");

			else if (NetBufferList->Status == NDIS_STATUS_INVALID_LENGTH)
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Status NDIS_STATUS_INVALID_LENGTH");

			else if (NetBufferList->Status == NDIS_STATUS_RESOURCES)
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Status NDIS_STATUS_RESOURCES");

			else if (NetBufferList->Status == NDIS_STATUS_FAILURE)
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Status NDIS_STATUS_FAILURE");

			else
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Status OTHER");

			NdisSetEvent(&SendCompleteEvent);

			return;
		}
	}
	*/

	NBL = NetBufferList;

	isDispatch = NDIS_TEST_SEND_AT_DISPATCH_LEVEL(SendCompleteFlags);
	//MiniP = (miniport*)fmh->MP;
	
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "FilterSendNetBufferListsComplete MiniportName= %ws , isDispatch: %d , Flags %d",MiniP->MiniportName,isDispatch, SendCompleteFlags);

	SPacket = 0;

	while (NBL != NULL)
	{
		SPacket++;
		NBL = NET_BUFFER_LIST_NEXT_NBL(NBL);
	
	}
	AcqSpinLock(&MP->SpinLock_, isDispatch);
	fmh->SendPacket = fmh->SendPacket - SPacket;
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "FilterSendNetBufferListsComplete MiniportName= %ws",((miniport*)fmh->MP)->MiniportName);
	RelSpinLock(&MP->SpinLock_, isDispatch);

	NdisFSendNetBufferListsComplete(fmh->MiniportHandle, NetBufferList, SendCompleteFlags);
}

void FilterCancelSendNetBufferLists(IN NDIS_HANDLE FilterModuleContext, IN PVOID CancelId)
{
	filterModuleHandle* fmh;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterCancelSendNetBufferLists execute......");

	/*
	if (CancelId == CancelID)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Internal Cancel.....");
		return;
	}
	*/

	fmh = (filterModuleHandle*)FilterModuleContext;
	NdisFCancelSendNetBufferLists(fmh->MiniportHandle, CancelId);
}

void FilterReceiveNetBufferLists(IN NDIS_HANDLE FilterModuleContext, IN PNET_BUFFER_LIST NetBufferLists, IN NDIS_PORT_NUMBER PortNumber, IN ULONG NumberOfNetBufferLists, IN ULONG ReceiveFlags)
{
	filterModuleHandle* fmh;
	ULONG RetFlags;
	BOOLEAN isDispatch;
	PNET_BUFFER  NB;
	PMDL mdl_ch;
	PVOID Buffer;
	int ByteCount;
	PNET_BUFFER_LIST NBL;
	miniport* MP;
	int DataOffset;

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterReceiveNetBufferLists execute......");

	isDispatch= NDIS_TEST_SEND_AT_DISPATCH_LEVEL(ReceiveFlags);
	NBL = NetBufferLists;

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	AcqSpinLock(&MP->SpinLock_, isDispatch);

	if (fmh->State != FilterRunning || fmh->PowerState != NdisDeviceStateD0)
	{
		RelSpinLock(&MP->SpinLock_, isDispatch);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter Recive is  Not Running State.....");

		if (NDIS_TEST_RECEIVE_CAN_PEND(ReceiveFlags))
		{
			RetFlags = 0;
			if (NDIS_TEST_SEND_AT_DISPATCH_LEVEL(ReceiveFlags))
			{
				NDIS_SET_RETURN_FLAG(RetFlags, NDIS_RECEIVE_FLAGS_DISPATCH_LEVEL);
			}

			NdisFReturnNetBufferLists(fmh->MiniportHandle, NetBufferLists, ReceiveFlags);
		}
	}
	else
	{
		RelSpinLock(&MP->SpinLock_, isDispatch);
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter  is Running State.....");

		if (NumberOfNetBufferLists >= 1)
		{
			AcqSpinLock(&MP->SpinLock_, isDispatch)

			fmh->RecvPacket = fmh->RecvPacket + NumberOfNetBufferLists;

			RelSpinLock(&MP->SpinLock_, isDispatch);
			

			if (MP->MainModule == fmh->MiniportHandle)
			{
				AcqSpinLock(&MP->SpinLock_, isDispatch);

				MP->RPacket[MP->RecvCount].DataSize = 0;

				while (NBL != NULL)
				{
					NB = NET_BUFFER_LIST_FIRST_NB(NBL);

					RtlZeroMemory(&MP->RPacket[MP->RecvCount], sizeof(EHeader));

					while (NB != NULL)
					{
						mdl_ch = NET_BUFFER_FIRST_MDL(NB);

						while (mdl_ch != NULL)
						{

							Buffer = NULL;

							ByteCount = 0;

							NdisQueryMdl(mdl_ch, &Buffer, &ByteCount, NormalPagePriority)
								
							if (Buffer != NULL && ByteCount > 0 && (MP->RPacket[MP->RecvCount].DataSize + ByteCount) <= 5000)
							{

								if (MP->RecvCount > 0 || MP->RecvCount < PACKET_COUNT)
								{
									DataOffset = MP->RPacket[MP->RecvCount].DataSize;
				
									RtlCopyMemory(&MP->RPacket[MP->RecvCount].NetworkData[DataOffset], Buffer, ByteCount);

								//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "MDL Byte Count : %d",ByteCount);

									MP->RPacket[MP->RecvCount].DataSize += ByteCount;

								}
							}
							mdl_ch = mdl_ch->Next;
						}
						NB = NET_BUFFER_NEXT_NB(NB);
					}

					if (MP->RPacket[MP->RecvCount].DataSize > 0)
					{

						MP->RPacket[MP->RecvCount].MediumType = fmh->FAP.MiniportPhysicalMediaType;

						MP->RPacket[MP->RecvCount].Medium = fmh->FAP.MiniportMediaType;

						MP->RPacket[MP->RecvCount].MacAddressLength = MP->Main_FAP.MacAddressLength;

						memcpy(MP->RPacket[MP->RecvCount].NetworkMiniportName, MP->MiniportName, 250);

						memcpy(MP->RPacket[MP->RecvCount].CurrentMacAddress, MP->Main_FAP.CurrentMacAddress, MP->Main_FAP.MacAddressLength);

						if (MP->RecvCount == PACKET_COUNT - 1) MP->RecvCount = 0;
						else MP->RecvCount++;

						if (MP->RecvLicz == PACKET_COUNT) MP->RecvLicz = 1;
						else MP->RecvLicz++;

					}

					NBL = NET_BUFFER_LIST_NEXT_NBL(NBL);
				}

				RelSpinLock(&MP->SpinLock_, isDispatch);
			}
			
			NdisFIndicateReceiveNetBufferLists(fmh->MiniportHandle, NetBufferLists, PortNumber, NumberOfNetBufferLists, ReceiveFlags);
			
			if (NDIS_TEST_RECEIVE_CANNOT_PEND(ReceiveFlags))
			{
				AcqSpinLock(&MP->SpinLock_, isDispatch)

				fmh->RecvPacket = fmh->RecvPacket - NumberOfNetBufferLists;

				RelSpinLock(&MP->SpinLock_, isDispatch);

				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "NDIS_RECEIVE_FLAGS_RESOURCES is set");
			} 

		}

	}


}

void FilterReturnNetBufferLists(IN NDIS_HANDLE FilterModuleContext, IN PNET_BUFFER_LIST NetBufferLists, IN ULONG ReturnFlags)
{
	filterModuleHandle* fmh;
	BOOLEAN isDispatch;
	ULONG NumberBufferList;
	PNET_BUFFER_LIST NBL;
	miniport* MP;

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterReturnNetBufferLists execute......");

	isDispatch = NDIS_TEST_SEND_AT_DISPATCH_LEVEL(ReturnFlags);

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	NBL = NetBufferLists;

	NumberBufferList = 0;

	while (NBL)
	{
		NumberBufferList++;

		NBL = NET_BUFFER_LIST_NEXT_NBL(NBL);
	}

	NdisFReturnNetBufferLists(fmh->MiniportHandle, NetBufferLists, ReturnFlags);

	AcqSpinLock(&MP->SpinLock_, isDispatch);

	fmh->RecvPacket = fmh->RecvPacket - NumberBufferList;

	RelSpinLock(&MP->SpinLock_, isDispatch);


}

NDIS_STATUS FilterOidRequest(IN NDIS_HANDLE FilterModuleContext, IN PNDIS_OID_REQUEST OidRequest)
{
	filterModuleHandle* fmh;
	PNDIS_OID_REQUEST COidRequest = 0;
	NDIS_OID_REQUEST** Context;
	NDIS_STATUS status;
	miniport* MP;

//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequest execute......");

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	/*
	if (OidRequest->DATA.SET_INFORMATION.Oid == OID_GEN_CURRENT_PACKET_FILTER && OidRequest->RequestType == NdisRequestSetInformation)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequest OID_GEN_CURRENT_PACKET_FILTER......Filter : %x",*((ULONG*)OidRequest->DATA.SET_INFORMATION.InformationBuffer));

	}
	if (OidRequest->DATA.QUERY_INFORMATION.Oid == OID_DOT11_OPERATION_MODE_CAPABILITY && OidRequest->RequestType == NdisRequestQueryInformation)
	{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequest OID_DOT11_OPERATION_MODE_CAPABILITY......");
	}

	if (OidRequest->DATA.QUERY_INFORMATION.Oid == OID_DOT11_CURRENT_OPERATION_MODE && OidRequest->RequestType == NdisRequestQueryInformation)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequest Query OID_DOT11_CURRENT_OPERATION_MODE ......");
	}

	if (OidRequest->DATA.SET_INFORMATION.Oid == OID_DOT11_CURRENT_OPERATION_MODE && OidRequest->RequestType == NdisRequestSetInformation)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequest Set OID_DOT11_CURRENT_OPERATION_MODE ......");
	}

	*/
	status=NdisAllocateCloneOidRequest(fmh->MiniportHandle, OidRequest, 'FHck', &COidRequest);
	
	if (status == NDIS_STATUS_SUCCESS)
	{
		Context = (NDIS_OID_REQUEST**)(&COidRequest->SourceReserved[0]);
		*Context = OidRequest;
		COidRequest->RequestId = OidRequest->RequestId;

		AcqSpinLock(&MP->SpinLock_, FALSE);
		
		fmh->PRequest = COidRequest;

		RelSpinLock(&MP->SpinLock_, FALSE);

		status=NdisFOidRequest(fmh->MiniportHandle, COidRequest);

		if(status!=STATUS_PENDING)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "NdisFOidRequest  !!!!STATUS_PENDING ");
			FilterOidRequestComplete(fmh, COidRequest, status);
			status = NDIS_STATUS_PENDING;
			
		}//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Statu PENDING");
	}
	return status;
}

void FilterOidRequestComplete(IN NDIS_HANDLE FilterModuleContext, IN PNDIS_OID_REQUEST OidRequest, IN NDIS_STATUS Status)
{
	filterModuleHandle* fmh;
	NDIS_OID_REQUEST *OrigReq;
	NDIS_OID_REQUEST ** Context;
	int i;
	PVOID Buffer=NULL;
	miniport* MP;
	BOOLEAN isInternal = FALSE;

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequestComplete execute......");


	if (OidRequest->DATA.QUERY_INFORMATION.Oid == OID_DOT11_OPERATION_MODE_CAPABILITY)
	{
		if(Status==NDIS_STATUS_SUCCESS)
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequestComplete SUCCESS......");
		else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequestComplete FAILED......");
		//ReqQueryStatus = Status;
	}

	if (OidRequest->DATA.SET_INFORMATION.Oid == OID_DOT11_CURRENT_OPERATION_MODE)
	{
		if (Status == NDIS_STATUS_SUCCESS)
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequestComplete SUCCESS......");
		else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterOidRequestComplete FAILED......");
		//ReqSetStatus = Status;
	}

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	Context = (NDIS_OID_REQUEST**)(&OidRequest->SourceReserved[0]);
	OrigReq = *Context;

	if (OrigReq == NULL)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "OrigReq===============NULL");


		AcqSpinLock(&MP->SpinLock_, FALSE);

		fmh->RequestCompleteState = 1;

		RelSpinLock(&MP->SpinLock_, FALSE);

		switch (OidRequest->RequestType)
		{

			case NdisRequestQueryInformation:
			{
				ReqQueryStatus = Status;

				NdisSetEvent(&RequestQueryEvent);

				break;
			}
			case NdisRequestSetInformation:
			{
				ReqSetStatus = Status;

				NdisSetEvent(&RequestSetEvent);

				break;
			}
			case NdisRequestMethod:
			{
				ReqMethodStatus = Status;

				NdisSetEvent(&RequestMethodEvent);

				break;
			}
		}

		return;
	}

	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->PRequest == OidRequest)
	{
			fmh->PRequest = NULL;

		RelSpinLock(&MP->SpinLock_, FALSE);
		
		if (OidRequest->RequestType == NdisRequestMethod)
		{
		
			OrigReq->DATA.METHOD_INFORMATION.OutputBufferLength = OidRequest->DATA.METHOD_INFORMATION.OutputBufferLength;
			OrigReq->DATA.METHOD_INFORMATION.BytesRead = OidRequest->DATA.METHOD_INFORMATION.BytesRead;
			OrigReq->DATA.METHOD_INFORMATION.BytesWritten = OidRequest->DATA.METHOD_INFORMATION.BytesWritten;
			OrigReq->DATA.METHOD_INFORMATION.BytesNeeded = OidRequest->DATA.METHOD_INFORMATION.BytesNeeded;
		}
		else if (OidRequest->RequestType == NdisRequestSetInformation)
		{
			OrigReq->DATA.SET_INFORMATION.BytesRead = OidRequest->DATA.SET_INFORMATION.BytesRead;
			OrigReq->DATA.SET_INFORMATION.BytesNeeded = OidRequest->DATA.SET_INFORMATION.BytesNeeded;
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "NdisRequestSetInformation.");
		}
			
		else if (OidRequest->RequestType == NdisRequestQueryInformation || OidRequest->RequestType == NdisRequestQueryStatistics)
		{
			OrigReq->DATA.QUERY_INFORMATION.BytesWritten = OidRequest->DATA.QUERY_INFORMATION.BytesWritten;
			OrigReq->DATA.QUERY_INFORMATION.BytesNeeded = OidRequest->DATA.QUERY_INFORMATION.BytesNeeded;
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "NdisRequestQueryInformation.");
		}

		(*Context) = NULL;

		NdisFreeCloneOidRequest(fmh->MiniportHandle, OidRequest);

		//if(!isInternal)
			NdisFOidRequestComplete(fmh->MiniportHandle, OrigReq, Status);

	}else RelSpinLock(&MP->SpinLock_, FALSE);

	//NdisSetEvent(&RequestEvent);
}

void FilterCancelOidRequest(IN NDIS_HANDLE FilterModuleContext, IN PVOID RequestId)
{
	filterModuleHandle* fmh;
	NDIS_OID_REQUEST** Context;
	PNDIS_OID_REQUEST OrigReq;
	miniport* MP;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "-------------------Filter FilterCancelOidRequest execute-----------------");
	
	OrigReq = NULL;

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->PRequest != NULL)
	{
		Context = (NDIS_OID_REQUEST**)&(fmh->PRequest->SourceReserved[0]);
		OrigReq = *Context;
	}
	if (OrigReq != NULL && OrigReq->RequestId == RequestId)
	{
		RelSpinLock(&MP->SpinLock_, FALSE);
		NdisFCancelOidRequest(fmh->MiniportHandle, RequestId);
	}
	else RelSpinLock(&MP->SpinLock_, FALSE);
}

void FilterDevicePnpEventNotify(IN NDIS_HANDLE FilterModuleContext, IN PNET_DEVICE_PNP_EVENT NetDevicePnPEvent)
{
	filterModuleHandle* fmh;

	//CreateFile(L"C:\\FilterDevicePnpEventNotify.txt");

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterDevicePnpEventNotify execute......");

	fmh = (filterModuleHandle*)FilterModuleContext;
	NdisFDevicePnPEventNotify(fmh->MiniportHandle, NetDevicePnPEvent);
}

NDIS_STATUS FilterNetPnpEvent(IN NDIS_HANDLE FilterModuleContext, IN PNET_PNP_EVENT_NOTIFICATION NetPnPEventNotification)
{
	NDIS_DEVICE_POWER_STATE DevPowerState;
	filterModuleHandle* fmh;
	miniport* MP;

	fmh = (filterModuleHandle*)FilterModuleContext;
	MP = (miniport*)fmh->MP;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "FilterNetPnpEvent execute......");

	switch (NetPnPEventNotification->NetPnPEvent.NetEvent)
	{
		case NetEventPause:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter Event Pause......%s",((miniport*)fmh->MP)->MiniportName);
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "MP Name: %s",((miniport*)fmh->MP)->MiniportName);
			AcqSpinLock(&MP->SpinLock_, FALSE);
			fmh->State = FilterPaused;
			RelSpinLock(&MP->SpinLock_, FALSE);

			break;
		}
		case NetEventRestart:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "FilterNetPnpEvent Restart......");
			break;
		}
		case NetEventSetPower:
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "FilterNetPnpEvent NetEventSetPower......");
			if (NetPnPEventNotification->NetPnPEvent.Buffer != NULL && NetPnPEventNotification->NetPnPEvent.BufferLength == sizeof(NDIS_DEVICE_POWER_STATE))
			{
				DevPowerState = *((NDIS_DEVICE_POWER_STATE*)NetPnPEventNotification->NetPnPEvent.Buffer);
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter change powes state success......");
				AcqSpinLock(&MP->SpinLock_, FALSE);
				fmh->PowerState = DevPowerState;
				RelSpinLock(&MP->SpinLock_, FALSE);
			}

			break; 
		}

	}

	fmh = (filterModuleHandle*)FilterModuleContext;
	NdisFNetPnPEvent(fmh->MiniportHandle, NetPnPEventNotification);

	return NDIS_STATUS_SUCCESS;
}

void FilterStatus(IN NDIS_HANDLE FilterModuleContext, IN PNDIS_STATUS_INDICATION StatusIndication)
{
	filterModuleHandle* fmh;
	NDIS_LINK_STATE* LinkSt;
	miniport* MP;

	fmh = (filterModuleHandle*)FilterModuleContext;

	MP = (miniport*)fmh->MP;

//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Filter FilterStatus execute code---%x", StatusIndication->StatusCode);

	if (StatusIndication->StatusCode == NDIS_STATUS_LINK_STATE)
	{
		LinkSt = (NDIS_LINK_STATE*)StatusIndication->StatusBuffer;


		AcqSpinLock(&MP->SpinLock_, FALSE);

		fmh->FAP.MediaConnectState = LinkSt->MediaConnectState;
		fmh->FAP.MediaDuplexState = LinkSt->MediaDuplexState;
		fmh->FAP.XmitLinkSpeed = LinkSt->XmitLinkSpeed;
		fmh->FAP.RcvLinkSpeed = LinkSt->RcvLinkSpeed;

		if (MP->ConnectState != LinkSt->MediaConnectState)
			MP->ConnectState = LinkSt->MediaConnectState;

		RelSpinLock(&MP->SpinLock_, FALSE);

	

		if (MP->ModeSet && LinkSt->MediaConnectState != NdisMediaStateConnected)
		{
		//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Mode Set status...........");

			AcqSpinLock(&MP->SpinLock_, FALSE);

			MP->ModeSet = FALSE;

			RelSpinLock(&MP->SpinLock_, FALSE);

			NdisSetEvent(&ModeSetEvent);

		}

	

	//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Filter_Link_State change--- %x",fmh->MiniportHandle);

	}

	NdisFIndicateStatus(fmh->MiniportHandle, StatusIndication);
}

NDIS_STATUS FilterDirectOidRequest(IN NDIS_HANDLE FilterModuleContext, IN PNDIS_OID_REQUEST OidRequest)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterDirectOidRequest execute......");
	return NDIS_STATUS_SUCCESS;
}

void FilterDirectOidRequestComplete(IN NDIS_HANDLE FilterModuleContext, IN PNDIS_OID_REQUEST OidRequest, IN NDIS_STATUS Status)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterDirectOidRequestComplete execute......");
}

void FilterCancelDirectOidRequest(IN NDIS_HANDLE FilterModuleContext, IN PVOID RequestId)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Filter FilterCancelDirectOidRequest execute......");
}



NDIS_STATUS RequestMPModeCap(filterModuleHandle *fmh, DOT11_OPERATION_MODE_CAPABILITY *OP_Mode)
{
	PNDIS_OID_REQUEST PRequest,CRequest;
	DOT11_OPERATION_MODE_CAPABILITY* ModeCapability;
	miniport* MP;
	
	NDIS_STATUS status_;

	MP = (miniport*)fmh->MP;

	PRequest = (PNDIS_OID_REQUEST)ExAllocatePool(NonPagedPool, sizeof(NDIS_OID_REQUEST));
	memset(PRequest, 0, sizeof(NDIS_OID_REQUEST));

	ModeCapability = (DOT11_OPERATION_MODE_CAPABILITY*)ExAllocatePool(NonPagedPool, sizeof(DOT11_OPERATION_MODE_CAPABILITY));

	PRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	PRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	PRequest->Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;
	PRequest->Timeout = 5000; // NDIS_OID_REQUEST_TIMEOUT_INFINITE;

	PRequest->PortNumber = 0;


	memset(ModeCapability, 0, sizeof(DOT11_OPERATION_MODE_CAPABILITY));

	PRequest->RequestType = NdisRequestQueryInformation;
	PRequest->DATA.QUERY_INFORMATION.Oid = OID_DOT11_OPERATION_MODE_CAPABILITY;
	PRequest->DATA.QUERY_INFORMATION.InformationBuffer = ModeCapability;
	PRequest->DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(DOT11_OPERATION_MODE_CAPABILITY);


	AcqSpinLock(&MP->SpinLock_, FALSE);

	ReqQueryStatus = NDIS_STATUS_FAILURE;

	fmh->RequestCompleteState = 0;

	RelSpinLock(&MP->SpinLock_, FALSE);

	status_ = NdisFOidRequest(fmh->MiniportHandle, PRequest);


		if (status_ != NDIS_STATUS_PENDING)
		{
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Monitor Mode status success....");
			FilterOidRequestComplete(fmh->MiniportHandle, PRequest, status_);

			NdisWaitEvent(&RequestSetEvent, 10000);
			NdisResetEvent(&RequestSetEvent);

		}
		else
		{
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Query Monitor Mode status Pending....");

			NdisWaitEvent(&RequestQueryEvent, 10000);
			NdisResetEvent(&RequestQueryEvent);

			status_ = ReqQueryStatus;
		}

	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->RequestCompleteState == 1)
	{

		if (status_ == NDIS_STATUS_SUCCESS)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Query Monitor Mode status Success....");

			memcpy(OP_Mode, ModeCapability, sizeof(DOT11_OPERATION_MODE_CAPABILITY));

		}
		fmh->RequestCompleteState = 0;

		RelSpinLock(&MP->SpinLock_, FALSE);
	}
	else
	{
		RelSpinLock(&MP->SpinLock_, FALSE);

		NdisFCancelOidRequest(fmh->MiniportHandle, PRequest->RequestId);

		status_ = NDIS_STATUS_FAILURE;

		Sleep(1500);
	}

	ExFreePool(ModeCapability);
	ExFreePool(PRequest);


	return status_;
}


NDIS_STATUS RequestMPModeSet(filterModuleHandle* fmh, ULONG CurrentMode)
{
	PNDIS_OID_REQUEST PRequest;
	DOT11_CURRENT_OPERATION_MODE *ModeSet;
	ULONG type;
	NDIS_STATUS status_, status_1;
	DOT11_RESET_REQUEST ResetReq;
	miniport *MP;
	int i;
	BOOLEAN PS = FALSE;

	MP = (miniport*)fmh->MP;


	if (CurrentMode == DOT11_OPERATION_MODE_EXTENSIBLE_AP || CurrentMode == DOT11_OPERATION_MODE_NETWORK_MONITOR || CurrentMode == DOT11_OPERATION_MODE_WFD_DEVICE ||\
		CurrentMode == DOT11_OPERATION_MODE_WFD_GROUP_OWNER || CurrentMode == DOT11_OPERATION_MODE_WFD_CLIENT)
	{
		if (MP->ConnectState == MediaConnectStateConnected)
		{
			
			return NDIS_STATUS_MEDIA_DISCONNECTED;
			
			AcqSpinLock(&MP->SpinLock_, FALSE);

			MP->ModeSet = TRUE;

			RelSpinLock(&MP->SpinLock_, FALSE);
			
			status_1 = RequestInit((NDIS_HANDLE)fmh, &PS, sizeof(BOOLEAN), OID_DOT11_NIC_POWER_STATE, Set);


			if (status_1 == NDIS_STATUS_SUCCESS)
			{
				NdisWaitEvent(&ModeSetEvent, 30000);
				NdisResetEvent(&ModeSetEvent);
			}
		
		}
	}


	PRequest = (PNDIS_OID_REQUEST)ExAllocatePool(NonPagedPool, sizeof(NDIS_OID_REQUEST));

	memset(PRequest, 0, sizeof(NDIS_OID_REQUEST));

	ModeSet = (DOT11_CURRENT_OPERATION_MODE*)ExAllocatePool(NonPagedPool, sizeof(DOT11_CURRENT_OPERATION_MODE));

	memset(ModeSet, 0, sizeof(DOT11_CURRENT_OPERATION_MODE));

	PRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	PRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	PRequest->Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;
	PRequest->Timeout = 5000; // NDIS_OID_REQUEST_TIMEOUT_INFINITE;

	PRequest->PortNumber = 0;



	ModeSet->uCurrentOpMode = CurrentMode;

	PRequest->RequestType = NdisRequestSetInformation;
	PRequest->DATA.SET_INFORMATION.Oid = OID_DOT11_CURRENT_OPERATION_MODE;
	PRequest->DATA.SET_INFORMATION.InformationBuffer = ModeSet;
	PRequest->DATA.SET_INFORMATION.InformationBufferLength = sizeof(DOT11_CURRENT_OPERATION_MODE);


	AcqSpinLock(&MP->SpinLock_, FALSE);

	ReqSetStatus = NDIS_STATUS_FAILURE;

	fmh->RequestCompleteState = 0;

	RelSpinLock(&MP->SpinLock_, FALSE);


	status_ = NdisFOidRequest(fmh->MiniportHandle, PRequest);


	if (status_ != NDIS_STATUS_PENDING)
	{
		//	if (status_ == NDIS_STATUS_SUCCESS)
				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Monitor Mode status success....");
		FilterOidRequestComplete(fmh->MiniportHandle, PRequest, status_);

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);
	}
	else
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Query Monitor Mode status Pending....");

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

		status_ = ReqSetStatus;
	}


	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->RequestCompleteState == 1)
	{
		fmh->RequestCompleteState = 0;

		RelSpinLock(&MP->SpinLock_, FALSE);
	}
	else if (fmh->RequestCompleteState == 0)
	{
		RelSpinLock(&MP->SpinLock_, FALSE);

		NdisFCancelOidRequest(fmh->MiniportHandle, PRequest->RequestId);

		status_ = NDIS_STATUS_FAILURE;

		Sleep(1500);
	}

	ExFreePool(PRequest);
	ExFreePool(ModeSet);


	

	if (fmh->FAP.MiniportPhysicalMediaType == NdisPhysicalMediumNative802_11 && status_ == NDIS_STATUS_SUCCESS)
	{
	//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "NdisPhysicalMediumNative802_11....Set Packet Filter");


		if (CurrentMode == DOT11_OPERATION_MODE_EXTENSIBLE_AP || CurrentMode == DOT11_OPERATION_MODE_NETWORK_MONITOR)
		{
			type = NDIS_PACKET_TYPE_PROMISCUOUS | NDIS_PACKET_TYPE_802_11_RAW_DATA | NDIS_PACKET_TYPE_802_11_PROMISCUOUS_MGMT | \
			NDIS_PACKET_TYPE_802_11_RAW_MGMT | NDIS_PACKET_TYPE_802_11_PROMISCUOUS_CTRL;
		

			status_1 = RequestPacketFilterSet(fmh, type);

			if (status_1 == NDIS_STATUS_SUCCESS)
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request set packet filter Success....");
		}
	}

/*

	if (CurrentMode == DOT11_OPERATION_MODE_EXTENSIBLE_STATION || CurrentMode == DOT11_OPERATION_MODE_STATION)
	{
		memset(&ResetReq, 0, sizeof(DOT11_RESET_REQUEST));

		ResetReq.dot11ResetType = dot11_reset_type_phy_and_mac;
		memcpy(ResetReq.dot11MacAddress, fmh->FAP.CurrentMacAddress, 6);
		ResetReq.bSetDefaultMIB = TRUE;

		status_1 = RequestResetMP(fmh, ResetReq);

		if (status_1 != NDIS_STATUS_SUCCESS)
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "RequestResetMP Failed.....");
		else
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "RequestResetMP Success.....");

		return status_1;

	}

*/

	return status_;
}

NDIS_STATUS RequestPacketFilterSet(filterModuleHandle* fmh, ULONG FilterMask)
{
	PNDIS_OID_REQUEST PRequest;
	ULONG* type;
	NDIS_STATUS status_;
	miniport* MP;

	MP = (miniport*)fmh->MP;

	PRequest = (PNDIS_OID_REQUEST)ExAllocatePool(NonPagedPool, sizeof(NDIS_OID_REQUEST));
	type = (ULONG*)ExAllocatePool(NonPagedPool, sizeof(ULONG));


	PRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	PRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	PRequest->Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;
	PRequest->Timeout = 5000; // NDIS_OID_REQUEST_TIMEOUT_INFINITE;

	PRequest->PortNumber = 0;

	PRequest->RequestType = NdisRequestSetInformation;
	PRequest->DATA.SET_INFORMATION.Oid = OID_GEN_CURRENT_PACKET_FILTER;
	PRequest->DATA.SET_INFORMATION.InformationBuffer = type;
	PRequest->DATA.SET_INFORMATION.InformationBufferLength = sizeof(ULONG);

	*type = FilterMask;


	AcqSpinLock(&MP->SpinLock_, FALSE);

	ReqSetStatus = NDIS_STATUS_FAILURE;

	fmh->RequestCompleteState = 0;

	RelSpinLock(&MP->SpinLock_, FALSE);

	status_ = NdisFOidRequest(fmh->MiniportHandle, PRequest);


	if (status_ != NDIS_STATUS_PENDING)
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Monitor Mode status success....");
		FilterOidRequestComplete(fmh->MiniportHandle, PRequest, status_);

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

	}
	else
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Query Monitor Mode status Pending....");

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

		status_ = ReqSetStatus;
	}

	

	if (status_ == NDIS_STATUS_SUCCESS)
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request set packet filter Success....");

	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->RequestCompleteState == 1)
	{
		fmh->RequestCompleteState = 0;

		RelSpinLock(&MP->SpinLock_, FALSE);
	}
	else if (fmh->RequestCompleteState == 0)
	{
		RelSpinLock(&MP->SpinLock_, FALSE);

		NdisFCancelOidRequest(fmh->MiniportHandle, PRequest->RequestId);

		status_ = NDIS_STATUS_FAILURE;

		Sleep(1500);
	}
	
	
	ExFreePool(PRequest);
	ExFreePool(type);

	return status_;

}


NDIS_STATUS RequestResetMP(filterModuleHandle* fmh, DOT11_RESET_REQUEST ResetReq)
{
	DOT11_RESET_REQUEST *ResetStruct;
	NDIS_OID_REQUEST* PRequest;
	NDIS_STATUS status_;
	miniport* MP;


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Reset Miniport : %s",((miniport*)fmh->MP)->MiniportName);

	MP = (miniport*)fmh->MP;

	PRequest = (NDIS_OID_REQUEST*)ExAllocatePool(NonPagedPool, sizeof(NDIS_OID_REQUEST));

	ResetStruct = (DOT11_RESET_REQUEST*)ExAllocatePool(NonPagedPool, sizeof(DOT11_RESET_REQUEST));

	memcpy(ResetStruct, &ResetReq, sizeof(DOT11_RESET_REQUEST));

	PRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	PRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	PRequest->Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;
	PRequest->Timeout = 5000; // NDIS_OID_REQUEST_TIMEOUT_INFINITE;

	PRequest->PortNumber = 0;

	PRequest->RequestType = NdisRequestMethod;
	PRequest->DATA.METHOD_INFORMATION.Oid = OID_DOT11_RESET_REQUEST;
	PRequest->DATA.METHOD_INFORMATION.InformationBuffer = ResetStruct;
	PRequest->DATA.METHOD_INFORMATION.InputBufferLength = sizeof(DOT11_RESET_REQUEST);
	PRequest->DATA.METHOD_INFORMATION.OutputBufferLength = sizeof(DOT11_RESET_REQUEST);


	ReqMethodStatus = NDIS_STATUS_FAILURE;

	AcqSpinLock(&MP->SpinLock_, FALSE);

	fmh->RequestCompleteState = 0;

	RelSpinLock(&MP->SpinLock_, FALSE);

	status_ = NdisFOidRequest(fmh->MiniportHandle, PRequest);


	if (status_ != NDIS_STATUS_PENDING)
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Reset status success....");
		FilterOidRequestComplete(fmh->MiniportHandle, PRequest, status_);

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

	}
	else
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Reset status Pending....");

		NdisWaitEvent(&RequestMethodEvent, 10000);
		NdisResetEvent(&RequestMethodEvent);

		status_ = ReqMethodStatus;
	}

	if (status_ == NDIS_STATUS_SUCCESS)
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Reset Success....");

	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->RequestCompleteState == 1)
	{
		fmh->RequestCompleteState = 0;

		RelSpinLock(&MP->SpinLock_, FALSE);
	}
	else if (fmh->RequestCompleteState == 0)
	{
		RelSpinLock(&MP->SpinLock_, FALSE);

		NdisFCancelOidRequest(fmh->MiniportHandle, PRequest->RequestId);

		status_ = NDIS_STATUS_FAILURE;

		Sleep(1500);
	}
	
	ExFreePool(PRequest);
	ExFreePool(ResetStruct);
	
	return status_;
}


NDIS_STATUS RequestDisconnectMP(filterModuleHandle* fmh, ULONG ConectionOp)
{
	PNDIS_OID_REQUEST PRequest;
	NDIS_STATUS status_;
	ULONG* st;
	miniport* MP;

	MP = (miniport*)fmh->MP;

	PRequest = (PNDIS_OID_REQUEST)ExAllocatePool(NonPagedPool, sizeof(NDIS_OID_REQUEST));
	st = (ULONG*)ExAllocatePool(NonPagedPool, sizeof(ULONG));

	PRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	PRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	PRequest->Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;
	PRequest->Timeout = 5000;// NDIS_OID_REQUEST_TIMEOUT_INFINITE;

	PRequest->PortNumber = 0;

	PRequest->RequestType = NdisRequestSetInformation;
	PRequest->DATA.SET_INFORMATION.Oid = ConectionOp == 0 ? OID_DOT11_DISCONNECT_REQUEST : OID_DOT11_CONNECT_REQUEST;
	PRequest->DATA.SET_INFORMATION.InformationBuffer = st;
	PRequest->DATA.SET_INFORMATION.InformationBufferLength = sizeof(ULONG);



	AcqSpinLock(&MP->SpinLock_, FALSE);

	ReqSetStatus = NDIS_STATUS_FAILURE;

	fmh->RequestCompleteState = 0;

	RelSpinLock(&MP->SpinLock_,FALSE);

	status_ = NdisFOidRequest(fmh->MiniportHandle, PRequest);


	if (status_ != NDIS_STATUS_PENDING)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disconnect status success....");
			
		FilterOidRequestComplete(fmh->MiniportHandle, PRequest, status_);

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disconnect status Pending....");

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

		status_ = ReqSetStatus;
	}


	if (status_ == NDIS_STATUS_SUCCESS)
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disconnect Success....");

	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->RequestCompleteState == 1)
	{
		fmh->RequestCompleteState = 0;

		RelSpinLock(&MP->SpinLock_, FALSE);
	}
	else if (fmh->RequestCompleteState == 0)
	{
		RelSpinLock(&MP->SpinLock_, FALSE);

		NdisFCancelOidRequest(fmh->MiniportHandle, PRequest->RequestId);

		status_ = NDIS_STATUS_FAILURE;

		Sleep(1500);
	}

	ExFreePool(PRequest);
	ExFreePool(st);

	return status_;
}


NDIS_STATUS RequestDisasociateMP(filterModuleHandle* fmh)
{
	PNDIS_OID_REQUEST PRequest;
	DOT11_DISASSOCIATE_PEER_REQUEST *DisStruct;
	NDIS_STATUS status_;
	miniport* MP;
	UCHAR MAC[6] = { 0x08,0x31,0xA4,0xED,0xBE,0x68 };
	UCHAR MAC1[6] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };

	MP = (miniport*)fmh->MP;

	PRequest = (PNDIS_OID_REQUEST)ExAllocatePool(NonPagedPool, sizeof(NDIS_OID_REQUEST));

	DisStruct = (DOT11_DISASSOCIATE_PEER_REQUEST*)ExAllocatePool(NonPagedPool, sizeof(DOT11_DISASSOCIATE_PEER_REQUEST));

	memset(DisStruct, 0, sizeof(DOT11_DISASSOCIATE_PEER_REQUEST));

	DisStruct->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	DisStruct->Header.Revision = DOT11_DISASSOCIATE_PEER_REQUEST_REVISION_1;
	DisStruct->Header.Size = sizeof(DOT11_DISASSOCIATE_PEER_REQUEST);

	memcpy(DisStruct->PeerMacAddr, MAC1, 6);
	//DisStruct->PeerMacAddr[0] = 0xFF;
	//DisStruct->PeerMacAddr[0] = 0xFF;
	DisStruct->usReason = DOT11_DISASSOC_REASON_OS;



	PRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	PRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	PRequest->Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;
	PRequest->Timeout = 5000; // NDIS_OID_REQUEST_TIMEOUT_INFINITE;

	PRequest->PortNumber = 0;

	PRequest->RequestType = NdisRequestSetInformation;
	PRequest->DATA.SET_INFORMATION.Oid = OID_DOT11_DISASSOCIATE_PEER_REQUEST;
	PRequest->DATA.SET_INFORMATION.InformationBuffer = DisStruct;
	PRequest->DATA.SET_INFORMATION.InformationBufferLength = sizeof(DOT11_DISASSOCIATE_PEER_REQUEST);


	AcqSpinLock(&MP->SpinLock_, FALSE);

	ReqSetStatus = NDIS_STATUS_FAILURE;

	fmh->RequestCompleteState = 0;

	RelSpinLock(&MP->SpinLock_, FALSE);


	status_ = NdisFOidRequest(fmh->MiniportHandle, PRequest);


	if (status_ != NDIS_STATUS_PENDING)
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disassociate status success....");

		FilterOidRequestComplete(fmh->MiniportHandle, PRequest, status_);

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

	}
	else
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disassociate Pending....");

		NdisWaitEvent(&RequestSetEvent, 10000);
		NdisResetEvent(&RequestSetEvent);

		status_ = ReqSetStatus;
	}


	if (status_ == NDIS_STATUS_SUCCESS)
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disassociate Success....");
	else
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disassociate Failed....");


	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->RequestCompleteState == 1)
	{
		fmh->RequestCompleteState = 0;

		RelSpinLock(&MP->SpinLock_, FALSE);
	}
	else if (fmh->RequestCompleteState == 0)
	{
		RelSpinLock(&MP->SpinLock_, FALSE);

		NdisFCancelOidRequest(fmh->MiniportHandle, PRequest->RequestId);

		status_ = NDIS_STATUS_FAILURE;

		Sleep(1500);
	}

	ExFreePool(PRequest);
	ExFreePool(DisStruct);

	return status_;
}


NDIS_STATUS RequestInit(NDIS_HANDLE FilterBindingContext, PVOID Buffer, ULONG BufferSize, NDIS_OID OID, RequestType Type)
{
	PNDIS_OID_REQUEST PRequest;
	PVOID Buff;
	NDIS_STATUS status_;
	filterModuleHandle *fmh;
	miniport* MP;
	PNDIS_EVENT RequestEvent=0;
	PNDIS_STATUS ReqStatus=0;

	fmh = (filterModuleHandle*)FilterBindingContext;

	MP = (miniport*)fmh->MP;

	PRequest = (PNDIS_OID_REQUEST)ExAllocatePool(NonPagedPool, sizeof(NDIS_OID_REQUEST));

	Buff = (PVOID)ExAllocatePool(NonPagedPool, BufferSize);

	memset(Buff, 0, BufferSize);

	memset(PRequest, 0, sizeof(NDIS_OID_REQUEST));

	PRequest->Header.Type = NDIS_OBJECT_TYPE_OID_REQUEST;
	PRequest->Header.Revision = NDIS_OID_REQUEST_REVISION_1;
	PRequest->Header.Size = NDIS_SIZEOF_OID_REQUEST_REVISION_1;
	PRequest->Timeout = 5000; // NDIS_OID_REQUEST_TIMEOUT_INFINITE;

	PRequest->PortNumber = 0;

	switch (Type)
	{
	case Set:
	{
		PRequest->RequestType = NdisRequestSetInformation;

		PRequest->DATA.SET_INFORMATION.InformationBuffer = Buff;
		PRequest->DATA.SET_INFORMATION.InformationBufferLength = BufferSize;
		PRequest->DATA.SET_INFORMATION.Oid = OID;

		ReqSetStatus = NDIS_STATUS_FAILURE;

		ReqStatus = &ReqSetStatus;

		RequestEvent = &RequestSetEvent;

		memcpy(Buff, Buffer, BufferSize);

		break;
	}
	case Query:
	{
		PRequest->RequestType = NdisRequestQueryInformation;

		PRequest->DATA.QUERY_INFORMATION.InformationBuffer = Buff;
		PRequest->DATA.QUERY_INFORMATION.InformationBufferLength = BufferSize;
		PRequest->DATA.QUERY_INFORMATION.Oid = OID;

		ReqQueryStatus = NDIS_STATUS_FAILURE;

		ReqStatus = &ReqQueryStatus;

		RequestEvent = &RequestQueryEvent;

		break;
	}
	case Method:
	{
		PRequest->RequestType = NdisRequestMethod;

		PRequest->DATA.METHOD_INFORMATION.InformationBuffer = Buff;
		PRequest->DATA.METHOD_INFORMATION.InputBufferLength = BufferSize;
		PRequest->DATA.METHOD_INFORMATION.OutputBufferLength = BufferSize;
		PRequest->DATA.METHOD_INFORMATION.Oid = OID;

		ReqMethodStatus = NDIS_STATUS_FAILURE;

		RequestEvent = &RequestMethodEvent;

		ReqStatus = &ReqMethodStatus;

		memcpy(Buff, Buffer, BufferSize);

		break;
	}
	}


	NdisAcquireSpinLock(&MP->SpinLock_);

	fmh->RequestCompleteState = 0;

	NdisReleaseSpinLock(&MP->SpinLock_);

	status_ = NdisFOidRequest(fmh->MiniportHandle, PRequest);


	if (status_ != NDIS_STATUS_PENDING)
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Disassociate status success....");

		FilterOidRequestComplete(fmh->MiniportHandle, PRequest, status_);

		NdisWaitEvent(RequestEvent, 10000);
		NdisResetEvent(RequestEvent);

	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Init Pending....");

		NdisWaitEvent(RequestEvent, 10000);
		NdisResetEvent(RequestEvent);

		status_ = *ReqStatus;
	}


	if (status_ == NDIS_STATUS_SUCCESS)
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Init Success....");
	else
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Request Init Failed....");


	AcqSpinLock(&MP->SpinLock_, FALSE);

	if (fmh->RequestCompleteState == 1)
	{
		fmh->RequestCompleteState = 0;

		RelSpinLock(&MP->SpinLock_, FALSE);
	}
	else if (fmh->RequestCompleteState == 0)
	{
		RelSpinLock(&MP->SpinLock_, FALSE);

		NdisFCancelOidRequest(fmh->MiniportHandle, PRequest->RequestId);

		status_ = NDIS_STATUS_FAILURE;

		Sleep(1500);
	}

	ExFreePool(PRequest);
	ExFreePool(Buff);

	return status_;
}
