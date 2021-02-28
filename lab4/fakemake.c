// Author: Logan Tillman

#include <stdio.h>
#include <string.h>
#include "fields.h"
#include "dllist.h"

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
		printf("Opening file \"%s\"\n", "fmakefile");
	}
	else {
		is = new_inputstruct(argv[1]);
		printf("Opening file \"%s\"\n", argv[1]);
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
			else if (strcmp(typeOfFile, "E") == 0) {
				if (executableName != NULL) {
					printf("Executable already exists\n");
					return -1;
				}
				executableName = strdup(is->fields[i]);
			}
		}
	}

	if (executableName == NULL ) {
		printf("Executable not specified\n");
		return -1;
	}
	
	Dllist tmp;
	printf("C list:\n");
	dll_traverse(tmp, cList) printf("%s\n", tmp->val.s);
	printf("H list:\n");
	dll_traverse(tmp, hList) printf("%s\n", tmp->val.s);
	printf("L list:\n");
	dll_traverse(tmp, lList) printf("%s\n", tmp->val.s);
	printf("E Name: %s\n", executableName);

	return 0;
}
