// CSSO-Tema3_SyncProcess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>


#define NUMBER_OF_WRITES	200
#define FILE_MAP_NAME		L"DATA_FILE"
#define EVENT_WRITE			L"FIRST_PROCESS_WRITES"
#define EVENT_READ			L"SECOND_PROCESS_READS"

using namespace std;

int* openMappedFile(HANDLE hFileMap) {
	int* fileData = (int*)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	if (fileData == nullptr)
		return NULL;
	return fileData;
}

int main()
{
	HANDLE hFileMap = OpenFileMapping(FILE_MAP_READ, FALSE, FILE_MAP_NAME);
	if (hFileMap == 0)
		return -1;

	HANDLE hEventWrite = OpenEvent(EVENT_ALL_ACCESS, false, EVENT_WRITE);
	HANDLE hEventRead = OpenEvent(EVENT_ALL_ACCESS, false, EVENT_READ);
	if (hEventWrite == 0 || hEventRead == 0)
		return -1;

	int* fileData = openMappedFile(hFileMap);
	for (int i = 1; i <= NUMBER_OF_WRITES; i++) {
		// Wait for FIRST_PROCESS_WRITES to signal. The other process has finished writing
		WaitForSingleObject(hEventWrite, INFINITE);
		/////////////////////////////////
			if (2 * fileData[0] == fileData[1])
				cout << "Read:" << fileData[0] << " " << fileData[1] << " Correct" << endl;
			else
				cout << "Incorrect" << endl;
		/////////////////////////////////
		// Finished reading. Signal SECOND_PROCESS_READS. The other process can begin to write
		if(!SetEvent(hEventRead)) cout << endl << "Could not set signal" << endl;
	}
	CloseHandle(hFileMap);
	CloseHandle(hEventWrite);
	CloseHandle(hEventRead);

	return 0;
}
