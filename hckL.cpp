#include "hckL.h"


__declspec(dllexport) int GetDevices(Dev_Lan* Dev, int size, int* size_req)
{
	HANDLE dev=NULL;
	Dev_Lan *D=NULL;
	wchar_t device_name[25] = L"\\\\.\\HackLan";
	DWORD ret;
	BOOL bl = FALSE;
	char* buf;
	int rozm,code;

	D = Dev;
	if (D == NULL || size_req==NULL) return -1;
	if (size < sizeof(Dev_Lan)) return 1;

	dev=CreateFileW(device_name, 0, 0, NULL,OPEN_EXISTING, 0, NULL);
	if (dev == INVALID_HANDLE_VALUE) return 2;
	buf = new char[size];
	bl=DeviceIoControl(dev, IO_DEV_INIT,NULL,0,buf,size, &ret, NULL);
	CloseHandle(dev);
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
	if (bl == FALSE) return 3;
	return code;


}
BOOL WINAPI DllMain(HINSTANCE hst, DWORD reson, LPVOID reserved)
{
	switch (reson)
	{
		case DLL_PROCESS_ATTACH:
		{
			break;
		}
		case DLL_PROCESS_DETACH:
		{
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
	}
	return true;
}