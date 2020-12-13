#include "CommandHandler.h"

int CommandHandler::runThread(char* command)
{
	HANDLE hThread = CreateThread(
		NULL,    // Thread attributes
		0,       // Stack size (0 = use default)
		(LPTHREAD_START_ROUTINE)ExecuteCommand(command), // Thread start address
		NULL,    // Parameter to pass to the thread
		0,       // Creation flags
		NULL);   // Thread id

	if (hThread != 0)
		WaitForSingleObject(hThread, INFINITE);
	else
		printf("THREAD CREATION FAILED\n");
	return 0;
}

int CommandHandler::ExecuteCommand(char* command)
{
	/*
	Exemplu de comenzi:
		. / client createfile a.txt
		. / client append a.txt ”hello world”
		. / client listdir D : \Facultate
		. / client run C : \Windows\system32\calc.exe
	*/
	Instruction currentInstruction;
	std::stringstream dataStream(command);
	std::string dataPiece;

	std::getline(dataStream, dataPiece, ' ');
	currentInstruction.instr = dataPiece;

	while (std::getline(dataStream, dataPiece, ' ')) {
		currentInstruction.args.push_back(dataPiece);
	}
	
	if (currentInstruction.instr == "createfile") {
		int result = createFile(currentInstruction.args.back().c_str());
	}
	else if (currentInstruction.instr == "append") {
		std::string textToBeAppended = currentInstruction.args.back();
		const char* fileName = currentInstruction.args.front().c_str();
		int result = appendToFile(fileName, textToBeAppended);
	}
	else if (currentInstruction.instr == "listdir") {
		listDirectory(currentInstruction.args.front());
	}
	else if (currentInstruction.instr == "run") {
		int result = runFile(currentInstruction.args.front());
	}
	else if (currentInstruction.instr == "deletefile") {
		int result = deleteFile(currentInstruction.args.back().c_str());
	}
	else if (currentInstruction.instr == "httpget") {
		httpGet(currentInstruction.args.front());
	}
	else if (currentInstruction.instr == "createreg") {
		int value = atoi(currentInstruction.args.back().c_str());
		currentInstruction.args.pop_back();
		std::string name = currentInstruction.args.back();
		currentInstruction.args.pop_back();
		std::string path = currentInstruction.args.back();
		createRegistry(path, name, value);
	}
	return 0;
}

int CommandHandler::createFile(const char* fileName)
{
	HANDLE hFile = CreateFileA(fileName,
		FILE_ALL_ACCESS,
		0,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Could not create the file\n");
		return GetLastError();
	}
	CloseHandle(hFile);
	return 0;
}

int CommandHandler::deleteFile(const char* fileName)
{
	return DeleteFileA(fileName);
}

int CommandHandler::appendToFile(const char* fileName, std::string textToBeAppended){
	HANDLE hFile = CreateFileA(fileName,
		FILE_APPEND_DATA,
		0,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("Could not open or create the file\n");
		return GetLastError();
	}

	DWORD numberOfBytesWritten;
	WriteFile(hFile, textToBeAppended.c_str(), textToBeAppended.length(), &numberOfBytesWritten, NULL);

	CloseHandle(hFile);

	return numberOfBytesWritten;
}

