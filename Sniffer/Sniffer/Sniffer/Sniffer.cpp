
#pragma comment(lib,"hckL.lib")
#pragma warning(disable : 4996)
#include"hckL.h"
#include "winioctl.h"
#include "fstream"
#include "windows.h"
#include"Protocols.h"
DWORD RecvThreadID, SendThreadID, MPRecvThreadID;
HANDLE RecvH, MPSendH, MPRecvH;
using namespace std;

Dev_Lan *LanD;
Userminiport*MiniP;

int i,j,req,rozm,NetCardsCount,karta,miniportCount;
char zn[150];
char * buf;
char operacja[10], karta_c[10];
bool opened,installedP,installedF;
WLAN_BSS_ENTRY * BSS;
int BSSCount;
int licznik = 0;
int RL = 0;
int MPRL = 0;
int MPSL = 0;
//E_Head Pack;

int Recive(EHeader Frame)
{
	E802_11  *EthernetFrame;

	EthernetFrame = (E802_11*)Frame.NetworkData;

	if (RL == 1000)RL = 1;
	else RL++;
	
	printf("Packet %d\n", RL);
	printf("Adres 1 : %x %x %x %x %x %x\n", EthernetFrame->Adres1[0], EthernetFrame->Adres1[1], EthernetFrame->Adres1[2],
		EthernetFrame->Adres1[3], EthernetFrame->Adres1[4], EthernetFrame->Adres1[5]);
	printf("Adres 2 : %x %x %x %x %x %x\n", EthernetFrame->Adres2[0], EthernetFrame->Adres2[1], EthernetFrame->Adres2[2],
		EthernetFrame->Adres2[3], EthernetFrame->Adres2[4], EthernetFrame->Adres2[5]);
	printf("Adres 3 : %x %x %x %x %x %x\n\n\n", EthernetFrame->Adres3[0], EthernetFrame->Adres3[1], EthernetFrame->Adres3[2],
		EthernetFrame->Adres3[3], EthernetFrame->Adres3[4], EthernetFrame->Adres3[5]);
	return 1;

}

int ReciveMP(EHeader Frame)
{

	E802_11* EthernetFrame;

	EthernetFrame = (E802_11*)Frame.NetworkData;

	if (MPRL == 1000)MPRL = 1;
	else MPRL++;

	printf("Packet %d\n", MPRL);
	printf("Adres 1 : %x %x %x %x %x %x\n", EthernetFrame->Adres1[0], EthernetFrame->Adres1[1], EthernetFrame->Adres1[2],
		EthernetFrame->Adres1[3], EthernetFrame->Adres1[4], EthernetFrame->Adres1[5]);
	printf("Adres 2 : %x %x %x %x %x %x\n", EthernetFrame->Adres2[0], EthernetFrame->Adres2[1], EthernetFrame->Adres2[2],
		EthernetFrame->Adres2[3], EthernetFrame->Adres2[4], EthernetFrame->Adres2[5]);
	printf("Adres 3 : %x %x %x %x %x %x\n\n\n", EthernetFrame->Adres3[0], EthernetFrame->Adres3[1], EthernetFrame->Adres3[2],
		EthernetFrame->Adres3[3], EthernetFrame->Adres3[4], EthernetFrame->Adres3[5]);

	return 1;

}

int SendMP(EHeader Frame)
{
	E802_11* EthernetFrame;

	EthernetFrame = (E802_11*)Frame.NetworkData;

	if (MPSL == 1000)MPSL = 1;
	else MPSL++;

	printf("Packet %d\n", MPSL);
	printf("Adres 1 : %x %x %x %x %x %x\n", EthernetFrame->Adres1[0], EthernetFrame->Adres1[1], EthernetFrame->Adres1[2],
		EthernetFrame->Adres1[3], EthernetFrame->Adres1[4], EthernetFrame->Adres1[5]);
	printf("Adres 2 : %x %x %x %x %x %x\n", EthernetFrame->Adres2[0], EthernetFrame->Adres2[1], EthernetFrame->Adres2[2],
		EthernetFrame->Adres2[3], EthernetFrame->Adres2[4], EthernetFrame->Adres2[5]);
	printf("Adres 3 : %x %x %x %x %x %x\n\n\n", EthernetFrame->Adres3[0], EthernetFrame->Adres3[1], EthernetFrame->Adres3[2],
		EthernetFrame->Adres3[3], EthernetFrame->Adres3[4], EthernetFrame->Adres3[5]);

	return 1;

}

