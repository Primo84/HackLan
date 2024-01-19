#pragma once

#pragma comment (lib,"wlanapi.lib")
#pragma comment(lib,"Setupapi.lib")
#include "windows.h"
#include "winioctl.h"
#include "stdio.h"
#include "stdlib.h"
#include "wlanapi.h"
#include "winsvc.h"
#include "setupapi.h"
#include "devguid.h"
#include "objbase.h"

#include "netcfgx.h"
#include "netcfgn.h"
#include "fileapi.h"
#include "winerror.h"
#include "shellapi.h"


const int PACKET_COUNT = 1000;

//ModeCap member of Device_Lan

#define OPERATION_MODE_UNKNOWN            0x00000000
#define OPERATION_MODE_STATION            0x00000001
#define OPERATION_MODE_AP                 0x00000002
#define OPERATION_MODE_EXTENSIBLE_STATION 0x00000004
#define OPERATION_MODE_EXTENSIBLE_AP      0x00000008
#define OPERATION_MODE_WFD_DEVICE         0x00000010
#define OPERATION_MODE_WFD_GROUP_OWNER    0x00000020
#define OPERATION_MODE_WFD_CLIENT         0x00000040
#define OPERATION_MODE_MANUFACTURING      0x40000000
#define OPERATION_MODE_NETWORK_MONITOR    0x80000000

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
	PhysicalMediumMax       // Not a real physical type, defined as an upper-bound
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

typedef enum _MEDIA_DUPLEX_STATE
{
	DuplexStateUnknown,
	DuplexStateHalf,
	DuplexStateFull
} MEDIA_DUPLEX_STATE;

typedef struct UserMiniport
{
	char MiniportName[250];
	int HandleCount;
	int miniportCount;
	int Index;
	int RecvHooked;
	int SendHooked;
	int licznik;
	ULONG64 XmitLinkSpeed;
	ULONG64 RcvLinkSpeed;
	USHORT MacAddressLength;
	UCHAR CurrentMacAddress[32];
	PHYSICALMEDIUM PhysicalMediumType;
	MEDIUM MediumType;
	MEDIA_CONNECT_STATE MediaConnectState;
	MEDIA_DUPLEX_STATE MediaDuplexState;

}Userminiport, * PUserminiport;

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



	typedef struct recive_packet
	{
		EHeader EHead[PACKET_COUNT];
		int odebrane;
	}RecvPack;

	typedef struct Packet_Info
	{
		ULONG64 BindingContext;				
		int licznik;

	}PacketInfo;

	typedef struct Send_Pack
	{
		ULONG64 BindingContext;			// BindingContext struktury Dev_Lan
		//int DataSize;
		EHeader Frame;

	}SendPack;


	typedef struct Packet_Send
	{
		Dev_Lan Adapter;	
		unsigned char Packet[5000];
		int DataSize;

	}PacketSend;


	//typedef struct _NDIS_802_11_SSID { ULONG SsidLength; UCHAR Ssid[32]; } NDIS_802_11_SSID;
	//typedef struct _NDIS_802_11_CONFIGURATION_FH { ULONG Length; ULONG HopPattern; ULONG HopSet; ULONG DwellTime; }CONFIGURATION_FH;
	//typedef struct _CONFIGURATION { ULONG Length; ULONG BeaconPeriod; ULONG ATIMWindow; ULONG DSConfig; CONFIGURATION_FH FHConfig; }CONFIGURATION;
	
	/*
	typedef enum _NETWORK_TYPE {
		Ndis802_11FHH,
		Ndis802_11DSS,
		Ndis802_11NetworkTypeMaxx,
	} NETWORK_TYPE;
	
	typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE {
		Ndis802_11IBSS,
		Ndis802_11Infrastructure,
		Ndis802_11AutoUnknown,
		Ndis802_11InfrastructureMax,
	}NETWORK_INFRASTRUCTURE;
	
	typedef struct _WLAN_BSSID {
		ULONG Length;
		unsigned char MacAddress[6];
		UCHAR Reserved[2];
		NDIS_802_11_SSID Ssid;
		ULONG Privacy;
		unsigned long int Rssi;

		NETWORK_TYPE NetworkTypeInUse;
		CONFIGURATION Configuration;

		NETWORK_INFRASTRUCTURE InfrastructureMode;
		unsigned char SupportedRates[8];
		ULONG
		IELength;         UCHAR IEs[1];
	} WLAN_BSSID;

	typedef struct _NDIS_802_11_BSSID_LIST_EX {
		ULONG NumberOfItems;
		WLAN_BSSID* WBssid;

	} BSSID_LIST;
	*/


