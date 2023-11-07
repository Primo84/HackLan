
#pragma once

#include "Lan.h"
#include "LanDrv.h"





NDIS_DEVICE_OBJECT_ATTRIBUTES ObjectAttributes;


#pragma NDIS_INIT_FUNCTION(DriverEntry)



_Use_decl_annotations_ NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, IN PUNICODE_STRING str)
{
	Driver->DriverUnload = &OnUnload;


	Init_Adapter = 0;

	//Driver_Hack = Driver;
	
	adapterCount = 0;

	miniportCount = 0;

	PACKET_COUNT = 1000;

	ADAPTER_COUNT = 50;

	DataSize = 5000;
	PoolHandle = NULL;
	PoolParamHandle = NULL;
	Buffer_List = NULL;
	allocated = 0;
	
	cards = (net_cards*)ExAllocatePool(NonPagedPool, ADAPTER_COUNT * sizeof(net_cards));

	NdisInitializeEvent(&RequestEvent);
	NdisInitializeEvent(&SendEvent);

	Device_Hack = NULL;

//	AllocateBufferList();

	for (i = 0; i < ADAPTER_COUNT; i++)
	{
		memset(&cards[i],0,sizeof(net_cards));
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ProtocolDrv start....Path Driver : %ws\n",Driver->DriverExtension->ServiceKeyName.Buffer);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ProtocolDrv start....Path str : %ws\n", str->Buffer);

	GetNetworkCards();

	for (i = 0; i < adapterCount; i++)
	{
		//cards[i].Packet = (EHead_802_11*)ExAllocatePool(NonPagedPool, PACKET_COUNT * sizeof(EHead_802_11));
		cards[i].indeks = 0;
		cards[i].licznik = 0;
		cards[i].openIndex = 0;
		cards[i].BindingContext = i;
		cards[i].Buffer = (NDIS_HANDLE)ExAllocatePool(NonPagedPool, PACKET_COUNT * sizeof(EHeader));
		cards[i].isOpened = FALSE;
		NdisAllocateSpinLock(&cards[i].SpinLockA);
		NdisInitializeEvent(&cards[i].CloseEvent);
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, "Zaalokowano Bufor pakietow dla karty %s",cards[i].Description);
	}

	
	
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)Driver->MajorFunction[i] = &HDeviceSkip;
	Driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &HDeviceIOControl;
	Driver->MajorFunction[IRP_MJ_CREATE] = &HDeviceCreate;
	Driver->MajorFunction[IRP_MJ_CLOSE] = &HDeviceClose;
	Driver->MajorFunction[IRP_MJ_CLEANUP] = &HDeviceCleanUp;
	Driver->MajorFunction[IRP_MJ_READ] = &HDeviceRead;
	Driver->MajorFunction[IRP_MJ_WRITE] = &HDeviceWrite;
	
	

	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, "Pryzpisanie funkcji sterownika....");

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Tworzenie Objektu Device");

	
	RtlInitUnicodeString((PUNICODE_STRING)&DevN, Dev_Name);
	RtlInitUnicodeString((PUNICODE_STRING)&DevDN, Dev_Dos_Name);

	
	
	st = IoCreateDevice(Driver, 0, &DevN, FILE_DEVICE_NETWORK, FILE_DEVICE_SECURE_OPEN, FALSE, &Device_Hack);

	if (st == STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Utworzono DeviceObject");
		
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nieutworzono DeviceObject");
		if (st == STATUS_INSUFFICIENT_RESOURCES) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bload tworzenia Device : STATUS_INSUFFICIENT_RESOURCES");
		else if (st == STATUS_OBJECT_NAME_COLLISION) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bload tworzenia Device : STATUS_OBJECT_NAME_COLLISION");
	}

	
	if (IoCreateSymbolicLink(&DevDN,&DevN) == STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Utworzono SymbolicLink");
	}
	else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "NieUtworzono SymbolicLink");

	Device_Hack->Flags = Device_Hack->Flags | DO_DIRECT_IO;
	Device_Hack->Flags = Device_Hack->Flags | DO_BUFFERED_IO;
	
	
	i = RegisterProtocol(NULL, Service_Name);
	if (i == 0)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Zarejestrowano protokó³ pomyœlnie z nazwa %ws\n", Service_Name.Buffer);
		Init_Adapter = 1;
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mo¿na zarejestrowaæ protoko³u kod b³êdu : %d\n", i);
		//return STATUS_UNSUCCESSFUL;
	}


	return STATUS_SUCCESS;

}


