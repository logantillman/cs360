// Author: Logan Tillman

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "fields.h"
#include "dllist.h"

long processHeaderFiles(Dllist hList);

long processCFiles(Dllist cList, Dllist fList, long hTime);

int main(int argc, char **argv) {
	
	IS is;
	Dllist cList = new_dllist();
	Dllist hList = new_dllist();
	Dllist lList = new_dllist();
	Dllist fList = new_dllist();
	char *executableName = NULL;
	int i = 0;

	if (argc == 1) {
		is = new_inputstruct("fmakefile");
	//	printf("Opening file \"%s\"\n", "fmakefile");
	}
	else {
		is = new_inputstruct(argv[1]);
	//	printf("Opening file \"%s\"\n", argv[1]);
	}

	// read each line of the file and print to stdout
	while (get_line(is) >= 0) {
		if (is->NF == 0) {
			continue;
		}

		char *typeOfFile = is->fields[0];

		for (i = 1; i < is->NF; i++) {
			if (strcmp(typeOfFile, "C") == 0) {
				dll_append(cList, new_jval_s(strdup(is->fields[i])));
			}
			else if (strcmp(typeOfFile, "H") == 0) {
				dll_append(hList, new_jval_s(strdup(is->fields[i])));
			}
			else if (strcmp(typeOfFile, "L") == 0) {
				dll_append(lList, new_jval_s(strdup(is->fields[i])));
			}
			else if (strcmp(typeOfFile, "F") == 0) {
				dll_append(fList, new_jval_s(strdup(is->fields[i])));
			}
			else if (strcmp(typeOfFile, "E") == 0) {
				if (executableName != NULL) {
					printf("fmakefile (%d) cannot have more than one E line\n", is->line);
					return -1;
				}
				executableName = strdup(is->fields[i]);
			}
			else {
				printf("Line doesn't match any types\n");
				return -1;
			}
		}
	}

	if (executableName == NULL) {
		fprintf(stderr, "No executable specified\n");
		return -1;
	}
	
	int hTime = processHeaderFiles(hList);
	//printf("H file max time: %d\n", hTime);

	int cReturn = processCFiles(cList, fList, hTime);

	struct stat buf;

	stat(executableName, &buf);

	if (cReturn == 1 || cReturn > buf.st_mtime) {
		int cFilesLength = 0;
		int fFilesLength = 0;
		int lFilesLength = 0;
		Dllist tmp;

		dll_traverse(tmp, cList) cFilesLength += strlen(tmp->val.s) + 1;
		dll_traverse(tmp, fList) fFilesLength += strlen(tmp->val.s) + 1;
		dll_traverse(tmp, lList) lFilesLength += strlen(tmp->val.s) + 1;


		int totalFilesLength = cFilesLength + fFilesLength + lFilesLength;
		char *oCompileString = malloc((7 + totalFilesLength + strlen(executableName)) * sizeof(char));
		//printf("Allocated oCompile: %d\n", 7 + totalFilesLength + strlen(executableName));
		int oStringLength = 0;
		strcpy(oCompileString + oStringLength, "gcc -o");
		oStringLength = strlen(oCompileString);
		strcat(oCompileString + oStringLength, " ");
		oStringLength = strlen(oCompileString);
		strcat(oCompileString + oStringLength, executableName);
		oStringLength = strlen(oCompileString);
		
		dll_traverse(tmp, fList) {
			strcat(oCompileString + oStringLength, " ");
			oStringLength = strlen(oCompileString);
			strcat(oCompileString + oStringLength, tmp->val.s);
			oStringLength = strlen(oCompileString);
		}
		
		dll_traverse(tmp, cList) {
			strcat(oCompileString + oStringLength, " ");
			oStringLength = strlen(oCompileString);
			char *cFileName = strdup(tmp->val.s);
			cFileName[strlen(cFileName) - 1] = 'o';
			strcat(oCompileString + oStringLength, cFileName);
			free(cFileName);
			oStringLength = strlen(oCompileString);
		}
		
		dll_traverse(tmp, lList) {
			strcat(oCompileString + oStringLength, " ");
			oStringLength = strlen(oCompileString);
			strcat(oCompileString + oStringLength, tmp->val.s);
			oStringLength = strlen(oCompileString);
		}
		if (system(oCompileString) == -1) {
			fprintf(stderr, "Command failed.  Fakemake exiting\n");
		}
		printf("%s\n", oCompileString);
	}
	else {
		printf("%s up to date\n", executableName);
	}
/*
	Dllist tmp;
	printf("C list:\n");
	dll_traverse(tmp, cList) printf("%s\n", tmp->val.s);
	printf("H list:\n");
	dll_traverse(tmp, hList) printf("%s\n", tmp->val.s);
	printf("L list:\n");
	dll_traverse(tmp, lList) printf("%s\n", tmp->val.s);
	printf("F list:\n");
	dll_traverse(tmp, fList) printf("%s\n", tmp->val.s);
	printf("E Name: %s\n", executableName);
*/
	return 0;
}

