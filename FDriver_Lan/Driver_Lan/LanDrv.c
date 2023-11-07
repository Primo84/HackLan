
#pragma once

#include "LanDrv.h"
#include "Filter.h"



NDIS_DEVICE_OBJECT_ATTRIBUTES ObjectAttributes;


#pragma NDIS_INIT_FUNCTION(DriverEntry)

NDIS_SWITCH_NIC_PARAMETERS NParam;
BOOLEAN PS;


_Use_decl_annotations_ NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, IN PUNICODE_STRING str)
{
	PLIST_ENTRY MP_PL, FM_PL;
	miniport* MP;
	filterModuleHandle* fmh;
	DOT11_OPERATION_MODE_CAPABILITY ModeCap;
	NDIS_STATUS status;
	NDIS_DEVICE_OBJECT_ATTRIBUTES Dev_Attrib;

	PDRIVER_DISPATCH PDispatch[IRP_MJ_MAXIMUM_FUNCTION + 1];

	Driver->DriverUnload = &OnUnload;
	

	FilterHandle = 0;
	miniportsCount = 0;
	PACKET_COUNT = 1000;
	isUnload = FALSE;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "FilterDrv start....Path str : %ws\n", str->Buffer);

	InitializeListHead(&MiniportsEntries);

	NdisInitializeEvent(&RequestQueryEvent);
	NdisInitializeEvent(&RequestSetEvent);
	NdisInitializeEvent(&RequestMethodEvent);
	NdisInitializeEvent(&ModeSetEvent);
	NdisInitializeEvent(&SendCompleteEvent);

	PoolParamHandle = NULL;

	RegisterFilterDriver(Driver);

	if (FilterHandle == NULL)
		return STATUS_UNSUCCESSFUL;


	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)PDispatch[i] = &HDeviceSkip;
	PDispatch[IRP_MJ_DEVICE_CONTROL] = &HDeviceIOControl;
	PDispatch[IRP_MJ_CREATE] = &HDeviceCreate;
	PDispatch[IRP_MJ_CLOSE] = &HDeviceClose;
	PDispatch[IRP_MJ_CLEANUP] = &HDeviceCleanUp;
	PDispatch[IRP_MJ_WRITE] = &HDeviceWrite;
	PDispatch[IRP_MJ_READ] = &HDeviceRead;
	
	
	

	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, "Pryzpisanie funkcji sterownika....");

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Tworzenie Objektu Device");

	RtlInitUnicodeString((PUNICODE_STRING)&DevN, Dev_Name);
	RtlInitUnicodeString((PUNICODE_STRING)&DevDN, Dev_Dos_Name);

	memset(&Dev_Attrib, 0, sizeof(NDIS_DEVICE_OBJECT_ATTRIBUTES));
	
	Dev_Attrib.Header.Type = NDIS_OBJECT_TYPE_DEVICE_OBJECT_ATTRIBUTES;
	Dev_Attrib.Header.Revision = NDIS_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
	Dev_Attrib.Header.Size = NDIS_SIZEOF_DEVICE_OBJECT_ATTRIBUTES_REVISION_1;
	Dev_Attrib.DeviceName = &DevN;
	Dev_Attrib.SymbolicName = &DevDN;
	Dev_Attrib.MajorFunctions = &PDispatch;
	
	st = NdisRegisterDeviceEx(FilterHandle, &Dev_Attrib, &Device_Hack, &DeviceHandle);
	
