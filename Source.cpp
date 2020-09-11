#include <cstdio>
#include <windows.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <Shlwapi.h>
#include <string>
#include <bitset>
#include <random>
#include <cmath>

#pragma comment(lib, "shlwapi.lib")
#pragma warning(disable : 4996)

//========================================================
//						Variables
//========================================================

bool shutDown_flag = false;

bool copyCreated_flag = false;
bool outputCreated_flag = false;

bool decryption_flag = 0;

bool use3DES_flag = false;

bool useCBC_flag = false;
std::string IV64; // Initialization vector

int position = 0;	// For block reading
bool file_end_flag = 0; // To check if all file is read


std::string path_File_Save; // path to file with encrypted data


char path_PlainText[255];
std::string path_Copy;
char path_key[255];

std::string key; // Main key
std::string bin_key; // Main key in binary

std::string key56bit; // After PC1 key in binary
std::string key48bit; // After PC2 Sub key in binary

std::string subkeys_DES[16]; // All subkeys in one place for DES

// All subkeys in one place for 3DES
std::string subkeys_3DES1[16];
std::string subkeys_3DES2[16];
std::string subkeys_3DES3[16]; 

std::string C28; // "Left" part of key56bit
std::string D28; // "Right" part of key56bit

std::string L32; // "Left" part of data from buffer
std::string R32; // "Right" part of data from buffer

std::string Right_48bit; // after expansion


int row;	// For s-boxes
int column;	// For s-boxes


std::string data64; // Data block for encryption from buffer
char buffer[64];	//Buffer stores read data form data64


int completeBlockNum;	// How many blocks will be in file
int uncompleteBlockNum;	// How many bits are off the block
bool uncompleteBlock_flag = false;

long int size; // Size of .bin file in bits

double shift;
double percentage;


std::string key_part1;
std::string key_part2;
std::string key_part3;


//========================================================
//========================================================


// Choose 3DES or DES
void Use_3DES() {
	std::string answer;

	while (true) {
		std::cout << "=========== Use DES or 3DES? [1 / 3] ===========" << std::endl;
		std::cout << ">> ";
		std::cin >> answer;

		if (answer == "1") {
			std::cout << "================= Ok, using DES ================\n" << std::endl;
			break;
		}
		else if (answer == "3") {
			use3DES_flag = true;
			std::cout << "================ Ok, using 3DES ================\n" << std::endl;
			break;
		}
		else {
			system("CLS");
			std::cout << "===== Sorry, but ->" << std::endl;
		}
	}
}


// Random key generator (Generates 64-bit HEX key)
void IV_generator() {
	char hex_numbers[] = "01";	// all numbers of hex base system to make HEX key

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(0, 1);

	for (int i = 0; i < 64; i++) {
		IV64 += hex_numbers[dist6(rng)];
	}

	std::cout << "=== Your IV is: [" << IV64 << "] ===" << std::endl;
}


// Enter IV
void Enter_IV() {
	std::string temp_IV;

	while (true) {
		std::cout << "============ Enter your IV ===========" << std::endl;
		std::cout << ">> ";
		std::cin >> temp_IV;

		if (temp_IV.length() != 64) {
			std::cout << "=== Sorry, but IV must be 64 character ->" << std::endl;
			break;
		}
		else {
			IV64 = temp_IV;
			break;
		}
	}
}


// Choose CBC or EBC
void Use_CBC() {
	std::string answer;

	while (true) {
		std::cout << "============ Use CBC or EBC? [C / E] ===========" << std::endl;
		std::cout << ">> ";
		std::cin >> answer;

		if (answer == "C") {
			useCBC_flag = true;
			std::cout << "================= Ok, using CBC ================\n" << std::endl;	
			break;
		}
		else if (answer == "E") {
			std::cout << "================ Ok, using EBC ================\n" << std::endl;
			break;
		}
		else {
			system("CLS");
			std::cout << "===== Sorry, but ->" << std::endl;
		}
	}
}


// XOR of IV 64bits
std::string XOR_IV64bits(std::string Text, std::string IV) {
	std::string temp_string;

	for (int i = 0; i < 64; i++) {
		if (Text[i] == IV[i]) {
			temp_string += '0';
		}
		else {
			temp_string += '1';
		}
	}
	
	return temp_string;
}


// Check if there is nothing else in our key
bool Invalid_key(std::string Key) {
	bool invalid_key_flag = false;

	if (Key.find_first_not_of("abcdefABCDEF01234567890") != std::string::npos) {
		invalid_key_flag = true;
	}
	return invalid_key_flag;
}


// Check if all characters the same 
bool All_char_same(std::string Key) {
	bool all_char_same_flag = false;
	int n = 0;

	for (int i = 1; i < 16; i++) {
		if (Key[i] == Key[0]) {
			n++;
		}
	}

	if (n == 15) {
		all_char_same_flag = true;
	}

	return all_char_same_flag;
}


