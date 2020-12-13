#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <Tlhelp32.h>
#include <iostream>
#include <cstdlib> // wcstombs
#include <list>

using namespace std;

struct InfoProcess {
	DWORD pid;
	DWORD ppid;
	char  exeName[256];
};

struct ProcessList {
	int					count = 0;
	list<InfoProcess>	processList;
};

bool compareProcessesByPPID(InfoProcess a, InfoProcess b) {
	return a.ppid < b.ppid;
}

int main()
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot failed.err = %d \n", GetLastError());
		return(-1);
	}


	ProcessList processList;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		printf("Process32First failed. err = %d \n", GetLastError());
		CloseHandle(hProcessSnap);
		return(-1);
	}
	do
	{
		InfoProcess currentProcess;
		currentProcess.pid = pe32.th32ProcessID;
		currentProcess.ppid = pe32.th32ParentProcessID;
		int size = wcstombs(currentProcess.exeName, pe32.szExeFile, sizeof(currentProcess.exeName));
		currentProcess.exeName[size] = '\0';

		processList.processList.push_back(currentProcess);
	} while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);

	string bufferToWriteToFile = "";
	for (list<InfoProcess>::iterator it = processList.processList.begin(); it != processList.processList.end(); it++) {
		char temp[16];
		_itoa(it->pid, temp, 10);
		bufferToWriteToFile += temp;
		bufferToWriteToFile += "|";

		_itoa(it->ppid, temp, 10);
		bufferToWriteToFile += temp;
		bufferToWriteToFile += "|";

		bufferToWriteToFile += it->exeName;
		bufferToWriteToFile += '\n';
	}

	cout << bufferToWriteToFile;
	unsigned char* pData = NULL;
	HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, bufferToWriteToFile.length(), L"ProcessesFile");
	if (hFileMap != 0) {
		pData = (unsigned char*)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);
		if( pData != NULL)
			memcpy(pData, bufferToWriteToFile.c_str(), bufferToWriteToFile.length());
	}
	
	if (pData != NULL)
		cout << "--------------------------FILE IN MEMORY:" << endl << pData;
	cin.get();
	return 0;
}