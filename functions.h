/* ngg3vm

This is the file that contains all the functions specified in the assignment, as well as their helper functions. The impelementation SHOULD be in its own .cpp file, but I didn't get around to properly organizing the project.

- print_DIR_entry():				Prints a given directory entry. Assumes disk head is at the start of a directory entry.
- seek_fat():					Moves cwd to start of FAT
- seek_fat_index(int16_t index):		Moves disk head to FAT entry specified by index
- ls():						Prints current directory, works with multi-clustered directory files
- cd(string target):				Moves cwd to directory specified by target. Does not work with multi-clustered directory files.
- seek(string target):				Traverses FS to directory specified by target. Handles both relative and absolute pathing.
- cpin(stirng internal, string external):	Skeleton code for copying file out of FS. Does not work.
- cpout(string internal, string external):	Copies a file out of the FS into the "real" FS. Works with multi-clustered files, but not multi-clustered directories.

A full timeline of changes can be found at https://github.com/GrigorianNick/OSDOS_Homework/commits/master/functions.h

- March 25: Created functions.h with cd and ls
- March 28: Added proper targetting for cd and absolute/relative pathing to ls
- March 29: Created seek functionality
- March 30: Fixed some magic numbers
- March 31: Added cpout, seek_fat, and seek_fat_index

*/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdint.h>
#include <bitset>
#include <string>

#include "sysinfo.h"

using namespace std;

void print_DIR_entry() {
	uint8_t file_name[8];
	uint8_t file_ext[3];
	uint8_t DIR_Attr;
	disk.read((char*)file_name, 8);
	disk.read((char*)file_ext, 3);
	disk.read((char*)&DIR_Attr, 1);
	if (((int)DIR_Attr & 2) == 2) { // Hidden folder, skip
		return;
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
}

void seek_fat() { // Only looking for first FAT. Deal with redundancy later.
	cwd = RsvdSecCnt * BytesPerSec;
}

void seek_fat_index(int16_t index) { // Move to FAT[index]. DOES NOT CHANGE CWD!
	seek_fat();
	disk.seekg(cwd, disk.beg);
	disk.seekg(index * 2, disk.cur);
}

void ls() {
	int num_dir_entries = (BytesPerSec * SecPerClus) / 32;
	if (cwd == root) { // Root can't be multi-clustered
		do {
			print_DIR_entry();
			disk.seekg(20, disk.cur);
			num_dir_entries--;
		} while ((int)disk.peek() != 0 && num_dir_entries > 0);
	}
	else { // We're looking at a different folder
		print_DIR_entry(); // This gets us to byte 12 of entry "."
		disk.seekg(14, disk.cur); // This gets us to the first cluster entry
		uint16_t next_clus;
		disk.read((char*)&next_clus, 2); // FAT entry we need to look at
		disk.seekg(4, disk.cur); // Bring us up to the next entry
		num_dir_entries--; // We already read an entry
		int cwd_bak = cwd; // Back up where we are
		do {
			disk.seekg(cwd, disk.beg); // Redundant for the first iteration
			do { // Print the directory like normal
				print_DIR_entry();
				disk.seekg(20, disk.cur);
				num_dir_entries--;
			} while((int)disk.peek() != 0 && num_dir_entries > 0);
			num_dir_entries = (BytesPerSec * SecPerClus) / 32; // Reset number of allowed entries
			seek_fat_index(next_clus); // Jump to FAT entry for this directory
			disk.read((char*)&next_clus, 2); // Read next dir we need to go to
			cwd = (next_clus + (((RootEntCnt * 31)/BytesPerSec)/SecPerClus) - 2) * SecPerClus * BytesPerSec + root; // Calculate our jump cluster 
		}
		while((int)next_clus != 65535); //65535 = xFFFF
		cwd = cwd_bak; // Restore us to the original diretory
	}
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
				cwd = (jump_target + (((RootEntCnt * 32)/BytesPerSec)/SecPerClus) - 2) * SecPerClus * BytesPerSec + root;
				break;
			}
		}
		else {
			disk.seekg(20, disk.cur);
		}
	} while((int)file_name[0] != 0);
}

