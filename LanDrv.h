#pragma once

typedef struct _net_cards
{
	char keyPath[150];
	char Description[150];
	char ServiceName[150];
}net_cards;


NTSTATUS DeviceIOControl(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS DeviceCreate(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS DeviceClose(__in DEVICE_OBJECT* Device, __inout IRP* irp);
NTSTATUS DeviceCleanUp(__in DEVICE_OBJECT* Device, __inout IRP* irp);

#define NumberOfCards 15;

#define FILE_DEV_DRV		0x00002A7B

#define IO_DEV_INIT			(ULONG)CTL_CODE(FILE_DEV_DRV,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	IO_DEV_VER			(ULONG)CTL_CODE(FILE_DEV_DRV,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)