//	st = IoCreateDevice(Driver, 0, &DevN, FILE_DEVICE_NETWORK, FILE_DEVICE_SECURE_OPEN, FALSE, &Device_Hack);


	if (st == STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Utworzono DeviceObject");

		Driver->DeviceObject = Device_Hack;
		
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nieutworzono DeviceObject");
		if (st == STATUS_INSUFFICIENT_RESOURCES) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bload tworzenia Device : STATUS_INSUFFICIENT_RESOURCES");
		else if (st == STATUS_OBJECT_NAME_COLLISION) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bload tworzenia Device : STATUS_OBJECT_NAME_COLLISION");
	}

	Device_Hack->Flags = Device_Hack->Flags | DO_BUFFERED_IO;
	

	MP_PL = MiniportsEntries.Flink;

	i = 0;

	while (MP_PL != &MiniportsEntries)
	{
		MP = (miniport*)CONTAINING_RECORD(MP_PL, miniport, ListE);

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Miniport--------- %s",MP->MiniportName);
		FM_PL = MP->ModeuleEntries.Flink;

		while (FM_PL != &MP->ModeuleEntries)
		{
			fmh = (filterModuleHandle*)CONTAINING_RECORD(FM_PL, filterModuleHandle, ListE);

			
			memset(&ModeCap, 0, sizeof(DOT11_OPERATION_MODE_CAPABILITY));

			status = RequestMPModeCap(fmh, &ModeCap);

			if (status == NDIS_STATUS_SUCCESS)
			{
				AcqSpinLock(&MP->SpinLock_, FALSE);

				fmh->OP_Mode = (DOT11_OPERATION_MODE_CAPABILITY*)ExAllocatePool(NonPagedPool, sizeof(DOT11_OPERATION_MODE_CAPABILITY));
				memcpy(fmh->OP_Mode, &ModeCap, sizeof(DOT11_OPERATION_MODE_CAPABILITY));

				if (fmh->OP_Mode->uOpModeCapability & DOT11_OPERATION_MODE_NETWORK_MONITOR == DOT11_OPERATION_MODE_NETWORK_MONITOR)
				{
					MP->NetMonSupported = 1;
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "NETWORK MONITORT SUPPORTED-------------");
				}

				RelSpinLock(&MP->SpinLock_, FALSE);

			}

			FM_PL = FM_PL->Flink;
		}
		MP_PL = MP_PL->Flink;
		i++;
	}

	return STATUS_SUCCESS;

}

VOID OnUnload(IN PDRIVER_OBJECT Driver)
{
	NDIS_STATUS status;
	int i;
	PLIST_ENTRY MP_PL;
	PLIST_ENTRY	FM_PL;
	miniport *MP;
	filterModuleHandle *fmh;
	

	if (FilterHandle != NULL && FilterRegistered == TRUE)
	{
		isUnload = TRUE;

		NdisFDeregisterFilterDriver(FilterHandle);
		//Sleep(5000);
		FilterRegistered = FALSE;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Filter Driver wyrejestrowano pomyœlnie");
	}
	
	if (miniportsCount > 0)
	{
		MP_PL = MiniportsEntries.Flink;

		while (MP_PL != &MiniportsEntries)
		{
			MP = (miniport*)CONTAINING_RECORD(MP_PL, miniport, ListE);

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Release Miniport--------- %s", MP->MiniportName);

			if (MP->HandleCount > 0)
			{

				FM_PL = MP->ModeuleEntries.Flink;

				while (FM_PL != &MP->ModeuleEntries)
				{
					fmh = (filterModuleHandle*)CONTAINING_RECORD(FM_PL, filterModuleHandle, ListE);

					RemoveEntryList(&fmh->ListE);

					if (fmh->OP_Mode != NULL)
						ExFreePool(fmh->OP_Mode);

					ExFreePool(fmh);

					MP->HandleCount--;

					FM_PL = FM_PL->Flink;
				}

				if(MP->HandleCount == 0)
				{
					RemoveEntryList(&MP->ListE);
					NdisFreeSpinLock(&MP->SpinLock_);

					ExFreePool(MP->SPacket);
					ExFreePool(MP->RPacket);
					ExFreePool(MP);

					if (miniportsCount > 0)
						miniportsCount--;

					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, "Miniport relased ");
				}
			}
			MP_PL = MP_PL->Flink;
		}
	}

	if (PoolParamHandle != NULL)
	{
		NdisFreeNetBufferListPool(PoolParamHandle);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Free PoolParamHandle");
	}


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, " Ending LanDrv....");

	if (Driver->DeviceObject != NULL)
	{
		//IoDeleteSymbolicLink((PUNICODE_STRING)&DevDN);
		//IoDeleteDevice(Driver->DeviceObject);
		NdisDeregisterDeviceEx(DeviceHandle);
	}
}