void seek(string target) { // Break <dst> into bite-sized pieces for cd
	if (target[0] == '/') { // Absolute pathing, reset to root DIR
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
	if (sub_target != "") cd(sub_target); // Needed since there isn't a closing slash to trigger the cd
}

void cpin(string internal, string external) { // Does not work
	int cwd_bak = cwd;
	string cwd_string_bak = cwd_string;
	fstream ext_file;
	seek(internal);
	disk.seekg(cwd, disk.beg);
	disk.seekg(26, disk.cur);
	uint16_t next_clus, curr_clus;
	disk.read((char*)&next_clus, 2);
	seek_fat_index(next_clus);
	do {
		curr_clus = next_clus;
		disk.read((char*)&next_clus, 2);
		seek_fat_index(next_clus);
	} while((int)next_clus != 65535);
	cwd = (curr_clus + (((RootEntCnt * 31)/BytesPerSec)/SecPerClus) - 2) * SecPerClus * BytesPerSec + root; // Jump to the last cluster in the directory
	disk.seekg(cwd, disk.beg);
	//Scan until we find an empty entry
	bool full = false;
	while ((int)disk.peek() != 0) {
		disk.seekg(32, disk.cur);
	}
	// Now at empty entry // Stopped here
	string target = "";
	for (int i = 0; i < internal.length(); i++) {
		if (internal[i] != '/') {
			target += internal[i];
		}
		else {
			target = "";
		}
	}
	// Now break the target down into file_name and file_ext
	string file_name = "";
	string file_ext = "";
	for (int i = 0; i < 8; i++) {
		if (target[i] == '.') break;
		file_name += target[i];
	}
	for (int i = 0; i < 3; i++) {
		file_ext += target[target.length() - 3 + i]; // We're just going to pretend everybody has a three letter extension
	}
	// Find next opening in directory
	uint8_t byte;
	byte = disk.peek();
	while((int)byte != 0) {
		disk.seekg(32, disk.cur);
		byte = disk.peek();
	}
	int dir_entry = disk.tellg(); // Store where our data entry is
	// Format data struct with 0x20
	uint8_t format[11] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
	disk.write((char*)format, 11);
	// Write file name
	disk.seekg(dir_entry, disk.beg);
	disk << file_name; // Write file name
	disk.seekg(dir_entry + 8, disk.beg);
	disk << file_ext; // Write file extension
	disk << 32; // We are an archive
	// Seek for first open cluster
	seek_fat();
	disk.seekg(cwd, disk.beg);
	byte = disk.peek();
	uint16_t clus_num = 0;
	while (byte != 0 && byte) { // Found an open cluster!
		clus_num++;
		disk.seekg(2, disk.cur);
		byte = disk.peek();
	}
	disk << 65535; // We only handle single-cluster files
	// Record which cluster we're starting in
	disk.seekg(dir_entry + 26, disk.beg);
	disk.write((char*)&clus_num, 2);
	// Move to cluster start
	cwd = (clus_num + (((RootEntCnt * 32)/BytesPerSec)/SecPerClus)-2) * SecPerClus * BytesPerSec + root;
	// Copy file over
	disk.seekg(cwd, disk.beg);
	ext_file.open(external.c_str());
	uint64_t file_size = 0;
	while (ext_file.read((char*)&byte,1)) {
		disk << byte;
		file_size++;
	}
	ext_file.close();
	// Record file size in dir entry
	disk.seekg(dir_entry + 28, disk.beg);
	disk << file_size;
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
	int num_dir_entries = (BytesPerSec * SecPerClus) / 32;
	// Loop through the DIR looking for our target 
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
			uint8_t byte;
			ext_file.open(external.c_str(), fstream::out);
			disk.read((char*)&file_loc, 2);
			disk.read((char*)&file_size, 4);
			// Loop through file, pull out and write its data into <dst> one byte at a time
			do {
				cwd = (file_loc + (((RootEntCnt * 32)/BytesPerSec/SecPerClus) - 2)) * SecPerClus * BytesPerSec + root;
				disk.seekg(cwd, disk.beg);
				for (int i = 0; i < BytesPerSec * SecPerClus && file_size > 0; i++) {
					disk.read((char*)&byte, 1);
					ext_file.write((char*)&byte, 1);
					file_size--;
				}
				seek_fat_index(file_loc);
				disk.read((char*)&file_loc, 2);
			} while ((int)file_loc != 65535); // xFFFF = 65535
			ext_file.close();
		}
		disk.seekg(20, disk.cur);
		disk.read((char*)file_name, 8);
		disk.read((char*)file_ext, 3);
		disk.read((char*)&DIR_Attr, 1);
		num_dir_entrries--;
	} while ((int)file_name[0] != 0 && num_dir_entries > 0);
}

