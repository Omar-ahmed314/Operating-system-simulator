#include "headers.h"
#include "schedulerData.h"
#include "SRTN.h"
void runProcess()
{
    int pid;
    pid = fork();
    if (pid == 0)
    {
        execl("process.out", "process.out", "5", (char *)NULL);
    }
    sleep(2);
    kill(pid, SIGSTOP);
    sleep(5);
    kill(pid, SIGCONT);
}
int algNum;
int noProcesses;
int processesDone; //to keep track if the program is finished

bool recievedProcess; //A flag that determines if a new process has just been recieved

struct PCBNode *PCB = 0;
struct PCBNode *runningPCB;
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
    //? where is starttime, remaining time?
    struct msgbuff message;
    //struct processData process;
    rec_val = msgrcv(upq_id, &message, sizeof(message.processData), 0, !IPC_NOWAIT);
    while (rec_val != -1)
    {
        struct processData *process = malloc(sizeof(struct processData));
        process->arrivaltime = message.processData.arrivaltime;
        process->id = message.processData.id;
        process->runningtime = message.processData.runningtime;
        process->priority = message.processData.priority;
        struct PCBNode *newProcess = malloc(sizeof(struct PCBNode));
        newProcess->pData = process;
        newProcess->state = ready;
        newProcess->hasStarted = false;
        newProcess->next = 0;
        insertNode(&PCB, newProcess);
        recievedProcess = true;
        rec_val = msgrcv(upq_id, &message, sizeof(message.processData), 0, IPC_NOWAIT);
        // printPCB_ID(PCB);
        // printf("%d %d \n", message.processData.arrivaltime, message.processData.id);
    }
    if (rec_val == -1)
    {
        return;
    }
}
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    destroyPCB(PCB);
    destroyClk(false);
    kill(getpid(), SIGKILL);
}

int main(int argc, char *argv[])
{
    initClk();
    signal(SIGINT, clearResources);
    signal(SIGUSR1, recieveProcess);
    key_t key_id;
    recievedProcess = false;
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
    algNum = mess.data[0];
    noProcesses = mess.data[2];
    // runProcess();
    int clk = getClk();
    int prevClk = clk;
    while (1)
    {
        if (prevClk != getClk())
        {
            printf("clk =  %d\n", prevClk);
            prevClk = getClk();
            struct PCBNode *temp = PCB;
            while (temp)
            {
                printf("%d ", temp->pData->id);
                temp = temp->next;
            }
            printf("\n");
        }
    }
    if (algNum == FCFS)
    {
        //Call Alg 1 with printing inside
    }
    else if (algNum == SJF)
    {
        //Call Alg 2 with printing inside
    }
    else if (algNum == HPF)
    {
        //Call Alg 3 with printing inside
    }
    else if (algNum == SRTN)
    {
        //Call Alg 4 with printing inside

        SRTNAlgorithm(PCB);
    }
    else if (algNum == RR)
    {
        //Call Alg 5 with printing inside
    }
    // initClk();
    while (true)
        ;

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
void HPFAlgorithm() //////////NOT FINAL
{
    while (processesDone < noProcesses || runningPCB != NULL)
    {

        if (recievedProcess)
        {
            struct PCBNode *temp = PCB;
            struct PCBNode *chosenProcess;
            chosenProcess = temp;
            while (temp != NULL)
            {
                if (temp->state == ready && temp->pData->priority < chosenProcess->pData->priority)
                {
                    chosenProcess = temp;
                }
                temp = temp->next;
            }
            if (runningPCB == NULL)
            {
                //run
            }
            else if (chosenProcess->pData->priority < runningPCB->pData->priority)
            {
                //stop running
                //run
            }
            recievedProcess = false;
        }
    }
}