// Weak and half weak keys check
bool Weak_key_check_DES(std::string Key) {
	bool weak_flag;

	if (Key == "0101010101010101") { weak_flag = true; }
	else if (Key == "FEFEFEFEFEFEFEFE") { weak_flag = true; }
	else if (Key == "1F1F1F1F0E0E0E0E") { weak_flag = true; }
	else if (Key == "E0E0E0E0F1F1F1F1") { weak_flag = true; }
	else if (Key == "01FE01FE01FE01FE" || Key == "FE01FE01FE01FE01" ) { weak_flag = true; }
	else if (Key == "1FE01FE01FE01FE0" || Key == "E0F1E0F1E0F1E0F1" ) { weak_flag = true; }
	else if (Key == "01E001E001F101F1" || Key == "E001E001F101F101" ) { weak_flag = true; }
	else if (Key == "1FFE1FFE0EFE0EFE" || Key == "FE1FFE1FFE0EFE0E" ) { weak_flag = true; }
	else if (Key == "011F011F010E010E" || Key == "1F011F010E010E01" ) { weak_flag = true; }
	else if (Key == "E0FEE0FEF1FEF1FE" || Key == "FEE0FEE0FEF1FEF1" ) { weak_flag = true; }
	else if (All_char_same(Key)) { weak_flag = true; }
	else { weak_flag = false; }

	return weak_flag;
}


// Weak and half weak keys check 3DES
bool Weak_key_check_3DES() {
	bool weak_flag;

	if (Weak_key_check_DES(key_part1)) {
		weak_flag = true;
	}
	else if (Weak_key_check_DES(key_part2)) {
		weak_flag = true;
	}
	else if (Weak_key_check_DES(key_part3)) {
		weak_flag = true;
	}
	else {
		weak_flag = false;
	}
	
	return weak_flag;
}


// If our asking file exists
bool File_exist(std::string file_name) {
	bool file_exist;

	std::ifstream Exist_file(file_name);

	if (Exist_file) {
		file_exist = true;
	}
	else {
		file_exist = false;
	}

	Exist_file.close();

	return file_exist;
}


// Asks where to save file
void Where_to_save() {
	bool quit = false;

	while (!quit) {
		std::cout << "\n===== Please, enter path where to save file ====" << std::endl;
		std::cout << ">> ";
		std::cin >> path_File_Save;

		if (path_File_Save == path_PlainText) {
			system("CLS");
			std::cout << "===== Sorry, but You cannot do this way ->" << std::endl;
		}
		else {
			std::ifstream Exist_file(path_File_Save);
			if (Exist_file) {
				system("CLS");
				std::cout << "===== Sorry, but You cannot do this way ->" << std::endl;
			}
			else {
				quit = true;
			}
		}
	}
}


// Gets direction of executable file
std::string GetWorkingDir() {
	char path[MAX_PATH] = "";
	GetCurrentDirectoryA(MAX_PATH, path);
	PathAddBackslashA(path);
	return path;
}


// Creates a copy of the PlainText in Copy.txt file to process it
void Copy_in_BIN (){
	bool quit = false;

	while(!quit) {
		std::cout << "\n=== Please, enter path to file for processing ==" << std::endl;
		std::cout << ">> ";
		std::cin >> path_PlainText;

		if (!File_exist(path_PlainText)) {
			system("CLS");
			std::cout << "===== Sorry, but there isn't such a file ->" << std::endl;
		}
		else {
			path_Copy = GetWorkingDir();
			path_Copy += "Copy.txt";

			std::ifstream Readfile(path_PlainText, std::ifstream::binary);
			std::ofstream Copyfile(path_Copy, std::fstream::binary);
			copyCreated_flag = true;

			char c;
			while (Readfile.get(c)) {
				for (int i = 7; i >= 0; i--) {
					Copyfile << ((c >> i) & 1);
				}
			}

			Readfile.close();
			Copyfile.close();

			quit = true;
		}
	}	
}


// Random key generator (Generates 64-bit HEX key)
void Key_generator(){
	char hex_numbers[] = "0123456789ABCDEF";	// all numbers of hex base system to make HEX key

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist6(0, 15); // distribution in range [1, 16]

	if (use3DES_flag) {
		for (int i = 0; i < 48; i++) {
			key += hex_numbers[dist6(rng)];
		}
	}
	else {
		for (int i = 0; i < 16; i++) {
			key += hex_numbers[dist6(rng)];
		}
	}
}


// Converts HEX string to "0" and "1"
std::string HEX2BIN(std::string str) {
	for (int i = 0; i < str.length(); i++) {
		switch (str[i])	{
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
		case 'a':
			bin_key += "1010";
			break;
		case 'B':
			bin_key += "1011";
			break;
		case 'b':
			bin_key += "1011";
			break;
		case 'C':
			bin_key += "1100";
			break;
		case 'c':
			bin_key += "1100";
			break;
		case 'D':
			bin_key += "1101";
			break;
		case 'd':
			bin_key += "1101";
			break;
		case 'E':
			bin_key += "1110";
			break;
		case 'e':
			bin_key += "1110";
			break;
		case 'F':
			bin_key += "1111";
			break;
		case 'f':
			bin_key += "1111";
			break;

		default:
			break;
		}

	}
	return bin_key;
}


