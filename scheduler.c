#include "headers.h"

enum ProcessState {
    ready,
    running,
    blocked
};

struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};
struct PCBNode
{
    int state;
    bool hasStarted;
    struct processData* pData;
    int waitingTime;
    int remainingTime;
    int startTime; 
    struct PCBNode* next;
};
struct PCBNode* PCB;
struct PCBNode* runningPCB;
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
int downq_id, upq_id, send_val, rec_val;
void recieveProcess(int signum)
{
    struct msgbuff message;
    //struct processData process;
    rec_val = msgrcv(upq_id, &message, sizeof(message.processData), 0, !IPC_NOWAIT);
    struct processData* process = malloc(sizeof(struct processData));
    process->arrivaltime = message.processData.arrivaltime;
    process->id = message.processData.id;
    process->runningtime = message.processData.runningtime;
    process->priority = message.processData.priority;
    struct PCBNode* newProcess = malloc(sizeof(struct PCBNode));
    newProcess->pData = process;
    newProcess->state = ready;
    newProcess->hasStarted = false;
    //TODO newProcess is inserted in PCB
    printf("%d %d \n", message.processData.arrivaltime, message.processData.id);
}
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    
    destroyClk(false);
    kill(getpid(), SIGKILL);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    signal(SIGUSR1, recieveProcess);
    key_t key_id;
    
    key_id = 99;
    downq_id = msgget(key_id, 0666 | IPC_CREAT);
    upq_id = msgget(key_id, 0666 | IPC_CREAT);
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
    struct msgbuff2 mess;
    rec_val = msgrcv(downq_id, &mess, sizeof(mess.data), 0, !IPC_NOWAIT);
    printf("%d %d %d \n", mess.data[0], mess.data[1], mess.data[2]);
    if (mess.data[0] == 1)
    {
        //Call Alg 1 with printing inside
    }
    else if (mess.data[0] == 2)
    {
        //Call Alg 2 with printing inside
    }
    else if (mess.data[0] == 3)
    {
        //Call Alg 3 with printing inside
    }
    else if (mess.data[0] == 4)
    {
        //Call Alg 4 with printing inside
    }
    else if (mess.data[0] == 5)
    {
        //Call Alg 5 with printing inside
    }
    while(true);
    //initClk();

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    //destroyClk(true);
}
