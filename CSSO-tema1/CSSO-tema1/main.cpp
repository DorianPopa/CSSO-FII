#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <string.h>

#define REGISTRY_BASE_PATH "Software\\"

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
	DWORD errorCode = RegCreateKeyEx(
		HKEY_CURRENT_USER,
		preparePath(path),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS| KEY_WOW64_64KEY,
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
	DWORD errorCode = RegSetValueEx(
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

bool isDirectory(WIN32_FIND_DATA findData) {
	if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		return true;
	return false;
}

void parseDirectory(LPSTR currentPath, HKEY currentHKey) {
	WIN32_FIND_DATA findData;
	HANDLE localDirectoryHandle = FindFirstFile(currentPath, &findData);
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
			HKEY hKey = createRegistryKey(currentPath);
			currentPath[finalLength] = '\\';
			currentPath[finalLength + 1] = '*';
			currentPath[finalLength + 2] = '\0';		// "C:\Users\Dorian\Documents\Facultate\CSSO\CSSO-tema1\Debug\a\*"

			parseDirectory(currentPath, hKey);
			currentPath[pathLength] = '\0';				// "C:\Users\Dorian\Documents\Facultate\CSSO\CSSO-tema1\Debug\"
		}
	}
	else {												// is a file
		printf("File at: %s%s\n", currentPath, findData.cFileName);
		createRegistryValue(currentHKey, findData.cFileName, findData.nFileSizeHigh);
	}

	while (FindNextFile(localDirectoryHandle, &findData)) {
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
				HKEY hKey = createRegistryKey(currentPath);
				currentPath[finalLength] = '\\';
				currentPath[finalLength + 1] = '*';
				currentPath[finalLength + 2] = '\0';

				parseDirectory(currentPath, hKey);
				currentPath[pathLength] = '\0';
			}
		}
		else {											// is a file
			printf("File at: %s%s\n", currentPath, findData.cFileName);
			createRegistryValue(currentHKey, findData.cFileName, findData.nFileSizeLow);
		}
	}

	FindClose(localDirectoryHandle);
	RegCloseKey(currentHKey);
}

int main() {
	LPSTR executablePath = new char[MAX_PATH];
	DWORD sizeOfPath;
	HKEY initialHKEY = (HKEY)INVALID_HANDLE_VALUE;

	if ((sizeOfPath = GetModuleFileNameA(NULL, executablePath, sizeof(char) * MAX_PATH)) != 0) {
		sizeOfPath--;									// "C:\Users\Dorian\Documents\Facultate\CSSO\CSSO-tema1\Debug\CSSO-tema1.exe"
		BYTE character = executablePath[sizeOfPath];
		while (character != '\\') {
			executablePath[sizeOfPath] = '\0';
			sizeOfPath--;
			character = executablePath[sizeOfPath];
		}
		initialHKEY = createRegistryKey(executablePath);
		executablePath[sizeOfPath + 1] = '*';
		executablePath[sizeOfPath + 2] = '\0';			// "C:\Users\Dorian\Documents\Facultate\CSSO\CSSO-tema1\Debug\*"
	}
	parseDirectory(executablePath, initialHKEY);

	return 0;
}
