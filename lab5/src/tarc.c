// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include "jrb.h"
#include "dllist.h"

/* Struct for file information */
typedef struct myStruct {
	char *relativePath;
	char *absolutePath;
	struct stat buffer;
} myStruct;

/* Recursive function that traverses a directory and reads each item */
void traverseDirectory(const char *fileName, JRB iNodeTree, char *relativePath);

/* Finds the relative path given a pathname */
char *findRelativePath(char *pathName);

/* Prints the size of the file name in little endian */
void printFileNameSize(int fileNameSize);

/* Prints a string in hex */
void printHexString(const char *string);

/* Prints a file's inode in little endian */
void printINode(unsigned long iNode);

/* Prints a file's mode in little endian */
void printMode(unsigned short mode);

/* Prints a file's last modification time in little endian */
void printModTime(long modTime);

/* Prints a file's size in little endian */
void printFileSize(long fileSize);

/* Prints a file's bytes */
void printBytes(const char *fileName);

/* Function for initializing my file struct */
myStruct * initializeStruct(char *relativePath, char *absolutePath, struct stat buffer);

/* Function for comparing jvals */
int compare(Jval v1, Jval v2);

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Incorrect amount of arguments\n");
		return -1;
	}	

	JRB iNodeTree = make_jrb();
	
	char *tmp = strdup(argv[1]);
	char *relativePath = findRelativePath(tmp);
	
	traverseDirectory(argv[1], iNodeTree, relativePath);

	free(tmp);
	return 0;
}

/* Recursive function that traverses a directory and reads each item */
void traverseDirectory(const char *fileName, JRB iNodeTree, char *relativePath) {
	DIR *d = opendir(fileName);

	if (d == NULL) {
		fprintf(stderr, "Couldn't open %s\n", fileName);
		exit(1);
	}

	Dllist dirList = new_dllist();
	Dllist fileList = new_dllist();
	struct dirent *de;
	struct stat buf;
	myStruct *myStr;
	myStruct *tmpStr;
	int exists;
		
	char *newPath = (char *) malloc((strlen(fileName) + 258) * sizeof(char));
	char *newRelativePath = (char *) malloc((strlen(relativePath) + 258) * sizeof(char));
	
	for (de = readdir(d); de != NULL; de = readdir(d)) {
		if (strcmp(de->d_name, ".") == 0) {
			exists = lstat(fileName, &buf);
			if (exists < 0) {
				fprintf(stderr, "%s doesn't exist\n", fileName);
				exit(1);
			}
			else {
				if (jrb_find_gen(iNodeTree, new_jval_l(buf.st_ino), compare) == NULL) {
					jrb_insert_gen(iNodeTree, new_jval_l(buf.st_ino), new_jval_i(0), compare);
					
					printFileNameSize(strlen(relativePath));
					printHexString(relativePath);
					printINode(buf.st_ino);
					printMode(buf.st_mode);
					printModTime(buf.st_mtime);
				}
				else {
					printFileNameSize(strlen(relativePath));
					printHexString(relativePath);
					printINode(buf.st_ino);
				}
			}
		}
		else if (strcmp(de->d_name, "..") != 0) {
			sprintf(newPath, "%s/%s", fileName, de->d_name);
			sprintf(newRelativePath, "%s/%s", relativePath, de->d_name);

			exists = lstat(newPath, &buf);
			if (exists < 0) {
				fprintf(stderr, "%s doesn't exist\n", newPath);
				exit(1);
			}
			else {
				myStr = initializeStruct(newRelativePath, newPath, buf);
				if (S_ISDIR(buf.st_mode)) {
					dll_append(dirList, new_jval_v((void *) myStr));
				}
				else {
					dll_append(fileList, new_jval_v((void *) myStr));
				}
			}
		}
	}

	closedir(d);

	Dllist tmp;
	dll_traverse(tmp, fileList) {
		tmpStr = (myStruct *) tmp->val.v;
		printFileNameSize(strlen(tmpStr->relativePath));
		printHexString(tmpStr->relativePath);
		printINode(tmpStr->buffer.st_ino);

		if (jrb_find_gen(iNodeTree, new_jval_l(tmpStr->buffer.st_ino), compare) == NULL) {
			jrb_insert_gen(iNodeTree, new_jval_l(tmpStr->buffer.st_ino), new_jval_i(0), compare);
		
			printMode(tmpStr->buffer.st_mode);
			printModTime(tmpStr->buffer.st_mtime);
			printFileSize(tmpStr->buffer.st_size);
			printBytes(tmpStr->absolutePath);
		}
		free(tmpStr->relativePath);
		free(tmpStr->absolutePath);
		free(tmpStr);
	}

	dll_traverse(tmp, dirList) {
		tmpStr = (myStruct *) tmp->val.v;
		traverseDirectory(tmpStr->absolutePath, iNodeTree, tmpStr->relativePath);
		free(tmpStr->relativePath);
		free(tmpStr->absolutePath);
		free(tmpStr);
	}

	free(newPath);
	free(newRelativePath);
	free_dllist(fileList);
	free_dllist(dirList);
}

/* Finds the relative path given a pathname */
char *findRelativePath(char *pathName) {
	char *foundSlash = strtok(pathName, "/");
	char *relative;

	while (foundSlash != NULL) {
		relative = foundSlash;
		foundSlash = strtok(NULL, "/");
	}
	return relative;
}

/* Prints the size of the file name in little endian */
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

void printFileSize(long fileSize) {
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

	fseek(file, 0, SEEK_END);

	long size = ftell(file);
	rewind(file);
	
	char *buf = (char *) malloc((size + 1) * sizeof(char));

	fread(buf, size, 1, file);
	fwrite(buf, 1, size, stdout);
	fclose(file);
	free(buf);
}

myStruct * initializeStruct(char *relativePath, char *absolutePath, struct stat buffer) {
	myStruct *str = malloc(sizeof(myStruct));
	str->relativePath = strdup(relativePath);
	str->absolutePath = strdup(absolutePath);
	str->buffer = buffer;

	return str;
}

int compare(Jval val1, Jval val2) {
	if (val1.l < val2.l) {
		return -1;
	}
	if (val1.l > val2.l) {
		return 1;
	}
	return 0;
}
