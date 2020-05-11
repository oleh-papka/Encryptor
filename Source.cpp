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

char path_PlainText[255];
char path_Copy[255];
char path_key[255];

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
	char answer;
	std::cin >> answer; // Get the answer

	if (answer == 'e') 
	{
		encryption_flag = 1;
		std::cout << "ok, encryption" << std::endl;
		Copy_in_BIN();	// Gets file and creates copy in .bin
		Key_Logic(); // Gets the key
	}
	else if (answer == 'd')
	{
		decryption_flag = 1;
		std::cout << "ok, decryption soon" << std::endl;
	}
	else if (answer == 'F')	// Easter egg (why not?)
	{
		std::cout << "Thanks, you paid respect" << std::endl;
		Hello();
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

	//std::cout << "Key is : " << key << std::endl;
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


// Asks for key where it or enter int if you have
void Key_entering() {
	std::cout << "Is this a file (.txt) otherwise enter it.  [f] - for file, [e] - enter it" << std::endl;

	char ans;	// Does key in file or enter it
	std::cin >> ans;
	if (ans == 'f') {
		std::cout << "Enter location of your file: ";
		std::cin >> path_key;
		std::cout << "Path to key " << path_key << std::endl;
		std::cout << "Soon!" << std::endl;
	}
	else if (ans == 'e') {
		std::cout << "Enter your key: ";
		std::cin >> key;
		std::cout << "Your key " << key << std::endl;
		std::cout << "Soon!" << std::endl;
	}
	else {
		std::cout << "Sorry, but ->" << std::endl;
		Key_entering();
	}
}


// Should we generate a key for you if you dont have it
void Key_generating() {
	char answer;

	std::cout << "Should we generate it? [y] - for yes, [n] - for no" << std::endl;
	std::cin >> answer; // Get the answer

	if (answer == 'y') {
		Key_generator();
		std::cout << "Ok, we created a key for you:" << key << std::endl;
	}
	else if (answer == 'n')
	{
		std::string entered_key;
		std::cout << "Then create it by yourself use HEX base \n\t (16 numbers only, other inputs will be ignored)" << std::endl;

		while (entered_key.length() != 16 ) {
			std::cout << "Enter your key use HEX base (16 numbers only): ";
			std::cin >> entered_key;
			std::cout << "Entered: " << entered_key << std::endl;
		}

		key = entered_key;
		std::cout << "Your key is: " << key << std::endl;
	}
	else
	{
		std::cout << "Sorry, but ->" << std::endl;
		Key_generating();
	}
}


// Asking for key and what to do
void Key_Logic() {

	char answer;	// Does user have a key
	
	std::cout << "Do you have key? [y] - for yes, [n] - for no" << std::endl;
	std::cin >> answer; // Get the answer

	if (answer == 'y') {
		Key_entering();
	}
	else if (answer == 'n') {
		Key_generating();
	}
	else {
		std::cout << "Sorry, but ->" << std::endl;
		Key_Logic();
	}
}




//========================================================
//========================================================




//========================================================
//						 Main
//========================================================

int main() {

	std::cout << "======= Hello! =======" << std::endl;
	
	//Hello();

	//Key_Logic();

	return 0;
}

//========================================================
//========================================================