#pragma once

#define NDIS620
#define NDIS620_MINIPORT 
#define NDIS_MINIPORT_DRIVER 1
#define NDIS_WDM 
#define NDIS_LEGACY_MINIPORT 1
#define NDIS_SUPPORT_NDIS60 1
#ifndef __NDISPROT__H
#define __NDISPROT__H
#endif
#define NDIS_LEGACY_PROTOCOL 1
#define NWF_WFD_SUPPORTED
//#define UM_NDIS620

//#define NTDDI_VERSION NTDDI_WIN7


#include"ndis.h"
#include"ntddk.h"
#include "wdm.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"



#define AcqSpinLock(SpinL,Dispatch) {							\
	if(Dispatch)												\
	{															\
		NdisDprAcquireSpinLock(SpinL);							\
																\
	} else NdisAcquireSpinLock(SpinL);							\
																\
}

#define RelSpinLock(SpinL,Dispatch) {							\
	if(Dispatch)												\
	{															\
		NdisDprReleaseSpinLock(SpinL);							\
																\
	} else NdisReleaseSpinLock(SpinL);							\
																\
}

typedef enum
{
	Set,
	Query,
	Method

}RequestType;

typedef enum _PHYSICALMEDIUM
{
	PhysicalMediumUnspecified,
	PhysicalMediumWirelessLan,
	PhysicalMediumCableModem,
	PhysicalMediumPhoneLine,
	PhysicalMediumPowerLine,
	PhysicalMediumDSL,      // includes ADSL and UADSL (G.Lite)
	PhysicalMediumFibreChannel,
	PhysicalMedium1394,
	PhysicalMediumWirelessWan,
	PhysicalMediumNative802_11,
	PhysicalMediumBluetooth,
	PhysicalMediumInfiniband,
	PhysicalMediumWiMax,
	PhysicalMediumUWB,
	PhysicalMedium802_3,
	PhysicalMedium802_5,
	PhysicalMediumIrda,
	PhysicalMediumWiredWAN,
	PhysicalMediumWiredCoWan,
	PhysicalMediumOther,
	PhysicalMediumNative802_15_4,
	PhysicalMediumMax
} PHYSICALMEDIUM;

typedef enum _MEDIUM
{
	Medium802_3,
	Medium802_5,
	MediumFddi,
	MediumWan,
	MediumLocalTalk,
	MediumDix,              // defined for convenience, not a real medium
	MediumArcnetRaw,
	MediumArcnet878_2,
	MediumAtm,
	MediumWirelessWan,
	MediumIrda,
	MediumBpc,
	MediumCoWan,
	Medium1394,
	MediumInfiniBand,
#if ((NTDDI_VERSION >= NTDDI_VISTA) || NDIS_SUPPORT_NDIS6)
	MediumTunnel,
	MediumNative802_11,
	MediumLoopback,
#endif // (NTDDI_VERSION >= NTDDI_VISTA)

#if (NTDDI_VERSION >= NTDDI_WIN7)
	MediumWiMAX,
	MediumIP,
#endif

	MediumMax               // Not a real medium, defined as an upper-bound
}MEDIUM;

typedef enum _MEDIA_CONNECT_STATE
{
	ConnectStateUnknown,
	ConnectStateConnected,
	ConnectStateDisconnected
} MEDIA_CONNECT_STATE;

typedef enum _ADAPTER_STATE {
	NdisprotInitializing,
	NdisprotRunning,
	NdisprotPausing,
	NdisprotPaused,
	NdisprotRestarting,
	NdisprotClosing
} ADAPTER_STATE;

typedef struct Device_Lan
{
	char name[250];
	char service_name[250];
	int NetCardsCount;
	ULONG64 BindingContext;
	ULONG ModeCap;
	ULONG CurrentMode;
	int NetMonSupported;
	ULONG MtuSize;
	ULONG64 MaxXmitLinkSpeed;
	ULONG64 XmitLinkSpeed;
	ULONG64 MaxRcvLinkSpeed;
	ULONG64 RcvLinkSpeed;
	USHORT MacAddressLength;
	UCHAR CurrentMacAddress[32];
	PHYSICALMEDIUM PhysicalMediumType;
	MEDIUM MediumType;
	MEDIA_CONNECT_STATE MediaConnectState;
}Dev_Lan;



typedef struct _Ethernet_Header
{
	unsigned short DataSize;
	unsigned char NetworkData[5000];
	PHYSICALMEDIUM MediumType;
	MEDIUM Medium;
	char NetworkMiniportName[250];
	unsigned short MacAddressLength;
	unsigned char CurrentMacAddress[32];
}EHeader;

typedef struct _net_cards
{
	wchar_t keyPath[250];
	char Description[250];
	char ServiceName[250];
	ULONG ModeCap;
	ULONG CurrentMode;
	DOT11_CURRENT_OPERATION_MODE OperationMode;
	int NetMonSupported;
	NDIS_BIND_PARAMETERS BindParam;

// Items for driver purposes only		

	int  NetCardsCount;
	NDIS_HANDLE Adapter_Handle;
	ULONG64 BindingContext;
	NDIS_HANDLE Buffer;
	NDIS_HANDLE PBContext;
	NDIS_SPIN_LOCK SpinLockA;
	int licznik,indeks;
	int openIndex;
	BOOLEAN UnbindExecute;
	NDIS_STATUS UnbindStatus;
	BOOLEAN isOpened;
	BOOLEAN isSendComplete;
	ADAPTER_STATE AState;
	NDIS_DEVICE_POWER_STATE PowerState;
	ULONG RequestCompleteStatus;
	NDIS_EVENT CloseEvent;
}net_cards;


typedef struct Packet_Send
{
	Dev_Lan Adapter;
	unsigned char Packet[5000];
	int DataSize;

}PacketSend;


ULONG32 PACKET_COUNT;
ULONG32 ADAPTER_COUNT;

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD OnUnload;