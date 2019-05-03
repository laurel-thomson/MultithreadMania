#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <mpi.h>
#include <math.h>
#define ARTICLE_SIZE 10000
#define STRING_SIZE 15


typedef struct {
  uint32_t virtualMem;
  uint32_t physicalMem;
} processMem_t;


void find_longest_substring(char* line1_file, char* line2_file, int
length1, int length2, int line1, int line2);
void LCS_intermediate(int id, char ** LCS);
void undoMalloc(int **file, int num_line);
void print_results(char ** LCS);
void GetProcessMemory(processMem_t*);
int parseLine(char *);

int NUM_THREADS;
char** File_Contents;
int Lines_Read;


void find_longest_substring(char* line1_file, char* line2_file, int
length1, int length2, int line1, int line2)
{
//local lcs keeps track of the common string between 2 lines

int **local_LCS = malloc((length1 + 1) * sizeof(int *));//int to keep score

for (int i = 0; i < (length1 + 1); i++)
local_LCS[i] = (int*)malloc((length2 + 1) * sizeof(int));

int cur_substr_len = 0;

int row, col;

// go through each character in first string and for each of these
characters, traverse every
//character in the second string to find a match
for (int i = 0; i <= length1; i++)
{
for (int j = 0; j <= length2; j++)
{
//if one of the strings is empty, entry in the table is zero
if (i == 0 || j == 0)
local_LCS[i][j] = 0;

//check the previous entry in both strings
else if (line1_file[i - 1] == line2_file[j - 1])
{
//a new consecutive match is found so add 1 to the previous match
local_LCS[i][j] = local_LCS[i - 1][j - 1] + 1;
if (cur_substr_len < local_LCS[i][j])
{
cur_substr_len = local_LCS[i][j];
row = i;
col = j;
}
}
else
local_LCS[i][j] = 0;//no match for the previous character
}
}

/*
if (cur_substr_len == 0)
{
printf("%d-%d: No Common Substring\n", line1, line2);
return;
}
*/
char* resultingArr = (char*)malloc((cur_substr_len + 1) *
sizeof(char));//ending character needs to be accomodated
resultingArr[cur_substr_len] = '\0';//otherwise it continues to add
random characters to the string

// go through the table formed above and trace back the longest string
while (local_LCS[row][col] != 0)
{
resultingArr[--cur_substr_len] = line1_file[row - 1];
row--;
col--;//decrementing both because the longest entry will be formed
diagonally, not vertical nor horizontal
}
undoMalloc(local_LCS, length1);
/*
//remove newline char
size_t length;
if ((length = strlen(resultingArr)) > 0) {
if (resultingArr[length - 1] == '\n')
resultingArr[length - 1] = '\0';
}
*/
// required longest common substring
printf("%d-%d: %s\n", line1, line2, resultingArr);
free(resultingArr);

}

void LCS_intermediate(int id, char ** LCS )
{

int startPos = (id) * ((double)Lines_Read / NUM_THREADS);
int endPos = startPos + ((double)Lines_Read / NUM_THREADS);
int first_line = startPos;
int second_line = first_line + 1;
int currPos = 0;
int length1 = 0;
int length2 = 0;
char local_LCS[Lines_Read / NUM_THREADS][STRING_SIZE];
char substring[STRING_SIZE];
int i, j, x, y, maxlen, len;
int comp = 0;
/*
for (currPos = startPos; currPos < endPos; currPos++)
{
maxlen = 0;
length1 = strlen(File_Contents[currPos]);
length2 = strlen(File_Contents[currPos + 1]);
for (i = 0; i <= length1; i++)
{
for (j = 0; j <= length2; j++)
{
if (File_Contents[currPos][i] == File_Contents[currPos + 1][j])
{
substring[0] = File_Contents[currPos][i];
len = 1;
x = i + 1;
y = j + 1;
while (File_Contents[currPos][x] == File_Contents[currPos + 1][y] && x
< length1 && y < length2 && len < STRING_SIZE)
{
substring[len] = File_Contents[currPos][x];
len++;
x++;
y++;
}
if (len > maxlen)
{
maxlen = len;
strcpy(local_LCS[comp], substring);
}
}
}
}
comp++;
} //put substring in global array


int z = 0;
for (currPos = startPos; currPos < endPos; currPos++)
{
strcpy(LCS[currPos], local_LCS[z]);
z++;
}
*/
for (currPos = startPos; (currPos < endPos) && (currPos + 1
<Lines_Read); currPos++)
{
length1 = strlen(File_Contents[currPos]);
length2 = strlen(File_Contents[currPos + 1]);

find_longest_substring(File_Contents[currPos], File_Contents[currPos
+1], length1, length2, first_line, second_line);
first_line = second_line;
second_line++;
}
}





