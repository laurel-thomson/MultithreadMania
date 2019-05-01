#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4
#define WIKI_ARRAY_SIZE 50
#define MAX_ENTRY_LENGTH 1000

char wiki_array[WIKI_ARRAY_SIZE][MAX_ENTRY_LENGTH];
char substrings[WIKI_ARRAY_SIZE-1][MAX_ENTRY_LENGTH];

pthread_mutex_t lock;
int lines_read;
int batch_number = 0;

int readFile(FILE *);
void calcSubstring(int);
void printResults();

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int main() {
		FILE * fp = fopen("/homes/dan/625/wiki_dump.txt","r");
        while (1)
        {
                lines_read = readFile(fp);
                if (lines_read == 0) break;

				pthread_mutex_init(&lock, NULL);
				pthread_t threads[NUM_THREADS];
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


				int result_code;
				void * status;
				int i;
				for (i = 0; i < NUM_THREADS; i++)
				{
						result_code = pthread_create(&threads[i], &attr, calcSubstring, (void *)i);
						if (result_code) {
								printf("ERROR; return code from pthread_create() is %d\n", result_code);
								exit(-1);
						}
				}

				//free attribute and wait for the other threads
				pthread_attr_destroy(&attr);
				for(i=0; i<NUM_THREADS; i++) {
						result_code = pthread_join(threads[i], &status);
						if (result_code)
						{
								printf("ERROR; return code from pthread_join() is %d\n", result_code);
								exit(-1);
						}
				}
				
                printResults();
                batch_number++;
                if (lines_read < WIKI_ARRAY_SIZE) break;
				pthread_mutex_destroy(&lock);
				pthread_exit(NULL);
        }
        fclose(fp);
		
		
        return 0;
}

int readFile(FILE * fp)
{
        if(fp == NULL)
        {
                printf("fail");
                return;
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
        int index;

        for (index = threadID; index < WIKI_ARRAY_SIZE - 1; index += NUM_THREADS)
        {
                //dynamic programming table for comparing two wiki entries
                char * string1;
                char * string2;
                char substring[MAX_ENTRY_LENGTH]; //the longest common substring
                int m; //length of string 1
                int n; //length of string 2

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

                //critical section
                pthread_mutex_lock (&lock);
                strcpy(substrings[index],substring);
                pthread_mutex_unlock (&lock);
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