NTSTATUS HDeviceRead(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	PIO_STACK_LOCATION Stack;
	int licznik, licznik1,count;
	char* buf;
	PVOID buf_out;
	ULONG_PTR ByteReturned;
	int BindingIndex;
	int Operacja;
	EHeader* Packet=NULL;
	int i;
	PLIST_ENTRY PL;
	miniport* MP=NULL;
	BOOLEAN finded;
	USHORT *licz;

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HDeviceRead execute...");

	Stack = IoGetCurrentIrpStackLocation(irp);

	licznik = 0;
	licznik1 = 0;
	ByteReturned = 0;
	finded = FALSE;

	switch (Stack->MajorFunction)
	{
		case IRP_MJ_READ:
		{
			buf_out = irp->AssociatedIrp.SystemBuffer;
			licznik1 = Stack->Parameters.Read.ByteOffset.LowPart;
			licz = (USHORT*)&Stack->Parameters.Read.ByteOffset.HighPart;
			BindingIndex = (int)*licz;
			licz++;
			Operacja = (int)*licz;

		
			PL = MiniportsEntries.Flink;

			while (PL != &MiniportsEntries)
			{
				MP = (miniport*)CONTAINING_RECORD(PL, miniport, ListE);

				AcqSpinLock(&MP->SpinLock_, FALSE);
				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "MP =%s", MP->MiniportName);
			
				if (MP->Index == BindingIndex)
				{
					finded = TRUE;
				//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "MP finded... %s",MP->MiniportName);
					RelSpinLock(&MP->SpinLock_, FALSE);
					break;
				}
			
				RelSpinLock(&MP->SpinLock_, FALSE);
				PL = PL->Flink;
			}
	
		
			if (finded)
			{
				AcqSpinLock(&MP->SpinLock_, FALSE);

				if (Operacja == 0)
				{
					Packet = (EHeader*)MP->RPacket;
					licznik = MP->RecvLicz;
				}
				else if (Operacja == 1)
				{
					Packet = (EHeader*)MP->SPacket;
					licznik = MP->SendLicz;
				}
				else finded = FALSE;

				RelSpinLock(&MP->SpinLock_, FALSE);

				if (finded)
				{

					if (licznik1 != licznik)
					{
						if (licznik1 >= 1000)licznik1 = 0;

						if (licznik1 < licznik)
						{
							count = licznik - licznik1;

							AcqSpinLock(&MP->SpinLock_, FALSE);

							memcpy(buf_out, &Packet[licznik1], count * sizeof(EHeader));
							buf = (char*)buf_out;
							buf = buf + (PACKET_COUNT * sizeof(EHeader));
							memcpy((void*)buf, (void*)&count, sizeof(int));

							RelSpinLock(&MP->SpinLock_, FALSE);
						}

						else
						{
							count = PACKET_COUNT - licznik1;

							AcqSpinLock(&MP->SpinLock_, FALSE);

							memcpy(buf_out, &Packet[licznik1], count * sizeof(EHeader));
							buf = (char*)buf_out;
							buf = buf + (PACKET_COUNT * sizeof(EHeader));
							memcpy((void*)buf, (void*)&count, sizeof(int));

							RelSpinLock(&MP->SpinLock_, FALSE);

						}
						ByteReturned = Stack->Parameters.Read.Length;
					}
				}
			}
		
			break;
		}
		default: break;
	}

	irp->IoStatus.Information = ByteReturned;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS HDeviceWrite(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	PIO_STACK_LOCATION Stack;
	PacketSend* PS;
	PLIST_ENTRY PL;
	BOOLEAN isFinded;
	miniport* MP;
	NET_BUFFER_LIST_POOL_PARAMETERS ListPoolParam;

	NET_BUFFER_LIST* BufferList;
	NET_BUFFER* NB;
	PMDL _mdl, mdlCh;
	PVOID Buffer, Buff;
	ULONG BufferSize;
	LARGE_INTEGER LI;
	ULONG CancelID = 0xAAFF4FC;
	NDIS_STATUS ST, RET_ST;
	PDOT11_EXTSTA_SEND_CONTEXT PSendContext;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HDeviceWrite execute...");

	Stack = IoGetCurrentIrpStackLocation(irp);
	RET_ST = NDIS_STATUS_SUCCESS;

	BufferList = NULL;
	mdlCh = NULL;
	_mdl = NULL;
	Buff = NULL;
	PSendContext = NULL;

	switch (Stack->MajorFunction)
	{
	case IRP_MJ_WRITE:
	{
		PS = (PacketSend*)irp->AssociatedIrp.SystemBuffer;



		//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Adapter Name =  %s ", PS->Adapter.name );

		PL = MiniportsEntries.Flink;

		isFinded = FALSE;

		do
		{
			MP = (miniport*)CONTAINING_RECORD(PL, miniport, ListE);

			if (strcmp(PS->Adapter.name, MP->MiniportName) == 0)
			{
				isFinded = TRUE;
				break;
			}

			PL = PL->Flink;

		} while (PL != &MiniportsEntries);

		if (isFinded)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Miniport finded %s ", MP->MiniportName);

			if (MP->MainModule != NULL)
			{
				if (PoolParamHandle == NULL)
				{
					memset(&ListPoolParam, 0, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));

					ListPoolParam.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
					ListPoolParam.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
					ListPoolParam.Header.Size = sizeof(NET_BUFFER_LIST_POOL_PARAMETERS);

					ListPoolParam.ProtocolId = NDIS_PROTOCOL_ID_DEFAULT;
					ListPoolParam.PoolTag = 'HCKL';
					ListPoolParam.ContextSize = 0;
					ListPoolParam.fAllocateNetBuffer = TRUE;
					ListPoolParam.DataSize = 0;

					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Allocate Net Buffer List Pool");

					PoolParamHandle = NdisAllocateNetBufferListPool(MP->MainModule, &ListPoolParam);
				}

				Buff = NdisAllocateMemoryWithTagPriority(PoolParamHandle, PS->DataSize, 'HCkl', NormalPoolPriority);

				if (Buff != NULL)
				{

					if (PoolParamHandle != NULL)
					{

						_mdl = NdisAllocateMdl(PoolParamHandle, Buff, PS->DataSize);

						if (_mdl != NULL)
						{

							DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Allocate Pool Param Success.... %x", PoolParamHandle);

							BufferList = NdisAllocateNetBufferAndNetBufferList(PoolParamHandle, 0, MEMORY_ALLOCATION_ALIGNMENT * 2, _mdl, 0, PS->DataSize);

							if (BufferList != NULL)
							{

								DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Allocate NetBuffer List Success %x.... ", BufferList);

								NB = NET_BUFFER_LIST_FIRST_NB(BufferList);

								if (NB != NULL)
								{
									mdlCh = NET_BUFFER_FIRST_MDL(NB);

									if (mdlCh != NULL)
									{
										mdlCh->Next = NULL;

										Buffer = NULL;
										BufferSize = 0;

										NdisQueryMdl(mdlCh, &Buffer, &BufferSize, NormalPagePriority);

										if (Buffer != NULL)
										{

											//	ST = NdisAllocateNetBufferListContext(BufferList, 0 /*MEMORY_ALLOCATION_ALIGNMENT * 2 */, MEMORY_ALLOCATION_ALIGNMENT * 2, 'hckL');


											PSendContext = (PDOT11_EXTSTA_SEND_CONTEXT)NdisAllocateMemoryWithTagPriority(PoolParamHandle, sizeof(DOT11_EXTSTA_SEND_CONTEXT), 'Hckl', NormalPagePriority);

											NET_BUFFER_LIST_INFO(BufferList, MediaSpecificInformation) = PSendContext;

											PSendContext = NET_BUFFER_LIST_INFO(BufferList, MediaSpecificInformation);

											if (PSendContext != NULL)
											{
												PSendContext->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
												PSendContext->Header.Revision = DOT11_EXTSTA_SEND_CONTEXT_REVISION_1;
												PSendContext->Header.Size = sizeof(DOT11_EXTSTA_SEND_CONTEXT);

												PSendContext->usExemptionActionType = DOT11_EXEMPT_ALWAYS;
												PSendContext->uPhyId = 0;
												PSendContext->uDelayedSleepValue = 0;
												PSendContext->pvMediaSpecificInfo = NULL;
												PSendContext->uSendFlags = 0;

												NET_BUFFER_NEXT_NB(NB) = NULL;
												NET_BUFFER_DATA_OFFSET(NB) = 0;
												NET_BUFFER_DATA_LENGTH(NB) = PS->DataSize;
												NB->CurrentMdl = _mdl;
												NB->CurrentMdlOffset = 0;

												BufferList->SourceHandle = MP->MainModule;

												NET_BUFFER_LIST_NEXT_NBL(BufferList) = NULL;

												NET_BUFFER_LIST_FLAGS(BufferList) = NBL_FLAGS_PROTOCOL_RESERVED;


												BufferList->ProtocolReserved[1] = (PVOID)BufferList;

												((UCHAR*)&CancelID)[0] = NdisGeneratePartialCancelId();

												NDIS_SET_NET_BUFFER_LIST_CANCEL_ID(BufferList, CancelID);

												memset(Buffer, 0, BufferSize);

												memcpy(Buffer, PS->Packet, BufferSize);

												NdisFSendNetBufferLists(MP->MainModule, BufferList, NDIS_DEFAULT_PORT_NUMBER, NDIS_SEND_FLAGS_DISPATCH_LEVEL);

												NdisWaitEvent(&SendCompleteEvent, 5000);
												NdisResetEvent(&SendCompleteEvent);

											}

											DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Buffer is Sending... Size = %d", BufferSize);

											if (BufferList->ProtocolReserved[2] == BufferList)
											{
												RET_ST = BufferList->Status;
												DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "SendComplete Execute");
											}
											else
											{
												RET_ST = NDIS_STATUS_SEND_ABORTED;
												NdisFCancelSendNetBufferLists(MP->MainModule, (PVOID)CancelID);
											}
										}
									}
								}

							}
						}
					}

					if (PSendContext != NULL)
					{
						NdisFreeMemoryWithTagPriority(PoolParamHandle, PSendContext, 'Hckl');

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Free PSendContext");
					}

					if (Buff != NULL)
					{

						NdisFreeMemoryWithTagPriority(PoolParamHandle, Buff, 'HCkl');

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Free Buff");
					}

					if (_mdl != NULL)
					{
						NdisFreeMdl(_mdl);

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Free _mdl");
					}

					if (BufferList != NULL)
					{
						NdisFreeNetBufferList(BufferList);

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Free BufferList");
					}


				}
				else RET_ST = NDIS_STATUS_FAILURE;
			}
		}

		break;
	}

	default: break;
	}

	irp->IoStatus.Status = RET_ST;

	irp->IoStatus.Information = Stack->Parameters.Write.Length;

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS HDeviceSkip(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceSkip execute...");
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}