int CommandHandler::httpGet(std::string fileAddressPath) {
	// students.info.uaic.ro/~george.popoiu/CSSO/info.txt
	HINTERNET hInternet = InternetOpenA("High Quality Application", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	std::string fileName = fileAddressPath.substr(fileAddressPath.find_last_of("/\\") + 1);
	std::string linkPath = fileAddressPath.substr(0, fileAddressPath.length() - fileName.length());
	std::string serverAddr = fileAddressPath.substr(0, fileAddressPath.find_first_of("/\\"));
	linkPath = linkPath.substr(serverAddr.length() + 1, linkPath.length() - serverAddr.length());

	std::cout << fileAddressPath << '\n' << serverAddr << '\n' << linkPath << '\n' << fileName << '\n';

	HINTERNET hConnectHTTP = InternetConnectA
	(
		hInternet,
		serverAddr.c_str(),
		INTERNET_DEFAULT_HTTP_PORT,
		NULL,
		NULL,
		INTERNET_SERVICE_HTTP,
		NULL,
		0
	);

	LPCSTR rgpszAcceptTypes[] = { "*/*", NULL };
	std::string requestPath = linkPath + fileName;
	HINTERNET hRequest = HttpOpenRequestA
	(
		hConnectHTTP,
		"GET",
		requestPath.c_str(),
		NULL,
		NULL,
		rgpszAcceptTypes,
		NULL,
		0
	);

	if (HttpSendRequestA(hRequest, NULL, 0, NULL, 0) == true)
		std::cout << "GET Request Sent!" << std::endl;
	else
		std::cout << "GET Request Failed!" << GetLastError() << std::endl;

	const int bufferSize = 2048;
	char buffer[bufferSize];
	DWORD bytesRead;
	InternetReadFile(hRequest, buffer, bufferSize * sizeof(char), &bytesRead);
	buffer[bytesRead] = NULL;
	std::cout << "Got response: " << std::endl;
	for (int i = 0; i < bytesRead; i++) {
		std::cout << buffer[i];
	}

	createFile(fileName.c_str());
	appendToFile(fileName.c_str(), std::string(buffer));
	return 0;
}

int CommandHandler::runFile(std::string filePath)
{
	STARTUPINFO info;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&info, sizeof(info));
	info.cb = sizeof(info);
	ZeroMemory(&processInfo, sizeof(processInfo));
	// Run the downloaded file as a new process
	CreateProcess
	(
		std::wstring(filePath.begin(), filePath.end()).c_str(),
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
	return 0;
}

bool isDirectory(WIN32_FIND_DATAA findData) {
	if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return true;
	return false;
}

void parseDirectory(LPSTR currentPath) {
	WIN32_FIND_DATAA  findData;
	HANDLE localDirectoryHandle = FindFirstFileA(currentPath, &findData);
	currentPath[strlen(currentPath) - 1] = '\0';		// "C:\Users\Dorian\Documents\Facultate\CSSO\CSSO-tema1\Debug\"

	if (isDirectory(findData)) {						// is a directory
		if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
			printf("Directory at: %s%s\n", currentPath, findData.cFileName);
		}
		else {
			DWORD pathLength = strlen(currentPath);
			DWORD j = 0;
			DWORD finalLength = pathLength + strlen(findData.cFileName);
			for (DWORD i = pathLength - 1; i <= finalLength; i++) {
				currentPath[i] = findData.cFileName[j];
				j++;
			}
			printf("Directory at: %s\n", currentPath);
			currentPath[finalLength] = '\\';
			currentPath[finalLength + 1] = '*';
			currentPath[finalLength + 2] = '\0';		// "C:\Users\Dorian\Documents\Facultate\CSSO\CSSO-tema1\Debug\a\*"

			parseDirectory(currentPath);
			currentPath[pathLength] = '\0';				// "C:\Users\Dorian\Documents\Facultate\CSSO\CSSO-tema1\Debug\"
		}
	}
	else {												// is a file
		printf("File at: %s%s\n", currentPath, findData.cFileName);
	}

	while (FindNextFileA(localDirectoryHandle, &findData)) {
		if (isDirectory(findData)) {					// is a directory
			if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
				printf("Directory at: %s%s\n", currentPath, findData.cFileName);
			}
			else {
				DWORD pathLength = strlen(currentPath);
				DWORD j = 0;
				DWORD finalLength = pathLength + strlen(findData.cFileName);
				for (DWORD i = pathLength; i < finalLength; i++) {
					currentPath[i] = findData.cFileName[j];
					j++;
				}
				printf("Directory at: %s\n", currentPath);
				currentPath[finalLength] = '\\';
				currentPath[finalLength + 1] = '*';
				currentPath[finalLength + 2] = '\0';

				parseDirectory(currentPath);
				currentPath[pathLength] = '\0';
			}
		}
		else {											// is a file
			printf("File at: %s%s\n", currentPath, findData.cFileName);
		}
	}

	FindClose(localDirectoryHandle);
}

int CommandHandler::listDirectory(std::string path)
{
	path += "\\*";
	LPSTR directoryPath = new char[MAX_PATH];
	const char* cPath = path.c_str();
	directoryPath = (LPSTR)cPath;
	printf(directoryPath);
	parseDirectory(directoryPath);
	return 0;
}

LPSTR preparePath(LPSTR path) {
	LPSTR finalPath = new char[MAX_PATH];
	strcpy(finalPath, "Software\\");
	LPSTR p = strstr(path, "CSSO");
	if (!p)	return NULL;

	strcat(finalPath, p);
	return	finalPath;
}

HKEY createRegistryKey(LPSTR path) {
	HKEY hKey;
	DWORD errorCode = RegCreateKeyExA(
		HKEY_CURRENT_USER,
		preparePath(path),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS | KEY_WOW64_64KEY,
		NULL,
		&hKey,
		NULL
	);
	if (errorCode != ERROR_SUCCESS)
		printf("Registry subkey creation failed!\n");
	else
		printf("Registry subkey creation successful for: %s\n", path);

	return hKey;
}

void createRegistryValue(HKEY hKey, LPSTR name, DWORD value) {
	DWORD errorCode = RegSetValueExA(
		hKey,
		name,
		NULL,
		REG_DWORD,
		(LPBYTE)&value,
		sizeof(DWORD)
	);
	if (errorCode != ERROR_SUCCESS)
		printf("Registry value creation failed!\n");
	else
		printf("Registry value creation successful for: NAME: %s and VALUE: %d\n", name, value);
}

int CommandHandler::createRegistry(std::string path, std::string name, int value)
{
	HKEY hKey = createRegistryKey((LPSTR)path.c_str());
	createRegistryValue(hKey, (LPSTR)name.c_str(), value);

	return 0;
}

