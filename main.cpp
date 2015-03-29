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
			string target = input.substr(3);
			bool target_found;
			do {
				disk.read((char*)file_name, 8);
				disk.read((char*)file_ext, 3);
				disk.read((char*)&DIR_Attr, 1);
				target_found = true;
				for (int i = 0; i < target.length(); i++) {
					if ((int)target[i] != (int)file_name[i]) {
						target_found = false;
					}
				}
				if (target_found) {
					disk.seekg(14, disk.cur);
					uint16_t jump_target;
					disk.read((char*)&jump_target, 2);
					if ((int)jump_target == 0) { // ls'ing root
						disk.seekg(root, disk.beg);
					}
					else {
						disk.seekg((jump_target + 2) * SecPerClus * BytesPerSec + root, disk.beg);
					}
					ls();
					break;
				}
				else {
					disk.seekg(20, disk.cur);
					cout << "not it!" << endl;
				}
			}
			while ((int)file_name[0] != 0);
		}
		else if (input == "cd") { // Take us back to root, since we don't have home directories
			cwd_string = "/";
			cwd = root;
		}
		else if (input.substr(0,2) == "cd") { // Need to break the target down into bite sized chunks for cd
			string target = input.substr(3);
			if (target[0] == '/') { // Absoute path, start at root
				cwd_string = "/";
				cwd = root;
				disk.seekg(cwd, disk.beg);
			}
			string sub_target = "";
			for (int i = 0; i < target.length(); i++) {
				if (target[i] != '/') { // Good to keep building sub_target
					sub_target += target[i];
					cout << sub_target << endl;
				}
				else {
					if (sub_target == "") continue;
					cd (sub_target);
					disk.seekg(cwd, disk.beg);
					sub_target = "";
				}
			}
			cd(sub_target);
		}
		disk.seekg(cwd, disk.beg);
		cout << ": " << cwd_string << " > ";
		getline(cin, input);
	}
	disk.close();
	return 0;
}