//*************************DeviceIOControl*********************************


NTSTATUS HDeviceCreate(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceCreate execute...");
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS HDeviceClose(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceClose execute...");
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS HDeviceCleanUp(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceCleanUp execute...");
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS HDeviceIOControl(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	PIO_STACK_LOCATION stack;
	ULONG code;
	PVOID buf_in, buf_out;
	ULONG in_len, out_len;
	char str[25] = "";
	int rozmiar_buf;
	int i;
	PLIST_ENTRY PL;
	miniport* MP;
	Userminiport *UMP;
	PLIST_ENTRY PLFM;
	filterModuleHandle *fmh;
	Dev_Lan *DevL;
	BOOLEAN isFinded;
	NDIS_STATUS St;
	int isSuccess;

	DOT11_RESET_REQUEST ResetReq;

	stack = IoGetCurrentIrpStackLocation(irp);
	code = stack->Parameters.DeviceIoControl.IoControlCode;
	in_len = stack->Parameters.DeviceIoControl.InputBufferLength;
	out_len = stack->Parameters.DeviceIoControl.OutputBufferLength;
	buf_in = irp->AssociatedIrp.SystemBuffer;
	buf_out = irp->UserBuffer;


	if (stack->MajorFunction == IRP_MJ_DEVICE_CONTROL)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceIoControl execute...");
		switch(code)
		{
			case IO_MINIPORTS_INIT:
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IO_DEV_INIT execute...");
				if (out_len < 5) break;
				memset(buf_out, 0, out_len);
				if (miniportsCount == 0)
				{
					sprintf(str, "BBBsB");
					memcpy(buf_out, str, strlen(str));
					break;
				}
				rozmiar_buf = miniportsCount * sizeof(Userminiport);
				if (rozmiar_buf > out_len)
				{
					sprintf(str, "AAAsA%d", rozmiar_buf);
					memcpy(buf_out, str, strlen(str));
					break;
				}
				PL = MiniportsEntries.Flink;
				i = 0;
				UMP = (Userminiport*)buf_out;

				while (PL != &MiniportsEntries)
				{
					MP = (miniport*)CONTAINING_RECORD(PL, miniport, ListE);
					memset(UMP, 0, sizeof(Userminiport));

					memcpy(UMP->MiniportName, MP->MiniportName,250);

					UMP->HandleCount = MP->HandleCount;
					UMP->Index = MP->Index;
					UMP->miniportCount = miniportsCount;
					UMP->RecvHooked = 0;
					UMP->SendHooked = 0;
					UMP->licznik = 0;

					if (MP->HandleCount > 0)
					{
						PLFM = MP->ModeuleEntries.Flink;

						while (PLFM != &MP->ModeuleEntries)
						{
							fmh = (filterModuleHandle*)CONTAINING_RECORD(PLFM, filterModuleHandle, ListE);

							if (fmh->MiniportHandle == MP->MainModule)
							{
							//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Main Module Finded copy Data to UMP ");

								UMP->MediaConnectState = (MEDIA_CONNECT_STATE)fmh->FAP.MediaConnectState;
								UMP->PhysicalMediumType = (PHYSICALMEDIUM)fmh->FAP.MiniportPhysicalMediaType;
								UMP->MediaDuplexState = (MEDIA_DUPLEX_STATE)fmh->FAP.MediaDuplexState;
								UMP->RcvLinkSpeed = fmh->FAP.RcvLinkSpeed;
								UMP->XmitLinkSpeed = fmh->FAP.XmitLinkSpeed;
								UMP->MacAddressLength = fmh->FAP.MacAddressLength;
								memcpy(UMP->CurrentMacAddress, fmh->FAP.CurrentMacAddress, 32);
							}

							PLFM = PLFM->Flink;
						}
					}
					PL = PL->Flink;
					if (PL != &MiniportsEntries)
						UMP++;
				}
				break;
			}

			case IO_SET_OP_MODE:
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IO_SET_OP_MODE execute ");

				DevL = (Dev_Lan*)buf_in;

				PL = MiniportsEntries.Flink;

				isFinded = FALSE;

				do 
				{
					MP = (miniport*)CONTAINING_RECORD(PL, miniport, ListE);

					if (strcmp(DevL->name, MP->MiniportName) == 0)
					{
						isFinded = TRUE;
						break;
					}

					PL = PL->Flink;

				} while (PL != &MiniportsEntries);

				if (isFinded)
				{
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Miniport finded %s ", MP->MiniportName);

					isSuccess = 0;

					PLFM = MP->ModeuleEntries.Flink;

					memset(&ResetReq, 0, sizeof(DOT11_RESET_REQUEST));

					ResetReq.dot11ResetType = dot11_reset_type_phy_and_mac;
					ResetReq.bSetDefaultMIB = TRUE;

					while (PLFM != &MP->ModeuleEntries)
					{
						fmh = (filterModuleHandle*)CONTAINING_RECORD(PLFM, filterModuleHandle, ListE);

						if (fmh->MiniportHandle == MP->MainModule)
						{

							St = RequestMPModeSet(fmh, DevL->CurrentMode);
						
							
							if (St == NDIS_STATUS_SUCCESS)
								isSuccess = 1;
							else if (St == NDIS_STATUS_MEDIA_DISCONNECTED)
								isSuccess = 2;

							break;
						}

						PLFM = PLFM->Flink;
					}
					

				}
				memcpy(buf_out, &isSuccess, sizeof(int));

				break;
			}

			case IO_QUERY_STATE:
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IO_SET_QUERY_MODE execute ");

				UMP = (Userminiport*)buf_in;
				PL = MiniportsEntries.Flink;

				isSuccess = 0;
				isFinded = FALSE;

				do
				{
					MP = (miniport*)CONTAINING_RECORD(PL, miniport, ListE);

					if (strcmp(UMP->MiniportName, MP->MiniportName) == 0)
					{
						isFinded = TRUE;
						break;
					}

					PL = PL->Flink;

				} while (PL != &MiniportsEntries);

				if (isFinded)
				{
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Miniport finded %s ", MP->MiniportName);
					
					UMP = (Userminiport*)buf_out;

					if (MP->HandleCount > 0)
					{
						PLFM = MP->ModeuleEntries.Flink;
						fmh = (filterModuleHandle*)CONTAINING_RECORD(PLFM, filterModuleHandle, ListE);

						UMP->MediaConnectState = (MEDIA_CONNECT_STATE)MP->ConnectState;
						UMP->MediaDuplexState = (MEDIA_DUPLEX_STATE)fmh->FAP.MediaDuplexState;
						UMP->RcvLinkSpeed = fmh->FAP.RcvLinkSpeed;
						UMP->XmitLinkSpeed = fmh->FAP.XmitLinkSpeed;

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Module Handle: %x", fmh->MiniportHandle);
					}

				}

				break;
			}

		}
	}
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}





