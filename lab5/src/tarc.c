// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "jrb.h"
#include "dllist.h"

void traverseDirectory(const char *fileName, JRB iNodeTree, char *relativePath);

char *findRelativePath(char *pathName);

void printFileNameSize(int fileNameSize);

void printHexString(const char *string);

void printINode(unsigned long iNode);

void printMode(unsigned short mode);

void printModTime(long modTime);

void printFileSize(const char *fileName);

void printBytes(const char *fileName);

//void printLittleEndian(int num, int size);

int compare(Jval v1, Jval v2);

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Incorrect amount of arguments\n");
		return -1;
	}	

	JRB iNodeTree = make_jrb();
	
	char *tmp = strdup(argv[1]);
	char *relativePath = findRelativePath(tmp);
	//printf("%s %s\n", argv[1], relativePath);
	traverseDirectory(argv[1], iNodeTree, relativePath);

	free(tmp);
	return 0;
}

void traverseDirectory(const char *fileName, JRB iNodeTree, char *relativePath) {
	DIR *d = opendir(fileName);
	
	/* If directory failed to open, print an error */
	if (d == NULL) {
		fprintf(stderr, "Couldn't open %s directory\n", fileName);
		exit(1);
	}
	
	struct dirent *de;
	struct stat buf;
	Dllist dirList = new_dllist();
	int exists;

	int fileNameSize = strlen(fileName);
	int dirFileNameSize = fileNameSize + 10;
	char *dirFileName = (char *) malloc(sizeof(char) * dirFileNameSize);

	/* Error checking for failed malloc */
	if (dirFileName == NULL) {
		fprintf(stderr, "malloc error\n");
		exit(1);
	}

	/* Printing out the info for the directory */
	exists = stat(fileName, &buf);
	if (exists < 0) {
		fprintf(stderr, "Couldn't stat %s\n", fileName);
		exit(1);
	}

	/* Printing mandatory information */
	printFileNameSize(strlen(relativePath));
	printHexString(relativePath);
	printINode(buf.st_ino);

	/* Printing if first time inode is seen */
	JRB iNodeLoc = jrb_find_gen(iNodeTree, new_jval_l(buf.st_ino), compare);
	if (iNodeLoc == NULL && S_ISDIR(buf.st_mode)) {
		jrb_insert_gen(iNodeTree, new_jval_l(buf.st_ino), new_jval_i(0), compare);
		printMode(buf.st_mode);
		printModTime(buf.st_mtime);
	}

	/* Adding the / to the fileName */
	strcpy(dirFileName, fileName);
	strcat(dirFileName + fileNameSize, "/");

	/* Reading everything in the directory */
	for (de = readdir(d); de != NULL; de = readdir(d)) {
		if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
			continue;
		}

		int sz = strlen(de->d_name);
		if (dirFileNameSize < fileNameSize + sz + 2) {
			dirFileNameSize = fileNameSize + sz + 10;
			dirFileName = realloc(dirFileName, dirFileNameSize);
		}
		strcpy(dirFileName + fileNameSize + 1, de->d_name);

		exists = stat(dirFileName, &buf);
		if (exists < 0) {
			fprintf(stderr, "Couldn't stat %s\n", de->d_name);
			exit(1);
		}
		
		/* Checking if the file inode already exists in the tree */
		iNodeLoc = jrb_find_gen(iNodeTree, new_jval_l(buf.st_ino), compare);
		if (iNodeLoc == NULL && !S_ISDIR(buf.st_mode)) {
			jrb_insert_gen(iNodeTree, new_jval_l(buf.st_ino), new_jval_i(0), compare);
		}

		/* If it's a directory, add it to the list of directories to visit */
		if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
			dll_append(dirList, new_jval_s(strdup(dirFileName)));
		}
	
		/* If it's a file, print out the necessary info */
		else if (!S_ISDIR(buf.st_mode)) {
			char *tmp = strdup(dirFileName);
			char *fn = findRelativePath(tmp);
			char *filePath = (char *) malloc((strlen(relativePath) + 1 + strlen(fn)) * sizeof(char));
			strcpy(filePath, relativePath);
			int filePathSize = strlen(filePath);
			strcat(filePath + filePathSize, "/");
			strcat(filePath + filePathSize + 1, fn);

			printFileNameSize(strlen(filePath));
			printHexString(filePath);
			printINode(buf.st_ino);

			if (iNodeLoc == NULL) {
				printMode(buf.st_mode);
				printModTime(buf.st_mtime);
				printFileSize(dirFileName);
				printBytes(dirFileName);
			}
			free(tmp);
		}
	}
	
	/* Close the current directory */
	closedir(d);
	
	/* Recursively traverse the nested directories */
	Dllist tmp;
	dll_traverse(tmp, dirList) {
		printf("%s Dir list: %s\n", relativePath, tmp->val.s);
		int relativePathSize = strlen(relativePath);
		strcat(relativePath + relativePathSize, "/");
		char *tmpString = strdup(tmp->val.s);
		strcat(relativePath + relativePathSize + 1, findRelativePath(tmpString));
		printf("New dir: %s\n", relativePath);
		free(tmpString);
		traverseDirectory(tmp->val.s, iNodeTree, relativePath);
	}

	/* Free all of the directories in the list */
	dll_traverse(tmp, dirList) {
		free(tmp->val.s);
	}

	/* Free the memory allocated for the list and the directory file name */
	free_dllist(dirList);
	free(dirFileName);
}

