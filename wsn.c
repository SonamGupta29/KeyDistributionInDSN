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

	int count = d, points[500][500] = {0}, i, x, y, j, k, l;
	double distance = 0, avgDistance = 0, commRange = 0;
	sensor sensorList[10000];

	FILE *fp = fopen(DAT_FILE, "w");
	FILE *fplot = fopen("wsn.gp", "w");

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
				sensorList[count].x = i;
				sensorList[count].y = j;
				if(++count != d)
					fprintf(fp, "\n");
			}
		}
	}
	
	printf("Reading data file...\n");
	fprintf(fplot, "plot '%s'", DAT_FILE);

	//	Close the handles
	fclose(fp);
	fclose(fplot);

	system("gnuplot -p 'wsn.gp'");
	printf("Scaling communication range...\n");

	//	Calculate the average distance for each point

	for(i = 0; i < d; i++)
		for(j = 0; j < d; j++)
			if(i != j)
				count++,
				distance += sqrt(pow((sensorList[i].x - sensorList[j].x), 2) 
							   + pow((sensorList[i].y - sensorList[j].y), 2));

	printf("Average distance = %.2lf m\n", (avgDistance = distance / count));
	printf("Communication range of sensor nodes = %.2lf m\n", (commRange = avgDistance / 10));

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

	printf("Computing physical neighbors...\n");
	printf("Average neighborhood size = %d\n", (int)ceil(count / (float)d));
	printf("EG scheme\n");
	printf("Distributing keys...\n");
	
	//	Distribute the keys to the sensors

	//	Set the arry of Key_poolsize so that we can avoid the repeatation in random numbers
	int randomKeyPoolArray[40001] = {0}, key = 0;

	for(i = 0; i < d; i++)
	{
		sensorList[i].keyring = (int*)malloc(sizeof(int) * (KEYRING_SIZE + 10));
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
		sensorList[i].keynbr = (int*)malloc(sizeof(int) * 1000);
		for(j = 0; j < sensorList[i].phynbrsize; j++)
		{
			//	Check each physical nighbour sensor node key list with the current 
			//	sensor node, if they have common key then they are key nighbours
			int flag = 0;
			currentPhyNei = sensorList[i].phynbr[j];

			//	Get the keyring of the current sensor node
			int counter = 0;
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

	printf("Computing key neighborhood in direct key establishment phase...\n");
	
	int totalPhyNei = 0, totalKeyNei = 0;
	double top = 0, down = 0, theoriticalResult = 1;

	for(i = 0; i < d; i++)
	{
		totalKeyNei += sensorList[i].keynbrsize,
		totalPhyNei += sensorList[i].phynbrsize;
	}

	printf("Simulated average network connectivity = %.4lf\n", totalKeyNei / (double)(totalPhyNei));

	for(i = 0; i < KEYRING_SIZE; i++)
	{
		top = KEYPOOL_SIZE - KEYRING_SIZE - i;
		down = KEYPOOL_SIZE - i;
		theoriticalResult *= (top / down);
	}	
	printf("Theoretical connectivity = %.4lf\n", 1 - theoriticalResult);


	/*//	Calculate the average distance for each point
	count = 0;
	for(i = 0; i < 500; i++)
		for(j = 0; j < 500; j++)
			if(points[i][j])
				for(k = 0; k < 500; k++)
					for(l = 0; l < 500; l++)
						if(points[k][l])
							count++, distance += sqrt(pow((k - i), 2) + pow((l - j), 2));

	avgDistance = distance / count;

	printf("Average distance = %.2lf m\n", avgDistance);
	printf("Communication range of sensor nodes = %.2lf m\n", avgDistance / 10);

	//	Compute the physical neighbours i.e. compute the neighbours which are within 
	// the radius of the communication range and add it to the structure of the sensor node

	int innercount = 0, outercount = 0;
	double avgSize = 0;
	count = 0;

	printf("Computing physical neighbors...\n");

	for(i = 0; i < 500; i++)
		for(j = 0; j < 500; j++)
			if(points[i][j])
			{
				//	Assign the values to the sensor struct
				sensorList[count].x = i;
				sensorList[count].y = j;
				
				//	Calculate the neighbours for the point within the radius of the avgdistance 
				// 	which is our radius 

				for(k = 0; k < 500; k++)
				{
					for(l = 0; l < 500; l++)
					{
						if(points[k][l] && (i != k || j != l))
						{
							if((avgDistance / 10) >=  sqrt(pow((i - k), 2) + pow((j - l), 2)))
							{
								outercount++;
								// printf("(%d, %d) --> (%d, %d)\n", k, l, i, j);


							}
						}
					}
				}
				sensorList[count].phynbrsize = outercount;
				innercount++;
				count++;
			}

	printf("Average neighborhood size %d\n", (int)ceil(outercount / (double)innercount));
	printf("EG scheme\n");
	printf("Distributing keys...\n");
	*/

	return 0;
}