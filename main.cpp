#include <iostream>
#include <fstream>
#include <stdint.h> // needed for uint8_t
#include <bitset>

using namespace std;

void print_bits(uint64_t in) { // toy test
	std::bitset<64> x(in);
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			cout << x[j + (8*i)];
		}
		cout << "|";
	}
	cout << endl;
}

void endian_swap(uint8_t* in, int size) { // Convert between endians inplace
	uint8_t temp;
	for (int i = 0; i < size/2; i++) {
		temp = in[i];
		in[i] = in[size - i - 1];
		in[size - i - 1] = temp;
	}
}

int main( int argc, char *argv[] ) {
	if (argc != 2) {
		cout << "Error: improper number of arguments." << endl;
		return 1;
	}
	fstream myfile;
	myfile.open(argv[1]);
	myfile.close();
	string input;
	/*cout << ":";
	cin >> input;
	while (input != "exit") {
		cout << ":";
		cin >> input;
	}*/

	uint64_t i = 2168595480;
	cout << sizeof(i) << endl;
	print_bits(i);
	endian_swap((uint8_t*)&i, sizeof(i));
	print_bits(i);
	return 0;
}
