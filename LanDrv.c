
//#include "winioctl.h"
#include "ntddk.h"
#include "LanDrv.h"
#include "string.h"
#include "stdio.h"
#include "Lan.h"







DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD OnUnload;

UNICODE_STRING netKeyU, ValueU, Value, DevN, DevDN;
ANSI_STRING ValueA;
OBJECT_ATTRIBUTES obj, ValueO;
net_cards cards[25];
HANDLE hn, hn1;
char key[] = "\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards";
NTSTATUS st;
int licznik, i;
ULONG ret, rozm;
PKEY_FULL_INFORMATION key_info;
PKEY_BASIC_INFORMATION KeyBasicInfo;
PKEY_VALUE_FULL_INFORMATION ValueInfo;
LPWSTR data;
wchar_t Dev_Name[] = L"\\Device\\HackLan";
wchar_t Dev_Dos_Name[] = L"\\DosDevices\\HackLan";
DEVICE_OBJECT Device_Hack;
int rozmiar_buf;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT Driver, IN PUNICODE_STRING str)
{
	Driver->DriverUnload = OnUnload;
	//i = RegisterProtocol();
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "LanDrv star....");
	licznik = 0;
	RtlInitUnicodeString(&netKeyU, L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards");
	if (st != STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna utworzyc ciagu wiers:26");
		return STATUS_UNSUCCESSFUL;
	}
	InitializeObjectAttributes(&obj, &netKeyU, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	st = ZwOpenKey(&hn, KEY_ALL_ACCESS, &obj);
	if (st != STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc klucza rejestru wiersz:26 %d");
		return STATUS_UNSUCCESSFUL;
	}

	//*********************** Odczytywanie informacji o ilosci podkluczy rejestru**************************
	rozm = sizeof(KEY_FULL_INFORMATION);
	ret = 0;
	key_info = (PKEY_FULL_INFORMATION)ExAllocatePool(NonPagedPool, rozm);

	if (key_info == NULL)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc zaalokowac pamieci dla klucza");
		ZwClose(hn);
		return STATUS_UNSUCCESSFUL;
	}

	st = ZwQueryKey(hn, KeyFullInformation, key_info, rozm, &ret);

	if (st == STATUS_BUFFER_OVERFLOW || st == STATUS_BUFFER_TOO_SMALL)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Buffer overflow zainicjowano %d a  potrebne %d", rozm, ret);
		ExFreePool(key_info);
		rozm = ret;
		key_info = (PKEY_FULL_INFORMATION)ExAllocatePool(NonPagedPool, rozm);
		if (key_info == NULL)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc zaalokowac pamieci dla klucza");
			ZwClose(hn);
			return STATUS_UNSUCCESSFUL;
		}

		RtlZeroMemory(key_info, rozm);
		st = ZwQueryKey(hn, KeyFullInformation, key_info, rozm, &ret);
		if (st != STATUS_SUCCESS)
		{
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Totalny Blad funkcji ZwQueryKey");
			ExFreePool(key_info);
			ZwClose(hn);
			return STATUS_UNSUCCESSFUL;
		}
	}
	//********************Jesli sukces to wylicza klucze i odczytuje potrzebne informacje do struktury*****************


	if (st == STATUS_SUCCESS)
	{
		licznik = key_info->SubKeys;
		ExFreePool(key_info);

		for (i = 0; i < licznik; i++)
		{

			rozm = sizeof(KEY_BASIC_INFORMATION);
			ret = 0;
			KeyBasicInfo = (PKEY_BASIC_INFORMATION)ExAllocatePool(NonPagedPool, rozm);
			if (KeyBasicInfo == NULL)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc zaalokowac pamieci dla KeyBasicInfo");
				ZwClose(hn);
				return STATUS_UNSUCCESSFUL;
			}

			st = ZwEnumerateKey(hn, i, KeyBasicInformation, KeyBasicInfo, rozm, &ret);

			if (st == STATUS_BUFFER_OVERFLOW || st == STATUS_BUFFER_TOO_SMALL)
			{

				ExFreePool(KeyBasicInfo);
				rozm = ret;

				KeyBasicInfo = (PKEY_BASIC_INFORMATION)ExAllocatePool(NonPagedPool, rozm);
				if (KeyBasicInfo == NULL)
				{
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc zaalokowac pamieci dla KeyBasicInfo");
					ZwClose(hn);
					return STATUS_UNSUCCESSFUL;
				}
				st = ZwEnumerateKey(hn, i, KeyBasicInformation, KeyBasicInfo, rozm, &ret);
			}
			if (st != STATUS_SUCCESS)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Totalny Blad funkcji ZwEnumerateKey");
				ExFreePool(KeyBasicInfo);
				ZwClose(hn);
				return STATUS_UNSUCCESSFUL;
			}
			sprintf(cards[i].keyPath, "%s\\%s", key, KeyBasicInfo->Name);

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nazwa podklucza : %s", cards[i].keyPath);

			ExFreePool(KeyBasicInfo);

			RtlInitAnsiString(&ValueA, cards[i].keyPath);
			RtlAnsiStringToUnicodeString(&ValueU, &ValueA, TRUE);
			InitializeObjectAttributes(&ValueO, &ValueU, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

			if (!NT_SUCCESS(ZwOpenKey(&hn1, KEY_ALL_ACCESS, &ValueO)))
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc podklucza %d", i);
				RtlFreeUnicodeString(&ValueU);
				ZwClose(hn);
				return STATUS_UNSUCCESSFUL;
			}
			else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Otworzono podklucz %d", i);


			//******************Odczytuje wartosc Description(Nazwa karty scieciowej) podklucza rejestru******************



			RtlInitUnicodeString(&Value, L"Description");
			rozm = sizeof(KEY_VALUE_FULL_INFORMATION);
			ret = 0;

			ValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(NonPagedPool, rozm);
			if (ValueInfo == NULL)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc zaalokowac pamieci dla danych wartosci klcza");
				RtlFreeUnicodeString(&ValueU);
				ZwClose(hn);
				ZwClose(hn1);
				return STATUS_UNSUCCESSFUL;
			}

			RtlZeroMemory(ValueInfo, rozm);


			st = ZwQueryValueKey(hn1, &Value, KeyValueFullInformation, ValueInfo, rozm, &ret);

			if (st == STATUS_BUFFER_OVERFLOW || st == STATUS_BUFFER_TOO_SMALL)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Buffer overflow zainicjowano %d a  potrebne %d", rozm, ret);
				ExFreePool(ValueInfo);
				rozm = ret;
				ValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(NonPagedPool, rozm);
				if (ValueInfo == NULL)
				{
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc zaalokowac pamieci dla danych wartosci klcza");
					RtlFreeUnicodeString(&ValueU);
					ZwClose(hn);
					ZwClose(hn1);
					return STATUS_UNSUCCESSFUL;
				}

				RtlZeroMemory(ValueInfo, rozm);
				st = ZwQueryValueKey(hn1, &Value, KeyValueFullInformation, ValueInfo, rozm, &ret);
			}
			if (st != STATUS_SUCCESS)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Totalny Blad");
			}

			data = (LPWSTR)ExAllocatePool(NonPagedPool, ValueInfo->DataLength);
			RtlCopyMemory((PVOID)data, (PVOID)((char*)ValueInfo + ValueInfo->DataOffset), ValueInfo->DataLength);
			RtlInitUnicodeString(&Value, data);
			RtlUnicodeToUTF8N((PCHAR)cards[i].Description, 150, &ret, Value.Buffer, Value.Length);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Description : %s", cards[i].Description);

			ExFreePool(data);
			ExFreePool(ValueInfo);

			//*************Odczytuje wartoswc ServiceName(Nazwa urzadenia do ktorego mozna sie podlaczyc) podklucza rejestru**********************

			RtlInitUnicodeString(&Value, L"ServiceName");

			rozm = sizeof(KEY_VALUE_FULL_INFORMATION);
			ret = 0;
			ValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(NonPagedPool, rozm);
			RtlZeroMemory(ValueInfo, rozm);

			st = ZwQueryValueKey(hn1, &Value, KeyValueFullInformation, ValueInfo, rozm, &ret);

			if (st == STATUS_BUFFER_OVERFLOW || st == STATUS_BUFFER_TOO_SMALL)
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Buffer overflow zainicjowano %d a  potrebne %d", rozm, ret);
				ExFreePool(ValueInfo);
				rozm = ret;
				ValueInfo = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(NonPagedPool, rozm);
				if (ValueInfo == NULL)
				{
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nie mozna otworzyc zaalokowac pamieci dla danych wartosci klcza");
					RtlFreeUnicodeString(&ValueU);
					ZwClose(hn);
					ZwClose(hn1);
					return STATUS_UNSUCCESSFUL;
				}
				RtlZeroMemory(ValueInfo, rozm);
				st = ZwQueryValueKey(hn1, &Value, KeyValueFullInformation, ValueInfo, rozm, &ret);
			}

			data = (LPWSTR)ExAllocatePool(NonPagedPool, ValueInfo->DataLength);
			RtlCopyMemory((PVOID)data, (PVOID)((char*)ValueInfo + ValueInfo->DataOffset), ValueInfo->DataLength);
			RtlInitUnicodeString(&Value, data);
			RtlUnicodeToUTF8N((PCHAR)cards[i].ServiceName, 150, &ret, Value.Buffer, Value.Length);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ServiceName : %s", cards[i].ServiceName);

			ExFreePool(data);
			ExFreePool(ValueInfo);
			ZwClose(hn1);
			RtlFreeUnicodeString(&ValueU);
		}



		ZwClose(hn);

	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Blad funkcij EnumerateKey error: %d", (int)st);
		return STATUS_UNSUCCESSFUL;
	}

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Tworzenie Objektu Device");

	RtlInitUnicodeString(&DevN, Dev_Name);
	RtlInitUnicodeString(&DevDN, Dev_Dos_Name);

	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) Driver->MajorFunction[i] = NULL;
	Driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = &DeviceIOControl;
	Driver->MajorFunction[IRP_MJ_CREATE] = &DeviceCreate;
	Driver->MajorFunction[IRP_MJ_CLOSE] = &DeviceClose;
	Driver->MajorFunction[IRP_MJ_CLEANUP] = &DeviceCleanUp;
	st = IoCreateDevice(Driver, 0, &DevN, FILE_DEV_DRV, 0, FALSE, &Device_Hack);

	if (st == STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Utworzono DeviceObject");
	}
	else
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Nieutworzono DeviceObject");
		if (st == STATUS_INSUFFICIENT_RESOURCES) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bload tworzenia Device : STATUS_INSUFFICIENT_RESOURCES");
		else if (st == STATUS_OBJECT_NAME_COLLISION) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Bload tworzenia Device : STATUS_OBJECT_NAME_COLLISION");
	}
	if (IoCreateSymbolicLink(&DevDN, &DevN) == STATUS_SUCCESS)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Utworzono SymbolicLink");
	}
	else DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "NieUtworzono SymbolicLink");

	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, "Pryzpisanie funkcji sterownika....");



	return STATUS_SUCCESS;

}