// Asks for a key
void Key_entering_3DES() {
	bool quit = false;
	std::string tempKey;

	while (!quit) {
		std::cout << "\n======= Enter your HEX (48 character) key ======" << std::endl;
		std::cout << ">> ";
		std::cin >> tempKey;

		for (int i = 0; i < 16; i++) {
			key_part1 += tempKey[i];
		}
		for (int i = 16; i < 32; i++) {
			key_part2 += tempKey[i];
		}
		for (int i = 32; i < 47; i++) {
			key_part3 += tempKey[i];
		}
		

		if (tempKey.length() != 48) {
			std::cout << "=== Sorry, but key must be 48 character ->" << std::endl;
			key_part1 = "";
			key_part2 = "";
			key_part3 = "";
		}
		else if (Invalid_key(tempKey)) {
			std::cout << "=== Sorry, but you used invalid character(s) ->" << std::endl;
			key_part1 = "";
			key_part2 = "";
			key_part3 = "";
		}
		else if (Weak_key_check_3DES()) {
			std::cout << "=== Sorry, but your key too weak ->" << std::endl;
			key_part1 = "";
			key_part2 = "";
			key_part3 = "";
		}
		else {
			quit = true;
			key = tempKey;
		}
	}
}


// Asks for key
void Key_entering_DES() {
	bool quit = false;
	std::string tempKey;

	while (!quit) {
		std::cout << "\n======= Enter your HEX (16 character) key ======" << std::endl;
		std::cout << ">> ";
		std::cin >> tempKey;

		if (tempKey.length() != 16) {
			std::cout << "=== Sorry, but key must be 16 character ->" << std::endl;
		}
		else if (Invalid_key(tempKey)) {
			std::cout << "=== Sorry, but you used invalid character(s) ->" << std::endl;
		}
		else if (Weak_key_check_DES(tempKey)) {
			std::cout << "=== Sorry, but your key too weak ->" << std::endl;
		}
		else {
			quit = true;
			key = tempKey;
		}
	}	
}


// Should we generate a key for you if you dont have it
void Key_generating_DES() {
	std::string answer;
	bool quit = false;

	while (!quit) {
		std::cout << "\n====== Generate a key? [y] - yes, [n] - no =====" << std::endl;
		std::cout << ">> ";
		std::cin >> answer; // Get the answer

		if (answer == "y") {
			Key_generator();
			std::cout << "======= Your key is: [" << key << "] ========" << std::endl;
			quit = true;
		}
		else if (answer == "n") {
			Key_entering_DES();
			std::cout << "======= Your key is: [" << key << "] ========" << std::endl;
			quit = true;
		}
		else {
			system("CLS");
			std::cout << "=== Sorry, but ->" << std::endl;
		}
	}	
}


// Should we generate a key for you if you dont have it
void Key_gentering_3DES() {
	std::string answer;
	bool quit = false;

	while (!quit) {
		std::cout << "\n====== Generate a key? [y] - yes, [n] - no =====" << std::endl;
		std::cout << ">> ";
		std::cin >> answer; // Get the answer

		if (answer == "y") {
			Key_generator();

			for (int i = 0; i < 16; i++) {
				key_part1 += key[i];
			}
			for (int i = 16; i < 32; i++) {
				key_part2 += key[i];
			}
			for (int i = 32; i < 47; i++) {
				key_part3 += key[i];
			}

			std::cout << "======= Your key is: [" << key << "] ========" << std::endl;
			quit = true;
		}
		else if (answer == "n") {
			Key_entering_3DES();
			std::cout << "======= Your key is: [" << key << "] ========" << std::endl;
			quit = true;
		}
		else {
			system("CLS");
			std::cout << "=== Sorry, but ->" << std::endl;
		}
	}
}


// Asking for key and what to do
void Key_Logic() {
	std::string answer;	// Does user have a key
	bool quit = false;

	while(!quit) {
		std::cout << "\n===== Do You have key? [y] - yes, [n] - no =====" << std::endl;
		std::cout << ">> ";
		std::cin >> answer; // Get the answer

		if (answer == "y") {
			if (use3DES_flag) {
				Key_entering_3DES();
				quit = true;
			}
			else {
				Key_entering_DES();
				quit = true;
			}
		}
		else if (answer == "n") {
			if (use3DES_flag) {
				Key_gentering_3DES();
				quit = true;
			}
			else {
				Key_generating_DES();
				quit = true;
			}
		}
		else {
			system("CLS");
			std::cout << "=== Sorry, but ->" << std::endl;
		}
	}
}