typedef int  RecivePacketHandler(EHeader EthernetFrame);

typedef struct MP_SendRecive
{
	RecivePacketHandler* Recv;
	Userminiport* MiniP;
	//WORD Operacja;
}MP_SR;



#define FILE_DEV_DRV		0x00002A7B
#define FILE_DEV_DRVF		0x00002A7A

		//	Kody steruj¹ce sterownika protoko³u

#define IO_DEV_INIT				(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x801,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	IO_OPEN_ADAPTER			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x802,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define	IO_CLOSE_ADAPTER		(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x803,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_SEND_PACKET			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x805,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_QUERY_CONECTED		(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x806,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IO_SET_OP_MODE			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)
		
		//	Kody steruj¹ce sterownika filtra

#define IO_MINIPORTS_INIT				(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x807,METHOD_BUFFERED,FILE_ANY_ACCESS)


		//Kody steruj¹ce wspólne dla sterownika i filtra

#define IO_QUERY_STATE			(ULONG)CTL_CODE(FILE_DEVICE_NETWORK,0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)

extern "C"
{
	int CopyFileSystem(char* HLInstallerPath, char* ServName);
	int ExtractFilePath(LPSTR source, LPSTR dest, int size);

	DWORD WINAPI Fun(PVOID p);
	DWORD WINAPI MPFunR(PVOID p);
	DWORD WINAPI MPFunS(PVOID p);


	// Funkcja konwertuje parametr unsigned short na flagi struktury FrameControl


	/*
		Funkcja zwraca listê zainstalowanych kart sieciowych
			Parametry wejœciowe:
						Dev_Lan - uchwyt do struktur przechowuj¹cej informacjê o karcie sieciowej
						size - rozmiar struktur
			Zwraca:
						0  - powodzenie
						-1 - gdy wska¿nik Dev = NULL lub size_reg = NULL
						1 - gdy size jest za ma³e , wtedy size_req zawiera iloœæ potrzebnego miejsca do zaalokowania
						2 - gdy nie mo¿na otworzyæ uchwytu do sterownika
						3 - gdy nie ma zainstalowanej ¿adnej karty sieciowej
						4 - b³¹d komunikacji ze sterownikiem
	*/

	__declspec(dllexport) int GetDevices(Dev_Lan* Dev, int size, int* size_req);


	/*

		Funkcja otwiera adapter

		Zwraca :
					0 - powodzenie
					1 - gdy nie mo¿na otworzyæ uchwytu do sterownika
					2 - b³¹d komunikacji ze sterownikiem
					3 - gdy bufor wejœciowy jest mniejszy ni¿ rozmiar struktury Dev_Lan
					4 - adapter jest juz otwarty
					5-	nieznany blad
	*/
	__declspec(dllexport) int OpenAdapter(Dev_Lan Dev);

	/*

					Jeœli adapter zosta³ wczeœniej otwarty to zamyka ten adapter

	*/

	__declspec(dllexport) int CloseAdapter();


	/*
			Funkcja ustawia funkcje zwrotn¹ odbierania pakietów


	*/

	__declspec(dllexport) HANDLE SetRecive(RecivePacketHandler* RecvHandler, DWORD * ThreadId);


	/*
		Funkcja ustawia funkcje zwrotn¹ odbierania pakietów z filtra miniportów, w których
		Ustawiona jest wartoœæ Hooked na 1 w strukturze MiniP.
		Funkcja kopiuje strukturê MiniP.

	*/

	__declspec(dllexport) HANDLE SetReciveMPHandler(RecivePacketHandler* Recv, Userminiport* MiniP, DWORD* ThreadId);


	/*
		Funkcja ustawia funkcje zwrotn¹ odbierania wysy³anych pakietów z filtra miniportów, w których
		Ustawiona jest wartoœæ Hooked na 1 w strukturze MiniP
		Funkcja kopiuje strukturê MiniP.

	*/

	__declspec(dllexport) HANDLE SetSendMPHandler(RecivePacketHandler* Recv, Userminiport* MiniP, DWORD* ThreadId);

	/*

				Jeœli funkcja zosta³ wczeœniej zainstalowana to zatrzymuje odbieranie pakietów Filtra

				Zwraca :

					0	-	Zatrzymano 
					1	-	Nie zosta³a wczeœniej zainstalowana
*/

	__declspec(dllexport) int CloseReciveMPHandler();


	/*

				Jeœli funkcja zosta³ wczeœniej zainstalowana to zatrzymuje odbieranie pakietów Filtra

				Zwraca :

					0	-	Zatrzymano
					1	-	Nie zosta³a wczeœniej zainstalowana
*/

	__declspec(dllexport) int CloseSendMPHandler();



/*
				Funkcja wysy³a surowy pakiet. Mo¿na u¿yæ karty ethernet oraz wifi
				Nale¿y pamiêtaæ aby ramka ethernethowa by³a prawid³owo sformatowana w standardzie 802.3
				Przy wysy³aniu za pomoc¹ karty wifi nale¿y podaæ prawid³owy ¿ród³owy adres MAC karty sieciowej.


				Zwraca : 

					0	-	Powodzenie
				   -1	-	Adapter == NULL
					1	-	Jeœli nie mo¿na pobraæ uchwytu sterownika protoko³u
					2	-	Za ma³a lub za du¿a wartoœæ DataSize albo Packet == NULL


*/


	__declspec(dllexport) int SendFramePacket(Dev_Lan *Adapter, unsigned char *Packet, int DataSize);

	/*

			Funkcja pobiera liste roterow znajdujacych sie w zasiegu


			Zwraca :

					   0	-		Powodzenia
					  -1	-		Adapter nie zostal otwarty
					  -2	-       Nieznany blad
					   1	-       Za ma³y bufor, w zmiennej ItemsCount zwrascasna jest liczba wykrytych ruterow

			Funkcja alokuje pamiec dla parametru BSS. Po pomyslnej zakonczonej operacji nalezy zwolnic pamiec za pomoca
			funkcji Free(BSS);

	*/
	__declspec(dllexport) int GetBSSIDlist(WLAN_BSS_ENTRY* BSS, int* ItemsCount);


	/*
			Funkcja pobiera informacjê na temat iloœci pod³¹czonych aplikacji do danej karty sieciowej.
			Jest to potrzebne, m. in. w celu sprawdzenia czy przed zamkniêciem sterownika protoko³u
			podczas deinstalacji sterowika s¹ jakieœ inne aplikacje, które odbieraj¹ pakiety.

			Zwraca :
									0 -	Powodzenie
									1 - Nie mo¿na uzyskaæ uchwytu sterownika protoko³u
									2-	B³¹d podczas wysy³ania polecenia

	*/


	__declspec(dllexport) int GetConnected(int* ConnectedCount, Dev_Lan* DevLan);

	/*
	
			Funkcja Pobiera listê zainstalowanych miniportów. Wcelu zainstalowania funkcji odbierania pakietów trzeba najpierw 
			zainicjowaæ miniporty za pomoc¹ tej funkcji.

			Zwraca:

								 0 - Powodzenie
								-1 - MP=NULL lub regSize=NULL;
								 1 - Za ma³y bufor do pobrania listy miniportów
								 2 - Nie mozna nawi¹zaæ po³¹czenia ze sterownikiem
								 3 - Nie ma zainstalowanych ¿adnych minmiportów
								 4 - B³¹d funkcji DeviceIoControl
	
	*/

	__declspec(dllexport) int Init_Miniports(Userminiport* MP, int size, int* reqSize);


	/*
			Funkcja pobiera (aktualizuje) informacjê na temat karty sieciowej.

			Zwraca :
									0 -	Powodzenie
									1 - Nie mo¿na uzyskaæ uchwytu sterownika protoko³u
									2-	B³¹d podczas wysy³ania polecenia

	*/


	__declspec(dllexport) int GetAdapterParams(Dev_Lan* DevLan);


	/*
			Funkcja pobiera (aktualizuje) informacjê na temat miniportu.

			Zwraca :
									0 -	Powodzenie
									1 - Nie mo¿na uzyskaæ uchwytu sterownika protoko³u
									2-	B³¹d podczas wysy³ania polecenia

	*/


	__declspec(dllexport) int GetMiniportParams(Userminiport* DevLan);


	/*
			Funkcja pobiera (aktualizuje) informacjê na temat miniportu.

			Zwraca :
									0 -	Powodzenie
									1 - Nie mo¿na uzyskaæ uchwytu sterownika protoko³u
									2 -	B³¹d podczas wysy³ania polecenia
									3 - B³¹d rz¹dania sterownika filtra
									4 - B³¹d z powodu pod³¹czonej karty do punktu dostêpu (AP).
	*/


	__declspec(dllexport) int SetOpMode(Dev_Lan* dev, int OP_Mode );




	/*

				Funkcja instaluje sterownik filtra w systemie. W zmiennej path nalezy podaæ scie¿kê dostêpu do katalogu, w którym
				znajduj¹ siê pliki : FHacklan.sys,HLInstaller.sys,hlanfilter.inf i hacklan.cat
				Lub NULL jesli pliki znajduj¹ sie w biezacym folderze. Ta funkcja wymaga podniesionych uprawnien.

					Zwraca :
									  0	-	Powodzenie
									  1	-	Us³uga zosta³a  zainstalowana wczeœniej
									  2	-	Nie mozna otworzyc Menadzera uslug. Prawdopodobnie brak uprwanien.
									  3 -	Nie mo¿na otworzyæ pliku inf instalatora
									  4 -	Nie mo¿na skopiowaæ pliku inf instalatora
									  5 -	Problem z instalacj¹ komponentu sieciowego
									 -2	-	Nie mo¿na uruchomiæ us³ugi

					  startType:

							#define SERVICE_BOOT_START             0x00000000
							#define SERVICE_SYSTEM_START           0x00000001
							#define SERVICE_AUTO_START             0x00000002
							#define SERVICE_DEMAND_START           0x00000003
							#define SERVICE_DISABLED               0x00000004


		*/

	

	__declspec(dllexport) int InstallFilterDriver(LPSTR HacklanSys_path, DWORD startType, HWND WindowH);

		/*
			Funkcja odinstalowuje Us³ugê filtra z systemu


		*/

	__declspec(dllexport) int StopAndUinstallFilterDriver();






	/*

				Funkcja instaluje sterownik protoko³u w systemie. W zmiennej path nalezy podaæ scie¿kê dostêpu do katalogu, w którym
				znajduj¹ siê pliki : PHacklan.sys,HLInstaller.sys,hlanprot.inf i hacklan.cat
				Lub NULL jesli pliki znajduj¹ sie w biezacym folderze. Ta funkcja wymaga podniesionych uprawnien.

					Zwraca :
									  0	-	Powodzenie
									  1	-	Us³uga zosta³a  zainstalowana wczeœniej
									  2	-	Nie mozna otworzyc Menadzera uslug. Prawdopodobnie brak uprwanien.
									  3 -	Nie mo¿na otworzyæ pliku inf instalatora
									  4 -	Nie mo¿na skopiowaæ pliku inf instalatora
									  5 -	Problem z instalacj¹ komponentu sieciowego
									 -2	-	Nie mo¿na uruchomiæ us³ugi

					  startType:

							#define SERVICE_BOOT_START             0x00000000
							#define SERVICE_SYSTEM_START           0x00000001
							#define SERVICE_AUTO_START             0x00000002
							#define SERVICE_DEMAND_START           0x00000003
							#define SERVICE_DISABLED               0x00000004


		*/


	__declspec(dllexport) int InstallProtocolDriver(LPSTR HacklanSys_path, DWORD startType, HWND WindowH);



	/*
		Funkcja odinstalowuje Us³ugê protoko³u z systemu


	*/

	__declspec(dllexport) int StopAndUinstallProtocolDriver();


}