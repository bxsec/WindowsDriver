// Test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Windows.h>
#include <winsvc.h>
#include <conio.h>
#include <winioctl.h>

#define DEVICE_NAME		L"NtModeDrv"
#define DRIVER_PATH		L".\\ntmodeldrv.sys"

#define IOCTRL_BASE 0x800

#define MYIOCTRL_CODE(i) \
    CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTRL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define PRINT_CODE		MYIOCTRL_CODE(0)
#define BYE_CODE		MYIOCTRL_CODE(1)
#define WRITE_HELLO		MYIOCTRL_CODE(2)

BOOL LoadDriver(TCHAR* lpszDriverName, TCHAR* lpszDriverPath)
{
	//char szDriverImagePath[256] = "D:\\DriverTest\\ntmodelDrv.sys";
	TCHAR szDriverImagePath[256] = { 0 };
	//�õ���ɵ�����·��
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;    //SCM�������ľ��
	SC_HANDLE hServiceDDK = NULL;    //NT��������ķ�����

									 //�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		//OpenSCManagerʧ��
		printf("OpenSCManager() Failed %d!\n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		//OpenSCManager�ɹ�
		printf("OpenSCManager() Ok 1!\n");
	}

	//������������Ӧ�ķ���
	hServiceDDK = CreateService(hServiceMgr,
		lpszDriverName,     //������ע����е�����
		lpszDriverName,     //ע������������DisplayNameֵ
		SERVICE_ALL_ACCESS, //������������ķ���Ȩ��
		SERVICE_KERNEL_DRIVER,  //��ʾ���صķ���������
		SERVICE_DEMAND_START,   //ע������������ Start ֵ
		SERVICE_ERROR_IGNORE,   //ע������������ ErrorControl ֵ
		szDriverImagePath,      //ע������������ ImagePath ֵ
		NULL,   //GroupOrder  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\GroupOrderList
		NULL,
		NULL,
		NULL,
		NULL);
	DWORD dwRtn;
	//�жϷ����Ƿ�ʧ��
	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//��������ԭ�򴴽�����ʧ��
			printf("CreateService() Failed %d !\n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//���񴴽�ʧ�ܣ������ڷ����Ѿ�������
			printf("CreateService() Failed Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS!\n");
		}

		//���������Ѿ����أ�ֻ��Ҫ��
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			//����򿪷���Ҳʧ�ܣ�����ζ����
			dwRtn = GetLastError();
			printf("OpenService() Failed %d!\n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			printf("OpenService() ok !\n");
		}
	}
	else
	{
		printf("CreateService() ok !\n");
	}

	//�����������
	bRet = StartService(hServiceDDK, NULL, NULL);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			printf("StartService() Failed %d !\n", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				//�豸����ס
				printf("StartService() Failed ERROR_IO_PENDING!\n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				//�����Ѿ�����
				printf("StartService() Failed ERROR_SERVICE_ALREADY_RUNNING !\n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;
	//�뿪ǰ�رվ��
BeforeLeave:
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

//ж����������
BOOL UnloadDriver(TCHAR *szSvrName)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL; //SCM�������ľ��
	SC_HANDLE hServiceDDK = NULL; //NT��������ķ�����
	SERVICE_STATUS SvrSta;
	//��SCM������
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		//��SCM������ʧ��
		printf("OpenSCManager() Failed %d!\n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		//��SCM�������ɹ�
		printf("OpenSCManager() ok !\n");
	}

	// ����������Ӧ�ķ���
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);
	if (hServiceDDK == NULL)
	{
		//����������Ӧ�ķ���ʧ��
		printf("OpenService() Failed %d!\n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenService() ok !\n");
	}

	//ֹͣ�����������ֹͣʧ�ܣ�ֻ���������������ٶ�̬����
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
	{
		printf("ControlService() Failed %d!\n", GetLastError());
	}
	else
	{
		printf("ControlService() ok !\n");
	}

	//��̬ж����������
	if (!DeleteService(hServiceDDK))
	{
		//ж��ʧ��
		printf("DeleteSrevice() Failed %d!\n", GetLastError());
	}
	else
	{
		//ж�سɹ�
		printf("DelServer:deleteService() ok\n");
	}
	bRet = TRUE;
BeforeLeave:
	//�뿪ǰ
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

void TestDriver()
{
	//������������
	HANDLE hDevice = CreateFile(_T("\\\\.\\NtModeDrv"),
		GENERIC_WRITE | GENERIC_READ,
		8,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		printf("Create Device ok!\n");
	}
	else
	{
		printf("Create Device Failed %d!\n", GetLastError());
		return;
	}
	CHAR bufRead[1024] = { 0 };
	WCHAR bufWrite[1024] = L"Hello,world";

	DWORD dwRead = 0;
	DWORD dwWrite = 0;

	ReadFile(hDevice, bufRead, 1024, &dwRead, NULL);
	printf("Read done!\n");
	WriteFile(hDevice, bufWrite, (wcslen(bufWrite) + 1) * sizeof(WCHAR), &dwWrite, NULL);

	printf("Write done!\n");

	TCHAR bufInput[1024] = L"Hello,world";
	TCHAR bufOutput[1024] = { 0 };
	DWORD dwRet = 0;

	WCHAR bufFileInput[1024] = L"C:\\docs\\hi.txt";

	DeviceIoControl(
		hDevice,
		PRINT_CODE,
		bufInput,
		sizeof(bufFileInput),
		bufOutput,
		sizeof(bufOutput),
		&dwRet,
		NULL);
	DeviceIoControl(
		hDevice,
		WRITE_HELLO,
		NULL,
		0,
		NULL,
		0,
		&dwRet,
		NULL);
	DeviceIoControl(
		hDevice,
		BYE_CODE,
		NULL,
		0,
		NULL,
		0,
		&dwRet,
		NULL);

	printf("DeviceIoControl done!\n");
	CloseHandle(hDevice);
}

int main()
{
	//��������
	BOOL bRet = LoadDriver(DEVICE_NAME, DRIVER_PATH);
	if (!bRet)
	{
		printf("LoadNTDriver error\n");
		return 0;
	}
	//���سɹ�
	printf("press any key to create device!\n");
	getchar();
	TestDriver();

	//��ʱ���ͨ��ע������������鿴�������ӵ�������֤
	printf("press any key to stop service!\n");
	_getch();

	//ж������
	bRet = UnloadDriver(DEVICE_NAME);
	if (!bRet)
	{
		printf("UnloadNTDriver error!\n");
		return 0;
	}
	return 0;
}
