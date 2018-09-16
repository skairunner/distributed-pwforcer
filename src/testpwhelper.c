#include <passwordhelper.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	char* pwd = "1234";
	short pwdlen = 4;
	char* pwd2 = malloc(sizeof(char) * (pwdlen + 1));
	pwd2[4] = 0;
	unsigned long long index = get_index(pwd, pwdlen);
	printf("Pwd  : %s\n", pwd);
	printf("Index: %llu\n", index);
	nth_pwd(index, pwdlen, pwd2);
	printf("nth  : %s\n", pwd2);

	return 0;
}