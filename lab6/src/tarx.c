// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "jrb.h"

int readFileNameSize();

char *readFileName(int fnSize);

long readINode();

int readMode();

long readModTime();

long readFileSize();

char *readBytes(long fileSize);

int compare(Jval v1, Jval v2);

int main() {
	JRB iNodeTree = make_jrb();

	while (1) {
		int fileNameSize = readFileNameSize();

		if (fileNameSize == -1) {
			break;
		}
		// printf("\nfnSize: %d\n", fileNameSize);
		
		char *fileName = readFileName(fileNameSize);
		// printf("fn: %s\n", fileName);

		long iNode = readINode();
		// printf("iNode: %d\n", iNode);

		if (jrb_find_gen(iNodeTree, new_jval_l(iNode), compare) == NULL) {
		jrb_insert_gen(iNodeTree, new_jval_l(iNode), new_jval_i(0), compare);
		// printf("New jrb\n");
		}
		else {
			// printf("Jrb already exists\n");
		}

		int mode = readMode();
		// printf("mode: %d\n", mode);

		long modTime = readModTime();
		printf("modTime: %d\n", modTime);

		// printf("Mode %d Mtime %d\nName: %s\n\n", mode, modTime, fileName);

		if (S_ISDIR(mode)) {
			// printf("is directory\n");
			if (mkdir(fileName, mode) == -1) {
				fprintf(stderr, "Failed to create directory %s\n", fileName);
			}
			chmod(fileName, mode);
		}
		else {
			// printf("is file\n");
			long fileSize = readFileSize();
			// printf("fileSize: %d\n", fileSize);

			char *bytes = readBytes(fileSize);
			// printf("bytes: %s\n", bytes);

			FILE *file = fopen(fileName, "w");
			if (file == NULL) {
				fprintf(stderr, "Error creating %s\n", fileName);
				exit(1);
			}
			chmod(fileName, mode);
			fwrite(bytes, 1, fileSize, file);

			fclose(file);
		}

		struct timeval timeArray[2];
		gettimeofday(&timeArray[0], NULL);
		timeArray[0].tv_usec = 0;
		timeArray[1].tv_sec = modTime;
		timeArray[1].tv_usec = 0;
		if (utimes(fileName, timeArray) == -1) {
			fprintf(stderr, "Error with utimes\n");
		}
		else {
			printf("Calling utimes on %s\n", fileName);		
			printf("Curr - %d %d\n", timeArray[0].tv_sec, timeArray[0].tv_usec);
			printf("ModTime - %d %d\n", timeArray[1].tv_sec, timeArray[1].tv_usec);
		}
		// Add filename and modTime to dll and set mod time after this loop

	}

	return 0;
}

int readFileNameSize() {
	int fileNameSize, i;

	if(!fread(&fileNameSize, sizeof(int), 1, stdin)) {
		return -1;
	}
	
	return fileNameSize;
}

char *readFileName(int fnSize) {
	char *fileName = (char *) malloc(fnSize * sizeof(char) + 1);
	char temp;
	int i;

	fileName[fnSize] = '\0';
	for (i = 0; i < fnSize; i++) {
		if(!fread(&temp, 1, 1, stdin)) {
			fprintf(stderr, "Error reading in file name\n");
			exit(1);
		}
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

	// printf("BYTES - fSize: %d\n", fileSize);
	bytes[fileSize] = '\0';
	for (i = 0; i < fileSize; i++) {
		// printf("BYTES - made it here (i = %d)\n", i);
		fread(&temp, 1, 1, stdin);
		bytes[i] = temp;
	}
	
	return bytes;
}

int compare(Jval v1, Jval v2)           /* Adding a comparison function for inodes. */
{
  if (v1.l < v2.l) return -1;
  if (v1.l > v2.l) return 1;
  return 0;
}