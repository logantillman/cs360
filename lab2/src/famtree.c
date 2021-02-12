#include <stdio.h>
#include "jrb.h"
#include "fields.h"

typedef struct person {
	char *name;
	char *sex;
	struct person *father;
	struct person *mother;
	struct person **children;
	// Stuff for DFS and Topological sort
} Person;

int main(int argc, char **argv) {
	IS is = new_inputstruct(NULL);
	char type[50];
	char name[100];		// Might cause some issues in the future (this comment is for reference)
	Person *person = NULL;
	JRB tree = make_jrb();
	int length;
	int i;

	while (get_line(is) >= 0) {
		if (is->NF < 2) {
			continue;
		}

		strcpy(type, is->fields[0]);
		printf("Type: %s\n", type);

		/* Calculating the length of the name */
		length = strlen(is->fields[1]);
		for (i = 2; i < is->NF; i++) {
			length += (strlen(is->fields[i]) + 1);
		}
		
		/* Building the name string */
		strcpy(name, is->fields[1]);
		length = strlen(name);
		for (i = 2; i < is->NF; i++) {
			strcat(name + length, " ");
			strcat(name + length, is->fields[i]);
			length += strlen(name + length);
		}

		printf("Name: %s\n", name);

		//printf("Length: %d\n", length);
		
		//FIXME Check if person exists in the people tree

		/* If person doesn't exist in the tree, allocate and add */
		if (strcmp(type, "SEX") != 0) {
			printf("Status: %s\n", jrb_find_str(tree, name));
			if (jrb_find_str(tree, name) == NULL) {
				printf("Inserting person\n\n");
				person = (Person *) malloc(sizeof(Person));
				person->name = (char *) malloc(sizeof(char) * (length + 1));
				jrb_insert_str(tree, person->name, new_jval_v((void *) person));
			}
		}
	}
	
	return 0;
}
