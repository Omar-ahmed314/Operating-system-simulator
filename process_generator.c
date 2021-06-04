#include "headers.h"
struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};



int cid, downq_id, upq_id, send_val, rec_val;

struct msgbuff
{
    int mtype;
    struct processData processData;
};

struct msgbuff2
{
    int mtype;
    int data[3]; //data[0] = algo, data[1] = args, data[2] = no. of processes.
};

//void clearResources(int);
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    msgctl(downq_id, IPC_RMID, (struct msqid_ds *)0);
    msgctl(upq_id, IPC_RMID, (struct msqid_ds *)0);
    kill(cid, SIGINT);
    destroyClk(true);
    killpg(getpgrp(), SIGKILL);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    key_t key_id;
    
    key_id = 99;
    downq_id = msgget(key_id, 0666 | IPC_CREAT);
    upq_id = msgget(key_id, 0666 | IPC_CREAT);
    struct msgbuff message;
    if (downq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
     if (upq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }

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
    int algoNum = atoi(argv[3]);
    int algoArgs;
    int pid;
    struct msgbuff2 mess;
    mess.mtype =1;
    mess.data[0] = algoNum;
    if(algoNum == 5)
    {   
        algoArgs = atoi(argv[5]);
        mess.data[1] = algoArgs;
    }
    mess.data[2] = count-1;       
    // 3. Initiate and create the scheduler and clock processes.

    //Sending Data
    
    
    send_val = msgsnd(upq_id, &mess, sizeof(mess.data), !IPC_NOWAIT);
    // initialize the clock
    int sid=0;
   
    pid = fork();
    cid = pid;
    if(pid == 0)
    {
        execl("clk.out","clk.out",(char *)NULL);
    }  
    // initialize the scheduler
    pid = fork();
    sid = pid;
    if(pid == 0)
    {
        execl("scheduler.out","scheduler.out",(char *)NULL);
    }
        
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function. 
    int x = getClk();
    printf("Current Time is %d\n", x);
    // TODO Generation Main Loop
    int i=0;

    printf("PG: Algorithm number is %d\n",algoNum);
    
    while (i<count-1) //condition that processes are all done
    {
        x = getClk();
        
        if (x == (prcsArray[i].arrivaltime))
        {
            //printf("Process %d Started at time %d \n", prcsArray[i].id, x);
            message.processData.id = prcsArray[i].id;
            message.processData.arrivaltime = prcsArray[i].arrivaltime;
            message.processData.runningtime = prcsArray[i].runningtime;
            message.processData.priority = prcsArray[i].priority;
            send_val = msgsnd(upq_id, &message, sizeof(message.processData), !IPC_NOWAIT);
            kill(sid, SIGUSR1);
            i++;
        }
    }
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
    return 0;
}


