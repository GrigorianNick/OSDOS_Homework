#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdint.h>
#include <bitset>
#include <string>

#include "sysinfo.h"

using namespace std;

void ls() {
	uint8_t file_name[8];
	uint8_t file_ext[3];
	uint8_t DIR_Attr;
	disk.read((char*)file_name, 8);
	disk.read((char*)file_ext, 3);
	disk.read((char*)&DIR_Attr, 1);
	do {
		if (((int)DIR_Attr & 2) == 2) { // Hidden folder, skip
			disk.seekg(20, disk.cur);
			disk.read((char*)file_name, 8);
			disk.read((char*)file_ext, 3);
			disk.read((char*)&DIR_Attr, 1);
			continue;
		}
		if (((int)DIR_Attr & 16) == 16) { // Directory, D w/ 2 spaces
			cout << "D  ";
		}
		else if (((int)DIR_Attr & 8) == 8) { // Volume ID, Volume Label: w/ 1 space
			cout << "Volume Label: ";
		}
		else { // File, 3 spaces
			cout << "   ";
		}
		for (int i = 0; i < 8; i++) {
			if ((int)file_name[i] != 32) {
				cout << file_name[i];
			}
			else {
				break;
			}
		}
		if (((int)DIR_Attr & 16) != 16 && (int)file_ext[0] != 32) { // This entry isn't a directory and it has an ext
			cout << "." << file_ext;
		}
		else if (((int)DIR_Attr & 16) == 16 && (int)file_ext[0] != 32) { // We're a directory that took all 11 bytes
			cout << file_ext;
		}
		cout << endl; // Carriage return!
		disk.seekg(20, disk.cur);
		disk.read((char*)file_name, 8);
		disk.read((char*)file_ext, 3);
		disk.read((char*)&DIR_Attr, 1);
	} while ((int)file_name[0] != 0);
}

void cd(string target) {
	uint8_t file_name[8];
	uint8_t file_ext[3];
	uint8_t DIR_Attr;
	bool target_found;
	/*disk.read((char*)file_name, 8);
	disk.read((char*)file_ext, 3);
	disk.read((char*)&DIR_Attr, 1);*/
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
			if ((int)jump_target == 0) {
				cwd = root;
			}
			else {
				cout << cwd << endl << jump_target << endl;
				cwd = (jump_target + 2) * SecPerClus * BytesPerSec + root;
				cout << cwd << endl;
				break;
			}
		}
		else {
			disk.seekg(20, disk.cur);
			cout << "didn't find the target" << endl;
		}
	} while((int)file_name[0] != 0);
}
