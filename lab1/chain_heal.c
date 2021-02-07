#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Node structure */
typedef struct node {
  char *name;
  int x, y;
  int currentPP, maxPP, pp;
  struct node *prev;
  int adjSize;
  struct node **adj;
  int visited;
} Node;


/* global information */
typedef struct global
{
  int bestPathLength;
  Node **bestPath;
  int *healing;
  int numJumps;
  int bestHealing; 
  double powerReduction;
  
} Global;


/* check if n1 can jump to n2 given a range */
int isInRange(Node *n1, Node *n2, int range) {
	int distance = 0;
	if (n1->x > n2->x)
		distance += n1->x - n2->x;
	else
		distance += n2->x - n1->x;
	if (n1->y > n2->y)
		distance += n1->y - n2->y;
	else
		distance += n2->y - n1->y;
	return distance <= range;
}

/* perform DFS 
   "power" stores the points on the chain heal after "hop" hops. 
*/
void DFS(Node *from, Node *n, int hop, int totalHealing, double power, Global *g) {
	if (n->visited == 1 || hop > g->numJumps) {
		return;
	}
	n->pp = calculateHealing(n, rint(power));
	//printf("n->pp for %s is %d\n", n->name, n->pp);
	if (n != from) {
		n->prev = from;
		//printf("setting %s->prev to %s\n", n->name, from->name);
	}
	n->visited = 1;
	totalHealing += n->pp;
	//printf("n = %s\n", n->name);
	//printf("Node:%s Hop %d\n", n->name, hop);
	//printf("Total healing: %d\n", totalHealing);
	int i;
	for (i = 0; i < n->adjSize; i++) {
		if (from != n->adj[i]) {
			double powerReduction = 1 - g->powerReduction;
			DFS(n, n->adj[i], hop + 1, totalHealing, power * powerReduction, g);
		}
	}

	if (totalHealing > g->bestHealing) {
		g->bestHealing = totalHealing;
		
		Node *cursor = n;
		i = 0;
		//g->bestPath = malloc(g->numJumps * sizeof(Node *));
		//g->healing = malloc(g->numJumps * sizeof(int *));
		g->bestPathLength = 0;
		while (cursor != NULL && i < g->numJumps) {
			//printf("::	%s\n", cursor->name);
			g->bestPath[i] = cursor;
			g->healing[i] = cursor->pp;	
			if (cursor->prev == cursor) {
				break;
			}

			cursor = cursor->prev;
			g->bestPathLength++;
			i++;
		}
	}

	n->visited = 0;
}

int calculateHealing(Node *person, double power) {
	int possiblePP = person->currentPP + power;

	//printf("curr:%d max:%d poss:%d\n", person->currentPP, person->maxPP, possiblePP);

	if (possiblePP > person->maxPP) {
		return person->maxPP - person->currentPP;
	}
	return power;
}

int main(int argc, char **argv) {
	if (argc < 6) {
		printf("Usage: ./chain_heal intial_range jump_range num_jumps intial_power power_reduction < input_file\n");
		return -1;
	}

	Global *global = malloc(sizeof(Global));
	const int initialRange = atoi(argv[1]);
	const int jumpRange = atoi(argv[2]);
	global->numJumps = atoi(argv[3]);
	const int initialPower = atoi(argv[4]);
	global->powerReduction = atof(argv[5]);
	global->bestHealing = 0;
	global->bestPath = malloc(sizeof(Node *) * global->numJumps);
	global->healing = malloc(sizeof(int *) * global->numJumps);
	int x, y, currentPP, maxPP;
	char name[100];
	int numNodes = 0;
	Node *prev = NULL;

	while (scanf("%d %d %d %d %s", &x, &y, &currentPP, &maxPP, name) != -1) {
		Node *newNode = malloc(sizeof(Node));
		newNode->x = x;
		newNode->y = y;
		newNode->currentPP = currentPP;
		newNode->maxPP = maxPP;
		newNode->name = strdup(name);
		newNode->prev = prev;
		prev = newNode;
		numNodes++;
		//printf("%d %d %d %d %s prev=%s %d\n", newNode->x, newNode->y, newNode->currentPP, newNode->maxPP, newNode->name, newNode->prev->name, numNodes);
	}
	
	Node **nodeArray;
	nodeArray = malloc(numNodes * sizeof(Node *));
	int i, j = 0;

	// Create array of nodes
	while (prev != NULL) {
		nodeArray[i] = prev;
		//printf("called %s\n", nodeArray[i]->name);
		prev = prev->prev;
		i++;
	}

	// Create adj list for each node
	for (i = 0; i < numNodes; i++) {
		nodeArray[i]->adjSize = 0;
		for (j = 0; j < numNodes; j++) {
			if (i == j) {
				continue;
			}
			if (isInRange(nodeArray[i], nodeArray[j], jumpRange)) {
				nodeArray[i]->adjSize++;
			}
		}
		nodeArray[i]->adj = malloc(nodeArray[i]->adjSize * sizeof(Node *));

		int k = 0;

		//printf("%s's adj list: \n", nodeArray[i]->name);
		for (j = 0; j < numNodes; j++) {
			if (i == j) {
				continue;
			}
			if (isInRange(nodeArray[i], nodeArray[j], jumpRange)) {
				nodeArray[i]->adj[k] = nodeArray[j];
				//printf("	%s\n", nodeArray[i]->adj[k]->name);
				k++;
			}
		}
	}	

	for (i = 0; i < numNodes; i++) {
		nodeArray[i]->visited = 0;
		for (j = 0; j < nodeArray[i]->adjSize; j++) {
			nodeArray[i]->adj[j]->visited = 0;
		}
		
		if (isInRange(nodeArray[numNodes - 1], nodeArray[i], initialRange)) {
			int hop = 1;
			int totalHealing = 0;
			//printf("Calling DFS on %s\n", nodeArray[i]->name);
			DFS(nodeArray[i], nodeArray[i], hop, totalHealing, initialPower, global);
		}
	}

	//printf("path size = %d\n", global->bestPathLength);

	for (i = global->bestPathLength - 1; i >= 0; i--) {
		printf("%s %d\n", global->bestPath[i]->name, global->healing[i]);
	}

	printf("Total_Healing %d\n", global->bestHealing);
	
	return 0;
}
