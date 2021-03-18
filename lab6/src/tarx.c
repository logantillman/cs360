// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>

int readFileNameSize();

char *readFileName(int fnSize);

long readINode();

short readMode();

long readModTime();

long readFileSize();

int main() {
	
	int fileNameSize = readFileNameSize();
	printf("fnSize: %d\n", fileNameSize);
	
	//char *fileName = (char *) malloc((fileNameSize + 1) * sizeof(char));
	char *fileName = readFileName(fileNameSize);
	printf("fn: %s\n", fileName);
	return 0;
}

int readFileNameSize() {
	int fileNameSize;

	fread(&fileNameSize, 1, sizeof(int), stdin);
		
	return fileNameSize;
}

char *readFileName(int fnSize) {
	char *fileName;
	
	fread(&fileName, 1, 1, stdin);
	
	printf("Read fn: %x\n", fileName);

	return fileName;
}