void print_results(char ** LCS)
{
int j; // then print out the totals
for (int i = 0; i < Lines_Read - 1; i++)
{
j = i + 1;
printf(" %d & %d - %s\n", i, j, LCS[i]);
}
}

int main(int argc, char *argv[])
{

struct timeval t1, t2;
double elapsedTime;
gettimeofday(&t1, NULL);

if (argc != 3)
{
printf("Enter the filename followed by the number of lines\n");
return 0;
}
  processMem_t myMemory;
FILE * fp = fopen(argv[1], "r");
if (fp == NULL) {
printf("File not found \n");
return 0;
}
int input_lines = atol(argv[2]);
Lines_Read = input_lines;
//read file into array


File_Contents = malloc(sizeof(char*) * input_lines);
int i=0;
while(i<input_lines)
{
if (ferror(fp) || feof(fp)) break;
File_Contents[i] = malloc(sizeof(char) * ARTICLE_SIZE);
fgets(File_Contents[i], ARTICLE_SIZE, fp);
i++;
}
char * LCS[Lines_Read - 1];
for (int i = 0; i < Lines_Read - 1; i++)
LCS[i] = (char *)malloc(sizeof(char) * (STRING_SIZE));

//process array in lcs function
int process_number, rank;

MPI_Status Status;

//intialize mpi program; master/root is rank 0
int root = MPI_Init(&argc, &argv);
if (root != MPI_SUCCESS) {
printf("Error starting MPI program. Terminating.\n");
MPI_Abort(MPI_COMM_WORLD, root);
}

MPI_Comm_size(MPI_COMM_WORLD, &process_number);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//determine the number of threads and set it
NUM_THREADS = process_number;
LCS_intermediate(rank, LCS);
//print_results(LCS);
//When root process ends, terminate MPI

MPI_Finalize();
if (rank == 0) {
gettimeofday(&t2, NULL);
elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; //sec to ms
elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; // us to ms
printf("%s, %f\n", getenv("SLURM_CPUS_ON_NODE"), elapsedTime);
  printf("DATA, %d, %u, %u\n", NUM_THREADS, myMemory.virtualMem,
myMemory.physicalMem);
printf("Main: program completed. Exiting.\n");
}
//free the array
undoMalloc((int **)File_Contents, Lines_Read);
fclose(fp);

GetProcessMemory(&myMemory);
return 0;
}

int parseLine(char *line) {
// This assumes that a digit will be found and the line ends in " Kb".
int i = strlen(line);
const char *p = line;
while (*p < '0' || *p > '9') p++;
line[i - 3] = '\0';
i = atoi(p);
printf("in parse: %d", i);
return i;
}// end parseLine

void undoMalloc(int **file, int num_line)
{
for (int i = 0; i < num_line; i++)
{
free(file[i]);
}
free(file);
}

void GetProcessMemory(processMem_t* processMem) {
FILE *file = fopen("/proc/self/status", "r");
char line[128];

while (fgets(line, 128, file) != NULL) {
//printf("%s", line);
if (strncmp(line, "VmSize:", 7) == 0) {
processMem->virtualMem = parseLine(line);
}

if (strncmp(line, "VmRSS:", 6) == 0) {
processMem->physicalMem = parseLine(line);
}
}
fclose(file);
}// end GetProcessMemory