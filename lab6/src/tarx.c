// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include "jrb.h"
#include "dllist.h"

/* Function that reads the file name size */
int readFileNameSize();

/* Reads the file name given the size */
char *readFileName(int fnSize);

/* Reads the inode number */
long readINode();

/* Reads the mode */
int readMode();

/* Reads the modification time */
long readModTime();

/* Reads the file size */
long readFileSize();

/* Reads the bytes of the file */
char *readBytes(long fileSize);

/* Function that compares the inode values for the jrb tree */
int compare(Jval v1, Jval v2);

int main() {
	
	/* Tree to store the inodes */
	JRB iNodeTree = make_jrb();

	/* Lists to store information about the files */
	Dllist fnList = new_dllist();
	Dllist timeList = new_dllist();
	Dllist modeList = new_dllist();

	/* Reading stdin */
	while (1) {

		int fileNameSize = readFileNameSize();

		/* Exits the loop if it couldn't read anymore from stdin */
		if (fileNameSize == -1) {
			break;
		}
		
		char *fileName = readFileName(fileNameSize);

		long iNode = readINode();

		/* Checking if inode already exists in the tree */
		JRB foundINode = jrb_find_gen(iNodeTree, new_jval_l(iNode), compare);

		/* If it's a distinct inode, insert it into the tree */
		if (foundINode == NULL) {
			jrb_insert_gen(iNodeTree, new_jval_l(iNode), new_jval_s(strdup(fileName)), compare);
		}

		/* If it already exists in the tree, create a link with the original file */
		else {
			link(foundINode->val.s, fileName);
			continue;
		}

		int mode = readMode();

		long modTime = readModTime();

		/* If it's a directory, create it */
		if (S_ISDIR(mode)) {
			if (mkdir(fileName, 0777) == -1) {
				fprintf(stderr, "Failed to create directory %s\n", fileName);
				exit(1);
			}
		}

		/* If it's a file */
		else {
			
			/* Get file size and read the bytes */
			long fileSize = readFileSize();
			char *bytes = readBytes(fileSize);

			/* Open the file for writing */
			FILE *file = fopen(fileName, "w");
			if (file == NULL) {
				fprintf(stderr, "Couldn't open file\n");
				exit(1);
			}

			/* Write the bytes to the file, then close it */
			fwrite(bytes, 1, fileSize, file);
			free(bytes);
			fclose(file);
		}

		/* Create the timeval array for utimes */
		struct timeval *timeArray = (struct timeval *) malloc(2 * sizeof(struct timeval));

		gettimeofday(&timeArray[0], NULL);
		timeArray[0].tv_usec = 0;
		timeArray[1].tv_sec = modTime;
		timeArray[1].tv_usec = 0;

		/* Append the information to the lists (This helps get passed issues with permissions and access times) */
		dll_append(fnList, new_jval_s(strdup(fileName)));
		dll_append(timeList, new_jval_v((void *) timeArray));
		dll_append(modeList, new_jval_i(mode));
	}

	/* Iterators for my list traverse */
	Dllist fnIt = dll_last(fnList);
	Dllist timeIt = dll_last(timeList);
	Dllist modeIt = dll_last(modeList);

	/* Traverse through the lists, set the modtime, and chmod the file/directory */
	while(fnIt != dll_nil(fnList) && timeIt != dll_nil(timeList)) {
		struct timeval *tmpStruct = (struct timeval *) timeIt->val.v;

		if (utimes(fnIt->val.s, tmpStruct) == -1) {
			fprintf(stderr, "Error with utimes\n");
			exit(1);
		}

		chmod(fnIt->val.s, modeIt->val.i);
		
		fnIt = fnIt->blink;
		timeIt = timeIt->blink;
		modeIt = modeIt->blink;
	}

	/* Freeing memory from the Dllists */
	Dllist tmp;
	dll_traverse(tmp, fnList) free(tmp->val.s);
	dll_traverse(tmp, timeList) {
		struct timeval *tmpStruct = (struct timeval *) timeIt->val.v;
		free(tmpStruct);
	}

	free_dllist(fnList);
	free_dllist(timeList);
	free_dllist(modeIt);

	return 0;
}

/* Function that reads the file name size */
int readFileNameSize() {
	int fileNameSize, i;

	/* Returning -1 so main knows when to exit loop */
	if(!fread(&fileNameSize, sizeof(int), 1, stdin)) {
		return -1;
	}
	
	return fileNameSize;
}

/* Reads the file name given the size */
char *readFileName(int fnSize) {
	char *fileName = (char *) malloc(fnSize * sizeof(char) + 1);
	char temp;
	int i;

	/* Setting the null character */
	fileName[fnSize] = '\0';

	/* Reading bytes one at a time */
	for (i = 0; i < fnSize; i++) {
		if(!fread(&temp, 1, 1, stdin)) {
			fprintf(stderr, "Error reading in file name\n");
			exit(1);
		}
		fileName[i] = temp;
	}

	return fileName;
}

/* Reads the inode number */
long readINode() {
	long iNode;

	if(!fread(&iNode, sizeof(long), 1, stdin)) {
		fprintf(stderr, "Couldn't read iNode\n");
		exit(1);
	}

	return iNode;
}

/* Reads the mode */
int readMode() {
	int mode;

	fread(&mode, sizeof(int), 1, stdin);

	return mode;
}

/* Reads the modification time */
long readModTime() {
	long modTime;

	if(!fread(&modTime, sizeof(long), 1, stdin)) {
		fprintf(stderr, "Error reading modTime\n");
		exit(1);
	}

	return modTime;
}

/* Reads the file size */
long readFileSize() {
	long fileSize;

	if(!fread(&fileSize, sizeof(long), 1, stdin)) {
		fprintf(stderr, "Couldn't read file size\n");
		exit(1);
	}

	return fileSize;
}

/* Reads the bytes of the file */
char *readBytes(long fileSize) {
	char *bytes = (char *) malloc(fileSize * sizeof(char) + 1);
	char temp;
	int i;

	/* Setting the null character */
	bytes[fileSize] = '\0';

	/* Reading bytes one at a time */
	for (i = 0; i < fileSize; i++) {
		if(!fread(&temp, 1, 1, stdin)) {
			fprintf(stderr, "Error reading bytes\n");
			exit(1);
		}
		bytes[i] = temp;
	}
	
	return bytes;
}

/* Function that compares the inode values for the jrb tree */
int compare(Jval v1, Jval v2) {
  if (v1.l < v2.l) return -1;
  if (v1.l > v2.l) return 1;
  return 0;
}