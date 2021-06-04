#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int prevtime;


int main(int agrc, char *argv[])
{
    initClk();
    //TODO The process needs to get the remaining time from somewhere
    //THIS WILL BE ALL CHANGED TO SHARED MEMORY PROBABLY
    remainingtime = atoi(argv[1]);
    printf("hello process just started with time %s\n", argv[1]);
    prevtime=getClk();
    while (remainingtime > 0)
    {
        if (getClk() != prevtime)
        {
            remainingtime--;
            printf("remaning time: %d\n", remainingtime);
            prevtime=getClk();
        }
    }
    printf("PROCESS IS DEAD \n");
    destroyClk(false);

    return 0;
}
