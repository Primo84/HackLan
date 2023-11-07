#include "HLI.h"


NTSTATUS DriverEntry(IN DRIVER_OBJECT* DriverObject, IN PUNICODE_STRING RegistryPath)
{
	int code;

	DriverObject->DriverUnload = &OnUnload;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HLPInstaller star..........");

	RtlInitUnicodeString(&UniStr, RegKey);
	InitializeObjectAttributes(&obAtr, &UniStr, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, 0, NULL);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Key reg : %ws",UniStr.Buffer);

	stat = ZwOpenKey(&keyH, KEY_ALL_ACCESS, &obAtr);

	if (stat == STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Key Opened");
		ValueInf = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, sizeof(KEY_VALUE_FULL_INFORMATION),0);
		memset(ValueInf, 0, sizeof(KEY_VALUE_FULL_INFORMATION));
		RtlInitUnicodeString(&UniStr, Value);
		stat = ZwQueryValueKey(keyH, &UniStr, KeyValueFullInformation, ValueInf, sizeof(KEY_VALUE_FULL_INFORMATION), &rLen);
		if (stat == STATUS_BUFFER_TOO_SMALL || stat == STATUS_BUFFER_OVERFLOW)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Buffer to small realocat buffer");
			ExFreePool(ValueInf);
			rLen = rLen + sizeof(KEY_VALUE_FULL_INFORMATION);
			ValueInf = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePoolWithTag(NonPagedPool, rLen,0);
			memset(ValueInf, 0, rLen);
			stat = ZwQueryValueKey(keyH, &UniStr, KeyValueFullInformation, ValueInf, rLen, &rLen);
		}else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Error code : %x",stat);
		if (stat == STATUS_SUCCESS)
		{
			buf = (char*)ValueInf;
			buf = buf + ValueInf->DataOffset;
			memset(ImagePath, 0 ,250 * sizeof(wchar_t));
			wcscpy(ImagePath, &(((wchar_t*)buf)[4]));
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HacklanPath : %ws", ImagePath);
			code = CopyFileSystemF(ImagePath);
			if(code==0)
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "File copy success...");
					else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "File copy failed... code : %d",code);

		}
		if (ValueInf != NULL)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ValueInf Relased");
			ExFreePool(ValueInf);
		}
		ZwClose(keyH);
	}

	return STATUS_SUCCESS;
}

void OnUnload(IN DRIVER_OBJECT* DriverObject)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "HLPInstaller ending..........");
}


int CopyFileSystemF(wchar_t *FilePath)
{
	HANDLE SourFileH, DestFileH;
	char* bufor;
	wchar_t DestFilePath[]= L"\\DosDevices\\C:\\Windows\\System32\\drivers\\PHacklan.sys";
	wchar_t SourceFilePath[250];
	OBJECT_ATTRIBUTES SourceObj, DestObj;
	UNICODE_STRING SourceStr, DestStr;
	IO_STATUS_BLOCK SourcePioStatus, DestPioStatus;
	FILE_STANDARD_INFORMATION Standardinfo;
	FILE_POSITION_INFORMATION SrcFilePos, DstFilePos;
	NTSTATUS status;
	int j,i;


	if (FilePath == NULL)
		return 1;
	if (wcslen(FilePath) <= 0)
		return 1;

	j = wcslen(FilePath);
	while (FilePath[j] != '\\')
	{
		j--;
	}


	memset(SourceFilePath, 0, 250 * sizeof(wchar_t));
	wcscpy(SourceFilePath, L"\\DosDevices\\");
	for (i = 0; i < j+1; i++)
		SourceFilePath[12+ i] = FilePath[i];

	wcscpy(&SourceFilePath[wcslen(SourceFilePath)], L"PHacklan.sys");

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "SourceFilePath = %ws", SourceFilePath);

	RtlInitUnicodeString(&SourceStr, SourceFilePath);
	RtlInitUnicodeString(&DestStr, DestFilePath);

	InitializeObjectAttributes(&SourceObj, &SourceStr, OBJ_KERNEL_HANDLE, NULL, NULL);
	InitializeObjectAttributes(&DestObj, &DestStr, OBJ_KERNEL_HANDLE, NULL, NULL);

	status = ZwCreateFile(&DestFileH, GENERIC_READ | GENERIC_WRITE, &DestObj, &DestPioStatus, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (status == STATUS_SUCCESS)
	{
		status = ZwOpenFile(&SourFileH, GENERIC_READ | GENERIC_WRITE, &SourceObj, &SourcePioStatus, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_SYNCHRONOUS_IO_NONALERT);
		if (status == STATUS_SUCCESS)
		{
			status = ZwQueryInformationFile(SourFileH, &SourcePioStatus, &Standardinfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
			if (status != STATUS_SUCCESS)
			{
				ZwClose(SourFileH);
				ZwClose(DestFileH);
				return 2;
			}
			SrcFilePos.CurrentByteOffset.QuadPart = 0;
			status = ZwSetInformationFile(SourFileH, &SourcePioStatus, &SrcFilePos, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
			if (status != STATUS_SUCCESS)
			{
				ZwClose(SourFileH);
				ZwClose(DestFileH);
				return 2;
			}
			DstFilePos.CurrentByteOffset.QuadPart = 0;
			status = ZwSetInformationFile(DestFileH, &DestPioStatus, &DstFilePos, sizeof(FILE_POSITION_INFORMATION), FilePositionInformation);
			if (status != STATUS_SUCCESS)
			{
				ZwClose(SourFileH);
				ZwClose(DestFileH);
				return 2;
			}

			bufor = (char*)ExAllocatePoolWithTag(NonPagedPool, Standardinfo.EndOfFile.QuadPart,'Hckl');

			ZwReadFile(SourFileH, NULL, NULL, NULL, &SourcePioStatus, bufor, (ULONG)Standardinfo.EndOfFile.QuadPart, NULL, NULL);
			ZwWriteFile(DestFileH, NULL, NULL, NULL, &DestPioStatus, bufor, (ULONG)Standardinfo.EndOfFile.QuadPart, NULL, NULL);
			ExFreePool(bufor);
			ZwClose(SourFileH);
		}
		else
		{
			ZwClose(DestFileH);
			return 5;
		}
		ZwClose(DestFileH);

	}
	else return 6;

	return 0;
}