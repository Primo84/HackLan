#pragma once

#include"FileDef.h"


NDIS_FILTER_DRIVER_CHARACTERISTICS FilterDriverChar;
NDIS_HANDLE FilterHandle;
BOOLEAN FilterRegistered;
NDIS_EVENT RequestQueryEvent, RequestSetEvent, RequestMethodEvent, ModeSetEvent, SendCompleteEvent;
NDIS_STATUS ReqQueryStatus, ReqSetStatus, ReqMethodStatus;

NDIS_HANDLE PoolParamHandle;


int miniportsCount;
LIST_ENTRY MiniportsEntries;

int RegisterFilterDriver(PDRIVER_OBJECT Driver);


FILTER_SET_OPTIONS FilterSetOptions;
FILTER_SET_MODULE_OPTIONS SetFilterModuleOptions;
FILTER_ATTACH FilterAttach;
FILTER_DETACH  FilterDetach;
FILTER_RESTART FilterRestart;
FILTER_PAUSE FilterPause;
FILTER_SEND_NET_BUFFER_LISTS FilterSendNetBufferLists;
FILTER_SEND_NET_BUFFER_LISTS_COMPLETE FilterSendNetBufferListsComplete;
FILTER_CANCEL_SEND_NET_BUFFER_LISTS FilterCancelSendNetBufferLists;
FILTER_RECEIVE_NET_BUFFER_LISTS FilterReceiveNetBufferLists;
FILTER_RETURN_NET_BUFFER_LISTS FilterReturnNetBufferLists;
FILTER_OID_REQUEST FilterOidRequest;
FILTER_OID_REQUEST_COMPLETE FilterOidRequestComplete;
FILTER_CANCEL_OID_REQUEST FilterCancelOidRequest;
FILTER_DEVICE_PNP_EVENT_NOTIFY FilterDevicePnpEventNotify;
FILTER_NET_PNP_EVENT FilterNetPnpEvent;
FILTER_STATUS FilterStatus;
FILTER_DIRECT_OID_REQUEST FilterDirectOidRequest;
FILTER_DIRECT_OID_REQUEST_COMPLETE FilterDirectOidRequestComplete;
FILTER_CANCEL_DIRECT_OID_REQUEST FilterCancelDirectOidRequest;


NDIS_STATUS RequestMPModeCap(filterModuleHandle *fmh, DOT11_OPERATION_MODE_CAPABILITY *OP_Mode);

NDIS_STATUS RequestMPModeSet(filterModuleHandle* fmh, ULONG CurrentMode);

NDIS_STATUS RequestPacketFilterSet(filterModuleHandle* fmh, ULONG FilterMask);

NDIS_STATUS RequestResetMP(filterModuleHandle* fmh, DOT11_RESET_REQUEST ResetReq);

NDIS_STATUS RequestDisconnectMP(filterModuleHandle* fmh, ULONG ConectionOp);

NDIS_STATUS RequestDisasociateMP(filterModuleHandle* fmh);

NDIS_STATUS RequestInit(NDIS_HANDLE FilterBindingContext, PVOID Buffer, ULONG BufferSize, NDIS_OID OID, RequestType Type);