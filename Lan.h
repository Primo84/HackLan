
#include "ndis.h"

NDIS_HANDLE Protocol_Handle; Adapter_Handle;
NDIS_STATUS status;

NDIS_PROTOCOL_CHARACTERISTICS ProtocolCharacteristics;
NDIS_STRING STRNDIS = NDIS_STRING_CONST("Hack_Lan");


NDIS_STATUS OpenCompleteAdapter(__in  NDIS_HANDLE BindingContext, __in NDIS_STATUS  Status, __in NDIS_STATUS ErrorStatus);
VOID CloseAdapterHandle(__in  NDIS_HANDLE ProtocolBindingContext, __in NDIS_STATUS Status);
VOID SendCompleteHandle(__in  NDIS_HANDLE ProtocolBindingContext, __in PNDIS_PACKET Packet, __in NDIS_STATUS Status);
VOID TransferData(__in  NDIS_HANDLE ProtocolBindingContext, __in PNDIS_PACKET Packet, __in NDIS_STATUS Status, __in UINT BytesTransferred);

int RegisterProtocol()
{
	RtlZeroMemory(&ProtocolCharacteristics, sizeof(NDIS_PROTOCOL_CHARACTERISTICS));
	ProtocolCharacteristics.MinorNdisVersion = 0;
	ProtocolCharacteristics.MajorNdisVersion = 0x04;
	ProtocolCharacteristics.Name = STRNDIS;
	ProtocolCharacteristics.OpenAdapterCompleteHandler = OpenCompleteAdapter;
	ProtocolCharacteristics.CloseAdapterCompleteHandler = CloseAdapterHandle;
	ProtocolCharacteristics.SendCompleteHandler = SendCompleteHandle;
	ProtocolCharacteristics.TransferDataCompleteHandler=TransferData;

	return 0;
}




NDIS_STATUS OpenCompleteAdapter(__in  NDIS_HANDLE BindingContext, __in  NDIS_STATUS  Status, __in  NDIS_STATUS ErrorStatus)
{
	return NDIS_STATUS_SUCCESS;
}

VOID CloseAdapterHandle(__in  NDIS_HANDLE ProtocolBindingContext, __in  NDIS_STATUS Status)
{
	if (Status == NDIS_STATUS_SUCCESS)
		NdisCompleteUnbindAdapter(ProtocolBindingContext, NDIS_STATUS_SUCCESS);

}

VOID SendCompleteHandle(__in  NDIS_HANDLE ProtocolBindingContext, __in  PNDIS_PACKET Packet, __in  NDIS_STATUS Status)
{

}

VOID TransferData(__in  NDIS_HANDLE ProtocolBindingContext, __in PNDIS_PACKET Packet, __in NDIS_STATUS Status, __in UINT BytesTransferred)
{
	
}