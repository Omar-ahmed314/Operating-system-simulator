//#include "headers.h"
#include<stdio.h>
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
    printf("hi");
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    struct processData* first;
    FILE * prcs;
    
    prcs = fopen("processes.txt", "r");
    
    int count = 0;
    char * first_line;
    fgets(first_line, 100, prcs);
    
    while(!feof(prcs))
    {
        fgets(first_line, 100, prcs);
        count++;
    }
    printf("%d ", count);
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock.
    ///initClk();
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


