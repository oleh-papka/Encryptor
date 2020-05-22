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

std::string Left_48bit; // "Left" part after expansion


std::string data64[64]; // Data block for encryption
char buffer[64];	//Buffer stores read data form data64


int completeBlockNum;	// How many blocks will be in file
int uncompleteBlockNum;	// How many bits are off the block
bool uncompleteBlock_flag = false;

long int size; // Size of .bin file in bytes

//========================================================
//========================================================



//========================================================
//						Functions
//========================================================

// Gets direction of executable file
std::string GetWorkingDir() {
	char path[MAX_PATH] = "";
	GetCurrentDirectoryA(MAX_PATH, path);
	PathAddBackslashA(path);
	return path;
}

//std::string path_Copy;

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


// Permutated Choice 1 from 64 to 56 bits
std::string PC_1(std::string Str64bit, std::string Str56bit) {

	std::cout << "Key before: " << Str64bit << std::endl;

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


	std::cout << "PC1 key now: " << Str56bit << std::endl;




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


	std::cout << "Key before: " << Str56bit << std::endl;

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
void L_and_R_divider(char DataBuffer[64]) {

	L32 = "";
	R32 = "";

	for (int i = 0; i < 64; i++) {
		if (i < 32)
		{
			C28 += DataBuffer[i];
		}
		else {
			D28 += DataBuffer[i];
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
std::string Expansion(std::string Left_32bit) {

	std::string Left_48bit;

	std::cout << "Before Expansion: " << Left_32bit << std::endl;

	int expansionTable[48] ={31, 0, 1, 2, 3, 4, 3, 4,
							5, 6, 7, 8, 9, 8, 9, 10,
							11, 12, 11, 12, 13, 14, 15, 16,
							15, 16, 17, 18, 19, 20, 19, 20,
							21, 22, 23, 24, 23, 24, 25, 26,
							27, 28, 27, 28, 29, 30, 31, 0 };

	for (int i = 0; i < 48; i++) {
		Left_48bit += Left_32bit[expansionTable[i]];
	}

	std::cout << "After Expansion: " << Left_48bit << std::endl;


	return Left_48bit;
}


// XOR of Left 48 bits and round key
void XOR(std::string Left_48bit, std::string Round_SubKey) {

	std::string temp_string;

	for (int i = 0; i < 48; i++) {

		if (Left_48bit[i] == Round_SubKey[i]) {
			temp_string += '0';
		}
		else {
			temp_string += '1';
		}
	}
	
	Left_48bit = "";
	Left_48bit = temp_string;
}


// Keyschedule
void Key_Schedule() {
	HEX2BIN(key); // Now key is in BIN
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
			uncompleteBlockNum = size % 64;
		}
		else {
			uncompleteBlockNum = 64 - size;
		}
		
		uncompleteBlock_flag = true;
		std::cout << "Uncomplete block bits = " << uncompleteBlockNum << std::endl;

		completeBlockNum = (size - uncompleteBlockNum) / 64;
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

	std::string permutatedStr;

	std::cout << "IP str before: " << StrToIP << std::endl;

	int IP_table[64] = { 57, 49, 41, 33, 25, 17, 9, 1,
						59,	51,	43,	35,	27,	19,	11,	3,
						61, 53,	45,	37,	29,	21,	13,	5,
						63,	55,	47,	39,	31,	23,	15,	7,
						56,	48,	40,	32,	24,	16,	8,	0,
						58,	50,	42,	34,	26,	18,	10,	2,
						60,	52,	44,	36,	28,	20,	12,	4,
						62,	54,	46,	38,	30,	22,	14,	6 };

	for (int i = 0; i < 64; i++) {
		permutatedStr += StrToIP[IP_table[i]];
	}
	
	std::cout << "IP str now: " << permutatedStr << std::endl;

	return permutatedStr;
}

// Final Permutation
std::string FP(std::string StrToFP) {

	std::string permutatedStr;

	std::cout << "FP str before: " << StrToFP << std::endl;

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
		permutatedStr += StrToFP[FP_table[i]];
	}

	std::cout << "FP str now: " << permutatedStr << std::endl;

	return permutatedStr;
}


// Outputs encrypted 64 bit data block
void Writing_Encrypted_64bit(std::string path, std::string data) {

	

	std::ofstream File_output(path, std::ofstream::binary | std::ios_base::app); // Opening that file for writing
	std::string writing_dat;	// Temporary variable
	for (int i = 0; i < 8; i++)
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



//========================================================
//========================================================




//========================================================
//						 Main
//========================================================

int main() {

	std::cout << "======= Hello! =======" << std::endl;

	//key = "F1FFFF3FFAFFF5FF"; // our test key
	
	return 0;
}

//========================================================
//========================================================