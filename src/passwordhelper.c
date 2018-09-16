#include <stdio.h>
#include <stdlib.h>

const char* digittable = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

int numFromDigit(char digit) {
	switch(digit) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': return 10;
		case 'b': return 11;
		case 'c': return 12;
		case 'd': return 13;
		case 'e': return 14;
		case 'f': return 15;
		case 'g': return 16;
		case 'h': return 17;
		case 'i': return 18;
		case 'j': return 19;
		case 'k': return 20;
		case 'l': return 21;
		case 'm': return 22;
		case 'n': return 23;
		case 'o': return 24;
		case 'p': return 25;
		case 'q': return 26;
		case 'r': return 27;
		case 's': return 28;
		case 't': return 29;
		case 'u': return 30;
		case 'v': return 31;
		case 'w': return 32;
		case 'x': return 33;
		case 'y': return 34;
		case 'z': return 35;
		case 'A': return 36;
		case 'B': return 37;
		case 'C': return 38;
		case 'D': return 39;
		case 'E': return 40;
		case 'F': return 41;
		case 'G': return 42;
		case 'H': return 43;
		case 'I': return 44;
		case 'J': return 45;
		case 'K': return 46;
		case 'L': return 47;
		case 'M': return 48;
		case 'N': return 49;
		case 'O': return 50;
		case 'P': return 51;
		case 'Q': return 52;
		case 'R': return 53;
		case 'S': return 54;
		case 'T': return 55;
		case 'U': return 56;
		case 'V': return 57;
		case 'W': return 58;
		case 'X': return 59;
		case 'Y': return 60;
		case 'Z': return 61;
		default: break;
	}
	printf("Digit %c is not valid\n", digit);
	exit(1);
}

// This is essentially the task of converting index to base 62, pad with 0
void nth_pwd(unsigned long long index, short pwlen, char* nthpwd) {
	for (int i = 0; i < pwlen; i++)
		nthpwd[i] = '0';

	int remainder;
	int c = 0;
	while(index >= 62) {
		remainder = index % 62;
		index = index / 62;
		nthpwd[pwlen - c - 1] = digittable[remainder]; // fills in least sig. first
		c++;
	}
	nthpwd[pwlen - c - 1] = digittable[index];
}

unsigned long long get_index(char* pwd, short pwdlen) {
	unsigned long long retval = 0;
	unsigned long long temp = 0;
	for (int i = 0; i < pwdlen; i++) {
		temp = 1;
		for (int j = 0; j < pwdlen - i - 1; j++) {
			temp *= 62;
		}
		retval += temp * numFromDigit(pwd[i]);
	}
	return retval;
}

void calc_pwrange(int nworkers, int rangeindex, short pwlen, unsigned long long* start, unsigned long long* range) {
	unsigned long long totalpwds = 1;
	for (short i = 0; i < pwlen; i++) {
		totalpwds *= 62;
	}
	*range = totalpwds / nworkers;
	*start = rangeindex * *range;
	// last worker
	if (rangeindex == nworkers - 1) {
		*range = totalpwds - *start; // make sure none are missed from rounding
	}
}
