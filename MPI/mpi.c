#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <sys/time.h>

#define CHUNK_SIZE 50
#define STRING_SIZE 1024

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

typedef struct {
  uint32_t virtualMem;
  uint32_t physicalMem;
} processMem_t;

struct timeval t1, t2;
double elapsedTime;
int NUM_THREADS;
int lines_read;
int batch_number = 0;
char wiki_array[CHUNK_SIZE][STRING_SIZE];
char substrings[CHUNK_SIZE][STRING_SIZE];
char local_substrings[CHUNK_SIZE][STRING_SIZE];

int readFile(FILE *);
void calcSubstring(void*);
void printResults();
int parseLine(char *);
void writeOutput();
void GetProcessMemory(processMem_t*);

int main(int argc, char *argv[]) {
	MPI_Status status;
	int rc, rank, numtasks;
	
	FILE * fp = fopen("/homes/dan/625/wiki_dump.txt","r");
	
	rc = MPI_Init(&argc,&argv);
	if (rc != MPI_SUCCESS) {
		printf ("Error starting MPI program. Terminating.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
	}
	
	gettimeofday(&t1, NULL);
	
	while (1)
	{
		MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
		MPI_Comm_rank(MPI_COMM_WORLD,&rank);

		NUM_THREADS = numtasks;

		printf("size = %d rank = %d\n", numtasks, rank);
		fflush(stdout);

		if (rank == 0) {
			lines_read = readFile(fp);
			if (lines_read == 0) break;
		}

		MPI_Bcast(wiki_array, CHUNK_SIZE * STRING_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);

		calcSubstring(&rank);

		MPI_Reduce(local_substrings, substrings, CHUNK_SIZE * STRING_SIZE, MPI_CHAR, MPI_SUM, 0, MPI_COMM_WORLD);

		if (rank == 0) {
			printResults();	
		}
		
		batch_number++;
		if (lines_read < CHUNK_SIZE) break;
	}
	
	gettimeofday(&t2, NULL);
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
	
	fclose(fp);
	
	processMem_t myMemory;
	GetProcessMemory(&myMemory);
	printf("Elapsed time: %f\n",elapsedTime);
	printf("Virtual Memory Usage: %d\n",myMemory.virtualMem);
	printf("Physical Memory Usage: %d\n",myMemory.physicalMem);
	printf("Number of threads: %d\n",NUM_THREADS);	
	
	MPI_Finalize();
}

int readFile(FILE * fp)
{
	if(fp == NULL)
	{
		printf("fail");
		return 0;
	}
	
	char buff[STRING_SIZE];
	int line_number = 0;
	int i;
	for (i = 0; i < CHUNK_SIZE; i++)
	{
		if (fgets(buff,STRING_SIZE,fp) == NULL)
		{
			return i;
		}
		strcpy(wiki_array[line_number],buff);
		line_number++;
	}
	return i;
}

void calcSubstring(void * rank)
{
	int threadID = *((int *)rank);
	
	//dynamic programming table for comparing two wiki entries
	char * string1;
	char * string2;
	char substring[STRING_SIZE]; //the longest common substring
	int m; //length of string 1
	int n; //length of string 2
	int index;
	
	for (index = threadID; index < CHUNK_SIZE - 1; index += NUM_THREADS)
	{
		string1 = wiki_array[index];
		string2 = wiki_array[index+1];
		m = strlen(string1); 
		n = strlen(string2);
		int L[m+1][n+1];
		int i,j;
		
		//calculate the dynamic programming table
		for (i = 0; i <=m; i++)
		{
			for (j = 0; j <= n; j++)
			{
				if (i == 0 || j == 0)
				{
					L[i][j] = 0;
				}
				else if (string1[i-1] == string2[j-1])
				{
					L[i][j] = L[i-1][j-1] + 1;
				}
				else
				{
					L[i][j] = MAX(L[i-1][j],L[i][j-1]);
				}
			}
		}
		
		//make another pass through the table to find the longest common substring
		int s_index = 0;
		i = 0;
		j = 0;
	
		while (i < m && j < n)
		{
			if (string1[i] == string2[j])
			{
				substring[s_index] = string1[i];
				i++; j++; s_index++;
			}
			else if (L[i+1][j] >= L[i][j+1]) i++;
			else j++;
		}
		substring[s_index] = '\0';
		strcpy(local_substrings[index],substring);
	}
}

void printResults()
{
	int i;
	for (i = 0; i < CHUNK_SIZE; i++)
	{
		printf("Line: %d, LCS: %s\n",batch_number*CHUNK_SIZE + i,substrings[i]);
	}
}

void GetProcessMemory(processMem_t* processMem) {
	FILE *file = fopen("/proc/self/status", "r");
	char line[128];

	while (fgets(line, 128, file) != NULL) {
		if (strncmp(line, "VmSize:", 7) == 0) {
			processMem->virtualMem = parseLine(line);
		}

		if (strncmp(line, "VmRSS:", 6) == 0) {
			processMem->physicalMem = parseLine(line);
		}
	}
	fclose(file);
}

int parseLine(char *line) {
	int i = strlen(line);
	const char *p = line;
	while (*p < '0' || *p > '9') p++;
	line[i - 3] = '\0';
	return atoi(p);
}