//*******************************Service Stop******************************



VOID OnUnload(IN PDRIVER_OBJECT Driver)
{
	DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, " Ending LanDrv....");
	IoDeleteSymbolicLink(&DevDN);
	IoDeleteDevice(Driver->DeviceObject);
}




//*************************DeviceIOControl*********************************




NTSTATUS DeviceCreate(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{

	return STATUS_SUCCESS;
}

NTSTATUS DeviceClose(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{

	return STATUS_SUCCESS;
}
NTSTATUS DeviceCleanUp(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{

	return STATUS_SUCCESS;
}

NTSTATUS DeviceIOControl(__in DEVICE_OBJECT* Device, __inout IRP* irp)
{
	PIO_STACK_LOCATION stack;
	ULONG code;
	PVOID buf_in, buf_out;
	char str[25] = "";
	ULONG in_len, out_len;
	char* bt;
	stack = IoGetCurrentIrpStackLocation(irp);
	code = stack->Parameters.DeviceIoControl.IoControlCode;
	in_len = stack->Parameters.DeviceIoControl.InputBufferLength;
	out_len = stack->Parameters.DeviceIoControl.OutputBufferLength;
	buf_in = irp->AssociatedIrp.SystemBuffer;
	buf_out = irp->UserBuffer;


	if (stack->MajorFunction == IRP_MJ_DEVICE_CONTROL)
	{
		DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_INFO_LEVEL, "DeviceIoControl execute...");
		switch (code)
		{
		case IO_DEV_INIT:
		{
			rozmiar_buf = licznik * 300;
			memset(buf_out, 0, out_len);
			if (rozmiar_buf > out_len)
			{
				sprintf(str, "AAAsA%d", rozmiar_buf);
				memcpy(buf_out, str, strlen(str));
			}
			else
			{
				bt = (char*)buf_out;
				for (i = 0; i < licznik; i++)
				{
					memcpy(bt, cards[i].Description, 150);
					bt = bt + 150;
					memcpy(bt, cards[i].ServiceName, 150);
					if (i < licznik - 1) bt = bt + 150;
				}
			}

			break;
		}
		case IO_DEV_VER:
		{
			break;
		}
		}
	}
	irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}





