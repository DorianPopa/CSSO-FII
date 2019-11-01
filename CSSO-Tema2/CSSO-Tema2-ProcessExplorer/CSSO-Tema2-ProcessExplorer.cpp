// CSSO-Tema2-ProcessExplorer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <list>
#include <sstream>

using namespace std;

struct InfoProcess {
	DWORD pid;
	DWORD ppid;
	string exeName;
	list<InfoProcess> children;
};

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		printf("LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		printf("AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		printf("The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}


InfoProcess splitProcessLine(string processLine) {
	stringstream ss(processLine);
	string dataPiece;
	InfoProcess newProcess;

	getline(ss, dataPiece, '|');
	newProcess.pid = atoi(dataPiece.c_str());
	getline(ss, dataPiece, '|');
	newProcess.ppid = atoi(dataPiece.c_str());
	getline(ss, dataPiece, '|');
	newProcess.exeName = dataPiece;

	return newProcess;
}

list<InfoProcess> splitFileData(unsigned char* rawData) {
	stringstream ss((char*)rawData);
	string singleProcess;
	list<InfoProcess> returnList;

	while (getline(ss, singleProcess, '\n')) {
		returnList.push_back(splitProcessLine(singleProcess));
	}
	return returnList;
}

bool compareProcessesByPID(InfoProcess a, InfoProcess b) {
	return a.pid < b.pid;
}

void killProcess(DWORD processId) {
	HANDLE processToKill;
	processToKill = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	TerminateProcess(processToKill, 1);
}

list<InfoProcess> getChildren(InfoProcess currentProcess, list<InfoProcess> processList) {
	list<InfoProcess> childrenList;
	for (auto p : processList) {
		if (p.ppid == currentProcess.pid && p.pid != currentProcess.pid) {
			childrenList.push_back(p);
		}
	}
	return childrenList;
}


void printProcessWithChildren(InfoProcess &pToPrint, int depth, int treeNumber, list<InfoProcess> &processList) {
	if (depth == 0)
		cout << endl << "[Tree number: " << treeNumber << "]" << endl;
	for (int i = 0; i < depth; i++) {
		cout << '\t';
	}
	cout << "PID: " << pToPrint.pid << " PPID: " << pToPrint.ppid << " NAME: " << pToPrint.exeName << endl;
	list<InfoProcess> globalChildrenList = getChildren(pToPrint, processList);
	pToPrint.children = globalChildrenList;
	for (auto p : globalChildrenList) {
		printProcessWithChildren(p, depth + 1, treeNumber, processList);
	}
}

void printTrees(list<InfoProcess> &pList) {
	int numberOfTrees = 0;

	for (list<InfoProcess>::iterator it = pList.begin(); it != pList.end(); it++) {
		int parentPID = it->ppid;
		bool isRoot = true;
		for (auto p : pList) {
			if (p.pid == parentPID && p.pid != 0)
				isRoot = false;
		}
		if (isRoot) {
			numberOfTrees++;
			printProcessWithChildren(*it, 0, numberOfTrees, pList);
		}	
	}
}

int countChildren(InfoProcess process) {
	int currentChildrenSize = process.children.size();
	for (auto p : process.children) {
		currentChildrenSize += countChildren(p);
	}
	return currentChildrenSize;
}

void recursiveKill(int pid, list<InfoProcess> pList) {
	for (auto p : pList) {
		if (p.pid == pid) {
			for (auto child : p.children) {
				recursiveKill(child.pid, pList);
			}
			killProcess(pid);
		}
	}
}

void userInput(list<InfoProcess> pList) {
	cout << endl <<  "1: Se citeste un nume de proces de la tastatura si se afiseaza pentru fiecare proces cu acel nume existent pe sistem pid - ul acestuia si numarul de descendenti." << endl;
	cout << endl << "2: Se citeste pid-ul unui proces si se vor inchide toate procesele din subarborele care are radacina in acel PID." << endl;
	int option;
	cin >> option;
	if (option == 1) {
		cout << "Enter process name: ";
		string processName;
		cin >> processName;

		list<InfoProcess> processListNameMatch;
		for (auto p : pList) {
			if (p.exeName == processName)
				processListNameMatch.push_back(p);
		}
		for (auto p : processListNameMatch) {
			cout << "PID: " << p.pid << " Number of children: " << countChildren(p) << endl;
		}
	}
	else if (option == 2) {
		cout << "Enter process PID: ";
		int pid;
		cin >> pid;
		recursiveKill(pid, pList);
	}
	else {
		cout << endl << "Bad input" << endl << endl;
		userInput(pList);
	}
	userInput(pList);
}


int main()
{
	SetPrivilege(GetCurrentProcess(), L"SeDebugPrivilege", true);	// not working


	HANDLE hFileMap = OpenFileMapping(FILE_MAP_READ, FALSE, L"ProcessesFile");
	if (hFileMap == 0)	
		return -1;

	unsigned char* fileData = NULL;
	fileData = (unsigned char*)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	if (fileData == 0)	
		return -1;
	
	list<InfoProcess>	processList;
	processList = splitFileData(fileData);

	processList.sort(compareProcessesByPID);

	for (list<InfoProcess>::iterator currentProcess = processList.begin(); currentProcess != processList.end(); currentProcess++) {
		currentProcess->children = getChildren(*currentProcess, processList);
	}

	printTrees(processList);
	userInput(processList);
	return 0;
}
