// Author: Logan Tillman

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "jrb.h"
#include "dllist.h"

/* Function that converts a number to a binary string */
char* decToBinary(int number);

int main(int argc, char **argv) {
	JRB tree = make_jrb();
	char *codeFile = argv[1];
	char *inputFile = argv[2];
	int buffSize = 10000;
	char *codeBuffer = codeBuffer = (char *) malloc(buffSize * sizeof(char));
	char *word = (char *) malloc(buffSize * sizeof(char));
	char *key, *val = NULL;
	int i = 0;
	int length = 0;
	int longestPossibleKey = 0;

	/* Reading the code file */
	FILE *file = fopen(codeFile, "r");
	strcpy(word, "");
	strcpy(codeBuffer, "");

	/* Parsing the input and adding the key/value pairs to the tree */
	while (fread(codeBuffer, sizeof(char), 1, file) != 0) {
		if (strcmp(codeBuffer, "") == 0) {
			if (strcmp(word, "") != 0) {
				i++;
				if (i == 2) {
					key = strdup(word);
					jrb_insert_str(tree, key, new_jval_v((void *) val));
					if (strlen(key) > longestPossibleKey) {
						longestPossibleKey = strlen(key);
					}
					i = 0;
				}
				else {
					val = strdup(word);
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
	
	/* Freeing memory and closing the file */
	free(codeBuffer);
	free(word);
	fclose(file);
	
	/* Error checking the input file size */
	file = fopen(inputFile, "r");
	fseek(file, 0, SEEK_END);
	int inputFileSize = ftell(file);

	if (inputFileSize < 4) {
		fprintf(stderr, "Error: file is not the correct size.\n");
		return -1;
	}

	/* Reading in the number of bits to be read */
	int numBitsToRead;
	fseek(file, -4, SEEK_END);
	fread(&numBitsToRead, sizeof(int), 1, file);
	
	if (numBitsToRead > ((inputFileSize - 4) * 8)) {
		fprintf(stderr, "Error: Total bits = %d, but file's size is %d\n", numBitsToRead, inputFileSize);
		return -1;
	}
	
	int numBytesToRead = ceil(numBitsToRead / 8.0);
	char *binaryString = (char *) malloc((numBytesToRead * 8) + 1);
	char *searchString = (char *) malloc((numBitsToRead * sizeof(char)) + 1);
	word = (char *) malloc((numBitsToRead * sizeof(char)) + 1);
	
	fseek(file, 0, SEEK_SET);
	int byte;
	length = 0;
	strcpy(binaryString, "");
	
	/* Reading in the bytes and converting them into a binary string */
	for (i = 0; i < numBytesToRead; i++) {
		byte = fgetc(file);
		char *binaryByte = decToBinary(byte);
		strcat(binaryString + length, binaryByte);
		length += strlen(binaryString + length);
		free(binaryByte);
	}

	int wordLength = 0;
	length = 0;
	strcpy(searchString, "");
	strcpy(word, "");

	/* Building search string by looping through the binary string */
	for (i = 0; i < numBitsToRead; i++) {
		char *character = binaryString + i;
		strncat(searchString + length, character, 1);
		length++;
		if (strlen(searchString) > longestPossibleKey) {
			fprintf(stderr, "Unrecognized bits\n");
			return -1;
		}
		
		/* Searching for the search string in the JRB tree */
		JRB tmp = jrb_find_str(tree, searchString);
		if (tmp != NULL) {
			strcat(word + wordLength, tmp->val.v);
			strcpy(searchString, "");
			wordLength += strlen(word + wordLength);
			length = 0;
		}
	}

	printf("%s", word);
	fclose(file);

	return 0;
}

/* Function that converts a number to a binary string */
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

	return binaryString;
}