// Permutated Choice 1 from 64 to 56 bits
std::string PC_1(std::string Str64bit, std::string Str56bit) {
	int PC1_table[56] = { 56, 48, 40, 32, 24, 16, 8,
						 0, 57, 49, 41, 33, 25, 17,
						 9, 1, 58, 50, 42, 34, 26,
						 18, 10, 2, 59, 51, 43, 35,
						 62, 54, 46, 38, 30, 22, 14,
						 6, 61, 53, 45, 37, 29, 21,
						 13, 5, 60, 52, 44, 36, 28,
						 20, 12, 4, 27, 19, 11, 3 };

	for (int i = 0; i < 56; i++) {
		Str56bit += Str64bit[PC1_table[i]];
	}

	return Str56bit;
}


// Permutated Choice 2 from 56 to 48 bits
std::string PC_2(std::string Str56bit, std::string Str48bit) {
	int PC2_table[48] = {	13, 16, 10, 23, 0, 4,
							2, 27, 14, 5, 20, 9,
							22, 18, 11,	3, 25, 7,
							15, 6, 26, 19, 12, 1,
							40, 51, 30, 36,	46,	54,
							29,	39,	50,	44,	32,	47,
							43,	48,	38,	55,	33,	52,
							45,	41,	49,	35,	28,	31 };

	for (int i = 0; i < 48; i++) {
		Str48bit += Str56bit[PC2_table[i]];
	}

	return Str48bit;
}


// Divide key56bit on C-part and D-part
void C_and_D_divider(std::string Str56bit) {
	C28 = "";
	D28 = "";

	for (int i = 0; i < 56; i++) {
		if (i < 28) {
			C28 += Str56bit[i];
		}
		else {
			D28 += Str56bit[i];
		}
	}
}


// Left shift
std::string LS(std::string StrToShift, int NumberOfKey) {
	std::string str; // Temporary variable

	if (NumberOfKey == 1 || NumberOfKey == 2 || NumberOfKey == 9 || NumberOfKey == 16) {
		for (int i = 1; i < 28; i++) {
			str += StrToShift[i];
		}
		str += StrToShift[0];
	}
	else {
		for (int i = 2; i < 28; i++) {
			str += StrToShift[i];
		}
		str += StrToShift[0];
		str += StrToShift[1];
	}
	return str;
}


// Concatenate C-part and D-part
std::string C_plus_D(std::string Cpart, std::string Dpart) {
	key56bit = Cpart;
	key56bit += Dpart;

	return key56bit;
}


// Divides data from buffer (64 bits) on L and R parts
void L_and_R_divider(std::string DataBuffer) {
	L32 = "";
	R32 = "";

	for (int i = 0; i < 64; i++) {
		if (i < 32)	{
			L32 += DataBuffer[i];
		}
		else {
			R32 += DataBuffer[i];
		}
	}
}


// Left and Right concatenation
std::string L_plus_R() {
	std::string str = L32;
	str += R32;

	return str;
}


// Switching "Left" and "Right" parts
void L_R_switching() {
	std::string temp_string;

	temp_string = R32;
	R32 = L32;
	L32 = temp_string;

}


// Expansion Box
void Expansion(std::string Right_32bit) {
	int expansionTable[48] ={31, 0, 1, 2, 3, 4, 3, 4,
							5, 6, 7, 8, 9, 8, 9, 10,
							11, 12, 11, 12, 13, 14, 15, 16,
							15, 16, 17, 18, 19, 20, 19, 20,
							21, 22, 23, 24, 23, 24, 25, 26,
							27, 28, 27, 28, 29, 30, 31, 0 };

	for (int i = 0; i < 48; i++) {
		Right_48bit += Right_32bit[expansionTable[i]];
	}

}


// XOR of Right 48 bits and round key
std::string XOR_48bits(std::string R_48bit, std::string Round_SubKey) {
	std::string temp_string;

	for (int i = 0; i < 48; i++) {

		if (R_48bit[i] == Round_SubKey[i]) {
			temp_string += '0';
		}
		else {
			temp_string += '1';
		}
	}
	
	R_48bit = "";
	R_48bit = temp_string;

	return R_48bit;
}


// XOR of Left 32 bits and output of F-function
std::string XOR_32bits(std::string L_32bit, std::string F_output) {
	std::string temp_string;

	for (int i = 0; i < 32; i++) {

		if (L_32bit[i] == F_output[i]) {
			temp_string += '0';
		}
		else {
			temp_string += '1';
		}
	}

	L_32bit = "";
	L_32bit = temp_string;

	return L_32bit;
}


