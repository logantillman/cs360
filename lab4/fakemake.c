// Author: Logan Tillman

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "fields.h"
#include "dllist.h"

/* Processes the header files */
long processHeaderFiles(Dllist hList);

/* Processes the C files */
long processCFiles(Dllist cList, Dllist fList, long hTime);

/* Finds the total length of the list's elements (including spaces) */
int findLength(Dllist list);

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
	}
	else {
		is = new_inputstruct(argv[1]);
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
					fprintf(stderr, "fmakefile (%d) cannot have more than one E line\n", is->line);
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
	
	/* Exiting if we ran into an error while traversing the H files */
	if (hTime == -1) {
		return -1;
	}

	int cReturn = processCFiles(cList, fList, hTime);

	/* Exiting if we ran into an error while traversing the C files */
	if (cReturn == -1) {
		return -1;
	}

	struct stat buf;

	int exists = stat(executableName, &buf);

	if (exists < 0 || cReturn == 1 || cReturn > buf.st_mtime) {
		int cFilesLength = findLength(cList);
		int fFilesLength = findLength(fList);
		int lFilesLength = findLength(lList);
		Dllist tmp;

		int totalFilesLength = cFilesLength + fFilesLength + lFilesLength;
		char *oCompileString = malloc((7 + totalFilesLength + strlen(executableName)) * sizeof(char));

		strcpy(oCompileString, "gcc -o");
		int oStringLength = strlen(oCompileString);
		strcat(oCompileString + oStringLength, " ");
		strcat(oCompileString + oStringLength, executableName);
		oStringLength = strlen(oCompileString);
		
		dll_traverse(tmp, fList) {
			strcat(oCompileString + oStringLength, " ");
			strcat(oCompileString + oStringLength, tmp->val.s);
			oStringLength = strlen(oCompileString);
		}
		
		dll_traverse(tmp, cList) {
			strcat(oCompileString + oStringLength, " ");
			char *cFileName = strdup(tmp->val.s);
			cFileName[strlen(cFileName) - 1] = 'o';
			strcat(oCompileString + oStringLength, cFileName);
			free(cFileName);
			oStringLength = strlen(oCompileString);
		}
		
		dll_traverse(tmp, lList) {
			strcat(oCompileString + oStringLength, " ");
			strcat(oCompileString + oStringLength, tmp->val.s);
			oStringLength = strlen(oCompileString);
		}
		if (system(oCompileString) == -1) {
			fprintf(stderr, "Command failed.  Fakemake exiting\n");
			return -1;
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

/* Processes the header files */
long processHeaderFiles(Dllist hList) {
	Dllist tmp;
	struct stat buf;
	long maxTime;
	int exists;

	/* Initializing maxTime to the first element in the list */
	if (!dll_empty(hList)) {
		exists = stat(hList->flink->val.s, &buf);
		if (exists < 0) { 
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", hList->flink->val.s);
			return -1;
		}
		
		maxTime = buf.st_mtime;
	}

	/* Finding the maxTime for the header files */
	dll_traverse(tmp, hList) {
		exists = stat(tmp->val.s, &buf);
		if (exists < 0) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", tmp->val.s);
			return -1;
		}
	
		if (buf.st_mtime > maxTime) {
			maxTime = buf.st_mtime;
		}
	}

	return maxTime;
}

/* Processes the C files */
long processCFiles(Dllist cList, Dllist fList, long hTime) {
	Dllist tmp;
	struct stat buf;
	int remadeFiles = 0;
	int maxTime;
	int exists; 

	/* Assigning maxTime to the first element in the list */
	if (!dll_empty(cList)) {
		exists = stat(cList->flink->val.s, &buf);
		if (exists < 0) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", cList->flink->val.s);
			return -1;
		}
		maxTime = buf.st_mtime; 
	}

	int flagLength = findLength(fList);

	/* Finding the maxTime of the C list and compiling the files */
	dll_traverse(tmp, cList) {
		exists = stat(tmp->val.s, &buf);
		if (exists < 0) {
			fprintf(stderr, "fmakefile: %s: No such file or directory\n", tmp->val.s);
			return -1;
		}
		long cTime = buf.st_mtime;
		char *cFile = strdup(tmp->val.s);
		char *oFile = strdup(cFile);
		int len = strlen(cFile);
		oFile[len - 1] = 'o';
		
		exists = stat(oFile, &buf);

		/* Determing if we need to re-compile the C file */
		if (exists < 0 || buf.st_mtime < cTime || buf.st_mtime < hTime) {
			char *cFileString = (char *) malloc((strlen(oFile) + flagLength + 7) * sizeof(char));
			strcpy(cFileString, "gcc -c");
			int cStringLength = strlen(cFileString);

			Dllist tmp2;
			dll_traverse(tmp2, fList) {
				strcat(cFileString + cStringLength, " ");
				strcat(cFileString + cStringLength, tmp2->val.s);
				cStringLength = strlen(cFileString);
			}

			strcat(cFileString + cStringLength, " ");
			strcat(cFileString + cStringLength, cFile);
			if (system(cFileString) == -1) {
				fprintf(stderr, "Command failed.  Fakemake exiting\n");
				return -1;
			}
			remadeFiles = 1;
			printf("%s\n", cFileString);
		}
		else {

			/* Keeping track of the maxTime */
			if (buf.st_mtime > maxTime) {
				maxTime = buf.st_mtime;
			}
		}

		/* Freeing the temporary files */
		free(cFile);
		free(oFile);
	}

	if (remadeFiles) {
		return 1;
	}
	else {
		return maxTime;
	}
}

/* Finds the total length of the list's elements (including spaces) */
int findLength(Dllist list) {
	Dllist tmp;
	int length = 0;

	dll_traverse(tmp, list) length += strlen(tmp->val.s) + 1;

	return length;
}
