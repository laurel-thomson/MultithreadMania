#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define WIKI_ARRAY_SIZE 8


#define MAX_ENTRY_LENGTH 100

char wiki_array[WIKI_ARRAY_SIZE][MAX_ENTRY_LENGTH];
char substrings[WIKI_ARRAY_SIZE-1][MAX_ENTRY_LENGTH];

void readFile();
void calcSubstring(int);
void printResults();

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int main() {
	omp_set_num_threads(NUM_THREADS);
	
	readFile();

	#pragma omp parallel 
	{
		calcSubstring(omp_get_thread_num());
	}
	
	printResults();
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
	int index;
	
	#pragma omp private(L, string1, string2, m, n, substring)
	{
		for (index = threadID; index < WIKI_ARRAY_SIZE - 1; index += NUM_THREADS)
		{
			printf("Index = %d\n",index);
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
				else if (L[i+1,j] >= L[i,j+1]) i++;
				else j++;
			}
			//add the null terminating character to the end of the substring
			substring[s_index] = '\0';
			
			#pragma omp critical
			{
				strcpy(substrings[index],substring);
			}
		}
	}
}

void printResults()
{
	int i;
	for (i = 0; i < WIKI_ARRAY_SIZE-1; i++)
	{
		printf("Line: %d, LCS: %s\n",i,substrings[i]);
	}
}
