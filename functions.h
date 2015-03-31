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
			if ((int)file_ext[0] != 32) cout << ".";
			for (int i = 0; i < 3; i++) {
				if ((int)file_ext[i] != 32) {
					cout << file_ext[i];
				}
				else {
					break;
				}
			}
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
			if (target != "." && target != "..") { // We're delving deeper into the dir tree
				if (cwd_string != "/") { // need to start the slashes after root
					cwd_string += "/";
				}
				for (int i = 0; i < 8; i++) {
					if ((int)file_name[i] == 32) break; // reached end of file name
					cwd_string += file_name[i]; // append to cwd_string
				}
				for (int i = 0; i < 3; i++) {
					if ((int)file_ext[i] == 32) break; // reached end of file ext
					cwd_string += file_name[i]; // append to cwd_string
				}
			}
			else if (target == "..") { // Delete last folder
				size_t last_pos = cwd_string.find_last_of('/');
				if (last_pos != 0) { // We aren't cding back to root
					cwd_string = cwd_string.substr(0, last_pos);
				}
				else {
					cwd_string = "/";
				}
			}
			disk.seekg(14, disk.cur);
			uint16_t jump_target;
			disk.read((char*)&jump_target, 2);
			if ((int)jump_target == 0) {
				cwd = root;
			}
			else {
				cout << jump_target << endl << cwd << endl;
				cwd = (jump_target + (((RootEntCnt * 32)/BytesPerSec)/SecPerClus) - 2) * SecPerClus * BytesPerSec + root;
				break;
			}
		}
		else {
			disk.seekg(20, disk.cur);
		}
	} while((int)file_name[0] != 0);
}

void seek_fat() { // Only looking for first FAT. Deal with redundancy later.
	cwd = SecPerClus * BytesPerSec;
}

void seek_fat_index(int16_t index) {
	seek_fat();
	disk.seekg(cwd, disk.beg);
	disk.seekg(index * 2, disk.cur);
}

void seek(string target) {
	if (target[0] == '/') {
		cwd_string = "/";
		cwd = root;
		disk.seekg(cwd, disk.beg);
	}
	string sub_target = "";
	for (int i = 0; i < target.length(); i++) {
		if (target[i] != '/') {
			sub_target += target[i];
		}
		else {
			if (sub_target == "") continue;
			cd (sub_target);
			disk.seekg(cwd, disk.beg);
			sub_target = "";
		}
	}
	if (sub_target != "") cd(sub_target);
}

void cpout(string internal, string external) {
	int cwd_bak = cwd;
	string cwd_string_bak = cwd_string;
	fstream ext_file;
	seek(internal);
	// Now at parent directory, time to build file name
	string target;
	for (int i = 0; i < internal.length(); i++) {
		if (internal[i] != '/') {
			target += internal[i];
		}
		else {
			target = "";
		}
	}
	// Now we find the target's DIR entry
	disk.seekg(cwd, disk.beg);
	uint8_t file_name[8];
	uint8_t file_ext[3];
	uint8_t DIR_Attr;
	disk.read((char*)file_name, 8);
	disk.read((char*)file_ext, 3);
	disk.read((char*)&DIR_Attr, 1);
	do {
		bool found_target = true;
		//if ((int)DIR_Attr & 16 == 16) continue; // can't copy a directory out
		for (int i = 0; i < 8; i++) {
			if ((int)file_name[i] == 32) break;
			if ((int)file_name[i] != (int)target[i]) {
				found_target = false;
				break;
			}
		}
		if (found_target) { // Found the file name, now time to check the extension
			for (int i = 0; i < 3; i++) {
				if ((int)file_ext[i] != (int)(target[target.length() - 3 + i])) {
					found_target = false;
					break;
				}
			}
		}
		if (found_target) { // Found the file!
			disk.seekg(14, disk.cur);
			uint16_t file_loc;
			uint64_t file_size;
			cout << endl << file_loc << endl << file_size << endl;
			uint8_t byte;
			disk.read((char*)&file_loc, 2);
			disk.read((char*)&file_size, 4);
			//ext_file.open(external.c_str());
			do {
				seek_fat_index(file_loc);
				disk.read((char*)&file_loc, 2);
				cwd = (file_loc + 2) * SecPerClus * BytesPerSec + root;
				for (int i = 0; i < BytesPerSec * SecPerClus && file_size > 0; i++) {
					disk.read((char*)&byte, 1);
					cout << hex << (int)byte << endl;
					file_size--;
				}
			} while ((int)file_loc != 65535); // xFFFF = 65535
		}
		disk.seekg(20, disk.cur);
		disk.read((char*)file_name, 8);
		disk.read((char*)file_ext, 3);
		disk.read((char*)&DIR_Attr, 1);
	} while ((int)file_name[0] != 0);
}

