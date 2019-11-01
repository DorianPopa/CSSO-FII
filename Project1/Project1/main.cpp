#include <Windows.h>
#include <stdio.h>

void f(DWORD* dw) {

}

void g(LPDWORD dw) {

}

int main() {
	BYTE b;     // unsigned char b;
	WORD w;     // unsigned short w;
	DWORD dw;   // unsigned int dw;
	TCHAR c;    // char c;

	//LPSTR s = "adasdads";     // char*
	LPCSTR s2 = "sdadsdasd";  // const char*

	HANDLE h = CreateFile(
		"a.txt",                // lpFileName
		GENERIC_READ,           // dwDesiredAccess
		0,                      // dwShareMode
		NULL,                   // lpSecurityAttributes
		OPEN_EXISTING,          // dwCreationDisposition
		FILE_ATTRIBUTE_NORMAL,  // dwFlagsAndAttributes
		INVALID_HANDLE_VALUE);  // hTemplateFile
	if (h == INVALID_HANDLE_VALUE) {
		printf("Cannot open file. Error code: %d", GetLastError());
	}
	else {
		char text[10];
		DWORD citite;
		while (true) {
			ReadFile(h, text, 9, &citite, NULL);
			if (citite == 0) {
				break;
			}
			text[citite] = '\0';
			printf("%s", text);
		}
		CloseHandle(h);
		printf("\n");
	}
	
	WIN32_FIND_DATA find_data;
	HANDLE hDir = FindFirstFile("C:\\*", &find_data);
	printf("%s\n", find_data.cFileName);

	while (FindNextFile(hDir, &find_data)) {
		printf("%s\n", find_data.cFileName);
	}

	FindClose(hDir);
	
	return 0;
}