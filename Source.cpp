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
#pragma warning(disable : 4996)

//========================================================
//						Variables
//========================================================

bool encryption_flag = 0;
bool decryption_flag = 0;

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

std::string subkeys[16]; // All subkeys in one place


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

//========================================================
//========================================================



//========================================================
//						Functions
//========================================================



void Where_to_save() {

	std::cout << "Please enter path for encrypted file: ";
	std::cin >> path_File_Save;

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

	std::cout << "Please enter path to the file for encryption (PlainText): ";
	std::cin >> path_PlainText;

	path_Copy  = GetWorkingDir();

	path_Copy += "Copy.txt";

	std::ifstream Readfile (path_PlainText, std::ifstream::binary);
	std::ofstream Copyfile (path_Copy, std::fstream::binary);


	std::cout << "Reading bits... " << std::endl;
	char c;
	while (Readfile.get(c))
	{
		for (int i = 7; i >= 0; i--) // or (int i = 0; i < 8; i++)  if you want reverse bit order in bytes
			Copyfile << ((c >> i) & 1);
	}

	Readfile.close();
	Copyfile.close();
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
std::string HEX2BIN(std::string str) {

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
	return bin_key;
}


// Asks for key
void Key_entering() {
	std::cout << "Ok enter your HEX (16 character):" << std::endl;
	std::cin >> key;
	
	if (key.length() != 16) {
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


// Permutated Choice 1 from 64 to 56 bits
std::string PC_1(std::string Str64bit, std::string Str56bit) {

	//std::cout << "Key before: " << Str64bit << std::endl;

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


	//std::cout << "PC1 key now: " << Str56bit << std::endl;




	/*
	for (int i = 0; i < 64; i++)
	{
		if ((i + 1) % 8 == 0) {
		}
		else {
			Str56bit += Str64bit[i];
		}
	}
	*/



	return Str56bit;
}


// Permutated Choice 2 from 56 to 48 bits
std::string PC_2(std::string Str56bit, std::string Str48bit) {


	//std::cout << "Key before: " << Str56bit << std::endl;

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


	std::cout << "PC2 key now: " << Str48bit << std::endl;




	/*
	for (int i = 0; i < 56; i++)
	{
		if ((i+1)%8 == 0 || i == 0) {
		}
		else {
			Str48bit += Str56bit[i];
		}
	}
	*/

	return Str48bit;
}


// Divide key56bit on C-part and D-part
void C_and_D_divider(std::string Str56bit) {

	C28 = "";
	D28 = "";

	for (int i = 0; i < 56; i++) {
		if (i < 28)
		{
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
		for (int i = 1; i < 28; i++)
		{
			str += StrToShift[i];
		}
		str += StrToShift[0];
	}
	else {
		for (int i = 2; i < 28; i++)
		{
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
		if (i < 32)
		{
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

	//std::cout << "Before Expansion: " << Right_32bit << std::endl;

	int expansionTable[48] ={31, 0, 1, 2, 3, 4, 3, 4,
							5, 6, 7, 8, 9, 8, 9, 10,
							11, 12, 11, 12, 13, 14, 15, 16,
							15, 16, 17, 18, 19, 20, 19, 20,
							21, 22, 23, 24, 23, 24, 25, 26,
							27, 28, 27, 28, 29, 30, 31, 0 };

	for (int i = 0; i < 48; i++) {
		Right_48bit += Right_32bit[expansionTable[i]];
	}

	//std::cout << "After Expansion: " << Right_48bit << std::endl;
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

	//std::cout << "LOL this test R32bit block = " << R_32bit << std::endl;

	return L_32bit;
}



// Converting decimal to binary	
std::string Dec_to_Bin(int decimal) {

	std::string bin_output;

	switch (decimal)
	{
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


	std::string HEX_OUT;
	HEX_OUT = HEX2BIN(key); // Now key is in BIN
	bin_key = HEX_OUT;

	//std::cout << "+++++++++++++++++++++++++++ hey bitch key: " << bin_key << std::endl;


	key56bit = PC_1(bin_key, key56bit);
	
	for (int i = 1; i <= 16; i++)
	{
		C_and_D_divider(key56bit);

		C28 = LS(C28, i);

		D28 = LS(D28, i);

		key56bit = C_plus_D(C28, D28);

		subkeys[i-1] += PC_2(key56bit, key48bit);

		key48bit = "";
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

		std::cout << "Complete blocks = " << completeBlockNum << std::endl;
	}
	else {
		if (size > 64) {
			uncompleteBlockNum = ((size/64)+1)*64-size;
		}
		else {
			uncompleteBlockNum = 64 - size;
		}
		
		uncompleteBlock_flag = true;
		std::cout << "Uncomplete block bits = " << uncompleteBlockNum << std::endl;

		completeBlockNum = size / 64;
		std::cout << "Complete blocks = " << completeBlockNum << std::endl;
	}
}




// Reads data block by block (by 64 bits) Reading into var "buffer"
void BlockReading() {
	
	if (file_end_flag == true) {
		std::cout << "File already read!" << std::endl;
	}
	else{
		std::ifstream DataReading(path_Copy, std::ios::in | std::ios::binary);	// opening our file for reading

		DataReading.seekg(position);

		if (completeBlockNum == 0 && uncompleteBlock_flag == true) { // we reached last uncomplete block

			std::cout << "Reading with pattering" << std::endl; // fpr debugging

			DataReading.read(buffer, 64 - uncompleteBlockNum);	// Reads all data that lasts

			// Filling with pattern = "11111111"
			char pattern[8] = { '1','1','1','1','1','1','1','1' }; // pattern itself
			int j = 0; // temp var for writing patterns

			for (int i = 0; i < uncompleteBlockNum; i++) {

				buffer[(64 - uncompleteBlockNum) + i] = pattern[j];
				j++;

				if (j == 7) j = 0; // to contuinue pattering
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

//========== for debuging =========


void Otput_buffer() {
	std::cout << "Buffer now: ";
	for (int i = 0; i < 64; i++){

		std::cout << buffer[i];
	}
	std::cout << "\n";
}


//========== for debuging =========


// Initial Permutation
std::string IP(std::string StrToIP){

	std::string IpermutatedStr = "";

	//std::cout << "IP str before: " << StrToIP << std::endl;

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
	
	//std::cout << "IP str now: " << IpermutatedStr << std::endl;

	return IpermutatedStr;
}

// Final Permutation
std::string FP(std::string StrToFP) {

	std::string FpermutatedStr = "";

	//std::cout << "FP str before: " << StrToFP << std::endl;

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

	//std::cout << "FP str now: " << FpermutatedStr << std::endl;

	return FpermutatedStr;
}


// Outputs encrypted 64 bit data block
void Writing_Encrypted_64bit(std::string path, std::string data) {

	std::ofstream File_output(path, std::ofstream::binary | std::ios_base::app); // Opening that file for writing
	std::string writing_dat = "";

	int times = data.length() / 8;


	for (int i = 0; i < times; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			writing_dat += data[8 * i + j];
		}
		

		File_output << static_cast<uint_fast8_t>(std::bitset<8>(writing_dat).to_ulong());

		std::cout << "Info: " << writing_dat << std::endl;

		writing_dat = "";
	}

	File_output.close();
}


// F-function itself
std::string F_function(std::string subkey, std::string Round_R32) {

	Expansion(Round_R32);

	//std::cout << " SUB key= " << subkey << std::endl;

	std::string XORoutput;
	XORoutput = XOR_48bits(Right_48bit, subkey);

	std::string output;

	output = Permutation(S_Box_function(XORoutput));
	
	//std::cout << "====== F  output= " << output << std::endl;


	XORoutput = "";
	Right_48bit = "";

	return  output;
}


/*
	// allocate memory:
	char* buffer = new char[length];

	// read data as a block:
	is.read(buffer, length);

	is.close();

	// print content:
	std::cout.write(buffer, length);

	delete[] buffer;



*/


/* For .bin reading 

	char buffer[64];
	std::ifstream PlainTextBIN(path_Copy, std::ifstream::binary);
	myFile.read(buffer, 64);


	std::ifstream fin("C:\\file.txt");
char buffer[1024]; //I prefer array more than vector for such implementation

fin.read(buffer,sizeof(buffer));//first read get the first 1024 byte

fin.read(buffer,sizeof(buffer));//second read get the second 1024 byte
  


*/



std::string Check_for_pattern(std::string str_out_IP) {

	int pattern = 0;
	int for_delete = 0;
	std::string output;
	//completeBlockNum;

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
	else
	{
		output = str_out_IP;
	}

	return output;
}





// ================= Main processing function =====================

// Inputting all valuable data for next step
void Hello() {

	std::cout << "For encryption enter [e] , for decryption [d]" << std::endl;
	char answer;
	std::cin >> answer; // Get the answer

	if (answer == 'e')
	{
		encryption_flag = 1;
		std::cout << "ok, encryption" << std::endl;
		Copy_in_BIN();	// Gets file and creates copy of it's bits in .txt
		Key_Logic(); // Gets the key
	}
	else if (answer == 'd')
	{
		decryption_flag = 1;
		std::cout << "ok, decryption soon" << std::endl;
		Copy_in_BIN();	// Gets file and creates copy of it's bits in .txt
		Key_Logic(); // Gets the key


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


/*

 there we should put 
	Where_to_save();
	Key_Schedule();

*/


void Encryption() {

	size = FileSize(path_Copy);
	
	Block_Amount();

	while (file_end_flag != true) {
		
		BlockReading();

		Otput_buffer();

		std::string t_str_out_IP = "";
		//t_str_out_IP = IP(Buffer_to_string());
		t_str_out_IP = Buffer_to_string();
		
		L_and_R_divider(t_str_out_IP);


		for (int round = 0; round < 16; round++) {

			
			
			//std::cout << "=== R32 = " << R32 << std::endl;


			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys[round], R32));
			L32 = XORoutput;

			//std::cout << "=== Round = " << round << std::endl;

			L_R_switching();

			//std::cout << "=== After switching R32 = " << R32 << std::endl;
		}

		L_R_switching();
		
		std::string t_str_out_FP = "";
		//t_str_out_FP = FP(L_plus_R());
		t_str_out_FP = L_plus_R();


		Writing_Encrypted_64bit(path_File_Save, t_str_out_FP);
	}
	

}



void Decryption() {

	size = FileSize(path_Copy);

	Block_Amount();

	while (file_end_flag != true) {

		BlockReading();

		//Otput_buffer();


		std::string t_str_out_FP = "";
		//t_str_out_FP = FP(Buffer_to_string());
		t_str_out_FP = Buffer_to_string();

		L_and_R_divider(t_str_out_FP);

		//L_R_switching();

		for (int round = 0; round < 16; round++) {
					   
			//L_R_switching();
			//std::cout << "=== R32 = " << R32 << std::endl;


			std::string XORoutput;
			XORoutput = XOR_32bits(L32, F_function(subkeys[15 - round], R32));
			L32 = XORoutput;

			//std::cout << "=== Round = " << round << std::endl;

			L_R_switching();

			//std::cout << "=== After switching R32 = " << R32 << std::endl;
		}

		
		L_R_switching();


		std::string t_str_out_IP = "";
		//t_str_out_IP = IP(L_plus_R());
		t_str_out_IP = L_plus_R();

		std::string output = Check_for_pattern(t_str_out_IP);

		std::cout << output;

		Writing_Encrypted_64bit(path_File_Save, output);
		
	}



}



//========================================================
//========================================================




//========================================================
//						 Main
//========================================================

int main() {

	std::cout << "======= Oh, hi! =======" << std::endl;

	//key = "F1FFFF3FFAFFF5FF"; // our test key
	
	Hello();

	Where_to_save();


	Key_Schedule();

	
	if (decryption_flag == 1) {

		Decryption();

	}
	else {
		Encryption();
	}
	


	// test




	// end

	// declaring character array 
	char char_array[255];
	// copying the contents of the 
	// string to char array 
	strcpy(char_array, path_Copy.c_str());

	if (remove(char_array) == 0) {
		printf("Deleted successfully");
	}

	return 0;
}

//========================================================
//========================================================