#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "windows.h"
#include "commctrl.h"
#include "Richedit.h"



/*---------------------------------------------------------


				Ethernet 802.3


----------------------------------------------------------*/


typedef struct EHead_802_3
{
	unsigned char Preambula[7];
	unsigned char SFD;
	unsigned char MAC_Docelowy[6];
	unsigned char MAC_Zrodlowy[6];
	unsigned char Typ[2];
	unsigned char Dane[1500];
	unsigned char CRC[4];
}E802_3;

typedef struct EHead_802_11
{
	unsigned short FrameControl;
	unsigned char DurationId[2];
	unsigned char Adres1[6];
	unsigned char Adres2[6];
	unsigned char Adres3[6];
	unsigned char SequenceControl[2];
	unsigned char Adres4[6];
	unsigned char NetworkData[2312];
	unsigned char FCS[4];
}E802_11;


/*---------------------------------------------------------


				IPV4


----------------------------------------------------------*/

typedef struct Ip_Prot_V4
{
	unsigned char Wersja_DlugNagl; // : 4;
	//unsigned char DlugNaglowka : 4;
	unsigned char TypUslugi;
	unsigned short RozmiarPaketu;
	unsigned short Identyfikator;
	unsigned short Flagi_PrzesFragm;
	//unsigned short PrzesFramgmH :13;
	unsigned char TTL;
	unsigned char TypPakietu;
	unsigned short SumaKontrNagl;
	unsigned char AdresZrodl[4];
	unsigned char AdresDocel[4];
	//unsigned char Opcje[4];
}IpProt;

typedef struct Ip_Prot1
{
	unsigned char Wersja : 4;
	unsigned char DlugNaglowka : 4;
	unsigned char TypUslugi;
	WORD RozmiarPaketu;
	WORD Identyfikator;
	unsigned char Flagi : 3;
	WORD PrzesFramgmH : 13;
	unsigned char TTL;
	unsigned char TypPakietu;
	unsigned char SumaKontrNagl[2];
	unsigned char AdresZrodl[4];
	unsigned char AdresDocel[4];
	//unsigned char Opcje[4];
}IpProt1;


/*---------------------------------------------------------


				TCP


----------------------------------------------------------*/

typedef struct Tcp_Prot
{
	//WORD PortZrodl;
	//WORD PortDocel;
	//DWORD NumerSekw;
	//DWORD NumerPotwBajtu;
	unsigned char DlugNagl : 4;
	unsigned char Zarezerwowane : 6;
	unsigned char Znaczniki : 6;
	//WORD RozmOkna;
	//WORD SumaKontr : 6;
	//WORD WskaznikPilnychDanych;
	//DWORD Opcje;
}TcpProt;


/*---------------------------------------------------------


				IPV6
	

----------------------------------------------------------*/

typedef struct Klasa_Ruchu
{
	unsigned char DS : 6;
	unsigned char ECN : 2;
}KlasaRuchu;

typedef struct Unikalny_Adres_Lokalny
{
	unsigned char Prefiks : 7;
	unsigned char L : 1;
	unsigned __int64 Losowy : 40;
	WORD IdentPodsieci;
	unsigned __int64 IdentInterfejsu;
}UAL;

typedef struct Adres_Lokalny_Lacza
{
	WORD Prefiks : 10;
	unsigned __int64 Zera : 54;
	unsigned __int64 IdentInterfejsu;
}ALL;

typedef struct Adres_Multiemisji_Wezla
{
	unsigned char Prefiks;
	unsigned char Flaga : 4;
	unsigned char SC : 4;
	unsigned __int64 Zera_HighP;
	unsigned short Zera_LowP : 15;
	unsigned short TE : 9;
	unsigned int Adres : 24;

}AMW;

typedef struct Adres_Multiemisji_Unicast
{
	unsigned char Prefiks;
	unsigned char Flaga : 4;
	unsigned char SC : 4;
	unsigned char Rez: 4;
	unsigned char Surowy : 4;
	unsigned char Plen;
	unsigned __int64 PrefiksSieciowy;
	unsigned int IdentyfikatorGrupy;
}AMU;


typedef struct Naglowek_Hop_by_Hop
{
	unsigned char NastNaglowek;
	unsigned char DlugoscRozszerz;
	unsigned short Opcje_i_Wypeln1;
	unsigned int Opcje_i_Wupeln2;
	unsigned __int64 Opcje;
}Nagl_HbyH;

typedef struct Naglowek_Routingu
{
	unsigned char NastNaglowek;
	unsigned char DlugoscNagl;
	unsigned char TypRoutingu;
	unsigned char PozostSegment;
	unsigned int DaneSpeceficzne1;
	unsigned __int64 DaneSpeceficzne2;
}Nagl_Rout;


typedef struct Naglowek_Fragmentu
{
	unsigned char NastNaglowek;
	unsigned char Zareserwowane;
	unsigned short int PrsesuniecieFragm : 13;
	unsigned char Zarezerwowane1:2;
	unsigned char FlagaM : 1;
	unsigned int Identyfikacja;
}Nagl_Fragm;

typedef struct V_T_F
{
	unsigned char Version;
	unsigned char TrafficClass;
	unsigned long FlowLabel;
}VTF;

typedef struct IPV_6
{
	unsigned long	VTF; // 4 bit Version ; 8 bit Traffic class ; 20 bit Flow Label
	unsigned short DlugLadunku;
	unsigned char NastNaglowek;
	unsigned char LimitPrzesk;
	unsigned short AdrZrodl[8];
	unsigned short AdrDocel[8];
}IPV6;


/*---------------------------------------------------------


				ICMP


----------------------------------------------------------*/