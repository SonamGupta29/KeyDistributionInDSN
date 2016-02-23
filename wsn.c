#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

char DAT_FILE[100]; 	/* 	locations of sensor nodes in the target
							field to be stored in a data file */
int KEYRING_SIZE; 		/* 	key ring size m for each sensor node */
int KEYPOOL_SIZE;

/* key pool size M */
int n;

/* total number of sensor nodes to be deployed */
int d;

/* average number of neighbor nodes for each sensor node */
double RANGE;

/* communication range of sensor node */
double p; /* network connectivity during direct key establishment phase */
double pi; /* network connectivity during path key establishment phase
using i hops in the path */

/* a sensor node representation */
typedef struct {
	int x; /* x-coordinate of sensor node location in target field */
	int y; /* y-coordinate of sensor node location in target field */
	int *keyring; /* key ring */
	int phynbrsize; /* number of physical neighbors of a sensor node */
	int keynbrsize; /* number of key neighbors of a sensor node */
	int *phynbr; /* list of physical neighbors of a sensor node */
	int *keynbr; /* list of key neighbors of a sensor node */
} sensor;

int main(int argc, char **argv)
{

	if(argc != 4)
	{
		printf("Invalid Syntax...\n./a.out DATA-FILE KEYRING-SIZE KEYPOOL-SIZE\n");
		exit(0);
	}

	//	Assign the commnd line arguments to the variables

	//	Copy the filename in the DAT_FILE
	strcpy(DAT_FILE, argv[1]);

	//	Assign the Keyring size to the variable
	KEYRING_SIZE = atoi(argv[2]);

	//	Assign the Keypool size to the variable
	KEYPOOL_SIZE = atoi(argv[3]);

	//	Total number of sensor nodes to be deployed
	d = 10000;

	int count = 10000, points[500][500] = {0}, i, x, y, j, k, l;
	double distance = 0, avgDistance = 0;

	FILE *fp = fopen(DAT_FILE, "w");
	FILE *fplot = fopen("wsn.gp", "w");

	//	Generate the non-repeating random pairs
	while(count--)
		(points[x = rand()%500][y = rand()%500] == 0)? points[x][y] = 1 : count++;

	//	Displate the non-repeating the random pairs
	count = 0;
	for(i = 0; i < 500; i++) 	
		for(j = 0; j < 500; j++)
			if(points[i][j])
			{
				fprintf(fp, "%d %d", i,j);
				if(++count != 10000)
					fprintf(fp, "\n");
			}
	
	printf("Reading data file...\n");

	fprintf(fplot, "plot '%s'", DAT_FILE);

	//	Close the handles
	fclose(fp);
	fclose(fplot);

	system("gnuplot -p 'wsn.gp'");

	printf("Scaling communication range...\n");

	//	Calculate the average distance for each point
	count = 0;
	for(i = 0; i < 500; i++)
		for(j = 0; j < 500; j++)
			if(points[i][j])
				for(k = 0; k < 500; k++)
					for(l = 0; l < 500; l++)
						if(points[k][l])
							count++, distance += sqrt(pow((k - i), 2) + pow((l - j), 2));

	avgDistance = distance / count;

	printf("Average distance = %lf m\n", avgDistance);
	printf("Communication range of sensor nodes = %lf\n", avgDistance / 10);

	//	Compute the physical neighbours i.e. compute the neighbours which are within 
	// the radius of the communication range and add it to the structure of the sensor node

	sensor sensorList[10000];

	for(i = 0; i < 500; i++)
		for(j = 0; j < 500; j++)
			if(points[i][j])
			{
				sensorList[count++].x = i;
				sensorList[count++].y = j;
			}






	return 0;
}