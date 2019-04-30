#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define WIKI_ARRAY_SIZE 6
#define MAX_ENTRY_LENGTH 100

char wiki_array[WIKI_ARRAY_SIZE][MAX_ENTRY_LENGTH];
char substrings[WIKI_ARRAY_SIZE-1][MAX_ENTRY_LENGTH];

pthread_mutex_t lock;

void readFile();
void calcSubstring(int);
void printResults();

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int main() {
	pthread_mutex_init(&lock, NULL);
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	readFile();
	
	int result_code;

	for (int i = 0; i < NUM_THREADS; i++)
	{
		result_code = pthread_create(&threads[i], &attr, calcSubstring, (void *)i);
		if (result_code) {
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	
	//free attribute and wait for the other threads
	pthread_attr_destroy(&attr);
	for(i=0; i<NUM_THREADS; i++) {
		rc = pthread_join(threads[i], &status);
		if (rc) 
		{
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
	
	printResults();
	
	pthread_mutex_destroy(&lock);
	pthread_exit(NULL);
	return 0;
}

void readFile()
{
	FILE * fp = fopen("test.txt","r");
	if(fp == NULL)
	{
		printf("fail");
		return;
	}
	
	char buff[MAX_ENTRY_LENGTH];
	int line_number = 0;
	
	while(fgets(buff,MAX_ENTRY_LENGTH,fp) != NULL)
	{
		strcpy(wiki_array[line_number],buff);
		line_number++;
	}
	fclose(fp);
}

void calcSubstring(int threadID)
{
	//dynamic programming table for comparing two wiki entries
	char * string1;
	char * string2;
	char substring[MAX_ENTRY_LENGTH]; //the longest common substring
	int m; //length of string 1
	int n; //length of string 2
	
	//The last entry does not have an entry that follows it
	if (threadID == WIKI_ARRAY_SIZE - 1) return;
	
	#pragma omp private(L, string1, string2, m, n, substring)
	{
		string1 = wiki_array[threadID];
		string2 = wiki_array[threadID+1];
		m = strlen(string1);
		n = strlen(string2);
		int L[m+1][n+1];
		
		//calculate the dynamic programming table
		for (int i = 0; i <=m; i++)
		{
			for (int j = 0; j <= n; j++)
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
		int i = 0;
		int j = 0;
		
		while (i < m && j < n)
		{
			if (string1[i] == string2[j])
			{
				substring[s_index] = string1[i];
				i++; j++; s_index++;
			}
			else if (L[i+1,j] >= L[i,j+1]) i++;
			else j++;
		}
		
		//critical section
		pthread_mutex_lock (&lock);
		strcpy(substrings[threadID],substring);
		pthread_mutex_unlock (&lock);
	}
}

void printResults()
{
	for (int i = 0; i < WIKI_ARRAY_SIZE-1; i++)
	{
		printf("Line: %d, LCS: %s\n",i,substrings[i]);
	}
}