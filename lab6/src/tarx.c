// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>

int readFileNameSize();

char *readFileName(int fnSize);

long readINode();

int readMode();

long readModTime();

long readFileSize();

char *readBytes(long fileSize);

int main() {
	
	int fileNameSize = readFileNameSize();
	printf("fnSize: %d\n", fileNameSize);
	
	char *fileName = readFileName(fileNameSize);
	printf("fn: %s\n", fileName);

	long iNode = readINode();
	printf("iNode: %d\n", iNode);

	int mode = readMode();
	printf("mode: %d\n", mode);
	
	long modTime = readModTime();
	printf("modTime: %d\n", modTime);

	long fileSize = readFileSize();
	printf("fileSize: %d\n", fileSize);

	char *bytes = readBytes(fileSize);
	printf("bytes: %s\n", bytes);

	return 0;
}

int readFileNameSize() {
	int fileNameSize, i;

	fread(&fileNameSize, sizeof(int), 1, stdin);
	
	return fileNameSize;
}

char *readFileName(int fnSize) {
	char *fileName = (char *) malloc(fnSize * sizeof(char) + 1);
	char temp;
	int i;

	fileName[fnSize] = '\0';
	for (i = 0; i < fnSize; i++) {
		fread(&temp, 1, 1, stdin);
		fileName[i] = temp;
	}

	return fileName;
}

long readINode() {
	long iNode;

	fread(&iNode, sizeof(long), 1, stdin);

	return iNode;
}

int readMode() {
	int mode;

	fread(&mode, sizeof(int), 1, stdin);

	return mode;
}

long readModTime() {
	long modTime;

	fread(&modTime, sizeof(long), 1, stdin);

	return modTime;
}

long readFileSize() {
	long fileSize;

	fread(&fileSize, sizeof(long), 1, stdin);

	return fileSize;
}

char *readBytes(long fileSize) {
	char *bytes = (char *) malloc(fileSize * sizeof(char) + 1);
	char temp;
	int i;

	printf("BYTES - fSize: %d\n", fileSize);
	bytes[fileSize] = '\0';
	for (i = 0; i < fileSize; i++) {
		printf("BYTES - made it here (i = %d)\n", i);
		fread(&temp, 1, 1, stdin);
		bytes[i] = temp;
	}
	
	return bytes;
}