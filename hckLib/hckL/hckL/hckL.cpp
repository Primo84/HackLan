
#include "hckL.h"

DWORD handle,RecvMP_H,SendMP_H;
HANDLE Device;

wchar_t pdevice_name[25] = L"\\\\.\\PHacklan";
wchar_t fdevice_name[25] = L"\\\\.\\FHacklan";
wchar_t fdevice_name1[25] = L"\\\\.\\FMHacklan";
char EventName[] = "Global\\Hkcl_Event";
char MPEventName[] = "Global\\Hkcl_MPEvent";


int closed = 0;
int MPclosedR = 0;
int MPclosedS = 0;
Dev_Lan AdapterOpened;
//Userminiport* MiniP=NULL;
int opened = 0;
int MiniportInited = 0;
int MiniportRecHandled = 0;
int MiniportSendHandled = 0;
HANDLE Event_h = NULL;
HANDLE MPEvent_h = NULL;
OVERLAPPED OverL;
RecvPack Packet;
RecvPack MP_RPacket;
RecvPack MP_SPacket;
MP_SR MPSR;

DWORD WINAPI Fun(PVOID p)
{
	HANDLE dev = NULL;
	BOOL bl = FALSE;
	DWORD ret;
	int i;
	RecivePacketHandler* RecvH;
	PacketInfo PInfo;
	BOOL isBuffered = TRUE;


	memset(&Packet, 0, sizeof(RecvPack));
	PInfo.BindingContext = AdapterOpened.BindingContext;
	PInfo.licznik = 0;
	RecvH = (RecivePacketHandler*)p;

	do
	{
		Sleep(1000);
		memset(&Packet, 0, sizeof(RecvPack));
		Packet.odebrane = 0;
		
		if (Event_h == NULL)
			return 1;

		WaitForSingleObject(Event_h, INFINITE);
		//ResetEvent(Event_h);
		memset(&OverL, 0, sizeof(OVERLAPPED));
		OverL.Offset = PInfo.licznik;
		OverL.OffsetHigh= (DWORD)AdapterOpened.BindingContext;
		//OverL.hEvent = Event_h;
		ret = 0;
		dev = CreateFileW(pdevice_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED , NULL);

		if (dev == INVALID_HANDLE_VALUE)
		{
			SetEvent(Event_h);
			CloseHandle(Event_h);
			Event_h = NULL;
			return 1;
		}

	//	bl = DeviceIoControl(dev, IO_SET_RECIVE_HANDLER, &PInfo, sizeof(PacketInfo), &Packet, sizeof(RecvPack), &ret, NULL);
		
		bl = ReadFile(dev, &Packet, sizeof(RecvPack),&ret, &OverL);

		CloseHandle(dev);
		dev = NULL;

		SetEvent(Event_h);

		if (bl && Packet.odebrane>0)
		{
			if (PInfo.licznik >= PACKET_COUNT) PInfo.licznik = 0;

			PInfo.licznik = PInfo.licznik + Packet.odebrane;

			for (i = 0; i < Packet.odebrane; i++)
			{
				(*RecvH)(Packet.EHead[i], isBuffered);
			}
		}

		isBuffered == TRUE ? isBuffered = FALSE : 0;

	} while (closed == 0);

	if(dev!=NULL)
		CloseHandle(dev);
	if (Event_h != NULL)
	{
		SetEvent(Event_h);
		CloseHandle(Event_h);
		Event_h = NULL;
	}
	
	ExitThread(0);
	return 0;
}

