#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 64

#define PAGE_OFFSET 12

char line[MAX_LEN];
size_t len;
ssize_t read;

unsigned long long convert(const char * str) {
	unsigned long long result = 0;
	char c; 

	while ((c = *(str++))) {
		if (c>='0' && c<='9') {
			result = (result<<4) + c - '0';
		}else if (c>='a' && c<='f') {
			result = (result<<4) + c - 'a' + 10;
		}else {
			printf("Invalid Number!\n");
			exit(1);
		}

	}
	return result;

}


int main(int argc, char **argv) {

	if (argc != 2) {
		printf("[Usage] ./process profiling_filename\n");
		return 1;
	}

	FILE *fp = fopen(argv[1], "r");
	if (!fp) {
		printf("fopen %s error\n", argv[1]);
		return 1;
	}

	while (fgets(line, MAX_LEN, fp)) {
		if (strlen(line) != 15)	continue;
		line[strcspn(line, "\n")] = 0;
		strcpy(line, line+2);
		unsigned long long va = convert(line);
		unsigned long long vfn = va >> PAGE_OFFSET;
		printf("%llu\n", vfn);
		



	}

	return 0;
}


