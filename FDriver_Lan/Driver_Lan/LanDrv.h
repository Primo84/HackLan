#pragma once

//#include "Lan.h"
#include "FileDef.h"
//#include "wdf.h"


#define FILE_DEV_DRVF		0x00002A7A

#define IO_MINIPORTS_INIT		(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x807,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_SET_OP_MODE			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_QUERY_STATE			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)


DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD OnUnload;


NTSTATUS HDeviceIOControl(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceCreate(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceClose(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceCleanUp(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceRead(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceWrite(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS HDeviceSkip(__in DEVICE_OBJECT* Device, __inout IRP* irp);


PDEVICE_OBJECT Device_Hack;
NDIS_HANDLE DeviceHandle;


wchar_t Dev_Name[] = L"\\Device\\FHacklan";
wchar_t Dev_Dos_Name[] = L"\\DosDevices\\FHacklan";

UNICODE_STRING DevN, DevDN;
NTSTATUS st;
int i;


