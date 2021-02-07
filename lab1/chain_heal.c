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

/* calculates the amount of PP healed */
int calculateHealing(Node *person, double power);

/* creates an array of nodes */
Node** createNodeArray(Node* node, int numNodes);

/* creates the adj list for each node in the array */
void createAdjLists(Node **nodeArray, int numNodes, int jumpRange);

/* runs a DFS on nodes within the initialRange */
void performDFSOnConnectedNodes(Node **nodeArray, int numNodes, int initialRange, int initialPower, Global *global);

/* prints the desired output */
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

	const int initialRange = atoi(argv[1]);
	const int jumpRange = atoi(argv[2]);
	const int initialPower = atoi(argv[4]);
	
	Global *global = malloc(sizeof(Global));
	global->numJumps = atoi(argv[3]);
	global->powerReduction = atof(argv[5]);
	global->bestHealing = 0;
	global->bestPath = malloc(sizeof(Node *) * global->numJumps);
	global->healing = malloc(sizeof(int *) * global->numJumps);
	
	// Creating the nodes
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

/* calculates the amount of PP healed */
int calculateHealing(Node *person, double power) {
	// The amount of possible PP healing
	int possiblePP = person->currentPP + power;
	
	if (possiblePP > person->maxPP) {
		return person->maxPP - person->currentPP;
	}
	return power;
}

/* creates an array of nodes */
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

/* creates the adj list for each node in the array */
void createAdjLists(Node **nodeArray, int numNodes, int jumpRange) {
	int i, j, k;

	for (i = 0; i < numNodes; i++) {
		k = 0;

		// Calculating the size of the adj list
		nodeArray[i]->adjSize = 0;
		for (j = 0; j < numNodes; j++) {
			if (i == j) {
				continue;
			}
			if (isInRange(nodeArray[i], nodeArray[j], jumpRange)) {
				nodeArray[i]->adjSize++;
			}
		}

		// Allocating based on the size
		nodeArray[i]->adj = malloc(nodeArray[i]->adjSize * sizeof(Node *));

		// Putting nodes into their adj lists
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

/* runs a DFS on nodes within the initialRange */
void performDFSOnConnectedNodes(Node **nodeArray, int numNodes, int initialRange, int initialPower, Global *global) {
	Node *healer = nodeArray[numNodes - 1];
	int i;

	for (i = 0; i < numNodes; i++) {
		Node *companion = nodeArray[i];
		
		// Running DFS on companions within initialRange of the healer
		if (isInRange(healer, companion, initialRange)) {
			int hop = 1;
			int totalHealing = 0;
			DFS(companion, companion, hop, totalHealing, initialPower, global);
		}
	}
}

/* prints the desired output */
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
	
	// Recursively calling DFS for each adj node
	for (i = 0; i < n->adjSize; i++) {
		if (from != n->adj[i]) {
			double powerReduction = 1 - g->powerReduction;
			DFS(n, n->adj[i], hop + 1, totalHealing, power * powerReduction, g);
		}
	}

	if (totalHealing > g->bestHealing) {
		i = 0;
		g->bestHealing = totalHealing;
		g->bestPathLength = 0;
		
		// Tracing the best path using the prev fields
		Node *temp = n;
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
