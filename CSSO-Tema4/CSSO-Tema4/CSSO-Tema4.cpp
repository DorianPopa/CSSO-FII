// CSSO-Tema4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma comment(lib, "Wininet")
#include <Windows.h>
#include <wininet.h>
#include <iostream>
#include <string>
#include <sstream>
#include <list>

struct Instruction {
	std::string instr;
	std::string arg;
};

struct InstructionSet {
	DWORD nrOfInstr = 0;
	std::string addr;
	std::string usr;
	std::string pwd;
	std::list<Instruction> instrList;
};

// Function to populate an InstructionSet struct 
InstructionSet createInstructionSet(LPSTR data, DWORD sizeOfData) {
	InstructionSet returnSet;
	std::stringstream dataStream(data);
	std::string dataPiece;

	getline(dataStream, dataPiece);
	returnSet.nrOfInstr = atoi(dataPiece.c_str());
	getline(dataStream, dataPiece);
	returnSet.addr = dataPiece;
	getline(dataStream, dataPiece);
	returnSet.usr = dataPiece;
	getline(dataStream, dataPiece);
	returnSet.pwd = dataPiece;

	for (int i = 0; i < returnSet.nrOfInstr; i++) {
		getline(dataStream, dataPiece);
		std::string instrSplit;
		std::stringstream instrStream(dataPiece);
		getline(instrStream, instrSplit, ' ');
		Instruction currentInstruction;
		currentInstruction.instr = instrSplit;
		getline(instrStream, instrSplit, ' ');
		currentInstruction.arg = instrSplit;
		returnSet.instrList.push_back(currentInstruction);
	}
	return returnSet;
}

void ExecuteInstructionsFromFTPServer(InstructionSet executeSet, HINTERNET hFPTServer) {
	for (Instruction currentInstruction : executeSet.instrList) {
		if (currentInstruction.instr == "PUT") {
			// Get the filename
			std::string base_filename = currentInstruction.arg.substr(currentInstruction.arg.find_last_of("/\\") + 1);
			// Send the file to the server in binary format
			FtpPutFile
			(
				hFPTServer, 
				std::wstring(currentInstruction.arg.begin(), currentInstruction.arg.end()).c_str(),
				std::wstring(base_filename.begin(), base_filename.end()).c_str(),
				FTP_TRANSFER_TYPE_BINARY, 
				0
			);
		}
		else if (currentInstruction.instr == "RUN") {
			std::string base_filename = currentInstruction.arg.substr(currentInstruction.arg.find_last_of("/\\") + 1);
			FtpGetFile
			(
				hFPTServer,
				std::wstring(currentInstruction.arg.begin(), currentInstruction.arg.end()).c_str(),
				std::wstring(base_filename.begin(), base_filename.end()).c_str(),
				true,
				FILE_ATTRIBUTE_NORMAL,
				FTP_TRANSFER_TYPE_BINARY,
				0
			);

			// init dummy info structs
			STARTUPINFO info;
			PROCESS_INFORMATION processInfo;
			ZeroMemory(&info, sizeof(info));
			info.cb = sizeof(info);
			ZeroMemory(&processInfo, sizeof(processInfo));
			// Run the downloaded file as a new process
			CreateProcess
			(
				std::wstring(base_filename.begin(), base_filename.end()).c_str(),
				NULL,
				NULL,
				NULL,
				FALSE,
				NULL,
				NULL,
				NULL,
				&info,
				&processInfo
			);
		}
	}
}

int main()
{
	// Connect to the HTTP server
	HINTERNET hInternet = InternetOpen(L"High Quality Application", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	HINTERNET hConnectHTTP = InternetConnect
	(
		hInternet,
		L"students.info.uaic.ro",
		INTERNET_DEFAULT_HTTP_PORT,
		NULL,
		NULL,
		INTERNET_SERVICE_HTTP,
		NULL,
		0
	);

	// Init Request
	LPCTSTR rgpszAcceptTypes[] = { L"text/*", NULL };
	HINTERNET hRequest = HttpOpenRequest
	(
		hConnectHTTP,
		L"GET",
		L"~george.popoiu/CSSO/info.txt",
		NULL,
		NULL,
		rgpszAcceptTypes,
		NULL,
		0
	);

	// Send Request
	if (HttpSendRequest(hRequest, NULL, 0, NULL, 0) == true)
		std::cout << "Request Sent!" << std::endl;
	else
		std::cout << "Request Failed!" << GetLastError() << std::endl;

	// Read Request Response
	char buffer[256];
	DWORD bytesRead;
	InternetReadFile(hRequest, buffer, 256 * sizeof(char), &bytesRead);
	buffer[bytesRead] = NULL;
	std::cout << "Got response: " << std::endl;
	for (int i = 0; i < bytesRead; i++) {
		std::cout << buffer[i];
	}

	// Create the instruction set from the response data
	InstructionSet instructionSet = createInstructionSet(buffer, bytesRead);

	///////// Test Data
	InstructionSet test;
	test.addr = "localhost";
	test.usr = "1337h4x0r";
	test.pwd = "default";
	//////////

	// Connect to the FTP server
	HINTERNET hConnectFTP = InternetConnect
	(
		hInternet,
		std::wstring(test.addr.begin(),test.addr.end()).c_str(),
		INTERNET_DEFAULT_FTP_PORT,
		std::wstring(test.usr.begin(), test.usr.end()).c_str(),
		std::wstring(test.pwd.begin(), test.pwd.end()).c_str(),
		INTERNET_SERVICE_FTP,
		NULL,
		0
	);

	ExecuteInstructionsFromFTPServer(instructionSet, hConnectFTP);

	InternetCloseHandle(hConnectFTP);
	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnectHTTP);
	InternetCloseHandle(hInternet);
	return 0;
}