int main(int argc, char* argv[])
{
	bool close = false;
	int licznik, CCount;
	EHeader Frame;
	char path[] = "D:\\projekty\\hack_lan\\Driver";
	//WLAN_BSSID *W_BSSID;
	//BSSID_LIST List_Bssid;
	int bssid_size;

	UNREFERENCED_PARAMETER(licznik);

	opened = false;
	installedP = true;
	installedF = true;
	LanD = new Dev_Lan;
	MiniP = new Userminiport;
	licznik = 0;
	//memset(&Pack, 0, sizeof(Pack));
	memset(LanD, 0, sizeof(Dev_Lan));
	i = GetDevices(LanD, sizeof(Dev_Lan), &req);
	if (i == 3)
	{
		printf("Brak zainstalowanych kart sieciowych...\n");
		scanf_s("%s", zn, 150);
		//return 0;
	}
	if (i == 1)
	{
		delete(LanD);
		buf = new char[req];
		LanD = (Dev_Lan*)buf;
		rozm = req;
		memset(LanD, 0, req);
		i= GetDevices(LanD,rozm, &req);
	}
	if (i != 0)
	{
		printf("Funkcja GetDevice return : %d\n\n", i);
		scanf_s("%s", zn, 150);
		//delete(LanD);
		installedP = false;
		//return 0;
	}

	memset(MiniP, 0, sizeof(Userminiport));

	i = Init_Miniports(MiniP,sizeof(Userminiport), &req);
	if (i == 1)
	{
		delete(MiniP);
		buf = new char[req];
		MiniP = (Userminiport*)buf;
		rozm = req;
		memset(MiniP, 0, req);
		i = Init_Miniports(MiniP, rozm, &req);
	}
	if (i != 0)
	{
		printf("Funkcja GetMiniports return : %d\n\n", i);
		installedF = false;
		scanf_s("%s", zn, 150);
	}
	if (installedP)
	{
		printf("Lista zainstalowanych kart sieciowych:\n\n");
		for (i = 0; i < LanD->NetCardsCount; i++)
		{
			printf("Karta sieciowa nr %d\n", i + 1);
			printf("	Device Name : %s\n", LanD[i].name);
			printf("	Service Name : %s\n", LanD[i].service_name);
		}
		
	}
	if (installedF)
	{
		printf("Lista zainstalowanych Miniportow:\n\n");
		for (i = 0; i < MiniP->miniportCount; i++)
		{
			printf("Miniport nr %d\n", i + 1);
			printf("	Miniport Name: %ws\n", MiniP[i].MiniportName);
			printf("	Miniport Index : %d\n", MiniP[i].Index);
			MiniP[i].RecvHooked = 1;
			MiniP[i].SendHooked = 1;
		}
	}
	do
	{
		printf("\n\nWybierz operacje:\n		1-OpenAdapter\n		2-CloseAdapter\n		3-Set Recive Handler\n		4-Send Packet\n		5-Close Program\n		6-Pobierz liste dostepnych roterow\n		7-Instaluj Sterownik\n		8-Odinstaluj sterownik\n		9-Ilosc podlaczonych uchwytow\n		10-Set Recive Miniport Handler\n		11-Stop Recive Miniport Handler\n		12-Set Send Miniport Handler\n		13-Stop Send Miniport Handler\n");
		memset(operacja, 0, 10);
		scanf_s("%s", operacja, 10);

		if (_strnicmp(operacja, "1", strlen(operacja)) == 0)
		{
		if (installedP)
			{
				do
				{
					i = 1;
					karta = 0;
					printf("Wybierz numer karty lub wprowadz wartosc 0 zeby anulowac : ");
					memset(karta_c, 0, 10);
					scanf_s("%s", karta_c, 10);
					karta = atoi(karta_c);
					if (karta > LanD[0].NetCardsCount || karta < 0)
					{
						printf("Nieprawidlowy numer karty\n");
						i = 0;
					}
				} while (i == 0);
				if (karta > 0)
				{
					i = OpenAdapter(LanD[karta - 1]);
					if (i == 0) opened = true;
					else if (i == 4)
					{
						opened = true;
						printf("Adapter zostal juz otwarty\n");

					}
				}
			}
		}
		if (_strnicmp(operacja, "2", strlen(operacja)) == 0)
		{
			opened = true;
			if (opened == false)
			{
				printf("Nie otwarto zadnego adaptera");

			}
			else
			{
				printf("Zamykanie adaptera...\n");
				CloseAdapter();
			}
		}
		if (_strnicmp(operacja, "3", strlen(operacja)) == 0)
		{
			opened = true;
			if (opened == false)
			{
				printf("Nie otwarto zadnego adaptera");

			}
			else
			{
				RecvH = SetRecive(&Recive, &RecvThreadID);
				
				if (i == 0) printf("Zainstaloweana funkcje zwrotna odbierania pakietow");
				else printf("Blad instalacji funkcji odbierania pakietow---kod bledu %d", i);
			}

		}

		if (_strnicmp(operacja, "4", strlen(operacja)) == 0)
		{
			memset(&Frame, 0, sizeof(E802_11));
			//memcpy(&Frame, &Pack, sizeof(E_Head));
			SendFramePacket(&LanD[karta - 1],Frame.NetworkData,Frame.DataSize);
		}

		if (_strnicmp(operacja, "5", strlen(operacja)) == 0)
		{
			close = true;
		}
		if (_strnicmp(operacja, "6", strlen(operacja)) == 0)
		{
			BSSCount = 3;
			BSS= new WLAN_BSS_ENTRY[BSSCount];
			j=GetBSSIDlist(BSS,&BSSCount);
			if (j == 1)
			{
				delete(BSS);
				BSS = new WLAN_BSS_ENTRY[BSSCount];
				j = GetBSSIDlist(BSS, &BSSCount);
			}
			if (j==0)
			{
				printf("Wykryte rutery :\n");
				for (i = 0; i < BSSCount; i++)
				{
					printf("Nazwa : %s			Adres Mac : %x \n",BSS[i].dot11Ssid.ucSSID,BSS[i].dot11Bssid);
				}
				delete(BSS);
			}
		}
		if (_strnicmp(operacja, "7", strlen(operacja)) == 0)
		{
				
		
			if (InstallFilterDriver(NULL,SERVICE_DEMAND_START,0) == 0)
				printf("Zainstalowano sterownik filtra.....\n");
			else printf("Nie udalo sie zainstalowac sterownika\n");
		
			if (InstallProtocolDriver(NULL, SERVICE_DEMAND_START, 0) == 0)
				printf("Zainstalowano sterownik protoko³u.....\n");
			else printf("Nie udalo sie zainstalowac sterownika\n");
			
		}

		if (_strnicmp(operacja, "8", strlen(operacja)) == 0)
		{
		
			if (StopAndUinstallFilterDriver() == 0)
			printf("odinstalowano sterownik filtra.....\n");
			else printf("Nie udalo sie odinstalowac sterownika\n");
		
			if (StopAndUinstallProtocolDriver() == 0)
				printf("odinstalowano sterownik protoko³u.....\n");
			else printf("Nie udalo sie odinstalowac sterownika\n");
		}

		if (_strnicmp(operacja, "9", strlen(operacja)) == 0)
		{
			if (installedP)
			{
				do
				{
					i = 1;
					karta = 0;
					printf("Wybierz numer karty lub wprowadz wartosc 0 zeby anulowac : ");
					memset(karta_c, 0, 10);
					scanf_s("%s", karta_c, 10);
					karta = atoi(karta_c);
					if (karta > LanD[0].NetCardsCount || karta < 0)
					{
						printf("Nieprawidlowy numer karty\n");
						i = 0;
					}
				} while (i == 0);
				if (karta > 0)
				{
					i = GetConnected(&CCount, &LanD[karta-1]);
					if (i == 0)
					{
						printf("Ilosc podlaczonych uchwytow : %d\n", CCount);
					}
				}
			}
		}
		if (strcmp(operacja, "10")==0)
		{
			if (installedF)
			{
				MPRecvH=SetReciveMPHandler(&ReciveMP, MiniP, &MPRecvThreadID);
				if (i == 0) printf("Zainstaloweana funkcje zwrotna odbierania pakietow z filtra");
				else printf("Blad instalacji funkcji odbierania pakietow z filtra---kod bledu %d", i);
			}
		}

		if (strcmp(operacja, "11") == 0)
		{
			if (installedF)
			{
				i = CloseReciveMPHandler();
				if (i == 0) printf("Zatrzymano funkcje zwrotna odbierania pakietow z filtra");
				else printf("Blad zatrzymywania funkcji odbierania pakietow z filtra---kod bledu %d", i);
			}
		}

		if (strcmp(operacja, "12") == 0)
		{
			if (installedF)
			{
				MPSendH=SetSendMPHandler(&SendMP, MiniP, &SendThreadID);
				if (i == 0) printf("Zainstaloweana funkcje zwrotna odbierania pakietow z filtra");
				else printf("Blad instalacji funkcji odbierania pakietow z filtra---kod bledu %d", i);
			}
		}

		if (strcmp(operacja, "13") == 0)
		{
			if (installedF)
			{
				i = CloseSendMPHandler();
				if (i == 0) printf("Zatrzymano funkcje zwrotna odbierania pakietow z filtra");
				else printf("Blad zatrzymywania funkcji odbierania pakietow z filtra---kod bledu %d", i);
			}
		}
	} while (close == false);

	delete (LanD);
	if (MiniP);
	
	return 0;
}
