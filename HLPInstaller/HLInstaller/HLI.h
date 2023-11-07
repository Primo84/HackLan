#pragma once

#include "ntddk.h"

#include "wdm.h"


DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD OnUnload;



wchar_t RegKey[] = L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\HLPInstaller";

wchar_t Value[] = L"ImagePath";
wchar_t ImagePath[250];
HANDLE keyH;
OBJECT_ATTRIBUTES obAtr;
UNICODE_STRING UniStr;
NTSTATUS stat;
PKEY_VALUE_FULL_INFORMATION ValueInf;
ULONG rLen;
char* buf;


int CopyFileSystemF(wchar_t* FilePath);