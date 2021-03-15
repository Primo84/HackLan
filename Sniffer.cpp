
#pragma comment(lib,"hckL.lib")
#include"hckL.h"
#include "stdio.h"
#include "winioctl.h"
#include "fstream"
#include "windows.h"
using namespace std;

Dev_Lan *LanD;
int i,req,rozm;
char zn[150];

char * buf;

int main(int argc, char* argv[])
{
	LanD = new Dev_Lan;
	memset(LanD, 0, sizeof(Dev_Lan));
	i= GetDevices(LanD,sizeof(Dev_Lan),&req);
	if (i == 1)
	{
		delete(LanD);
		buf = new char[req];
		LanD = (Dev_Lan*)buf;
		rozm = req;
		memset(LanD, 0, req);
		i= GetDevices(LanD,rozm, &req);
	}
	LanD++;
	printf("Funkcja GetDevice return : %d", i);
	scanf_s("%s",zn);
	return 0;
}