//*******************************Service Stop******************************



VOID OnUnload(IN PDRIVER_OBJECT Driver)
{
	NDIS_STATUS status;
	int i;

	
	CloseAdapters();
	
	if (Init_Adapter == 1)
	{

		NdisDeregisterProtocolDriver(Protocol_Handle);
		Init_Adapter = 0;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Protokó³ wyrejestrowano pomyœlnie");
	}
	
	
	for (i = 0; i < adapterCount + miniportCount; i++)
	{

		ExFreePool(cards[i].Buffer);
		NdisFreeSpinLock(&cards[i].SpinLockA);
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, "Zwolniono Bufor pakietow dla karty %s", cards[i].Description);
	}

	if (PoolParamHandle != NULL)
	{
		NdisFreeNetBufferListPool(PoolParamHandle);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "PoolParamHandle relased....");
	}

	if (Buffer_List != NULL)
	{	
		if (Buffer_List->Context != NULL)
		{
			NdisFreeNetBufferListContext(Buffer_List, MEMORY_ALLOCATION_ALIGNMENT);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Context Buffer_List is relased.....");
		}

		if (Buffer != NULL)
		{
			ExFreePool(Buffer);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Zwolniono Bufor dla NetBufferList....");
		}

		if (mdl != NULL)
		{
			IoFreeMdl(mdl);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Zwolniono MDL dla NetBufferList....");
		}

		
		NdisFreeNetBufferList(Buffer_List);

		if (PoolHandle != NULL)
		{
			NdisFreeNetBufferListPool(PoolHandle);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "PoolHandle and Buffer_List relased....");
		}

	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, " Ending LanDrv....");


	if (cards != NULL)
		ExFreePool(cards);
	
	if (Driver->DeviceObject != NULL)
	{
		IoDeleteSymbolicLink((PUNICODE_STRING)&DevDN);
		IoDeleteDevice(Driver->DeviceObject);
	}
}