// Converting decimal to binary	
std::string Dec_to_Bin(int decimal) {
	std::string bin_output;

	switch (decimal) {
	case 0:
		bin_output = "0000";
		break;
	case 1:
		bin_output = "0001";
		break;
	case 2:
		bin_output = "0010";
		break;
	case 3:
		bin_output = "0011";
		break;
	case 4:
		bin_output = "0100";
		break;
	case 5:
		bin_output = "0101";
		break;
	case 6:
		bin_output = "0110";
		break;
	case 7:
		bin_output = "0111";
		break;
	case 8:
		bin_output = "1000";
		break;
	case 9:
		bin_output = "1001";
		break;
	case 10:
		bin_output = "1010";
		break;
	case 11:
		bin_output = "1011";
		break;
	case 12:
		bin_output = "1100";
		break;
	case 13:
		bin_output = "1101";
		break;
	case 14:
		bin_output = "1110";
		break;
	case 15:
		bin_output = "1111";
		break;

	default:
		break;
	}

	return bin_output;
}


// S-box 
std::string S_Box_function(std::string R_48bit) {
	std::string R_32bit;	// output of its function

	
	std::string t_row;	// temporary bin str row 
	std::string t_column;	// temporary bin str column

	std::string str_6_bit;	// 6-bit data block

	// S-box structure 8-sboxes 4-rows 16-columns
	int s_box_table[8][4][16] = { 
						{ 14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
						  0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
						  4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
						  15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13 },
						{ 15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
						  3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
						  0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
						  13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9 },
						{ 10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
						  13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
						  13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
						  1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12 },
						{ 7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
						  13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
						  10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
						  3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14 },
						{ 2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9,
						  14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
						  4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14,
						  11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3 },
						{ 12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
						  10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
						  9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
						  4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13 },
						{ 4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
						  13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
						  1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
						  6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12 },
						{ 13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
						  1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
						  7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
						  2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11 } };
	

	for (int Si = 0; Si < 8; Si++) {

		// Reading 6bit block of data //
		for (int j = 0; j < 6; j++) {
			str_6_bit += R_48bit[(Si * 6) + j];
		}

		// Processing 6bit block data //
		t_row += str_6_bit[0];
		t_row += str_6_bit[5];

		for (int i = 1; i < 5; i++) {
			t_column += str_6_bit[i];
		}

		// Convertion 6bit block data //
		// converting t_column to binary and writing it to column variable
		if (t_column == "0000") { column = 0; }
		if (t_column == "0001") { column = 1; }
		if (t_column == "0010") { column = 2; }
		if (t_column == "0011") { column = 3; }
		if (t_column == "0100") { column = 4; }
		if (t_column == "0101") { column = 5; }
		if (t_column == "0110") { column = 6; }
		if (t_column == "0111") { column = 7; }
		if (t_column == "1000") { column = 8; }
		if (t_column == "1001") { column = 9; }
		if (t_column == "1010") { column = 10; }
		if (t_column == "1011") { column = 11; }
		if (t_column == "1100") { column = 12; }
		if (t_column == "1101") { column = 13; }
		if (t_column == "1110") { column = 14; }
		if (t_column == "1111") { column = 15; }

		if (t_row == "00") { row = 0; }
		if (t_row == "01") { row = 1; }
		if (t_row == "10") { row = 2; }
		if (t_row == "11") { row = 3; }


		R_32bit += Dec_to_Bin(s_box_table[Si][row][column]);

		t_row = "";
		t_column = "";
		str_6_bit = "";
	}
	return R_32bit;
}


// Permutation of 32bit block data
std::string Permutation(std::string str_32bit) {
	std::string t_str;

	int Permutation_table[32] = { 15, 6, 19, 20, 28, 11, 27, 16,
								   0, 14, 22, 25,  4, 17, 30,  9,
								   1,  7, 23, 13, 31, 26,  2,  8,
								  18, 12, 29,  5, 21, 10,  3, 24 };

	for (int i = 0; i < 32; i++) {
		t_str += str_32bit[Permutation_table[i]];
	}
	return t_str;
}


// Keyschedule
void Key_Schedule() {
	if (use3DES_flag){
		
		std::string bin_key1;
		std::string bin_key2;
		std::string bin_key3;	

		bin_key1 = HEX2BIN(key_part1);
		bin_key2 = HEX2BIN(key_part2);
		bin_key3 = HEX2BIN(key_part3);

		
		//Creating subkeys1
		key56bit = PC_1(bin_key1, key56bit);

		for (int i = 0; i < 16; i++)
		{
			C_and_D_divider(key56bit);

			C28 = LS(C28, i);
			D28 = LS(D28, i);

			key56bit = C_plus_D(C28, D28);
			subkeys_3DES1[i] += PC_2(key56bit, key48bit);

			key48bit = "";
		}


		//Creating subkeys2
		key56bit = PC_1(bin_key2, key56bit);

		for (int i = 0; i < 16; i++)
		{
			C_and_D_divider(key56bit);

			C28 = LS(C28, i);
			D28 = LS(D28, i);

			key56bit = C_plus_D(C28, D28);
			subkeys_3DES2[i] += PC_2(key56bit, key48bit);

			key48bit = "";
		}


		//Creating subkeys3
		key56bit = PC_1(bin_key3, key56bit);

		for (int i = 0; i < 16; i++)
		{
			C_and_D_divider(key56bit);

			C28 = LS(C28, i);
			D28 = LS(D28, i);

			key56bit = C_plus_D(C28, D28);
			subkeys_3DES3[i] += PC_2(key56bit, key48bit);

			key48bit = "";
		}
	}
	else{		
		key56bit = PC_1(HEX2BIN(key), key56bit);
		
		for (int i = 1; i <= 16; i++)
		{
			C_and_D_divider(key56bit);

			C28 = LS(C28, i);
			D28 = LS(D28, i);

			key56bit = C_plus_D(C28, D28);
			subkeys_DES[i-1] += PC_2(key56bit, key48bit);

			key48bit = "";
		}
	}
}


