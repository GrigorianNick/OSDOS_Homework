/* ngg3vm

This file contains all the system info and the helper functions to read the sysinfo.

Changelog:
	- March 25: Created

*/

#include <iostream>

using namespace std;

string cwd_string; // string keeping track of where we are
int cwd; // absolute position of Current Working Directory
int root; // absolute position of root

// The FAT system
fstream disk;
// FAT system info
uint16_t BytesPerSec;
uint8_t SecPerClus;
uint16_t RsvdSecCnt;
uint8_t NumFATs;
uint16_t RootEntCnt;
uint16_t TotSec16;
uint8_t Media;
uint16_t FATSz16;
uint16_t SecPerTrk;
uint16_t NumHeads;
uint32_t HiddSec;
uint32_t TotSec32;
bool Fat32; // Is the disk a Fat32?
uint8_t DrvNum;
uint8_t Reserved1;
// Fat32 fields
uint32_t FATSz32;
uint16_t ExtFlags;
uint16_t FSVer;
uint32_t RootClus;
uint16_t FSInfo;
uint16_t BkBootSec;
uint8_t Reserved[12];
// Volume info fields
uint8_t BootSig;
bool VolInfo; // Are the Volumne Info fields present?
uint32_t VoIID;
uint8_t VoILab[11];
uint8_t FilSysType[8];

void print_bits(uint16_t in) { // toy test
        std::bitset<16> x(in);
        for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 8; j++) {
                        cout << x[j + (8*i)];
                }
                cout << "|";
        }
        cout << endl;
}

void printDiagnosticInfo() {
        cout << "DIAGNOSTICS" << endl << "=================================" << endl;
        cout << "BytesPerSec:\t" << (int)BytesPerSec << endl;
        cout << "SecPerClus:\t" << (int)SecPerClus << endl;
        cout << "RsvdSecCnt:\t" << (int)RsvdSecCnt << endl;
        cout << "NumFATs:\t" << (int)NumFATs << endl;
        cout << "RootEntCnt:\t" << (int)RootEntCnt << endl;
        cout << "TotSec16:\t" << (int)TotSec16 << endl;
        cout << "Media:\t\t" << (int)Media << endl;
        cout << "FATSz16:\t" << (int)FATSz16 << endl;
        cout << "SecPerTrk:\t" << (int)SecPerTrk << endl;
        cout << "NumHeads:\t" << (int)NumHeads << endl;
        cout << "HiddSec:\t" << (int)HiddSec << endl;
        cout << "TotSec32:\t" << (int)TotSec32 << endl;
        if (Fat32) {
                cout << "FATSz32:\t" << (int)FATSz32 << endl;
                cout << "ExtFlags:\t" << (int)ExtFlags << endl;
                cout << "FSVer:\t\t" << (int)FSVer << endl;
                cout << "RootClus:\t" << (int)RootClus << endl;
                cout << "FSInfo:\t\t" << (int)FSInfo << endl;
                cout << "BkBootSec:\t" << (int)BkBootSec << endl;
                cout << "DrvNum:\t\t" << (int)DrvNum << endl;
                cout << "Reserved1:\t" << (int)Reserved1 << endl;
        }
        else {
                cout << "DrvNum:\t\t" << (int)DrvNum << endl;
                cout << "Reserved1:\t" << (int)Reserved1 << endl;
        }
        cout << "BootSig:\t" << (int)BootSig << endl;
	if (VolInfo) {
                cout << "VoIID:\t\t" << (int)VoIID << endl;
                cout << "VoILab:\t\t" << VoILab << endl;
                cout << "FilSysType:\t" << FilSysType << endl;
        }
}

void loadFATInfo() {
        // Loading info
        disk.seekg(11, disk.beg);
        disk.read((char*)&BytesPerSec, 2);
        disk.read((char*)&SecPerClus, 1);
        disk.read((char*)&RsvdSecCnt, 2);
        disk.read((char*)&NumFATs, 1); // This should be 2
        disk.read((char*)&RootEntCnt, 2);
        disk.read((char*)&TotSec16, 2);
        disk.read((char*)&Media, 1);
        disk.read((char*)&FATSz16, 2); // 0 means we're a FAT32
        disk.read((char*)&SecPerTrk, 2);
        disk.read((char*)&NumHeads, 2);
        disk.read((char*)&HiddSec, 4);
        disk.read((char*)&TotSec32, 4);
        if ((int)FATSz16 == 0) {
                Fat32 = true;
                disk.read((char*)&FATSz32, 4);
                disk.read((char*)&ExtFlags, 2);
                disk.read((char*)&FSVer, 2);
                disk.read((char*)&RootClus, 4);
                disk.read((char*)&FSInfo, 2);
                disk.read((char*)&BkBootSec, 2);
                disk.read((char*)Reserved, 12);
                disk.read((char*)&DrvNum, 1);
                disk.read((char*)&Reserved1, 1);
        }
        else {
                disk.read((char*)&DrvNum, 1);
                disk.read((char*)&Reserved1, 1);
	}
        disk.read((char*)&BootSig, 1);  
        if (BootSig == 41) { // The next three flags are set
                VolInfo = true;         
                disk.read((char*)&VoIID, 4); 
                disk.read((char*)VoILab, 11);   
                disk.read((char*)FilSysType, 8);
        }                               
}