void Sleep(ULONG milisec)
{
	KTIMER Timer;
	LARGE_INTEGER li;

	li.QuadPart = (LONGLONG)milisec;
	li.QuadPart = li.QuadPart * -10000;

	KeInitializeTimer(&Timer);
	KeSetTimer(&Timer, li,NULL);
	KeWaitForSingleObject((PVOID)&Timer, Executive, KernelMode, FALSE, NULL);
}


int CreateFile(wchar_t* FileName)
{
	wchar_t FName[250];
	UNICODE_STRING UStr;
	OBJECT_ATTRIBUTES Obj;
	HANDLE FHn;
	IO_STATUS_BLOCK IoStatus;
	NTSTATUS St;

	memset(FName, 0, 250 * sizeof(wchar_t));

	wcscpy(FName, L"\\DosDevices\\");
	wcscpy(&FName[wcslen(FName)], FileName);

	RtlInitUnicodeString(&UStr, FName);
	InitializeObjectAttributes(&Obj, &UStr, OBJ_CASE_INSENSITIVE, NULL, NULL);
	FHn = NULL;
	st = ZwCreateFile(&FHn, GENERIC_ALL, &Obj, &IoStatus, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OVERWRITE_IF, 0, NULL, 0);
	if (st == STATUS_SUCCESS)

	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "CreateFile success...");
		if (FHn != NULL)
			ZwClose(FHn);
	}
	return 0;
}