// Get size of file in bits
long int FileSize(std::string filePath) {
	long int length;

	std::ifstream file(filePath, std::ifstream::binary);

	if (file) {
		file.seekg(0, file.end);
		length = file.tellg();
		file.seekg(0, file.beg);
	}
	file.close();
	return length;
}

 
// How many blocks will be
void Block_Amount() {
	if (size%64 == 0) {
		completeBlockNum = size / 64;
	}
	else {
		if (size > 64) {
			uncompleteBlockNum = ((size/64)+1)*64-size;
		}
		else {
			uncompleteBlockNum = 64 - size;
		}
		uncompleteBlock_flag = true;
		completeBlockNum = size / 64;
	}

	shift = 100.0 / (completeBlockNum + uncompleteBlock_flag);
	shift = std::round(shift * 10000) / 10000; // 4 digits precision
}


// Reads data block by block (by 64 bits) Reading into var "buffer"
void BlockReading() {
	if (file_end_flag != true) {
		std::ifstream DataReading(path_Copy, std::ios::in | std::ios::binary);	// opening our file for reading
		DataReading.seekg(position);

		if (completeBlockNum == 0 && uncompleteBlock_flag == true) { // we reached last uncomplete block
			DataReading.read(buffer, 64 - uncompleteBlockNum);	// Reads all data that lasts

			// Filling with pattern = "11111111"
			char pattern[8] = { '1','1','1','1','1','1','1','1' }; // pattern itself
			int j = 0; // temp var for writing patterns

			for (int i = 0; i < uncompleteBlockNum; i++) {

				buffer[(64 - uncompleteBlockNum) + i] = pattern[j];
				j++;

				if (j == 7){
					j = 0; // to contuinue pattering
				}
			}
		}

		// Reading info to var "buffer" till end of file or till last uncomplete block
		if (completeBlockNum != 0) {
			DataReading.read(buffer, 64);
			completeBlockNum--;
		}
		position += 64;

		if (size <= position) {
			file_end_flag = true;
		}
		DataReading.close();
	}
}


// From char array to normal string
std::string Buffer_to_string() {
	data64 = "";

	for (int i = 0; i < 64; i++) {
		data64 += buffer[i];
	}
	return data64;
}


// Initial Permutation
std::string IP(std::string StrToIP){
	std::string IpermutatedStr = "";

	int IP_table[64] = { 57, 49, 41, 33, 25, 17, 9, 1,
						59,	51,	43,	35,	27,	19,	11,	3,
						61, 53,	45,	37,	29,	21,	13,	5,
						63,	55,	47,	39,	31,	23,	15,	7,
						56,	48,	40,	32,	24,	16,	8,	0,
						58,	50,	42,	34,	26,	18,	10,	2,
						60,	52,	44,	36,	28,	20,	12,	4,
						62,	54,	46,	38,	30,	22,	14,	6 };

	for (int i = 0; i < 64; i++) {
		IpermutatedStr += StrToIP[IP_table[i]];
	}
	return IpermutatedStr;
}


// Final Permutation
std::string FP(std::string StrToFP) {
	std::string FpermutatedStr = "";

	int FP_table[64] = { 39, 7,	47,	15,	55,	23,	63,	31,
						38,	6,	46,	14,	54,	22,	62,	30,
						37,	5,	45,	13,	53,	21,	61,	29,
						36, 4,	44,	12,	52,	20,	60,	28,
						35,	3,	43,	11,	51,	19,	59,	27,
						34,	2,	42,	10,	50,	18,	58,	26,
						33,	1,	41,	9,	49,	17,	57,	25,
						32,	0,	40,	8,	48,	16,	56,	24
	};

	for (int i = 0; i < 64; i++) {
		FpermutatedStr += StrToFP[FP_table[i]];
	}
	return FpermutatedStr;
}


