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
	int firsthopcount; /* count of the sensor nodes which are reachable using a hop */
	int secondhopcount;	/* count of the sensor nodes which are reachable using second hop */
	int *keyring; /* key ring */
	int phynbrsize; /* number of physical neighbors of a sensor node */
	int keynbrsize; /* number of key neighbors of a sensor node */
	int nonkeynbrsize; /* number of key neighbors of a sensor node */
	int *phynbr; /* list of physical neighbors of a sensor node */
	int *keynbr; /* list of key neighbors of a sensor node */
	int *nonkeynbr; /* list of non key neighbors of a sensor node */
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
	// 	We can read the total number of nodes from the data file.
	d = 10000;

	int count = d, points[500][500] = {{0}}, i, x, y, j, k, l, m;
	double distance = 0, avgDistance = 0, commRange = 0;
	char GP_INPUT_DATA_FILE[100] = "gnuplotDatafile.dat";
	sensor sensorList[10000];

	FILE *fp = fopen(DAT_FILE, "w");
	FILE *fpgnuplotData = fopen(GP_INPUT_DATA_FILE, "w");
	FILE *fplot = fopen("wsn.gp", "w");

	//Write the number of the sensors 
	fprintf(fp, "%d\n", d);

	//	Generate the non-repeating random pairs
	while(count--)
		(points[x = rand()%500][y = rand()%500] == 0)? points[x][y] = 1 : count++;

	//	Displate the non-repeating the random pairs
	count = 0;
	for(i = 0; i < 500; i++)
	{
		for(j = 0; j < 500; j++)
		{
			if(points[i][j])
			{
				fprintf(fp, "%d %d", i,j);
				fprintf(fpgnuplotData, "%d %d", i,j);
				sensorList[count].x = i;
				sensorList[count].y = j;
				if(++count != d)
					fprintf(fp, "\n"), fprintf(fpgnuplotData, "\n");
			}
		}
	}
	
	printf("\nReading data file...\n");
	fprintf(fplot, "plot '%s'", GP_INPUT_DATA_FILE);

	//	Close the handles
	fclose(fp);
	fclose(fpgnuplotData);
	fclose(fplot);

	system("gnuplot -p 'wsn.gp'");
	printf("\nScaling communication range...\n");

	//	Calculate the average distance for each point

	for(i = 0; i < d; i++)
		for(j = 0; j < d; j++)
			if(i != j)
				count++, distance += sqrt(pow((sensorList[i].x - sensorList[j].x), 2) 
								  		+ pow((sensorList[i].y - sensorList[j].y), 2));

	printf("Average distance = %.2lf m\n", (avgDistance = distance / count));
	printf("Communication range of sensor nodes = %.2lf m (Considered)\n", (commRange = 27) );//(commRange = avgDistance / 10));
	printf("\nComputing physical neighbors...\n");

	//	Approximate the commRange value
	commRange = 27;
	//	Calculate the physical neighbours
	int singleSensorCounter = 0;
	count = 0;	
	for(i = 0; i < d; i++)
	{
		singleSensorCounter = 0;
		sensorList[i].phynbr = (int*)malloc(sizeof(int) * 1000);
		for(j = 0; j < d; j++)
		{
			if(i != j)
			{
				if(commRange >= sqrt(pow((sensorList[i].x - sensorList[j].x), 2) 
							   + pow((sensorList[i].y - sensorList[j].y), 2)))
				{
					sensorList[i].phynbr[singleSensorCounter++] = j;
					//printf("(%d, %d) --> (%d, %d)\n", sensorList[i].x, sensorList[i].y, sensorList[j].x, sensorList[j].y);
					count++;
				}
			}
		}
		sensorList[i].phynbrsize = singleSensorCounter;
	}

	printf("Average neighborhood size = %d (ceil value)\n", (int)ceil(count / (float)d));
	printf("\nEG scheme\n");
	printf("Distributing keys...\n");
	
	//	Distribute the keys to the sensors

	//	Set the arry of Key_poolsize so that we can avoid the repeatation in random numbers
	int randomKeyPoolArray[40001] = {0}, key = 0;

	for(i = 0; i < d; i++)
	{
		sensorList[i].keyring = (int*)malloc(sizeof(int) * (KEYRING_SIZE + 200));
		for(j = 0; j < KEYRING_SIZE;)
		{
			if(randomKeyPoolArray[key = rand() % KEYPOOL_SIZE] == 0)
			{
				randomKeyPoolArray[key]	= 1;
				sensorList[i].keyring[j] = key;
				j++;
			}
		}
		/*printf("\n%d -> ", i);
		for(j = 0; j < KEYRING_SIZE; j++)
			printf(" %d ", sensorList[i].keyring[j]);*/
		memset(randomKeyPoolArray, 0, sizeof randomKeyPoolArray);
	}

	//	Calculate the key neighbours for the sensor nodes
	int currentPhyNei = 0;
	for(i = 0; i < d; i++)
	{
		//	Get the list of physical neighbours of current sensor node
		int keyNbrCount = 0;
		sensorList[i].keynbr = (int*)malloc(sizeof(int) * 200);
		for(j = 0; j < sensorList[i].phynbrsize; j++)
		{
			//	Check each physical nighbour sensor node key list with the current 
			//	sensor node, if they have common key then they are key nighbours
			int flag = 0;
			currentPhyNei = sensorList[i].phynbr[j];

			//	Get the keyring of the current sensor node
			for(k = 0; k < KEYRING_SIZE; k++)
			{
				//	Get the key ring of the current physical neighbour node
				for(l = 0; l < KEYRING_SIZE; l++)
				{
					if(sensorList[i].keyring[k] == sensorList[currentPhyNei].keyring[l])
					{
						//printf("%d share a key with %d and key is %d\n",i, currentPhyNei, sensorList[currentPhyNei].keyring[l]);
						sensorList[i].keynbr[keyNbrCount++] = currentPhyNei;
						flag = 1;
						break;
					}
				}
				if(flag == 1)
					break;
			}
		}
		sensorList[i].keynbrsize = keyNbrCount;
	}

	printf("\nComputing key neighborhood in direct key establishment phase...\n");
	
	int totalPhyNei = 0, totalKeyNei = 0;
	double top = 0, down = 0, theoriticalResult = 1, d0;

	for(i = 0; i < d; i++)
		totalKeyNei += sensorList[i].keynbrsize,
		totalPhyNei += sensorList[i].phynbrsize;

	//printf("\nKey Nei : %d  Phy Nei : %d\n", totalKeyNei, totalPhyNei);

	d0 = totalPhyNei / (double)d;
	printf("Simulated average network connectivity = %.4lf\n", totalKeyNei / (double)(totalPhyNei));

	for(i = 0; i < KEYRING_SIZE; i++)
	{
		top = KEYPOOL_SIZE - KEYRING_SIZE - i;
		down = KEYPOOL_SIZE - i;
		theoriticalResult *= (top / down);
	}	
	printf("Theoretical connectivity = %.4lf\n", (p = 1 - theoriticalResult));

	printf("\nCalculating first hop connectivity (PKE)\n");

	// Get the non key neighbours of all the sensor nodes
	for(i = 0; i < d; i++)
	{
		int nonKeyNei[40000] = {0};
		sensorList[i].firsthopcount = sensorList[i].secondhopcount = 0;
		sensorList[i].nonkeynbr = (int*)malloc(sizeof(int) * (KEYRING_SIZE + 200));
		for(count = 0, j = 0; j < sensorList[i].phynbrsize; j++)
			for(nonKeyNei[sensorList[i].phynbr[j]] = 1, k = 0; k < sensorList[i].keynbrsize; k++)
				if(sensorList[i].phynbr[j] == sensorList[i].keynbr[k])
					nonKeyNei[sensorList[i].keynbr[k]] = 0;
		for(j = 0; j < 40000; j++)
			if(nonKeyNei[j] == 1)
				sensorList[i].nonkeynbr[count++] = j;
		sensorList[i].nonkeynbrsize = count;
	}


	//	First hop calculation; check if we can reach the nonkey neighbours from the key neighbours
	// 	for that check for the key neighbours of the current node key neighbours

	//	For all nodes
	for(i = 0; i < d; i++)
	{
		//	for current sensor node loop through all non key neighbours 
		for(j = 0; j < sensorList[i].nonkeynbrsize; j++)
		{
			int currentNonKeyNeighbour = sensorList[i].nonkeynbr[j];
			int flag = 0;
			//	check if we can reach to this non key neighbour from the key neighbours
			for(k = 0; k < sensorList[i].keynbrsize; k++)
			{
				//	Get the key neighbours of the key neighbours of the current sensor node
				//	if in that array we get the nonkeyneighbour that means we can reach to that
				// 	node using key neighbour as first hop
				int currentKeyNeighbour = sensorList[i].keynbr[k];

				for(l = 0; l < sensorList[currentKeyNeighbour].keynbrsize; l++)
				{
					if(currentNonKeyNeighbour == sensorList[currentKeyNeighbour].keynbr[l])
					{
						flag = 1;
						sensorList[i].firsthopcount++;
						break;
					}
				}
				if(flag == 1)
					break;
			}
		}
	}

	int totalFirstHopNei = 0;
	for(i = 0; i < d; i++)
		totalFirstHopNei += sensorList[i].firsthopcount;

	double p1 = 1 - ((1 - p)* pow((1 - (p * p)), d0));

	printf("Simulated average network connectivity = %lf\n", (totalFirstHopNei + totalKeyNei) / (double)(totalPhyNei));
	printf("Theoretical connectivity = %lf\n", p1);

	printf("\nCalculating second hop connectivity (PKE)\n");

	//	Second hop calculation; check if we can reach the nonkey neighbours from the key neighbours of the
	//	key neighbours of current node which will be our second hop

	//	For all nodes
	for(i = 0; i < d; i++)
	{
		//	for current sensor node loop through all non key neighbours 
		for(j = 0; j < sensorList[i].nonkeynbrsize; j++)
		{
			int currentNonKeyNeighbour = sensorList[i].nonkeynbr[j];
			int flag = 0;
			//	check if we can reach to this non key neighbour from the key neighbours
			for(k = 0; k < sensorList[i].keynbrsize; k++)
			{
				//	Get the key neighbours of the key neighbours of the current sensor node
				//	if in that array we get the nonkeyneighbour that means we can reach to that
				// 	node using key neighbour as first hop
				int currentKeyNeighbour = sensorList[i].keynbr[k];

				for(l = 0; l < sensorList[currentKeyNeighbour].keynbrsize; l++)
				{
					int currentKeyKeyNeighbour = sensorList[currentKeyNeighbour].keynbr[l];

					for(m = 0; m < sensorList[currentKeyKeyNeighbour].keynbrsize; m++)
					{
						if(currentNonKeyNeighbour == sensorList[currentKeyKeyNeighbour].keynbr[m])
						{
							flag = 1;
							sensorList[i].secondhopcount++;
							break;
						}
					}
					if(flag == 1)
						break;
				}
				if(flag == 1)
					break;
			}
		}
	}

	int totalSecondHopNei = 0;
	for(i = 0; i < d; i++)
		totalSecondHopNei += sensorList[i].secondhopcount;

	double p2 = 1 - ((1 - p1) * (pow((1 - (p * p1)),d0)));
    
	printf("Simulated average network connectivity = %lf\n", (totalSecondHopNei + totalKeyNei) / (double)(totalPhyNei));
	printf("Theoretical connectivity = %lf\n", p2);

	return 0;
}