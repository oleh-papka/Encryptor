#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <Windows.h>
#include <windows.h>
#include <Shlwapi.h>
#include <string>
#include <sstream>
#include <cstring>
#include <bitset>
#include <random>

#pragma comment(lib, "shlwapi.lib")


//========================================================
//						Variables
//========================================================

bool encryption_flag = 0;
bool decryption_flag = 0;

char answer;

char path_PlainText[255];
char path_Copy[255];

std::string key; // Main key
std::string bin_key; // Main key in binary

//========================================================
//========================================================



//========================================================
//						Functions
//========================================================

// Gets direction of execytable file
std::string GetWorkingDir() {
	char path[MAX_PATH] = "";
	GetCurrentDirectoryA(MAX_PATH, path);
	PathAddBackslashA(path);
	return path;
}


// Creates a copy of the PlainText in Copy.bin file to process it
void Copy_in_BIN (){

	std::cout << "Please enter path to the file for encryption (PlainText): ";
	std::cin >> path_PlainText;

	std::string path_Copy = GetWorkingDir();

	path_Copy += "Copy.bin";

	std::ifstream Readfile (path_PlainText, std::ifstream::binary);
	std::ofstream Copyfile (path_Copy,  std::fstream::binary);
	
	Copyfile << Readfile.rdbuf();

	Readfile.close();
	Copyfile.close();
}


// Choose encryptioon or decryption
void Hello() {

	std::cout << "For encryption enter [e] , for decryption [d]"<< std::endl;
	//answer = getchar();
	std::cin >> answer; // Get the answer

	if (answer == 'e') 
	{
		encryption_flag = 1;
		std::cout << "ok, encryption" << std::endl;
		Copy_in_BIN();	// Gets file and creates copy in .bin
	}
	else if (answer == 'd')
	{
		decryption_flag = 1;
		std::cout << "ok, decryption soon" << std::endl;
	}
	else if (answer == 'F')
	{
		std::cout << "Thanks, you paid respect" << std::endl;
	}
	else
	{
		std::cout << "Sorry, but ->" << std::endl;
		Hello();
	}

}


// Random key generator (Generates 64-bit HEX key)
void Key_generator(){

	char hex_numbers[] = "0123456789ABCDEF";	// all numbers of hex base system to make HEX key

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(0, 15); // distribution in range [1, 16]

	for (int i = 0; i < 16; i++){
		key += hex_numbers[dist6(rng)];
	}

	std::cout << "Key is : " << key << std::endl;
}


// Converts HEX string to "0" and "1"
void HEX2BIN(std::string str) {

	for (int i = 0; i < str.length(); i++)
	{
		switch (str[i])
		{
		case '0':
			bin_key += "0000";
			break;
		case '1':
			bin_key += "0001";
			break;
		case '2':
			bin_key += "0010";
			break;
		case '3':
			bin_key += "0011";
			break;
		case '4':
			bin_key += "0100";
			break;
		case '5':
			bin_key += "0101";
			break;
		case '6':
			bin_key += "0110";
			break;
		case '7':
			bin_key += "0111";
			break;
		case '8':
			bin_key += "1000";
			break;
		case '9':
			bin_key += "1001";
			break;
		case 'A':
			bin_key += "1010";
			break;
		case 'B':
			bin_key += "1011";
			break;
		case 'C':
			bin_key += "1100";
			break;
		case 'D':
			bin_key += "1101";
			break;
		case 'E':
			bin_key += "1110";
			break;
		case 'F':
			bin_key += "1111";
			break;

		default:
			break;
		}

	}
	std::cout << bin_key << std::endl;
}


// Converts DEC string to "0" and "1"
void DEC2BIN(std::string str) {
	for (int i = 0; i < str.length(); i++)
	{
		switch (str[i])
		{
		case '0':
			bin_key += "0000";
			break;
		case '1':
			bin_key += "0001";
			break;
		case '2':
			bin_key += "0010";
			break;
		case '3':
			bin_key += "0011";
			break;
		case '4':
			bin_key += "0100";
			break;
		case '5':
			bin_key += "0101";
			break;
		case '6':
			bin_key += "0110";
			break;
		case '7':
			bin_key += "0111";
			break;
		case '8':
			bin_key += "1000";
			break;
		case '9':
			bin_key += "1001";
			break;

		default:
			break;
		}
	}
	std::cout << bin_key << std::endl;
}


//========================================================
//========================================================




//========================================================
//						 Main
//========================================================

int main() {

	std::cout << "======= Hello! =======" << std::endl;
	
	//Hello();
	
	//Key_generator();
	
	//HEX2BIN(key);


	return 0;
}

//========================================================
//========================================================