// Outputs encrypted 64 bit data block
void Writing_64bit(std::string path, std::string data) {
	std::ofstream File_output(path, std::ofstream::binary | std::ios_base::app); // Opening that file for writing
	outputCreated_flag = true;
	
	if (shutDown_flag) {
		File_output.close();
		std::remove(path_File_Save.c_str());
		system("CLS");
	}

	std::string writing_dat = "";

	int times = data.length() / 8;


	for (int i = 0; i < times; i++)	{
		for (int j = 0; j < 8; j++)	{
			writing_dat += data[8 * i + j];
			if (shutDown_flag) {
				File_output.close();
				std::remove(path_File_Save.c_str());
				system("CLS");
			}
		}
		File_output << static_cast<uint_fast8_t>(std::bitset<8>(writing_dat).to_ulong());
		writing_dat = "";
		if (shutDown_flag) {
			File_output.close();
			std::remove(path_File_Save.c_str());
			system("CLS");
		}
	}
	File_output.close();
}


// F-function itself
std::string F_function(std::string subkey, std::string Round_R32) {
	Expansion(Round_R32);

	std::string XORoutput;
	XORoutput = XOR_48bits(Right_48bit, subkey);

	std::string output;
	output = Permutation(S_Box_function(XORoutput));

	XORoutput = "";
	Right_48bit = "";

	return  output;
}


// Checks for patterns
std::string Check_for_pattern(std::string str_out_IP) {
	int pattern = 0;
	int for_delete = 0;
	std::string output;

	if (completeBlockNum == 0) {
		for (int i = 1; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (str_out_IP[j + i * 8] == '1') {
					pattern++;
				}
			}

			if (pattern == 8) {
				for_delete++;
				pattern = 0;
			}
			else {
				pattern = 0;
			}
		}

		for (int i = 0; i < 8 - for_delete; i++) {
			for (int j = 0; j < 8; j++) {
				output += str_out_IP[j + i * 8];
			}
		}
	}
	else {
		output = str_out_IP;
	}

	return output;
}


// ================= Main processing function =====================


// Showing Progress in percents
void Progress_Bar() {
	int progr_bar = 30;
	int one_filling_amount = 100 / progr_bar;

	percentage += shift;
	int filling = percentage / (100.0 / progr_bar);

	if (std::round(percentage) == 100) {
		filling = progr_bar;
	}

	if (percentage == 0 || 100/percentage >= 1) {
		std::cout << " Progress: " << std::round(percentage) << "% [";
		for (int i = 0; i < progr_bar; i++) {
			if (filling != 0) {
				std::cout << "#";
				filling--;
			}
			else {
				std::cout << ".";
			}
		}
		std::cout << "]\r";
	}
}


// Inputting all valuable data for next step
void Hello() {
	std::string answer;
	bool quit = false;

	while(!quit) {
		std::cout << "==== For encryption - [e], decryption - [d] ====" << std::endl;
		std::cout << ">> ";
		std::cin >> answer; // Get the answer

		if (answer == "e") {
			Copy_in_BIN();
			Key_Logic();
			quit = true;
		}
		else if (answer == "d")	{
			decryption_flag = 1;
			Copy_in_BIN();

			if (use3DES_flag) {
				Key_entering_3DES();
			}
			else {
				Key_entering_DES();
			}
			
			quit = true;
		}
		else if (answer == "F")	{	// Easter egg
			std::cout << "========== Thanks, You paid respect! ==========" << std::endl;
		}
		else {
			system("CLS");
			std::cout << "=== Sorry, but ->" << std::endl;
		}
	}
}


void Encryption_DES() {
	size = FileSize(path_Copy);
	Block_Amount();

	while (file_end_flag != true) {
		BlockReading();

		std::string t_str_out_IP = "";

		if (useCBC_flag) {
			t_str_out_IP = IP(XOR_IV64bits(Buffer_to_string(), IV64));
		}
		else {
			t_str_out_IP = IP(Buffer_to_string());
		}
		
		L_and_R_divider(t_str_out_IP);


		for (int round = 0; round < 16; round++) {
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_DES[round], R32));
			L32 = XORoutput;

			L_R_switching();
		}

		L_R_switching();
		
		std::string t_str_out_FP = "";
		t_str_out_FP = FP(L_plus_R());

		IV64 = t_str_out_FP;

		Writing_64bit(path_File_Save, t_str_out_FP);

		Progress_Bar();
	}
}


void Decryption_DES() {
	size = FileSize(path_Copy);
	Block_Amount();

	while (file_end_flag != true) {
		BlockReading();

		std::string t_str_out_IP = "";
		t_str_out_IP = IP(Buffer_to_string());
		
		L_and_R_divider(t_str_out_IP);

		for (int round = 0; round < 16; round++) {  
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_DES[15 - round], R32));
			L32 = XORoutput;
			L_R_switching();
		}

		L_R_switching();

		std::string t_str_out_FP = "";
		t_str_out_FP = FP(L_plus_R());


		std::string output;
		if (useCBC_flag) {
			output = Check_for_pattern(XOR_IV64bits(t_str_out_FP, IV64));
			IV64 = Buffer_to_string();
		}
		else {
			output = Check_for_pattern(t_str_out_FP);
		}
				
		Writing_64bit(path_File_Save, output);
		Progress_Bar();
	}
}


