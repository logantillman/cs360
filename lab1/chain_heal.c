#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h> 

int main(int argc, char **argv) {
	if (argc < 6) {
		//FIXME add error check
	}

	const int initialRange = atoi(argv[1]);
	const int jumpRange = atoi(argv[2]);
	const int numJumps = atoi(argv[3]);
	const int initialPower = atoi(argv[4]);
	const double powerReduction = atof(argv[5]);

	printf("%d %d %d %d %f\n", initialRange, jumpRange, numJumps, initialPower, powerReduction);

	return 0;
}
