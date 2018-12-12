// creates a hash based on src in dest
// src - 64-bit string
// dest - 32-bit string
void hash(unsigned char* src, char* dest) {
	int sum = 0;
	int i;
	for (i = 0; i < 64; i ++) {
		sum += src[i];
	}
	for (i = 0; i < 32; i ++) {
		int tmp = 0;
		int j;
		for (j = 0; j < 16; j ++) {
			tmp += src[(i + j) % 32] * src[(i + j) % 32 + 32];
		}
		tmp = tmp % sum;
		tmp = tmp % 255;
		dest[i] = (char) tmp;
	}
}

// copies n bytes of the string at src to the string at dest
void strcopy(char* dest, char* src, int n) {
	int i;
	for (i = 0; i < n; i ++) {
		dest[i] = src[i];
	}
}

unsigned long long seed = 1836431607u;
unsigned long long int mod = 3537542985u;
// returns a random number
int getRand() {
	unsigned long long mult = 4; // IEEE vetted random number #randomized by the stack
	mult += 3774864283u;
	unsigned long long tmp = seed * mult;
	seed = tmp & mod;
	return seed;
}

// returns a 32-bit random string into the salt string
void getSalt(char* salt) {
	int* tmp = (int*) salt;
	int i;
	for (i = 0; i < 8; i ++) {
		tmp[i] = getRand();
	}
}

// takes a password and salt and hashes them 200,000 times,
// meaning that it will take approximately half a second to compute
// password - a 32-character block of memory
// salt - a 32-character string generated with getSalt()
// hashResult - 32-character return block
void hashPassword(char* password, char* salt, char* hashResult) {
	unsigned char combined[64];
	char tmp[32];
	strcopy((char*) combined, password, 32);
	strcopy((char*) combined + 32, salt, 32);
	
	hash(combined, tmp);
	
	int i;
	for (i = 0; i < 20000; i ++) {
		strcopy((char*) combined, tmp, 32);
		strcopy((char*) combined + 32, salt, 32);
		hash(combined, tmp);
	}
	
	strcopy(hashResult, tmp, 32);
}
