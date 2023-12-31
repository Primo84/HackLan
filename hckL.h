#pragma once
#include "windows.h"
#include "winioctl.h"

typedef struct Device_Lan
{
	char name[150];
	char service_name[150];
}Dev_Lan;

#define FILE_DEV_DRV		0x00002A7B

#define IO_DEV_INIT			(ULONG)CTL_CODE(FILE_DEV_DRV,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	IO_DEV_VER			(ULONG)CTL_CODE(FILE_DEV_DRV,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)


/*
	Funkcja zwraca list� zainstalowanych kart sieciowych
		Parametry wej�ciowe:
					Dev_Lan - uchwyt do struktur przechowuj�cej informacj� o karcie sieciowej
					size - rozmiar struktur
		Zwraca:		
					0  - powodzenie
					-1 - gdy wska�nik Dev = NULL lub size_reg = NULL
					1 - gdy size jest za ma�e , wtedy size_req zawiera ilo�� potrzebnego miejsca do zaalokowania
					2 - gdy nie mo�na otworzy� uchwytu do sterownika
*/

	__declspec(dllexport) int GetDevices(Dev_Lan *Dev,int size, int *size_req);
