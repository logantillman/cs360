// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "jrb.h"
#include "dllist.h"

void traverseDirectory(const char *fileName, JRB iNodeTree);

//void printInfo(const char *dirFileName);

//void printNewInfo(const char *dirFileName);

//void printFileInfo(const char *dirFileName);

void printFileNameSize(const char *fileName);

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
	traverseDirectory(argv[1], iNodeTree);

	return 0;
}

void traverseDirectory(const char *fileName, JRB iNodeTree) {
	DIR *d = opendir(fileName);
	if (d == NULL) {
		fprintf(stderr, "Couldn't open %s directory\n", fileName);
		//exit(1);
	}
	
	struct dirent *de;
	struct stat buf;
	Dllist dirList = new_dllist();
	int exists;

	int fileNameSize = strlen(fileName);
	int dirFileNameSize = fileNameSize + 10;
	char *dirFileName = (char *) malloc(sizeof(char) * dirFileNameSize);

	if (dirFileName == NULL) {
		fprintf(stderr, "malloc error\n");
		//exit(1);
	}

	//printInfo(fileName);
	//printNewInfo(fileName);
	
	strcpy(dirFileName, fileName);
	strcat(dirFileName + fileNameSize, "/");

	exists = stat(fileName, &buf);

	if (exists < 0) {
		fprintf(stderr, "Couldn't stat %s\n", fileName);
	}
	else {
		printFileNameSize(fileName);
		printHexString(fileName);
		printINode(buf.st_ino);
		printMode(buf.st_mode);			// Might have to check if first time seen
		printModTime(buf.st_mtime);		// Might have to check if first time seen
	}
	//printf("fn size: %d\n", strlen(dirFileName));
	//printf("fn: %s\n", dirFileName);
	//printf("inode %d\n", buf.st_ino);
//	printf("dirFileName: %s\n", dirFileName);

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
//		printf("dirFileName 2: %s\n", dirFileName);

		exists = stat(dirFileName, &buf);
		if (exists < 0) {
			fprintf(stderr, "Couldn't stat %s\n", de->d_name);
			//exit(1);
		}
		else {
//			printf("Opened %s\n", dirFileName);
		}
		
		JRB loc = jrb_find_gen(iNodeTree, new_jval_l(buf.st_ino), compare);

		if (loc == NULL) {
//			printf("Inserting %d into tree\n", buf.st_ino);
			jrb_insert_gen(iNodeTree, new_jval_l(buf.st_ino), new_jval_i(0), compare);
		}
		else {
			//printf("%s already exists\n", dirFileName);
		}

		if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
			dll_append(dirList, new_jval_s(strdup(dirFileName)));
		}
		else {
			//printInfo(dirFileName);
			printFileNameSize(dirFileName);
			printHexString(dirFileName);
			printINode(buf.st_ino);

			if (loc == NULL) {
				printMode(buf.st_mode);
				printModTime(buf.st_mtime);
				printFileSize(dirFileName);
				printBytes(dirFileName);
				//printNewInfo(dirFileName);
				//printFileInfo(dirFileName);
			}
		}
	}
	
	closedir(d);
	
	Dllist tmp;

	dll_traverse(tmp, dirList) {
		traverseDirectory(tmp->val.s, iNodeTree);
	}

	dll_traverse(tmp, dirList) {
		free(tmp->val.s);
	}
	free_dllist(dirList);
	free(dirFileName);
	
//	printf("Closed %s\n", fileName);
}

/*
void printInfo(const char *dirFileName) {
	struct stat buf;
	int exists = stat(dirFileName, &buf);

	if (exists < 0) {
		fprintf(stderr, "Error in printInfo\n");
		//exit(1);
	}
	else {
		//printf("\nfn size: %d\n", strlen(dirFileName));
		//printf("Printing fileNameLength\n");
		printLittleEndian(strlen(dirFileName), 4);
		//printf("fn: %s\n", dirFileName);
		//printf("Printing fileName\n");
		printf(" %s", dirFileName);
		printString(dirFileName);
		//printf("inode %d\n", buf.st_ino);
		//printf("Printing inode\n");
		printLittleEndian(buf.st_ino, 8);
	}
}

void printNewInfo(const char *dirFileName) {
	struct stat buf;
	int exists = stat(dirFileName, &buf);

	if (exists < 0) {
		fprintf(stderr, "Error in printNewInfo\n");
		//exit(1);
	}
	else {
		//printf("mode: %x\n", buf.st_mode);
		//printf("Printing mode\n");
		printLittleEndian(buf.st_mode, 4);
		//printf("mod time: %x\n", buf.st_mtime);
		//printf("Printfing mod time\n");
		printLittleEndian(buf.st_mtime, 8);
	}
}

void printFileInfo(const char *dirFileName) {
	FILE *file = fopen(dirFileName, "r");
	int bufferSize = 10000;
	char *codeBuffer = (char *) malloc(bufferSize * sizeof(char));
	char *word = (char *) malloc(bufferSize * sizeof(char));
	int wordLength = 0;
	while (fread(codeBuffer, sizeof(char), 1, file) != 0) {
		//if (strcmp(codeBuffer, "\n") != 0) {
			//printf("Concating %s\n", codeBuffer);
			strcat(word + wordLength, codeBuffer);
			wordLength = strlen(word);
		//}
	}
	
	struct stat buf;

	int exists = stat(dirFileName, &buf);
	
	if (exists < 0) {
		fprintf(stderr, "Error in printFileInfo\n");
		//exit(1);
	}
	else {
//		fwrite(buf.st_size, 8, 1, stdout);
//		fwrite(word, 1, 1, stdout);
		//printf("%d\n", buf.st_size);
		printLittleEndian(buf.st_size, 8);
		printString(word);
		//printf(" %s", word);
	}

	fclose(file);
}
*/

void printFileNameSize(const char *fileName) {
	int size = strlen(fileName);
	int i, j;	

	//printf("%s %d: ", fileName, size);
	for (i = 0; i < 4; i++) {
		j = size & 0xFF;
		size = size >> 8;
		fwrite(&j, 1, 1, stdout);
		//printf("%02x ", j);
	}

	//printf("\n");
}

void printHexString(const char *string) {
	//printf("%x\n", string);
	fwrite(string, sizeof(char), strlen(string), stdout);
	//printf("\n");
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
