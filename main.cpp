/* ngg3vm

This is the shell portion of the program. It prints the prompt, does basic user input parsing, and makes calls on the user's behalf.

All code can be viewed at https://github.com/GrigorianNick/OSDOS_Homework

compile: make

Complete changelog can be found here: https://github.com/GrigorianNick/OSDOS_Homework/commits/master/main.cpp

User actions:
	- cd:			Takes user to root
	- cd <dst>:		Moves user into <dst> subdirectory. Only works with monocluster directory files.
	- ls:			Lists contents of directory. Works with multi-cluster directory files.
	- ls <dst>:		Lists contents of <dst> ditrectory. If cd can reach <dst>, then it works with multi-cluster directory files.
	- cpout <src> <dst>:	Copies <src> out of the FS and into <dst> on the filesystem. If cd can reach <src>, then it works with multi-cluster directory files
	- cpin <src> <dst>:	Does not work.

Change-log:
	- March 22: Created main.cpp, read the BPB cluster
	- March 23: Added ls
	- March 24: Formatted the prompt, added cd
	- March 25: Bugfixing cd then pulled FS functions into functions.h and system info into sysinfo.h
	- March 28: Added cwd to prompt

*/

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
	//printDiagnosticInfo(); // Prints sysinfo stuff.
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
		else if (input.substr(0,5) == "cpout") {
			int cwd_bak = cwd;
			string cwd_string_bak = cwd_string;
			string arguments = input.substr(6);
			bool flag = true;
			string internal = "";
			string external = "";
			for (int i = 0; i < arguments.length(); i++) {
				if (arguments[i] == ' ') {
					flag = false;
					continue;
				}
				if (flag) internal += arguments[i];
				else external += arguments[i];
			}
			cpout(internal, external);
			cwd = cwd_bak;
			cwd_string = cwd_string_bak;
		}
		else if (input.substr(0,4) == "cpin") { // Doesn't work
			int cwd_bak = cwd;
			string cwd_string_bak = cwd_string;
			string arguments = input.substr(5);
			bool flag = false;
			string internal = "";
			string external = "";
			for (int i = 0; i < arguments.length(); i++) {
				if (arguments[i] == ' ') {
					flag = !flag;
					continue;
				}
				if (flag) internal += arguments[i];
				else external += arguments[i];
			}
			cpin(internal, external);
			cwd = cwd_bak;
			cwd_string = cwd_string_bak;
		}
		disk.seekg(cwd, disk.beg);
		cout << ": " << cwd_string << " > ";
		getline(cin, input);
	}
	disk.close();
	return 0;
}
