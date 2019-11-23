// CSSO-Tema3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>

#define NUMBER_OF_WRITES	200
#define FILE_MAP_NAME		L"DATA_FILE"
#define EVENT_WRITE			L"FIRST_PROCESS_WRITES"
#define EVENT_READ			L"SECOND_PROCESS_READS"

using namespace std;

HANDLE createMappedFile(LPCWSTR fileName, DWORD fileSize) {
	HANDLE hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, fileSize, fileName);
	if(hFileMap == 0)
		return INVALID_HANDLE_VALUE;
	return hFileMap;
}

int main()
{
	HANDLE hEventWrite = CreateEvent(NULL, false, false, EVENT_WRITE);
	HANDLE hEventRead = CreateEvent(NULL, false, true, EVENT_READ);
	if (hEventWrite == 0 || hEventRead == 0)
		return -1;

	HANDLE hFileMap = createMappedFile(FILE_MAP_NAME, 2 * sizeof(int));
	int* pData = (int*)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);

	int* fileData = (int*)malloc(2 * sizeof(int));
	for (int i = 1; i <= NUMBER_OF_WRITES; i++) {
		fileData[0] = i;
		fileData[1] = 2 * i;
		// Wait for SECOND_PROCESS_READS to signal. The other process has finished reading
		WaitForSingleObject(hEventRead, INFINITE);	
		/////////////////////////////////
			if (pData != NULL && fileData != NULL) {
				memcpy(pData, fileData, 2 * sizeof(int));
				cout << "Wrote:" << fileData[0] << " " << fileData[1] << endl;
			}
		/////////////////////////////////
		// Finished writing. Signal FIRST_PROCESS_WRITES. The other process can begin to read
		if (!SetEvent(hEventWrite)) cout << endl << "Could not set signal" << endl;
	}
	return 0;
}
