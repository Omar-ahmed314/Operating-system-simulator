//#include "headers.h"
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};

//void clearResources(int);
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    struct processData* prcsArray;

    // open the process file
    FILE * prcsFile;
    prcsFile = fopen(argv[1], "r");

    // count the process of the file
    int count = 0;
    char first_line[50];
    fgets(first_line, 100, prcsFile);
    while(!feof(prcsFile))
    {
        fgets(first_line, 100, prcsFile);
        count++;
    }

    // initialize the process array with calloc
    prcsArray = (struct processData*)calloc(count, sizeof(struct processData));
    fclose(prcsFile);
    
    prcsFile = fopen(argv[1], "r");

    char second_line[50];
    fgets(second_line, 100, prcsFile);
    for(int i = 0; i < count; i++)
    {
        fscanf(prcsFile, "%d %d %d %d", &prcsArray[i].id, &prcsArray[i].arrivaltime, &prcsArray[i].runningtime, &prcsArray[i].priority);
    }
    
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int algoNum = atoi(argv[2]);
    int algoArgs;
    int pid;
    if(algoNum == 5)
    algoArgs = atoi(argv[3]);
    // 3. Initiate and create the scheduler and clock processes.

    // initialize the clock
    pid = fork();
    if(pid == 0)
    execl("./clk.out");

    // initialize the scheduler
    pid = fork();
    if(pid == 0)
    execl("./scheduler.out");
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    ///int x = getClk();
    ///printf("Current Time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    ///destroyClk(true);
    return 0;
}


