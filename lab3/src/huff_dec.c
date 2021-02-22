// Author: Logan Tillman

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "jrb.h"
#include "dllist.h"

char* decToBinary(int number);

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Use the right amount of arguments plz\n");
	}
	FILE *file;
	JRB tree = make_jrb();
	char *codeFile = argv[1];
	char *inputFile = argv[2];
	int buffSize = 10000;
	char *codeBuffer = NULL;
	codeBuffer = (char *) malloc(buffSize * sizeof(char));
	char *word = (char *) malloc(buffSize * sizeof(char));
	char *key, *val = NULL; //= (char *) malloc(bufSize*sizeof(char));
	int i = 0;
	int length = 0;
	int longestPossibleKey = 0;

	// Error check code file
	// Read the code file
	file = fopen(codeFile, "r");
	
	while (fread(codeBuffer, sizeof(char), 1, file) != 0) {
		if (strcmp(codeBuffer, "") == 0) {
			if (strcmp(word, "") != 0) {
				i++;
				if (i == 2) {
					key = (char *) malloc(length * sizeof(char));
					strcpy(key, word);
					jrb_insert_str(tree, key, new_jval_v((void *) val));
					if (strlen(key) > longestPossibleKey) {
						longestPossibleKey = strlen(key);
//						printf("LPK: %d - %s\n", longestPossibleKey, key);
					}
					i = 0;
				}
				else {
					val = (char *) malloc(length * sizeof(char));
					strcpy(val, word);
				}
			}
			strcpy(word, "");
			length = 0;
		}
		else {
			strcat(word + length, codeBuffer);
			length++;
		}
	}
	
	JRB tmpTree;
	jrb_traverse(tmpTree, tree) {
//		printf("key: [%s] value: [%s]\n", tmpTree->key.v, tmpTree->val.v);
	}

	free(codeBuffer);
	free(word);
	fclose(file);
	
	int c, numToRead;
	file = fopen(inputFile, "r");
	
	fseek(file, 0, SEEK_END);
	int inputFileSize = ftell(file);

	if (inputFileSize < 4) {
		fprintf(stderr, "Error: file is not the correct size.\n");
		return -1;
	}

	fseek(file, -4, SEEK_END);
	fread(&numToRead, sizeof(int), 1, file);
	
	if (numToRead > ((inputFileSize - 4) * 8)) {
		fprintf(stderr, "Error: Total bits = %d, but file's size is %d\n", numToRead, inputFileSize);
		return -1;
	}
	
	buffSize = ceil(numToRead / 8.0);
//	printf("c: %d | 0x%08x bs: %d\n", numToRead, numToRead, buffSize);

//	printf("Allocating %d bytes to binaryString\n", numToRead);
	char *binaryString = (char *) malloc((numToRead * sizeof(char)) + 1);
	char *searchString = (char *) malloc((numToRead * sizeof(char)) + 1);

	fseek(file, 0, SEEK_SET);
	int t;
	
	length = 0;
	strcpy(binaryString, "");
	for (i = 0; i < buffSize; i++) {
		c = fgetc(file);
//		printf("%d 0x%2x\n", c, c);
		strcat(binaryString + length, decToBinary(c));
		length += strlen(binaryString + length);
	}

//	printf("End string %s\n", binaryString);

	JRB tmp;
//	int wordLength = 0;
	length = 0;
	strcpy(searchString, "");
//	strcpy(word, "");
	for (i = 0; i < numToRead; i++) {
//		printf("AT %c\n", binaryString[i]);
		char *character = binaryString + i;
		strncat(searchString + length, character, 1);
		length++;
		//tmp = jrb_find_str(tree, searchString);
		if (strlen(searchString) > longestPossibleKey) {
			printf("%d - %s\n", strlen(searchString), searchString);
			fprintf(stderr, "Unrecognized bits\n");
			return -1;
		}

		tmp = jrb_find_str(tree, searchString);
		if (tmp != NULL) {
			printf(tmp->val.v);
			//strcat(word + wordLength, tmp->val.v);
			strcpy(searchString, "");
			//wordLength += strlen(word + wordLength);
			length = 0;
		}
		else {
//			if (strcmp(searchString, "00000") == 0) printf("AHHHHHH %s %s\n", searchString, jrb_find_str(tree, "00000")->val.v);
	//		printf("Couldn't find %s\n", searchString);
		}
	}

//	printf("%s", word);
	fclose(file);
	// Error check the input file 

	return 0;
}

char* decToBinary(int number) {
	char *binaryString = malloc(9 * sizeof(char));
	binaryString[8] = '\0';
	int i = 0;
	while (i < 8) {
		if (number == 0)
			binaryString[i] = '0';
		else {
			binaryString[i] = (number % 2) + '0';
			number = number / 2;
		}
		i++;
	}

//	printf("bs: %s\n", binaryString);
	return binaryString;
}