long processHeaderFiles(Dllist hList) {
	Dllist tmp;
	struct stat buf;
	long maxTime;
	int exists;

	if (!dll_empty(hList)) {
		exists = stat(hList->flink->val.s, &buf);
		if (exists < 0) { 
			printf("Error. H file %s doesn't exist\n", hList->flink->val.s);
			return -1;
		}
		
		maxTime = buf.st_mtime;
	}

	dll_traverse(tmp, hList) {
		exists = stat(tmp->val.s, &buf);
		if (exists < 0) {
			printf("Error in the H file loop, %s doesn't exist\n", tmp->val.s);
			return -1;
		}
	
		//printf("Max time: %d Curr time: %d\n", buf.st_mtime);
		if (buf.st_mtime > maxTime) {
			maxTime = buf.st_mtime;
		}
	}

	return maxTime;
}

long processCFiles(Dllist cList, Dllist fList, long hTime) {
	Dllist tmp;
	struct stat buf;
	int remadeFiles = 0;
	int maxTime;
	int exists; 

	if (!dll_empty(cList)) {
		exists = stat(cList->flink->val.s, &buf);
		if (exists < 0) {
			printf("Error. C file %s doesn't exist\n", cList->flink->val.s);
			return -1;
		}
		maxTime = buf.st_mtime; 
	}

	int flagLength = 0;
	dll_traverse(tmp, fList) flagLength += strlen(tmp->val.s) + 1;

	dll_traverse(tmp, cList) {
		exists = stat(tmp->val.s, &buf);
		if (exists < 0) {
			printf("Error in the C file loop, %s doesn't exist\n", tmp->val.s);
			return -1;
		}
		long cTime = buf.st_mtime;
		char *cFile = strdup(tmp->val.s);
		char *oFile = strdup(cFile);
		int len = strlen(cFile);
		oFile[len - 1] = 'o';
		//printf("File: %s looking for %s\n", tmp->val.s, oFile);
		
		exists = stat(oFile, &buf);
		if (exists < 0 || buf.st_mtime < cTime || buf.st_mtime < hTime) {
			//printf("Need to remake this file\n");
			char *cFileString = (char *) malloc((strlen(oFile) + flagLength + 7) * sizeof(char));
			strcpy(cFileString, "");
			int cStringLength = 0;
			strcat(cFileString + cStringLength, "gcc -c");
			cStringLength += strlen(cFileString);

			Dllist tmp2;
			dll_traverse(tmp2, fList) {
				strcat(cFileString + cStringLength, " ");
				cStringLength = strlen(cFileString);
				strcat(cFileString + cStringLength, tmp2->val.s);
				cStringLength = strlen(cFileString);
				//printf("[%s] len: %d\n", cFileString, cStringLength);
			}

			strcat(cFileString + cStringLength, " ");
			cStringLength = strlen(cFileString);
			strcat(cFileString + cStringLength, cFile);
			if (system(cFileString) == -1) {
				fprintf(stderr, "Command failed.  Fakemake exiting\n");
			}
			remadeFiles = 1;
			printf("%s\n", cFileString);
		}
		else {
			if (buf.st_mtime > maxTime) {
				maxTime = buf.st_mtime;
			}
		}
		free(cFile);
		free(oFile);
	}

	if (remadeFiles) {
		//printf("Had to remake files, returning 1\n");
		return 1;
	}
	else {
		//printf("Returning max time: %d\n", maxTime);
		return maxTime;
	}
}