char *findRelativePath(char *pathName) {
	char *foundSlash = strtok(pathName, "/");
	char *relative;

	while (foundSlash != NULL) {
		relative = foundSlash;
		foundSlash = strtok(NULL, "/");
	}
	return relative;
}

void printFileNameSize(int fileNameSize) {
	int i, j;	

	for (i = 0; i < 4; i++) {
		j = fileNameSize & 0xFF;
		fileNameSize = fileNameSize >> 8;
		fwrite(&j, 1, 1, stdout);
	}
}

void printHexString(const char *string) {
	fwrite(string, sizeof(char), strlen(string), stdout);
}

void printINode(unsigned long iNode) {
	int i, j;
	
	for (i = 0; i < 8; i++) {
		j = iNode & 0xFF;
		iNode = iNode >> 8;
		fwrite(&j, 1, 1, stdout);
	}
}

void printMode(unsigned short mode) {
	int i, j;

	for (i = 0; i < 4; i++) {
		j = mode & 0xFF;
		mode = mode >> 8;
		fwrite(&j, 1, 1, stdout);
	}
}

void printModTime(long modTime) {
	int i, j;

	for (i = 0; i < 8; i++) {
		j = modTime & 0xFF;
		modTime = modTime >> 8;
		fwrite(&j, 1, 1, stdout);
	}
}

void printFileSize(const char *fileName) {
	FILE *file = fopen(fileName, "r");

	if (file == NULL) {
		fprintf(stderr, "Error opening %s\n", fileName);
		exit(1);
	}

	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	fclose(file);
	
	int i, j;

	for (i = 0; i < 8; i++) {
		j = fileSize & 0xFF;
		fileSize = fileSize >> 8;
		fwrite(&j, 1, 1, stdout);
	}
}

void printBytes(const char *fileName) {
	FILE *file = fopen(fileName, "r");
	
	if (file == NULL) {
		fprintf(stderr, "Couldnt open %s\n", fileName);
		exit(1);
	}


	// get size
	// allocate memory
	// read file
	// write contents
	int bufferSize = 10000;
	char *codeBuffer = (char *) malloc(bufferSize * sizeof(char));
	char *word = (char *) malloc(bufferSize * sizeof(char));
	int wordLength = 0;
	while (fread(codeBuffer, sizeof(char), 1, file) != 0) {
		strcat(word + wordLength, codeBuffer);
		wordLength = strlen(word);
	}
	fclose(file);
	
	printHexString(word);
}

/*
void printLittleEndian(int num, int size) {
	char *littleEndian = (char *) malloc(size * sizeof(char));
	int stringLength = 0;
	int i, j;
	int mask;

	for (i = 0; i < size; i++) {
		//printf("Stuck here\n");
		//printf(" %u", num & 0xFF);
		j = num & 0xFF;
		num = num >> 8;
		//printf("%02x ", j);
		fwrite(&j, 1, 1, stdout);
	}
	//printf("\n");
}
*/

int compare(Jval val1, Jval val2) {
	if (val1.l < val2.l) {
		return -1;
	}
	if (val1.l > val2.l) {
		return 1;
	}
	return 0;
}
