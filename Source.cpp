#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;

/*
	Program for encrypting and decrypting data
	master branch
*/

char text[200];

int main() {
	fstream read_file;
	fstream write_file;

	read_file.open("Data.txt", ios::in);
	write_file.open("Results.txt", ios::out);

	std::cout << "What to write to file?" << std::endl;
	cin.getline(text, sizeof(text));

	write_file << text << endl;

	std::cout << "Was writen:" << text << std::endl;

	read_file.close();
	write_file.close();

	return 0;
}