void Encryption_3DES() {
	size = FileSize(path_Copy);
	Block_Amount();

	while (file_end_flag != true) {
		BlockReading();

		std::string t_str_out_IP = "";

		if (useCBC_flag) {
			t_str_out_IP = IP(XOR_IV64bits(Buffer_to_string(), IV64));
		}
		else {
			t_str_out_IP = IP(Buffer_to_string());
		}
		
		L_and_R_divider(t_str_out_IP);
		
		for (int round = 0; round < 16; round++) {
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_3DES1[round], R32));
			L32 = XORoutput;

			L_R_switching();
		}
		L_R_switching();


		for (int round = 0; round < 16; round++) {
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_3DES2[15 - round], R32));
			L32 = XORoutput;
			L_R_switching();
		}
		L_R_switching();


		for (int round = 0; round < 16; round++) {
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_3DES3[round], R32));
			L32 = XORoutput;

			L_R_switching();
		}
		L_R_switching();


		std::string t_str_out_FP = "";
		t_str_out_FP = FP(L_plus_R());

		IV64 = t_str_out_FP;
		
		Writing_64bit(path_File_Save, t_str_out_FP);

		Progress_Bar();
	}
}


void Decryption_3DES() {
	size = FileSize(path_Copy);
	Block_Amount();

	while (file_end_flag != true) {
		BlockReading();

		std::string t_str_out_IP = "";
		t_str_out_IP = IP(Buffer_to_string());

		L_and_R_divider(t_str_out_IP);


		for (int round = 0; round < 16; round++) {
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_3DES3[15 - round], R32));
			L32 = XORoutput;
			L_R_switching();
		}
		L_R_switching();


		for (int round = 0; round < 16; round++) {
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_3DES2[round], R32));
			L32 = XORoutput;
			L_R_switching();
		}
		L_R_switching();


		for (int round = 0; round < 16; round++) {
			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys_3DES1[15 - round], R32));
			L32 = XORoutput;
			L_R_switching();
		}
		L_R_switching();


		std::string t_str_out_FP = "";
		t_str_out_FP = FP(L_plus_R());


		std::string output;
		if (useCBC_flag) {
			output = Check_for_pattern(XOR_IV64bits(t_str_out_FP, IV64));
			IV64 = Buffer_to_string();
		}
		else {
			output = Check_for_pattern(t_str_out_FP);
		}

		Writing_64bit(path_File_Save, output);
		Progress_Bar();
	}
}



//============================================================

// Signal handler
void Signal_handler(int sig) {
	shutDown_flag = true;

	if (copyCreated_flag) {
		std::remove(path_Copy.c_str());
	}
	else if (outputCreated_flag) {
		std::remove(path_File_Save.c_str());
	}
	system("CLS");
	std::cout << "============== Process terminated! ==============\n" << std::endl;
	exit(0);
}

//============================================================

int main() {
	//========================================================
	//					Signal handlers
	//========================================================
	
	signal(SIGINT, Signal_handler);
	signal(SIGTERM, Signal_handler);

	#ifdef SIGBREAK
		signal(SIGBREAK, Signal_handler);
	#endif

	//========================================================
	//					Console decorations
	//========================================================

	#ifdef _WIN32
			SetConsoleTitle(TEXT("Encryptor"));
	#elif __linux__
			std::cout << "\e]0;Encryptor\a\n";
	#endif

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 10);

	HWND console = GetConsoleWindow();
	RECT ConsoleRect;
	GetWindowRect(console, &ConsoleRect);
	MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 500, 300, TRUE);
	SetWindowPos(console, 0, 500, 250, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	
	//========================================================
	//					Actual program code
	//========================================================

	std::cout << "=================== Oh, hi! ====================\n" << std::endl;

	Use_CBC();

	Use_3DES();

	Hello();

	Where_to_save();

	Key_Schedule();
	
	std::cout << "====== Hold on one moment, data processing =====" << std::endl;
	std::cout << std::endl;

	if (decryption_flag && useCBC_flag) {
		Enter_IV();
	}
	else if (useCBC_flag) {
		IV_generator();
	}

	if (use3DES_flag) {
		if (decryption_flag == true) {
			Decryption_3DES();
			std::cout << std::endl;
		}
		else {
			Encryption_3DES();
			std::cout << std::endl;
		}
	}
	else {
		if (decryption_flag == true) {
			Decryption_DES();
			std::cout << std::endl;
		}
		else {
			Encryption_DES();
			std::cout << std::endl;
		}
	}


	std::remove(path_Copy.c_str());

	std::cout << "==================== Done! =====================" << std::endl;
	std::cout << std::endl;

	system("pause");
	return 0;
}

//============================================================