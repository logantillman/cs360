// Author: Logan Tillman

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
int isInRange(Node *n1, Node *n2, int range);

int calculateHealing(Node *person, double power);

Node** createNodeArray(Node* node, int numNodes);

void createAdjLists(Node **nodeArray, int numNodes, int jumpRange);

void performDFSOnConnectedNodes(Node **nodeArray, int numNodes, int initialRange, int initialPower, Global *global);

void printOutput(Global *global);
/* perform DFS 
   "power" stores the points on the chain heal after "hop" hops. 
*/
void DFS(Node *from, Node *n, int hop, int totalHealing, double power, Global *g);

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
	}

	Node **nodeArray = createNodeArray(prev, numNodes);
	
	createAdjLists(nodeArray, numNodes, jumpRange);
	
	performDFSOnConnectedNodes(nodeArray, numNodes, initialRange, initialPower, global);
	
	printOutput(global);
	
	return 0;
}

/* check if n1 can jump to n2 given a range */
int isInRange(Node *n1, Node *n2, int range) {
	int xDistance = n1->x - n2->x;
	int yDistance = n1->y - n2->y;
	int squaredDistance = (xDistance * xDistance) + (yDistance * yDistance);
	
	return squaredDistance <= (range * range);
}

int calculateHealing(Node *person, double power) {
	int possiblePP = person->currentPP + power;

	if (possiblePP > person->maxPP) {
		return person->maxPP - person->currentPP;
	}
	return power;
}

Node** createNodeArray(Node* node, int numNodes) {
	Node **nodeArray = malloc(numNodes * sizeof(Node *));
	int i, j = 0;

	while (node != NULL) {
		nodeArray[i] = node;
		node = node->prev;
		i++;
	}

	return nodeArray;
}

void createAdjLists(Node **nodeArray, int numNodes, int jumpRange) {
	int i, j;

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

		for (j = 0; j < numNodes; j++) {
			if (i == j) {
				continue;
			}
			if (isInRange(nodeArray[i], nodeArray[j], jumpRange)) {
				nodeArray[i]->adj[k] = nodeArray[j];
				k++;
			}
		}
	}	
}

void performDFSOnConnectedNodes(Node **nodeArray, int numNodes, int initialRange, int initialPower, Global *global) {
	int i, j;
	
	for (i = 0; i < numNodes; i++) {
		nodeArray[i]->visited = 0;
		for (j = 0; j < nodeArray[i]->adjSize; j++) {
			nodeArray[i]->adj[j]->visited = 0;
		}
		
		if (isInRange(nodeArray[numNodes - 1], nodeArray[i], initialRange)) {
			int hop = 1;
			int totalHealing = 0;
			DFS(nodeArray[i], nodeArray[i], hop, totalHealing, initialPower, global);
		}
	}
}

void printOutput(Global *global) {
	int i;
	
	for (i = global->bestPathLength - 1; i >= 0; i--) {
		printf("%s %d\n", global->bestPath[i]->name, global->healing[i]);
	}

	printf("Total_Healing %d\n", global->bestHealing);
}

/* perform DFS 
   "power" stores the points on the chain heal after "hop" hops. 
*/
void DFS(Node *from, Node *n, int hop, int totalHealing, double power, Global *g) {
	if (n->visited == 1 || hop > g->numJumps) {
		return;
	}

	int i;
	n->visited = 1;
	n->prev = from;
	n->pp = calculateHealing(n, rint(power));
	totalHealing += n->pp;
	
	for (i = 0; i < n->adjSize; i++) {
		if (from != n->adj[i]) {
			double powerReduction = 1 - g->powerReduction;
			DFS(n, n->adj[i], hop + 1, totalHealing, power * powerReduction, g);
		}
	}

	if (totalHealing > g->bestHealing) {
		g->bestHealing = totalHealing;
		
		Node *temp = n;
		i = 0;
		g->bestPathLength = 0;
		while (temp != NULL && i < g->numJumps) {
			g->bestPath[i] = temp;
			g->healing[i] = temp->pp;	
			g->bestPathLength++;
			
			if (temp->prev == temp) {
				break;
			}
			
			temp = temp->prev;
			i++;
		}
	}

	n->visited = 0;
}
