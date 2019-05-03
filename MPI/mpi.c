#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define WIKI_ARRAY_SIZE 50
#define MAX_ENTRY_LENGTH 1000

char wiki_array[WIKI_ARRAY_SIZE][MAX_ENTRY_LENGTH];
char substrings[WIKI_ARRAY_SIZE-1][MAX_ENTRY_LENGTH];
int lines_read;
int batch_number = 0;
int num_threads;

int readFile(FILE *);
void calcSubstring(int);
void printResults();

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int main(int argc, char *argv[]) {
	int num_tasks, rank, ierr;

	MPI_Status status;
	MPI_Request request;

	ierr = MPI_Init(&argc, &argv);
	FILE * fp = fopen("/homes/dan/625/wiki_dump.txt","r");

	if (ierr != MPI_SUCCESS)
	{
		printf("Error");
		MPI_Abort(MPI_COMM_WORLD, ierr);
	}
	else {
		MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
		num_threads = num_tasks;

		lines_read = readFile(fp);

		if (lines_read == 0) return 0;

		printResults();
		batch_number++;
		if (lines_read < WIKI_ARRAY_SIZE) return 0;
	}
	fclose(fp);
	return 0;
}

int readFile(FILE * fp)
{
	if(fp == NULL)
	{
		printf("fail");
		return 0;
	}

	char buff[MAX_ENTRY_LENGTH];
	int line_number = 0;
	int i;
	for (i = 0; i < WIKI_ARRAY_SIZE; i++)
	{
		if (fgets(buff,MAX_ENTRY_LENGTH,fp) == NULL)
		{
			return i;
		}
		strcpy(wiki_array[line_number],buff);
		line_number++;
	}
	return i;
}

void calcSubstring(int threadID)
{
	//dynamic programming table for comparing two wiki entries
	char * string1;
	char * string2;
	char substring[MAX_ENTRY_LENGTH]; //the longest common substring
	int m; //length of string 1
	int n; //length of string 2
	int index;

	for (index = threadID; index < WIKI_ARRAY_SIZE - 1; index += NUM_THREADS)
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

		strcpy(substrings[index],substring);
	}
}

void printResults()
{
	int i;
	for (i = 0; i < WIKI_ARRAY_SIZE-1; i++)
	{
		printf("Line: %d, LCS: %s\n",batch_number*WIKI_ARRAY_SIZE + i,substrings[i]);
	}
}