NTSTATUS HDeviceRead(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	PIO_STACK_LOCATION Stack;
	int licznik,licznik1,OP_AD,count;
	char *buf;
	PVOID buf_out;
	ULONG_PTR ByteReturned;
	ULONG64 BindingContext;
	EHeader *Packet;
	int i;

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HDeviceRead execute...");

	Stack=IoGetCurrentIrpStackLocation(irp);

	licznik = 0;
	licznik1 = 0;
	ByteReturned = 0;
	OP_AD = 0;

	switch (Stack->MajorFunction)
	{
		case IRP_MJ_READ:
		{
			buf_out = irp->AssociatedIrp.SystemBuffer;
			licznik1 = Stack->Parameters.Read.ByteOffset.LowPart;
			BindingContext = Stack->Parameters.Read.ByteOffset.HighPart;

			for (i = 0; i < adapterCount; i++)
			{
				NdisAcquireSpinLock(&cards[i].SpinLockA);

				if (BindingContext == cards[i].BindingContext)
				{
					OP_AD = i;
					NdisReleaseSpinLock(&cards[i].SpinLockA);
					break;
				}
				NdisReleaseSpinLock(&cards[i].SpinLockA);
			}

			NdisAcquireSpinLock(&cards[OP_AD].SpinLockA);

			Packet = (EHeader*)cards[OP_AD].Buffer;
			licznik = cards[OP_AD].licznik;

			NdisReleaseSpinLock(&cards[OP_AD].SpinLockA);

			if (licznik1 != licznik)
			{
				if (licznik1 >= 1000)licznik1 = 0;

				if (licznik1 < licznik)
				{
					count = licznik - licznik1;

					NdisAcquireSpinLock(&cards[OP_AD].SpinLockA);

					memcpy(buf_out, &Packet[licznik1], count * sizeof(EHeader));
					buf = (char*)buf_out;
					buf = buf + (PACKET_COUNT * sizeof(EHeader));
					memcpy((void*)buf, (void*)&count, sizeof(int));

					NdisReleaseSpinLock(&cards[OP_AD].SpinLockA);
				}

				else
				{
					count = PACKET_COUNT - licznik1;
					
					NdisAcquireSpinLock(&cards[OP_AD].SpinLockA);

					memcpy(buf_out, &Packet[licznik1], count * sizeof(EHeader));
					buf = (char*)buf_out;
					buf = buf + (PACKET_COUNT * sizeof(EHeader));
					memcpy((void*)buf, (void*)&count, sizeof(int));

					NdisReleaseSpinLock(&cards[OP_AD].SpinLockA);

				}
				ByteReturned = Stack->Parameters.Read.Length;
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
	NET_BUFFER_LIST_POOL_PARAMETERS ListPoolParam;
	NET_BUFFER_LIST* BufferList;
	NET_BUFFER* NB;
	PMDL _mdl,mdlCh;
	PVOID Buffer,Buff;
	ULONG BufferSize;
	int i, OP_AD;
	NDIS_STATUS ST, RET_ST;
	PDOT11_EXTSTA_SEND_CONTEXT PSendContext;

	OP_AD = 0;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HDeviceWrite execute...");

	Stack = IoGetCurrentIrpStackLocation(irp);
	RET_ST = NDIS_STATUS_SUCCESS;

	switch (Stack->MajorFunction)
	{
		case IRP_MJ_WRITE:
		{

			PS = (PacketSend*)irp->AssociatedIrp.SystemBuffer;

			for (i = 0; i < adapterCount; i++)
			{
				NdisAcquireSpinLock(&cards[i].SpinLockA);

				if (PS->Adapter.BindingContext == cards[i].BindingContext)
				{
					OP_AD = i;
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Adapter Finded Name =  %s ", cards[i].Description);
					NdisReleaseSpinLock(&cards[i].SpinLockA);
					break;
				}
				NdisReleaseSpinLock(&cards[i].SpinLockA);
			}

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Adapter Name =  %s ", PS->Adapter.name );

			if (PoolParamHandle == NULL)
			{
				memset(&ListPoolParam, 0, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));

				ListPoolParam.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
				ListPoolParam.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
				ListPoolParam.Header.Size = sizeof(NET_BUFFER_LIST_POOL_PARAMETERS);

				ListPoolParam.ProtocolId = NDIS_PROTOCOL_ID_DEFAULT;
				ListPoolParam.PoolTag = 'HcKL';
				ListPoolParam.ContextSize = 0;
				ListPoolParam.fAllocateNetBuffer = TRUE;
				ListPoolParam.DataSize = 0; // PS->DataSize;

				PoolParamHandle = NdisAllocateNetBufferListPool(cards[OP_AD].Adapter_Handle, &ListPoolParam);
			}

			BufferList = NULL;
			mdlCh = NULL;
			_mdl = NULL;
			Buff = NULL;
			PSendContext = NULL;

			if (PoolHandle != NULL)
			{
				Buff = NdisAllocateMemoryWithTagPriority(PoolHandle, PS->DataSize, 'hckl', NormalPoolPriority);

				if (Buff != NULL)
				{

					memset(Buff, 0, PS->DataSize );

					NdisMoveMemory(Buff, PS->Packet, PS->DataSize);

					//	memcpy(Buffer, PS->Packet, PS->DataSize);

					if(PoolParamHandle != NULL)
						_mdl = NdisAllocateMdl(PoolParamHandle, Buff, PS->DataSize);
	
					if (_mdl != NULL)
					{

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Allocate Pool Param Success.... %x", PoolHandle);

						BufferList = NdisAllocateNetBufferAndNetBufferList(PoolParamHandle, 0, MEMORY_ALLOCATION_ALIGNMENT * 2, _mdl, 0, PS->DataSize);

						if (BufferList != NULL)
						{
							DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Allocate NetBuffer List Success %x.... ", BufferList);

							NB = NET_BUFFER_LIST_FIRST_NB(BufferList);

							if (NB != NULL)
							{
								DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Currnet Mdl =  %x, MdlChain = %x.... ", NB->CurrentMdl, NB->MdlChain);

								NET_BUFFER_NEXT_NB(NB) = NULL;
								NET_BUFFER_DATA_OFFSET(NB) = 0;
								NET_BUFFER_DATA_LENGTH(NB) = PS->DataSize;
								NB->CurrentMdl = _mdl;
								NB->CurrentMdlOffset = 0;

								mdlCh = NET_BUFFER_FIRST_MDL(NB);

								if (mdlCh != NULL)
								{
									mdlCh->Next = NULL;

									Buffer = NULL;
									BufferSize = 0;

									NdisQueryMdl(mdlCh, &Buffer, &BufferSize, NormalPagePriority);

									if (Buffer != NULL)
									{

									//	ST = NdisAllocateNetBufferListContext(BufferList, MEMORY_ALLOCATION_ALIGNMENT * 2, MEMORY_ALLOCATION_ALIGNMENT * 2, 'Hckl');


										DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "PSendConrtextSize = %d", sizeof(DOT11_EXTSTA_SEND_CONTEXT));

										PSendContext = (PDOT11_EXTSTA_SEND_CONTEXT)NdisAllocateMemoryWithTagPriority(PoolHandle, sizeof(DOT11_EXTSTA_SEND_CONTEXT), 'HCKl', NormalPagePriority);

										NET_BUFFER_LIST_INFO(BufferList, MediaSpecificInformation) = PSendContext;

										PSendContext = NET_BUFFER_LIST_INFO(BufferList, MediaSpecificInformation);

										if (PSendContext != NULL)
										{
											DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "PSendConrtextSize is allocatet...........", sizeof(DOT11_EXTSTA_SEND_CONTEXT));

											PSendContext->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
											PSendContext->Header.Revision = DOT11_EXTSTA_SEND_CONTEXT_REVISION_1;
											PSendContext->Header.Size = sizeof(DOT11_EXTSTA_SEND_CONTEXT);

											PSendContext->usExemptionActionType = DOT11_EXEMPT_ALWAYS;
											PSendContext->uPhyId = 0;
											PSendContext->uDelayedSleepValue = 0;
											PSendContext->pvMediaSpecificInfo = NULL;
											PSendContext->uSendFlags = 0;
										}
										else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "PSendConrtextSize = NULL", sizeof(DOT11_EXTSTA_SEND_CONTEXT));

										BufferList->SourceHandle = cards[OP_AD].Adapter_Handle; // PoolHandle;

										NET_BUFFER_LIST_NEXT_NBL(BufferList) = NULL;

										NET_BUFFER_LIST_FLAGS(BufferList) = NBL_FLAGS_PROTOCOL_RESERVED;

										BufferList->ProtocolReserved[0] = (PVOID)BufferList;

										((UCHAR*)&CancelID)[0] = NdisGeneratePartialCancelId();

										NDIS_SET_NET_BUFFER_LIST_CANCEL_ID(BufferList, CancelID);

										NDIS_SET_NET_BUFFER_LIST_CANCEL_ID(BufferList, CancelID);


										NdisSendNetBufferLists(cards[OP_AD].Adapter_Handle, BufferList, NDIS_DEFAULT_PORT_NUMBER, NDIS_SEND_FLAGS_DISPATCH_LEVEL);

										NdisWaitEvent(&SendEvent, 5000);
										NdisResetEvent(&SendEvent);

										if (BufferList->ProtocolReserved[2] != BufferList)
											NdisCancelSendNetBufferLists(cards[OP_AD].Adapter_Handle, CancelID);
										else
											DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "SendBufferListComplete Execute.......");

									//	NdisFreeNetBufferListContext(BufferList, MEMORY_ALLOCATION_ALIGNMENT * 2);

										DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Buffer is Sending... Size = %d", BufferSize);
									}
								}
							}

						}
					}
				}
				
				if (PSendContext != NULL)
				{
					//	ExFreePool(Buff);

					NdisFreeMemoryWithTagPriority(PoolHandle, PSendContext, 'HCKl');

					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Free PSendContext");
				}

				if (Buff != NULL)
				{
					//ExFreePool(Buff);
					//NdisFreeMemory(Buff, PS->DataSize, 0);
					NdisFreeMemoryWithTagPriority(PoolHandle, Buff, 'hckl');

					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Free Buff");
				}

				if (_mdl != NULL)
				{
					//IoFreeMdl(_mdl);
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


NTSTATUS HDeviceCreate(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceCreate execute...");
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS HDeviceClose(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceClose execute...");
	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
NTSTATUS HDeviceCleanUp(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceCleanUp execute...");
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
	char str[25] = "";
	ULONG in_len, out_len;
	Dev_Lan* bt;
	int i,count;
	ULONG64 BindingContext;
	char* buf;
	EHeader* SendBuffer;
	PNET_BUFFER NetBuffer;
	PMDL mdl_;
	PVOID EBuffer = NULL;
	EHeader* Packet;
	int licznik,licznik1;
	int OP_AD;
	ULONG ByteReturned;
	DOT11_CURRENT_OPERATION_MODE OperationMode;
	NDIS_STATUS status_;
	ULONG DSize;

	ByteReturned = 0;
	licznik1 = 0;

	stack = IoGetCurrentIrpStackLocation(irp);

	code = stack->Parameters.DeviceIoControl.IoControlCode;
	in_len = stack->Parameters.DeviceIoControl.InputBufferLength;
	out_len = stack->Parameters.DeviceIoControl.OutputBufferLength;
	buf_in = irp->AssociatedIrp.SystemBuffer;
	buf_out = irp->UserBuffer;


	if (stack->MajorFunction == IRP_MJ_DEVICE_CONTROL)
	{
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DeviceIoControl execute...");
		switch (code)
		{
			case IO_DEV_INIT:
			{
				if (out_len <5) break;
				memset(buf_out, 0, out_len);
				if (adapterCount == 0)
				{
					sprintf(str, "BBBsB");
					memcpy(buf_out, str, strlen(str));
					ByteReturned = strlen(str);
					break;
				}
				rozmiar_buf = adapterCount * sizeof(Dev_Lan);
				if (rozmiar_buf > out_len)
				{
					sprintf(str, "AAAsA%d", rozmiar_buf);
					memcpy(buf_out, str, strlen(str));
					ByteReturned = strlen(str);
				}
				else
				{
			
					bt = (Dev_Lan*)buf_out;
					
					for (i = 0; i < adapterCount; i++)
					{
						NdisAcquireSpinLock(&cards[i].SpinLockA);

						memcpy(bt->name, cards[i].Description, 250);
						memcpy(bt->service_name, cards[i].ServiceName, 250);
						bt->BindingContext = cards[i].BindingContext;
						bt->NetCardsCount = adapterCount;
						bt->ModeCap = cards[i].ModeCap;
						bt->CurrentMode = cards[i].CurrentMode;
						bt->NetMonSupported = cards[i].NetMonSupported;
						bt->MtuSize = cards[i].BindParam.MtuSize;
						bt->MaxXmitLinkSpeed = cards[i].BindParam.MaxXmitLinkSpeed;
						bt->XmitLinkSpeed = cards[i].BindParam.XmitLinkSpeed;
						bt->MaxRcvLinkSpeed = cards[i].BindParam.MaxRcvLinkSpeed;
						bt->MacAddressLength = cards[i].BindParam.MacAddressLength;
						bt->PhysicalMediumType = (PHYSICALMEDIUM)cards[i].BindParam.PhysicalMediumType;
						bt->MediumType = (MEDIUM)cards[i].BindParam.MediaType;
						bt->MediaConnectState = (MEDIA_CONNECT_STATE)cards[i].BindParam.MediaConnectState;
						memcpy(bt->CurrentMacAddress, cards[i].BindParam.CurrentMacAddress, 32);

						NdisReleaseSpinLock(&cards[i].SpinLockA);
						if (i < adapterCount - 1) bt++;
					}

					ByteReturned = sizeof(net_cards) * adapterCount;
				}
				break;
			}
			case IO_OPEN_ADAPTER:
			{
				if (out_len < 5)break;
				memset(buf_out, 0, out_len);
				if (in_len < sizeof(Dev_Lan))
				{
					sprintf(str, "AAAsA");
					memcpy(buf_out, str, strlen(str));
					ByteReturned = strlen(str);
					break;
				}
				if (Init_Adapter == 1)
				{
					bt = (Dev_Lan*)buf_in;
					
					if (OpenAdapter(*bt) == 0)
					{
						sprintf(str, "Adapter O");
						memcpy(buf_out, str, strlen(str));
						ByteReturned = strlen(str);
					}
				}
				break;
			}
			
			case IO_CLOSE_ADAPTER:
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IO_CLOSE_ADAPTER execute ");
				bt = (Dev_Lan*)buf_in;
				for (i = 0; i < adapterCount; i++)
				{
					if (bt->BindingContext == cards[i].BindingContext)
					{
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Adapter to close finded i = %d ",i);
						OP_AD = i;
						break;
					}
				}
				if (cards[OP_AD].openIndex > 0)
				{
					NdisAcquireSpinLock(&cards[OP_AD].SpinLockA);
					cards[OP_AD].openIndex--;
					NdisReleaseSpinLock(&cards[OP_AD].SpinLockA);
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Adapter Openindex == %d ", cards[OP_AD].openIndex);
				}
				break;
			}

			case IO_QUERY_CONECTED:
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IO_QUERY_CONECTED execute ");
				bt = (Dev_Lan*)buf_in;
				for (i = 0; i < adapterCount; i++)
				{
					NdisAcquireSpinLock(&cards[i].SpinLockA);

					if (bt->BindingContext == cards[i].BindingContext)
					{
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Adapter %s - openindex = %d ",cards[i].Description,cards[i].openIndex);
						memcpy(buf_out, &cards[i].openIndex, sizeof(int));
						NdisReleaseSpinLock(&cards[i].SpinLockA);
						break;
					}
					NdisReleaseSpinLock(&cards[i].SpinLockA);
				}
				break;
			}

			case IO_SET_OP_MODE:
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IO_SET_OP_MODE execute ");
				bt = (Dev_Lan*)buf_in;

				for (i = 0; i < adapterCount; i++)
				{
					NdisAcquireSpinLock(&cards[i].SpinLockA);

					if (bt->BindingContext == cards[i].BindingContext)
					{
						cards[i].CurrentMode = bt->CurrentMode;

						NdisReleaseSpinLock(&cards[i].SpinLockA);

						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Adapter Mode Changed ");

						break;
					}
					NdisReleaseSpinLock(&cards[i].SpinLockA);
				}

				break;
			}
			case IO_QUERY_STATE:
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "IO_QUERY_STATE execute ");

				bt = (Dev_Lan*)buf_in;

				for (i = 0; i < adapterCount; i++)
				{

					if (bt->BindingContext == cards[i].BindingContext)
					{

						memset(&OperationMode, 0, sizeof(DOT11_CURRENT_OPERATION_MODE));

						status_ = RequestInit(&cards[i], &OperationMode, sizeof(DOT11_CURRENT_OPERATION_MODE), OID_DOT11_CURRENT_OPERATION_MODE, Query);

						NdisAcquireSpinLock(&cards[i].SpinLockA);

						if (status_ == NDIS_STATUS_SUCCESS)
						{
							DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "OID_DOT11_CURRENT_OPERATION_MODE Success.... ");

							memcpy(&cards[i].OperationMode, &OperationMode, sizeof(DOT11_CURRENT_OPERATION_MODE));
							cards[i].CurrentMode = OperationMode.uCurrentOpMode;

						}

						bt = (Dev_Lan*)buf_out;
						
						bt->CurrentMode = cards[i].CurrentMode;
						bt->XmitLinkSpeed = cards[i].BindParam.XmitLinkSpeed;
						bt->RcvLinkSpeed = cards[i].BindParam.RcvLinkSpeed;
						bt->MediaConnectState = (MEDIA_CONNECT_STATE)cards[i].BindParam.MediaConnectState;

						NdisReleaseSpinLock(&cards[i].SpinLockA);

						break;
					}
				}
				break;
			}
		}
	}
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
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
	KeSetTimer(&Timer, li, NULL);
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
	st=ZwCreateFile(&FHn, GENERIC_ALL, &Obj, &IoStatus, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OVERWRITE_IF, 0, NULL, 0);
	if (st == STATUS_SUCCESS)

	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "CreateFile success...");
		if (FHn != NULL)
			ZwClose(FHn);
	}
	return 0;
}





