#pragma once

//#include "Lan.h"
#include "FileDef.h"
//#include "wdf.h"


#define FILE_DEV_DRV		0x00002A7B


#define IO_DEV_INIT				(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	IO_OPEN_ADAPTER			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	IO_CLOSE_ADAPTER		(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x803,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_QUERY_CONECTED		(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x806,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_SET_OP_MODE			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_QUERY_STATE			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)



NTSTATUS HDeviceIOControl(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceCreate(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceClose(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceCleanUp(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceRead(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceWrite(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceSkip(__in DEVICE_OBJECT* Device, __inout IRP* irp);


PDEVICE_OBJECT Device_Hack, DeviceAttach;

wchar_t Dev_Name[] = L"\\Device\\PHacklan";
wchar_t Dev_Dos_Name[] = L"\\DosDevices\\PHacklan";
NDIS_STRING Service_Name = NDIS_STRING_CONST("PHACKLAN");

ULONG CancelID = 0xABFF4FC;

UNICODE_STRING DevN, DevDN;
//NDIS_MEDIUM medium[] = { NdisMediumNative802_11,NdisMedium802_3,NdisMedium802_5,NdisMediumFddi,NdisMediumWan,NdisMediumWirelessWan };

int CreateFile(wchar_t* FileName);