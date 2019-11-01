#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>

struct InfoProces {
	DWORD pid;
	DWORD ppid;
	char  exeName[256];
};

struct ProcessList {
	int         count;
	InfoProces  procese[2048];
};

int demoLab3() {
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if (!CreateProcess((LPCWSTR)R"(C:\Windows\System32\calc.exe)", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Cannot create process.\n");
		return 0;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	HANDLE hData = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024 * 1024, (LPCWSTR)R"(data)");
	if (hData == NULL) {
		printf("Cannot create file mapping. Error code: %d", GetLastError());
		return 0;
	}

	unsigned char* pData = (unsigned char*)MapViewOfFile(hData, FILE_MAP_WRITE, 0, 0, 0);
	if (pData == NULL) {
		printf("Cannot get pointer to file mapping. Error code: %d", GetLastError());
		CloseHandle(hData);
		return 0;
	}

	InfoProces ip;
	ip.pid = pi.dwProcessId;
	ip.ppid = 0;
	strcpy(ip.exeName, "calc.exe");
	memcpy(pData, &ip, sizeof(InfoProces));

	CloseHandle(hData);
	return 0;
}