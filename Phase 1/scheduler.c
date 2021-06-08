//TODO calculations file

//TODO when reading file: don't count # in any line
#include "headers.h"
#include "schedulerData.h"

void stopProcess(struct PCBNode *);
void startProcess(struct PCBNode *);
void HPFAlgorithm();
void RRAlgorithm();
void sendRem(int remainingTime);
void destroyRemMsg();
void empty() {}

int algNum, quantum;
int noProcesses, currentProcessesNumber;
int processesDone; //to keep track if the program is finished

bool recievedProcess = false; //A flag that determines if a new process has just been recieved

struct PCBNode *PCB = 0;
struct PCBNode *runningPCB;

float TWTA=0;
float TW=0;
int NonActive = 0; //Variables to calculate performance
// File handling
char stateStr[10];
FILE *pFileLog;
FILE *pFilePerf;
void eraseStateStr()
{
    memset(stateStr, 0, 10);
}
void setStateStr(int state)
{

    switch (state)
    {
    case started:
        strcpy(stateStr, "started");
        break;
    case stopped:
        strcpy(stateStr, "stopped");
        break;
    case resumed:
        strcpy(stateStr, "resumed");
        break;
    case finished:
        strcpy(stateStr, "finished");
        break;

    default:
        break;
    }
}

struct msgbuff
{
    long mtype;
    struct processData processData;
};
struct msgbuff2
{
    long mtype;
    int data[3]; //data[0] = algo, data[1] = args, data[2] = no. of processes.
};
int downq_id_rem, rec_val_rem;
key_t key_id_rem;
struct msgbuffRem
{
    long mtype;
    int remaining;
} mess_rem;
int downq_id, upq_id, send_val, rec_val;
struct msgbuff_nproc
{
    long mtype;
    int arrivedProccesses;
};
//@ receive handler
void recieveProcess(int signum)
{
    //? where is starttime, remaining time?
    struct msgbuff message;
    //struct processData process;
    rec_val = msgrcv(upq_id, &message, sizeof(message.processData), 0, !IPC_NOWAIT);
    struct processData *process = malloc(sizeof(struct processData));
    process->arrivaltime = message.processData.arrivaltime;
    process->id = message.processData.id;
    process->runningtime = message.processData.runningtime;
    process->priority = message.processData.priority;
    // //printf("^&*process of id = %d: priority = %d\n", process ? process->id : -1, process ? process->priority : -1);
    // //flush(stdout);
    struct PCBNode *newProcess = malloc(sizeof(struct PCBNode));
    newProcess->pData = process;
    newProcess->state = ready;
    newProcess->hasStarted = false;
    newProcess->next = 0;
    newProcess->pid = 0;
    newProcess->lastSeen = getClk();
    newProcess->waitingTime = 0;
    insertNode(&PCB, newProcess);
    recievedProcess = true;
    currentProcessesNumber++;
    // printf("received msg from PG\n");
    // printPCB(PCB);
    // //printf("%d %d \n", message.processData.arrivaltime, message.processData.id);
}
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    signal(SIGINT, SIG_DFL);
    destroyPCB(PCB);
    destroyRemMsg();
    destroyClk(true);
}
// @ Main function
int main(int argc, char *argv[])
{
    initClk();
    printf("SSSSSSSSSSSSS Scheduler starts @Clk = %d\n",getClk());
    signal(SIGINT, clearResources);
    mess_rem.mtype = 3;
    key_t key_id;
    recievedProcess = false;
    //files
    pFileLog = fopen("scheduler.log", "w");
    fprintf(pFileLog, "#At time x process y state arr w total z remain y wait k\n");
    //@ message queues
    key_id = 99;
    key_id_rem = 167;
    // ? why we need two message queues?
    // ? and why not keyfile?
    // ? why with the same ID?
    downq_id = msgget(key_id, 0666 | IPC_CREAT);
    upq_id = msgget(key_id, 0666 | IPC_CREAT);
    downq_id_rem = msgget(key_id_rem, 0666 | IPC_CREAT);
    if (downq_id == -1)
    {
        perror("Error in create\n");
        exit(-1);
    }
    if (upq_id == -1)
    {
        perror("Error in create\n");
        exit(-1);
    }
    if (downq_id_rem == -1)
    {
        perror("Error in create\n");
        exit(-1);
    }
    struct msgbuff2 mess;
    rec_val = msgrcv(downq_id, &mess, sizeof(mess.data), 0, !IPC_NOWAIT);
    //printf("%d %d %d \n", mess.data[0], mess.data[1], mess.data[2]);
    algNum = mess.data[0];
    quantum = mess.data[1];
    noProcesses = mess.data[2];
    // runProcess();
    int prevClk = 0;
    struct PCBNode *currentProcess = NULL;
    printf("Total number of processes = %d\n", noProcesses);
    printf("algorithm number = %d\n", algNum);
    printf("Quantum = %d\n", quantum);
    //flush(stdout);
    int processesCounter = 0;
    int previousId = 0;
    int roundRobinCounter = quantum;
    int arrivedProcessesCounter = 0;
    //printf("Entering\n");
    // @@@@@@ algorithms @@@@@
    while (processesCounter < noProcesses)
    {
        int currentClk = getClk();
        if (prevClk == currentClk)
        {
            continue;
        }

        //>>>> receive number of received processes:
        struct msgbuff_nproc msg_n;
        if (arrivedProcessesCounter < noProcesses) // there are still processes to receive
        {
            rec_val = msgrcv(upq_id, &msg_n, sizeof(msg_n.arrivedProccesses), 0, !IPC_NOWAIT);
            // printf("--- SC: Count of arrived proccesses received = %d\n",msg_n.arrivedProccesses);

            for (int i = 0; i < msg_n.arrivedProccesses; i++)
            {
                recieveProcess(0);         // no meaning for the parameter
                arrivedProcessesCounter++; // can make it with += arriv...
            }
            printf("At CLK %d, %d processes arrived. now count(PCB) = %d\n", getClk(), msg_n.arrivedProccesses, countPCB(PCB));
        }
        NonActive = currentClk>1&&!currentProcess?NonActive+1:NonActive;
        if (currentProcess)
        {
            //@ deleting
            currentProcess->remainingTime--;
            sendRem(currentProcess->remainingTime);
            if (currentProcess->remainingTime <= 0)
            {
                struct PCBNode *process = currentProcess; //lazy to rewrite
                setStateStr(finished);
                int TA= getClk()-process->pData->arrivaltime;
                float WTA=TA*1.0/process->pData->runningtime;
                TWTA+=WTA;
                TW+=process->waitingTime;
                fprintf(pFileLog, "At time %d process %d %s arr %d total %d remain %d wait %d TA %d WTA %.2g\n",
                        getClk(), process->pData->id, stateStr, process->pData->arrivaltime, process->pData->runningtime,
                        process->remainingTime, process->waitingTime, TA, WTA);

                if (algNum != RR) // % because it will be needed to get next one
                {
                    deleteByID(&PCB, currentProcess->pData->id);
                    processesCounter++;
                }
                else
                {
                    roundRobinCounter = quantum + 1; 
                    deleteByID(&PCB, currentProcess->pData->id);
                    currentProcess = NULL;
                    processesCounter++;
                }
            }
        }
        if (algNum != RR)
        {
            currentProcess = findTarget(algNum, PCB);
        }
        else // round robin
        {
            // TODO insert A in the the queue, not circular queue
            if (!currentProcess) // at the beginning of execution
            {
                currentProcess = PCB;
            }

            // printf("***** at clk %d: Current process ID = %d\n", getClk(), currentProcess ? currentProcess->pData->id : -1);
        }
        if (currentProcess)
        {
            // @ tracing
            printf("@@@CLK = %d: current process id = %d and PID = %d & PCB = ", currentClk, currentProcess->pData->id, currentProcess->pid);
            printPCB(PCB);
            // printf("------- real clk = %d\n", getClk());
            //context switching
            if (previousId != currentProcess->pData->id)
            {
                struct PCBNode *previousProc = searchID(PCB, previousId); // starting clk = 0 or previous deleted
                if (previousProc)
                {
                    stopProcess(previousProc);
                }
                startProcess(currentProcess);
                // changes for next loop
                previousId = currentProcess->pData->id;
            }
            
        }
        //@RR Switching
        if (algNum == RR)
        {
            roundRobinCounter--;
            if (roundRobinCounter <= 0)
            {
                roundRobinCounter = quantum;

                struct PCBNode *ptr = copyNode(currentProcess);
                ptr->next = NULL;
                deleteByID(&PCB, currentProcess->pData->id);
                insertNode(&PCB, ptr);
            }
            currentProcess = PCB;
        }
        prevClk = currentClk;
        // printf("$$$$ @ clk = %d PCB is ",getClk());
        // printPCB(PCB);
        // printf("------- real clk = %d\n", getClk());
    }
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.
    printf("Scheduler Ending\n");
    fclose(pFileLog);
    pFilePerf = fopen("scheduler.perf", "w");
    fprintf(pFilePerf, "CPU Utilization = %.3g%%\n", (getClk()-1-NonActive)*100.0/(getClk()-1));
    fprintf(pFilePerf, "Avg WTA = %.3g\n", TWTA/noProcesses);
    fprintf(pFilePerf, "Avg Waiting = %.2g\n", TW/noProcesses);
    fclose(pFilePerf);
    clearResources(SIGINT);
}
void stopProcess(struct PCBNode *process)
{
    if (process)
    {
        process->state = blocked;
        process->lastSeen = getClk();
        //TODO fprintf to the file that it is blocked
        kill(process->pid, SIGSTOP);
        int id = process->pData->id;
        // //printf("@CLK = %d: stopping prcess of id = %d\n", getClk(), id);
        setStateStr(stopped);
        fprintf(pFileLog, "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                getClk(), process->pData->id, stateStr, process->pData->arrivaltime, process->pData->runningtime,
                process->remainingTime, process->waitingTime);
    }
}
void startProcess(struct PCBNode *process)
{
    // fork or continue hasStarted
    if (!process)
    {
        return;
    }
    process->state = running;
    process->waitingTime += (getClk() - process->lastSeen);
    // fork
    if (!process->hasStarted)
    {
        process->startTime = getClk();
        int pid;
        pid = fork();
        if (pid == 0)
        {
            execl("process.out", "process.out", (char *)NULL);
        }
        process->remainingTime = process->pData->runningtime;
        process->pid = pid;
        sendRem(process->remainingTime);
        process->hasStarted = true;
        setStateStr(started);
        fprintf(pFileLog, "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                getClk(), process->pData->id, stateStr, process->pData->arrivaltime, process->pData->runningtime,
                process->remainingTime, process->waitingTime);
    }
    //conitnue
    else
    {
        setStateStr(resumed);
        fprintf(pFileLog, "At time %d process %d %s arr %d total %d remain %d wait %d\n",
                getClk(), process->pData->id, stateStr, process->pData->arrivaltime, process->pData->runningtime,
                process->remainingTime, process->waitingTime);
        kill(process->pid, SIGCONT);
    }
}
void sendRem(int remainingTime)
{
    if (remainingTime == 0)
    {
        printf("^^^^^^^^^^^^^^^Sending zero to process^^^^^^^^^^\n");
    }
    mess_rem.remaining = remainingTime;
    int send_val_rem = msgsnd(downq_id_rem, &mess_rem, sizeof(mess_rem.remaining), IPC_NOWAIT);
}
void destroyRemMsg()
{
    msgctl(downq_id_rem, IPC_RMID, (struct msqid_ds *)0);
}
