#include <iomanip>
#include <iostream>
#include <fstream>
#include <stdint.h> // needed for uint8_t
#include <bitset>
#include <string>

#include "functions.h"

using namespace std;

int main( int argc, char *argv[] ) {
	if (argc != 2) {
		cout << "Error: improper number of arguments." << endl;
		return 1;
	}
	disk.open(argv[1]);
	loadFATInfo();
	printDiagnosticInfo();
	//disk.seekg(FSInfo * BytesPerSec, disk.beg); // FSInfo sector
	//disk.seekg((FSInfo * BytesPerSec) + 484, disk.beg); // FSInfo secondary sig
	root = (RsvdSecCnt + FATSz16 * NumFATs) * BytesPerSec; // root directory
	cwd_string = "/";
	cwd = root;
	disk.seekg(cwd, disk.beg); // moving to root
	string input;
	cout << ": " << cwd_string << " > ";
	getline(cin, input);
	uint8_t file_name[8];
	uint8_t file_ext[3];
	uint8_t DIR_Attr;
	while (input != "exit") {
		if (input == "ls") {
			ls();
		}
		else if (input.substr(0,2) == "ls") { // ls w/ a target
			int cwd_bak = cwd;
			string cwd_string_bak = cwd_string;
			seek(input.substr(3));
			disk.seekg(cwd, disk.beg);
			ls();
			cwd = cwd_bak;
			cwd_string = cwd_string_bak;
		}
		else if (input == "cd") { // Take us back to root, since we don't have home directories
			cwd_string = "/";
			cwd = root;
		}
		else if (input.substr(0,2) == "cd") { // Need to break the target down into bite sized chunks for cd
			seek(input.substr(3));
		}
		else if (input == "test") {
			cpout("/PEOPLE/YYZ5W/THE-GAME.TXT", "");
		}
		disk.seekg(cwd, disk.beg);
		cout << ": " << cwd_string << " > ";
		getline(cin, input);
	}
	disk.close();
	return 0;
}