DWORD WINAPI MPFunR(PVOID p)
{
	HANDLE dev = NULL;
	BOOL bl = FALSE;
	DWORD ret;
	int i;
	int MPCount;
	RecivePacketHandler* RecvH;
	UserMiniport* UMP, *MiniP;
	WORD* w;
	WORD Operacja;
	BOOL isBuffered = TRUE;

	MPCount = MPSR.MiniP->miniportCount;
	RecvH = MPSR.Recv;

	Operacja = 0; // MPSR.Operacja;

	MiniP = (UserMiniport*)malloc(sizeof(UserMiniport) * MPCount);
	memcpy(MiniP, MPSR.MiniP, sizeof(UserMiniport) * MPCount);

	MPclosedR = 0;
	MiniportRecHandled = 1;

	do
	{
		Sleep(1000);

		memset(&MP_RPacket, 0, sizeof(RecvPack));

		if (MPEvent_h == NULL)
		{
			free(MiniP);
			MiniportRecHandled = 0;
			return 1;
		}

		UMP = MiniP;
		for (i = 0; i < MPCount; i++)
		{
			if (UMP != NULL)
			{
				if (UMP->RecvHooked == 1)
				{
					memset(&MP_RPacket, 0, sizeof(RecvPack));
					MP_RPacket.odebrane = 0;

					WaitForSingleObject(MPEvent_h, INFINITE);

					memset(&OverL, 0, sizeof(OVERLAPPED));
					OverL.Offset = UMP->licznik;
					w = (WORD*)&OverL.OffsetHigh;
					*w = (WORD)UMP->Index;
					w++;
					*w = (WORD)Operacja;

					ret = 0;
					dev = CreateFileW(fdevice_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

					if (dev == INVALID_HANDLE_VALUE)
					{
						SetEvent(MPEvent_h);
						if (MiniportSendHandled == 0)
						{
							CloseHandle(MPEvent_h);
							MPEvent_h = NULL;
						}
						free(MiniP);
						MiniportRecHandled = 0;
						return 1;
					}

					bl = ReadFile(dev, &MP_RPacket, sizeof(RecvPack), &ret, &OverL);

					CloseHandle(dev);
					dev = NULL;

					SetEvent(MPEvent_h);

					if (bl && MP_RPacket.odebrane > 0)
					{
						if (UMP->licznik >= PACKET_COUNT) UMP->licznik = 0;

						UMP->licznik = UMP->licznik + MP_RPacket.odebrane;

						for (i = 0; i < MP_RPacket.odebrane; i++)
						{
							(*RecvH)(MP_RPacket.EHead[i], isBuffered);
						}
					}
				}
				if (i < MPCount - 1) UMP++;
			}
		}

		isBuffered == TRUE ? isBuffered = FALSE : 0;

	} while (MPclosedR == 0);

	if (dev != NULL)
		CloseHandle(dev);

	if (MPEvent_h != NULL)
	{
		SetEvent(MPEvent_h);
		if (MiniportSendHandled == 0)
		{
			CloseHandle(MPEvent_h);
			MPEvent_h = NULL;
		}
	}
	free(MiniP);
	MiniportRecHandled = 0;
	ExitThread(0);
	return 0;
}


DWORD WINAPI MPFunS(PVOID p)
{
	HANDLE dev = NULL;
	BOOL bl = FALSE;
	DWORD ret;
	int i;
	int MPCount;
	RecivePacketHandler* RecvH;
	UserMiniport* UMP, * MiniP;
	WORD* w;
	WORD Operacja;
	BOOL isBuffered = TRUE;

	MPCount = MPSR.MiniP->miniportCount;
	RecvH = MPSR.Recv;
	Operacja = 1; // MPSR.Operacja;

	MiniP = (UserMiniport*)malloc(sizeof(UserMiniport) * MPCount);
	memcpy(MiniP, MPSR.MiniP, sizeof(UserMiniport) * MPCount);

	MPclosedS = 0;
	MiniportSendHandled = 1;

	do
	{
		Sleep(1000);

		memset(&MP_SPacket, 0, sizeof(RecvPack));

		if (MPEvent_h == NULL)
		{
			free(MiniP);
			MiniportSendHandled = 0;
			return 1;
		}

		UMP = MiniP;
		for (i = 0; i < MPCount; i++)
		{
			if (UMP != NULL)
			{
				if (UMP->SendHooked == 1)
				{
					memset(&MP_SPacket, 0, sizeof(RecvPack));
					MP_SPacket.odebrane = 0;

					WaitForSingleObject(MPEvent_h, INFINITE);

					memset(&OverL, 0, sizeof(OVERLAPPED));
					OverL.Offset = UMP->licznik;
					w = (WORD*)&OverL.OffsetHigh;
					*w = (WORD)UMP->Index;
					w++;
					*w = (WORD)Operacja;

					ret = 0;
					dev = CreateFileW(fdevice_name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

					if (dev == INVALID_HANDLE_VALUE)
					{
						SetEvent(MPEvent_h);
						if (MiniportRecHandled == 0)
						{
							CloseHandle(MPEvent_h);
							MPEvent_h = NULL;
						}
						free(MiniP);
						MiniportSendHandled = 0;
						return 1;
					}

					bl = ReadFile(dev, &MP_SPacket, sizeof(RecvPack), &ret, &OverL);

					CloseHandle(dev);
					dev = NULL;

					SetEvent(MPEvent_h);

					if (bl && MP_SPacket.odebrane > 0)
					{
						if (UMP->licznik >= PACKET_COUNT) UMP->licznik = 0;

						UMP->licznik = UMP->licznik + MP_SPacket.odebrane;

						for (i = 0; i < MP_SPacket.odebrane; i++)
						{
							(*RecvH)(MP_SPacket.EHead[i], isBuffered);
						}
					}
				}
				if (i < MPCount - 1) UMP++;
			}
		}

		isBuffered == TRUE ? isBuffered = FALSE : 0;

	} while (MPclosedS == 0);

	if (dev != NULL)
		CloseHandle(dev);

	if (MPEvent_h != NULL)
	{
		SetEvent(MPEvent_h);
		if (MiniportRecHandled == 0)
		{
			CloseHandle(MPEvent_h);
			MPEvent_h = NULL;
		}
	}
	free(MiniP);
	MiniportSendHandled = 0;
	ExitThread(0);
	return 0;
}


__declspec(dllexport) int GetDevices(Dev_Lan * Dev, int size, int* size_req)
{
	HANDLE dev = NULL;
	Dev_Lan* D = NULL;
	DWORD ret;
	BOOL bl = FALSE;
	char* buf;
	int rozm, code;

	D = Dev;
	if (D == NULL || size_req == NULL) return -1;
	if (size < sizeof(Dev_Lan)) return 1;

	dev = CreateFileW(pdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (dev == INVALID_HANDLE_VALUE) return 2;
	buf = new char[size];
	bl = DeviceIoControl(dev, IO_DEV_INIT, NULL, 0, buf, size, &ret, NULL);
	CloseHandle(dev);
	if (bl == FALSE) return 4;
	if (_strnicmp(buf, (char*)"BBBsB", 5) == 0)
	{
		return 3;
	}
	if (_strnicmp(buf, (char*)"AAAsA", 5) == 0)
	{
		rozm = atoi(&buf[5]);
		*size_req = rozm;
		code = 1;
	}
	else
	{
		memcpy((char*)D, buf, size);
		code = 0;
	}
	delete(buf);
	return code;


}

__declspec(dllexport) int OpenAdapter(Dev_Lan Dev)
{
	HANDLE dev = NULL;
	Dev_Lan* D = NULL;
	DWORD ret;
	BOOL bl = FALSE;
	char buf[250];

	if (opened == 1) return 4;
	dev = CreateFileW(pdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (dev == INVALID_HANDLE_VALUE) return 1;

	bl = DeviceIoControl(dev, IO_OPEN_ADAPTER, &Dev, sizeof(Dev_Lan), buf, 250, &ret, NULL);
	CloseHandle(dev);
	if (bl == FALSE) return 2;
	if (_strnicmp(buf, (char*)"AAAsA", 5) == 0)
	{
		return 3;
	}
	if (_strnicmp(buf, (char*)"Adapter O", 5) == 0)
	{
		AdapterOpened = Dev;
		memcpy(&AdapterOpened, &Dev, sizeof(Dev_Lan));
		opened = 1;
		return 0;
	}
	return 5;
}


__declspec(dllexport) int CloseAdapter()
{
	HANDLE dev = NULL;

	DWORD ret;
	BOOL bl = FALSE;

	closed = 1;
	Sleep(1000);
	if (opened == 1)
	{

		dev = CreateFileW(pdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (dev == INVALID_HANDLE_VALUE) return 1;

		bl = DeviceIoControl(dev, IO_CLOSE_ADAPTER, &AdapterOpened, sizeof(Dev_Lan), NULL, 0, &ret, NULL);
		CloseHandle(dev);
		opened = 0;
	}
	return 0;
}
__declspec(dllexport) int CloseReciveMPHandler()
{
	if (MiniportRecHandled != 0)
	{
		MPclosedR = 1;
		return 0;
	}

	return 1;
}

__declspec(dllexport) int CloseSendMPHandler()
{
	if (MiniportSendHandled != 0)
	{
		MPclosedS = 1;
		return 0;
	}

	return 1;
}

__declspec(dllexport) HANDLE SetRecive(RecivePacketHandler* RecvHandler, DWORD* ThreadId)
{
	HANDLE hn;

	hn = 0;
	if (opened == 1)
	{
		Event_h = NULL;
		Event_h = OpenEvent(SYNCHRONIZE | EVENT_ALL_ACCESS, FALSE, EventName);
		if (Event_h == NULL)
		{
			Event_h = CreateEventA(NULL, FALSE, TRUE, EventName);
		}
	//	memset(&Packet, 0, sizeof(RecvPack));
		closed = 0;
		hn=CreateThread(NULL, 0, &Fun, (PVOID)RecvHandler, THREAD_PRIORITY_NORMAL, &handle);

		if (ThreadId != NULL)
			*ThreadId = handle;
		Sleep(500);
	}
	return hn;
}
__declspec(dllexport) HANDLE SetReciveMPHandler(RecivePacketHandler* RecvHandler, Userminiport * UserMiniP, DWORD* ThreadId)
{
	HANDLE hn;

	hn = 0;

	if (MiniportInited == 1 && MiniportRecHandled==0)
	{
		if (MPEvent_h == NULL)
		{
			MPEvent_h = OpenEvent(SYNCHRONIZE | EVENT_ALL_ACCESS, FALSE, MPEventName);
			if (MPEvent_h == NULL)
			{
				MPEvent_h = CreateEventA(NULL, FALSE, TRUE, MPEventName);
			}
		}

		MPclosedR = 0;
		MPSR.MiniP = UserMiniP;
	//	MPSR.Operacja = 0;
		MPSR.Recv = RecvHandler;

		hn = CreateThread(NULL, 0, &MPFunR, NULL, THREAD_PRIORITY_NORMAL, &RecvMP_H);
		Sleep(5000);
	}

	if (ThreadId != NULL)
		*ThreadId = RecvMP_H;

	return hn;
}

__declspec(dllexport) HANDLE SetSendMPHandler(RecivePacketHandler* RecvHandler, Userminiport* UserMiniP, DWORD* ThreadId)
{
	HANDLE hn;

	hn = 0;

	if (MiniportInited == 1 && MiniportSendHandled == 0)
	{
		if (MPEvent_h == NULL)
		{
			MPEvent_h = OpenEvent(SYNCHRONIZE | EVENT_ALL_ACCESS, FALSE, MPEventName);
			if (MPEvent_h == NULL)
			{
				MPEvent_h = CreateEventA(NULL, FALSE, TRUE, MPEventName);
			}
		}

		MPclosedS = 0;
		MPSR.MiniP = UserMiniP;
		//	MPSR.Operacja = 0;
		MPSR.Recv = RecvHandler;

		hn = CreateThread(NULL, 0, &MPFunS, NULL, THREAD_PRIORITY_NORMAL, &SendMP_H);
		Sleep(5000);
	}

	if (ThreadId != NULL)
		*ThreadId = SendMP_H;

	return hn;
}

__declspec(dllexport) int SendFramePacket(Dev_Lan* Adapter, unsigned char* Packet, int DataSize)
{
	HANDLE dev = NULL;
	int RetVal;
	DWORD ret;
	PacketSend PS;
	BOOL bl;
	OVERLAPPED Over;

	if (Adapter == NULL)
		return -1;

	if (DataSize==0 || DataSize > 5000 || Packet == NULL) return 2;

	dev = CreateFileW(pdevice_name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (dev == INVALID_HANDLE_VALUE) return 1;

	memset(&PS, 0, sizeof(PacketSend));

	memset(&Over, 0, sizeof(OVERLAPPED));

	memcpy(&PS.Adapter, Adapter, sizeof(Dev_Lan));

	memcpy(PS.Packet, Packet, DataSize);

	PS.DataSize = DataSize;

	bl=WriteFile(dev, (LPCVOID)&PS, sizeof(PacketSend), &ret, &Over);

	CloseHandle(dev);

	if (!bl) return 1;

	return 0;
}

__declspec(dllexport) int GetBSSIDlist(WLAN_BSS_ENTRY* BSS, int* ItemsCount)
{
	DWORD Version, Status;
	HANDLE ClientHandle;
	PWLAN_INTERFACE_INFO_LIST List_Interface;
	PWLAN_BSS_LIST BSSList;
	int i, j;
	wchar_t name[150];

	if (opened != 1) return -1;
	memset(name, 0, sizeof(name));
	MultiByteToWideChar(CP_ACP, 0, AdapterOpened.name, 150, name, 150);
	Status = WlanOpenHandle(WLAN_API_VERSION, NULL, &Version, &ClientHandle);
	if (Status == ERROR_SUCCESS)
	{
		Status = WlanEnumInterfaces(ClientHandle, NULL, &List_Interface);
		if (Status == ERROR_SUCCESS)
		{
			for (i = 0; i < (int)List_Interface->dwNumberOfItems; i++)
			{
				if (_wcsicmp(name, List_Interface->InterfaceInfo[i].strInterfaceDescription) == 0)
				{
					Status = WlanGetNetworkBssList(ClientHandle, &List_Interface->InterfaceInfo[i].InterfaceGuid, NULL, dot11_BSS_type_any, FALSE, NULL, &BSSList);
					if (Status == ERROR_SUCCESS)
					{
						if (*ItemsCount >= (int)BSSList->dwNumberOfItems)
						{
							for (j = 0; j < (int)BSSList->dwNumberOfItems; j++)
							{
								memcpy(&BSS[j], &BSSList->wlanBssEntries[j], sizeof(WLAN_BSS_ENTRY));
							}
							i = 0;
						}
						else
						{
							i = 1;
							*ItemsCount = BSSList->dwNumberOfItems;
						}
						WlanFreeMemory(BSSList);
						WlanFreeMemory(List_Interface);
						WlanCloseHandle(ClientHandle, NULL);
						return i;
					}
				}
			}
			WlanFreeMemory(List_Interface);
		}
		WlanCloseHandle(ClientHandle, NULL);
	}
	return -2;
}

__declspec(dllexport) int GetConnected(int* ConnectedCount, Dev_Lan *DevLan)
{
	HANDLE dev = NULL;
	int CCount;
	bool bl;
	DWORD ret;

	if (ConnectedCount == NULL || DevLan == NULL) 
		return -1;

	dev = CreateFileW(pdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (dev == INVALID_HANDLE_VALUE) return 1;
	
	bl = DeviceIoControl(dev, IO_QUERY_CONECTED, (LPVOID)DevLan, sizeof(Dev_Lan), &CCount, sizeof(int), &ret, NULL);
	CloseHandle(dev);

	if (!bl) return 2;
	*ConnectedCount = CCount;
	return 0;
}


__declspec(dllexport) int GetAdapterParams(Dev_Lan* DevLan)
{
	HANDLE dev = NULL;
	int CCount;
	bool bl;
	DWORD ret;

	if (DevLan == NULL)
		return -1;

	dev = CreateFileW(pdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (dev == INVALID_HANDLE_VALUE) return 1;

	bl = DeviceIoControl(dev, IO_QUERY_STATE, (LPVOID)DevLan, sizeof(Dev_Lan), DevLan, sizeof(Dev_Lan), &ret, NULL);
	CloseHandle(dev);

	if (!bl) return 2;

	return 0;
}

__declspec(dllexport) int GetMiniportParams(Userminiport* UMP)
{
	HANDLE dev = NULL;
	int CCount;
	bool bl;
	DWORD ret;

	if (UMP == NULL)
		return -1;

	dev = CreateFileW(fdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (dev == INVALID_HANDLE_VALUE) return 1;

	bl = DeviceIoControl(dev, IO_QUERY_STATE, (LPVOID)UMP, sizeof(Userminiport), UMP, sizeof(Userminiport), &ret, NULL);
	CloseHandle(dev);

	if (!bl) return 2;

	return 0;
}

__declspec(dllexport) int SetOpMode(Dev_Lan* DevLan, int OP_Mode)
{
	HANDLE dev = NULL;
	int RetVal;
	bool bl;
	DWORD ret;
	Dev_Lan DL;


	if (DevLan == NULL)
		return -1;

	dev = CreateFileW(fdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (dev == INVALID_HANDLE_VALUE) return 1;

	memcpy(&DL, DevLan, sizeof(Dev_Lan));
	DL.CurrentMode = (ULONG)OP_Mode;

	bl = DeviceIoControl(dev, IO_SET_OP_MODE, (LPVOID)&DL, sizeof(Dev_Lan), &RetVal, sizeof(int), &ret, NULL);
	CloseHandle(dev);

	if (!bl) return 2;

	if (RetVal == 0)
		return 3;

	else if (RetVal == 2)
		return 4;

	else if (RetVal==1)
	{
		dev = CreateFileW(pdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
		if (dev == INVALID_HANDLE_VALUE) return 1;


		bl = DeviceIoControl(dev, IO_SET_OP_MODE, (LPVOID)&DL, sizeof(Dev_Lan), &RetVal, sizeof(int), &ret, NULL);

		CloseHandle(dev);

		return 0;
	}
	else
		return 2;
}


__declspec(dllexport) int Init_Miniports(Userminiport* MP, int size, int* reqSize)
{

	HANDLE dev = NULL;
	Userminiport* mp = NULL;
	DWORD ret;
	BOOL bl = FALSE;
	char* buf;
	int rozm, code;

	mp = MP;
	if (mp == NULL || reqSize == NULL) return -1;
	if (size < sizeof(Userminiport)) return 1;

	code = 0;

	dev = CreateFileW(fdevice_name, 0, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (dev == INVALID_HANDLE_VALUE) return 2;
	buf = new char[size];

	bl = DeviceIoControl(dev, IO_MINIPORTS_INIT, NULL, 0, buf, size, &ret, NULL);
	CloseHandle(dev);
	
	if (bl == FALSE) return 4;
	if (_strnicmp(buf, (char*)"BBBsB", 5) == 0)
	{
		return 3;
	}
	if (_strnicmp(buf, (char*)"AAAsA", 5) == 0)
	{
		rozm = atoi(&buf[5]);
		*reqSize = rozm;
		code = 1;
	}
	else
	{
		memcpy((char*)mp, buf, size);
		code = 0;
		MiniportInited = 1;
	}
	delete(buf);
	return code;
}


__declspec(dllexport) int InstallFilterDriver(LPSTR HacklanSys_path, DWORD startType, HWND WindowH)
{
	SC_HANDLE manager;
	SC_HANDLE service;
	HINF infFileH;
	char Service_Name[] = "FHacklan.sys";
	char OrderGroup[] = "NDIS";
	char FileInf_Name[150] = "hlanfilter.inf\0";
	char HLInstaller_Name[] = "HLFInstaller.sys";
	char HLINstallerService[] = "HLFInstaller";

	char DestPath[] = "C:\\Windows\\System32\\drivers\\FHacklan.sys";
	char DestInfFile[150];
	WCHAR ComponentId[] = L"MS_HLAN_FILTER\0";
	char srvName[150];
	char InfFile[150];
	char HLIname[150];
	char filepath[150];
	char path[150];
	LPCSTR file;
	int ret;
	UINT rcode;
	BOOL bl;
	PVOID Context = NULL;
	INetCfg *pnetcfg = NULL;
	INetCfgClassSetup *pnetcfgclass;
	INetCfgComponent *pnetcfgcomponent;
	INetCfgLock* pnetcfglock;
	OBO_TOKEN ObToken;
	HRESULT result;
	LPWSTR ClientDesc;
	DWORD reqsize;
	SERVICE_STATUS servst;

	ret = 0;
	reqsize = 0;

	memset(srvName, 0, 150);
	memset(InfFile, 0, 150);
	memset(HLIname, 0, 150);
	memset(DestInfFile, 0, 150);

	
	if (HacklanSys_path != NULL)
	{
		sprintf_s(srvName, "%s\\%s", HacklanSys_path, Service_Name);
		sprintf_s(InfFile, "%s\\%s", HacklanSys_path, FileInf_Name);
		sprintf_s(HLIname, "%s\\%s", HacklanSys_path, HLInstaller_Name);
	}
	else
	{
		memset(filepath, 0, 150);
		GetModuleFileNameA(NULL, filepath, 150);
		ExtractFilePath(filepath, path, 150);
		sprintf_s(srvName, "%s\\%s", path, Service_Name);
		sprintf_s(InfFile, "%s\\%s", path, FileInf_Name);
		sprintf_s(HLIname, "%s\\%s", path, HLInstaller_Name);
	}

	manager = NULL;
	service = NULL;
	manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (manager != NULL)
	{
		service = OpenService(manager, "FHacklan", SERVICE_ALL_ACCESS);

		if (service != NULL)
		{
			ret = 1;

			QueryServiceStatus(service, &servst);

			if (servst.dwCurrentState == SERVICE_STOPPED)
			{
				if (StartService(service, 0, NULL) == 0)
					ret = -2;
			}

			CloseServiceHandle(service);

			return ret;
		}
		else
		{

			infFileH = NULL;

			infFileH = SetupOpenInfFile(InfFile, NULL, INF_STYLE_WIN4, &rcode);

			if (infFileH != INVALID_HANDLE_VALUE)
			{
				if (!SetupCopyOEMInfA(InfFile, HacklanSys_path, SPOST_PATH, 0, DestInfFile, 150, &reqsize, NULL))
					rcode = 1;

				if(rcode==0)
					rcode = CopyFileSystem(HLIname, HLINstallerService);
				//rcode = SetupInstallFileEx(infFileH, NULL, Service_Name, path, DestPath, SP_COPY_NEWER_OR_SAME, NULL, Context, &bl);
				//rcode = 1;
				if (rcode != 0)
				{
					ret = 4;
					rcode = GetLastError();
				}
				else
				{
					SetupCloseInfFile(infFileH);

					if (CoInitialize(NULL) == S_OK)
					{
						result = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (LPVOID*)&pnetcfg);
						if (SUCCEEDED(result))
						{
							result = pnetcfg->QueryInterface(IID_INetCfgLock, (void**)&pnetcfglock);
							if (SUCCEEDED(result))
							{
								result = pnetcfglock->AcquireWriteLock(5000, L"HacklanFilter", &ClientDesc);
								if (result == S_OK)
								{
									result = pnetcfg->Initialize(NULL);
									if (SUCCEEDED(result))
									{

										result = pnetcfg->QueryNetCfgClass(&GUID_DEVCLASS_NETSERVICE, IID_INetCfgClassSetup, (void**)&pnetcfgclass);
										if (SUCCEEDED(result))
										{
											memset(&ObToken, 0, sizeof(OBO_TOKEN));
											ObToken.Type = OBO_USER;
											//ObToken.pncc = NULL;
											//ObToken.pszwDisplayName = NULL;
											//ObToken.pszwManufacturer = NULL;
											//ObToken.pszwProduct = NULL;
											//result = pnetcfgclass->SelectAndInstall(WindowH, &ObToken, &pnetcfgcomponent);

											
											result = pnetcfgclass->Install(ComponentId, &ObToken, 0, 0, NULL, NULL, &pnetcfgcomponent);

											if (result == NETCFG_S_REBOOT || result ==  NETCFG_E_NEED_REBOOT || result == S_OK)
											{
												result = pnetcfg->Apply();
												if (pnetcfgcomponent != NULL)
												pnetcfgcomponent->Release();
											}
											else ret = 5;

											pnetcfgclass->Release();
										}
										else ret = 5;
										pnetcfg->Uninitialize();
									}
									else ret = 5;
								}
								else ret = 5;
								pnetcfglock->Release();
							}
							else ret = 5;
							pnetcfg->Release();

						}
						else ret = 5;

						CoUninitialize();
					}
					else ret = 5;
				}
			}
			else
			{
				ret = 3;
				rcode = GetLastError();
			}
			if (ret == 0)
			{
				service = CreateService(manager, "FHacklan", "FHacklan", SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, startType, SERVICE_ERROR_NORMAL, DestPath, OrderGroup, NULL, NULL, NULL, NULL);
				if (service != NULL)
				{
					if (StartService(service, 0, NULL) == 0)
						ret = -2;
					CloseServiceHandle(service);
				}
			}
		}

		CloseServiceHandle(manager);
	}
	else ret = 2;

	return ret;
}

__declspec(dllexport) int StopAndUinstallFilterDriver()
{
	SC_HANDLE service,manager;
	char Name[] = "FHacklan";
	WCHAR ComponentId[] = L"MS_HLAN_FILTER";
	LPWSTR ClientDesc;
	int ret;
	SERVICE_STATUS status;
	INetCfg* pnetcfg = NULL;
	INetCfgClassSetup* pnetcfgclass;
	INetCfgComponent* pnetcfgcomponent;
	INetCfgLock* pnetcfglock;
	OBO_TOKEN ObToken;
	HRESULT result;

	ret = -2;
	manager = NULL;
	service = NULL;

	manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
	if (manager != NULL)
	{
		service = OpenService(manager, Name, SERVICE_ALL_ACCESS);
		if (service != NULL)
		{

			if (ControlService(service, SERVICE_CONTROL_STOP, &status))
			{
				while (QueryServiceStatus(service, &status))
				{
					if (status.dwCurrentState == SERVICE_STOP_PENDING)
					{
						break;
					}
					else break;
				}
				if (status.dwCurrentState == SERVICE_STOPPED)
				{

				}

			}
			Sleep(1000);
			if (DeleteService(service))
				ret = 0;
			CloseServiceHandle(service);
		}
		else
		{
			ret = 1;
		}

		CloseServiceHandle(manager);

		if (CoInitialize(NULL) == S_OK)
		{
			result = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (LPVOID*)&pnetcfg);
			if (SUCCEEDED(result))
			{
				result = pnetcfg->QueryInterface(IID_INetCfgLock, (void**)&pnetcfglock);
				if (SUCCEEDED(result))
				{
					result = pnetcfglock->AcquireWriteLock(5000, L"HaclanFilter", &ClientDesc);
					if (result == S_OK)
					{
						result = pnetcfg->Initialize(NULL);
						if (SUCCEEDED(result))
						{

							result = pnetcfg->QueryNetCfgClass(&GUID_DEVCLASS_NETSERVICE, IID_INetCfgClassSetup, (void**)&pnetcfgclass);
							if (SUCCEEDED(result))
							{
								memset(&ObToken, 0, sizeof(OBO_TOKEN));
								ObToken.Type = OBO_USER;
								//ObToken.pncc = NULL;
								//ObToken.pszwDisplayName = NULL;
								//ObToken.pszwManufacturer = NULL;
								//ObToken.pszwProduct = NULL;
								//result = pnetcfgclass->SelectAndInstall(WindowH, &ObToken, &pnetcfgcomponent);

								result = pnetcfg->FindComponent(ComponentId, &pnetcfgcomponent);
								if (result == S_OK)
								{
									result = pnetcfgclass->DeInstall(pnetcfgcomponent, &ObToken, NULL);
									result = pnetcfg->Apply();
									ret = 0;
									if (pnetcfgcomponent != NULL)
										pnetcfgcomponent->Release();
								}
								else ret = 5;

								pnetcfgclass->Release();
							}
							else ret = 5;
							pnetcfg->Uninitialize();
						}
						else ret = 5;
					}
					else ret = 5;
					pnetcfglock->Release();
				}
				else ret = 5;
				pnetcfg->Release();

			}
			else ret = 5;

			CoUninitialize();
		}
		else ret = 5;
	}
	else ret = -1;
return ret;
}

__declspec(dllexport) int InstallProtocolDriver(LPSTR HacklanSys_path, DWORD startType, HWND WindowH)
{
	SC_HANDLE manager;
	SC_HANDLE service;
	HINF infFileH;
	char Service_Name[] = "PHacklan.sys";
	char OrderGroup[] = "NDIS";
	char FileInf_Name[150] = "hlanprot.inf\0";
	char HLInstaller_Name[] = "HLPInstaller.sys";
	char HLINstallerService[] = "HLPInstaller";
	char DestPath[] = "C:\\Windows\\System32\\drivers\\PHacklan.sys";
	char DestInfFile[150];
	WCHAR ComponentId[] = L"MS_HLAN_PROT";
	char srvName[150];
	char InfFile[150];
	char HLIname[150];
	char filepath[150];
	char path[150];
	LPCSTR file;
	int ret;
	UINT rcode;
	BOOL bl;
	PVOID Context = NULL;
	INetCfg* pnetcfg = NULL;
	INetCfgClassSetup* pnetcfgclass;
	INetCfgComponent* pnetcfgcomponent;
	INetCfgLock* pnetcfglock;
	OBO_TOKEN ObToken;
	HRESULT result;
	LPWSTR ClientDesc;
	DWORD reqsize;
	SERVICE_STATUS servst;
	LPWSTR Cid;
	ret = 0;
	reqsize = 0;

	memset(srvName, 0, 150);
	memset(InfFile, 0, 150);
	memset(HLIname, 0, 150);
	memset(DestInfFile, 0, 150);


	if (HacklanSys_path != NULL)
	{
		sprintf_s(srvName, "%s\\%s", HacklanSys_path, Service_Name);
		sprintf_s(InfFile, "%s\\%s", HacklanSys_path, FileInf_Name);
		sprintf_s(HLIname, "%s\\%s", HacklanSys_path, HLInstaller_Name);
	}
	else
	{
		memset(filepath, 0, 150);
		GetModuleFileNameA(NULL, filepath, 150);
		ExtractFilePath(filepath, path, 150);
		sprintf_s(srvName, "%s\\%s", path, Service_Name);
		sprintf_s(InfFile, "%s\\%s", path, FileInf_Name);
		sprintf_s(HLIname, "%s\\%s", path, HLInstaller_Name);
	}

	manager = NULL;
	service = NULL;
	manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (manager != NULL)
	{
		service = OpenService(manager, "PHacklan", SERVICE_ALL_ACCESS);
		if (service != NULL)
		{
			QueryServiceStatus(service, &servst);
			if (servst.dwCurrentState == SERVICE_STOPPED)
			{
				if (StartService(service, 0, NULL) == 0)
					ret = -2;
			}

			ret = 1;
			CloseServiceHandle(service);
		}
		else
		{

			infFileH = NULL;

			infFileH = SetupOpenInfFile(InfFile, NULL, INF_STYLE_WIN4, &rcode);
			if (infFileH != INVALID_HANDLE_VALUE)
			{
				if (!SetupCopyOEMInfA(InfFile, HacklanSys_path, SPOST_PATH, 0, DestInfFile, 150, &reqsize, NULL))
					rcode = 1;
				else
					rcode = CopyFileSystem(HLIname, HLINstallerService);
				//rcode = SetupInstallFileEx(infFileH, NULL, Service_Name, path, DestPath, SP_COPY_NEWER_OR_SAME, NULL, Context, &bl);
				//rcode = 1;
				if (rcode != 0)
				{
					ret = 4;
					rcode = GetLastError();
				}
				else
				{
					SetupCloseInfFile(infFileH);
					if (CoInitialize(NULL) == S_OK)
					{
						result = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (LPVOID*)&pnetcfg);
						if (SUCCEEDED(result))
						{
							result = pnetcfg->QueryInterface(IID_INetCfgLock, (void**)&pnetcfglock);
							if (SUCCEEDED(result))
							{
								result = pnetcfglock->AcquireWriteLock(5000, L"HacklanProtocol", &ClientDesc);
								if (result == S_OK)
								{
									result = pnetcfg->Initialize(NULL);

									if (SUCCEEDED(result))
									{

										result = pnetcfg->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS, IID_INetCfgClassSetup, (void**)&pnetcfgclass);
										if (SUCCEEDED(result))
										{
											memset(&ObToken, 0, sizeof(OBO_TOKEN));
											ObToken.Type = OBO_USER;
											//ObToken.pncc = NULL;
											//ObToken.pszwDisplayName = NULL;
											//ObToken.pszwManufacturer = NULL;
											//ObToken.pszwProduct = NULL;
										//	result = pnetcfgclass->SelectAndInstall(WindowH, &ObToken, &pnetcfgcomponent);
									
											result = pnetcfgclass->Install(ComponentId, &ObToken, 0, 0, NULL, NULL, &pnetcfgcomponent);
											if (result == NETCFG_S_REBOOT || result == NETCFG_E_NEED_REBOOT || result == S_OK)
											{
												result = pnetcfg->Apply();
												if (pnetcfgcomponent != NULL)
													pnetcfgcomponent->Release();
											}
											else ret = 5;

											pnetcfgclass->Release();
										}
										else ret = 5;
										pnetcfg->Uninitialize();
									}
									else ret = 5;
								}
								else ret = 5;
								pnetcfglock->Release();
							}
							else ret = 5;
							pnetcfg->Release();

						}
						else ret = 5;

						CoUninitialize();
					}
					else ret = 5;
				}
			}
			else
			{
				ret = 3;
				rcode = GetLastError();
			}
			if (ret == 0)
			{
				service = CreateService(manager, "PHacklan", "PHacklan", SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, startType, SERVICE_ERROR_NORMAL, DestPath, OrderGroup, NULL, NULL, NULL, NULL);
				if (service != NULL)
				{
					if (StartService(service, 0, NULL) == 0)
						ret = -2;
					CloseServiceHandle(service);
				}
			}
		}

		CloseServiceHandle(manager);
	}
	else ret = 2;
	return ret;
}


__declspec(dllexport) int StopAndUinstallProtocolDriver()
{
	SC_HANDLE manager;
	SC_HANDLE service;
	char Name[] = "PHacklan";
	WCHAR ComponentId[] = L"MS_HLAN_PROT";
	LPWSTR ClientDesc;
	int ret;
	SERVICE_STATUS status;
	INetCfg* pnetcfg = NULL;
	INetCfgClassSetup* pnetcfgclass;
	INetCfgComponent* pnetcfgcomponent;
	INetCfgLock* pnetcfglock;
	OBO_TOKEN ObToken;
	HRESULT result;

	ret = -2;
	manager = NULL;
	service = NULL;

	manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (manager != NULL)
	{
		service = OpenService(manager, Name, SERVICE_ALL_ACCESS);
		if (service != NULL)
		{

			if (ControlService(service, SERVICE_CONTROL_STOP, &status))
			{
				while (QueryServiceStatus(service, &status))
				{
					if (status.dwCurrentState == SERVICE_STOP_PENDING)
					{
						break;
					}
					else break;
				}
				if (status.dwCurrentState == SERVICE_STOPPED)
				{

				}

			}
			Sleep(1000);
			if (DeleteService(service))
				ret = 0;
			CloseServiceHandle(service);
		}
		else
		{
			ret = 1;
		}
		CloseServiceHandle(manager);

		if (CoInitialize(NULL) == S_OK)
		{
			result = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER, IID_INetCfg, (LPVOID*)&pnetcfg);
			if (SUCCEEDED(result))
			{
				result = pnetcfg->QueryInterface(IID_INetCfgLock, (void**)&pnetcfglock);
				if (SUCCEEDED(result))
				{
					result = pnetcfglock->AcquireWriteLock(5000, L"HaclanProtocol", &ClientDesc);
					if (result == S_OK)
					{
						result = pnetcfg->Initialize(NULL);
						if (SUCCEEDED(result))
						{

							result = pnetcfg->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS, IID_INetCfgClassSetup, (void**)&pnetcfgclass);
							if (SUCCEEDED(result))
							{
								memset(&ObToken, 0, sizeof(OBO_TOKEN));
								ObToken.Type = OBO_USER;
								//ObToken.pncc = NULL;
								//ObToken.pszwDisplayName = NULL;
								//ObToken.pszwManufacturer = NULL;
								//ObToken.pszwProduct = NULL;
								//result = pnetcfgclass->SelectAndInstall(WindowH, &ObToken, &pnetcfgcomponent);

								result = pnetcfg->FindComponent(ComponentId, &pnetcfgcomponent);
								if (result == S_OK)
								{
									result = pnetcfgclass->DeInstall(pnetcfgcomponent, &ObToken, NULL);
									result = pnetcfg->Apply();
									ret = 0;
									if (pnetcfgcomponent != NULL)
										pnetcfgcomponent->Release();
								}
								else ret = 5;

								pnetcfgclass->Release();
							}
							else ret = 5;
							pnetcfg->Uninitialize();
						}
						else ret = 5;
					}
					else ret = 5;
					pnetcfglock->Release();
				}
				else ret = 5;
				pnetcfg->Release();

			}
			else ret = 5;

			CoUninitialize();
		}
		else ret = 5;

	}
	else ret = -1;
	return ret;
}


int CopyFileSystem(char* HLInstallerPath, char* ServName)
{
	SC_HANDLE manager;
	SC_HANDLE service;
	SERVICE_STATUS status;
	int ret;

	ret = 0;

	if (HLInstallerPath == NULL || ServName == NULL) return 1;

	manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (manager != NULL)
	{
		service = CreateService(manager, ServName, ServName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, HLInstallerPath, NULL, NULL, NULL, NULL, NULL);
		if (service != NULL)
		{
			if (StartService(service, 0, NULL) == 0)
				ret = 2;
			if (ControlService(service, SERVICE_CONTROL_STOP, &status))
			{
				while (QueryServiceStatus(service, &status))
				{
					if (status.dwCurrentState == SERVICE_STOP_PENDING)
					{
						break;
					}
					else break;
				}
				if (status.dwCurrentState == SERVICE_STOPPED)
				{

				}

			}
			Sleep(1000);
			if (!DeleteService(service))
				ret = 3;
			CloseServiceHandle(service);
		}
		else ret = 4;

		CloseServiceHandle(manager);
	}
	else ret = 5;


	return ret;
}

int ExtractFilePath(LPSTR source, LPSTR dest, int size)
{
	int i;

	if (source == NULL || dest == NULL || size < 0)
		return -1;
	memset(dest, 0, size);
	for (i = size; source[i] != '\\'; i--);
	memcpy(dest, source, i);
	return 0;
}


extern "C" BOOL APIENTRY DllMain(HINSTANCE hst, DWORD reson, LPVOID reserved)
{
	switch (reson)
	{
	case DLL_PROCESS_ATTACH:
	{
		//CoInitialize(NULL);
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		//CoUninitialize();
		break;
	}
	case DLL_THREAD_ATTACH:
	{
		break;
	}
	case DLL_THREAD_DETACH:
	{
		break;
	}

	default:break;
	}
	return true;
}
