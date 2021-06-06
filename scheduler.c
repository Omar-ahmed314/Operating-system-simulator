//TODO processes Don't end
//TODO process PG,  scheduler, clk don't end
//TODO Round Robin needs modification
//! one process -> didn't end
//! clearing resources results in problems
#include "headers.h"
#include "schedulerData.h"
#include "SRTN.h"

void stopProcess(struct PCBNode *);
void startProcess(struct PCBNode *);
void HPFAlgorithm();
void RRAlgorithm();
void sendRem(int remainingTime);
void destroyRemMsg();
void empty() {}
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
int algNum, quantum;
int noProcesses, currentProcessesNumber;
int processesDone; //to keep track if the program is finished

bool recievedProcess = false; //A flag that determines if a new process has just been recieved

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
int downq_id_rem, rec_val_rem;
key_t key_id_rem;
struct msgbuffRem
{
    int mtype;
    int remaining;
} mess_rem;
int downq_id, upq_id, send_val, rec_val;

//@ receive handler
void recieveProcess(int signum)
{
    //? where is starttime, remaining time?
    struct msgbuff message;
    //struct processData process;
    rec_val = msgrcv(upq_id, &message, sizeof(message.processData), 0, IPC_NOWAIT);
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
    insertNode(&PCB, newProcess);
    recievedProcess = true;
    currentProcessesNumber++;
    // //printf("received msg from PG\n");
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
    //? what should the last integer be?
    // setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    initClk();
    signal(SIGINT, clearResources);
    signal(SIGUSR1, recieveProcess);

    key_t key_id;
    recievedProcess = false;
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
    int clk = getClk();
    int prevClk = clk;
    struct PCBNode *currentProcess = NULL;
    //printf("algorithm number = %d\n", algNum);
    //flush(stdout);
    int processesCounter = 0;
    int previousId = 0;
    int roundRobinCounter = quantum;
    //printf("Entering\n");
    // @@@@@@ algorithms @@@@@
    while (processesCounter < noProcesses)
    {
        if (recieveProcess)
        {
            recievedProcess = false;
        }
        if (prevClk != getClk())
        {
            // struct msgbuff message;
            // //struct processData process;
            // rec_val = msgrcv(upq_id, &message, sizeof(message.processData), 0, IPC_NOWAIT);
            // //printf("Msgrcv with no wait svalue = %d & errno = %d\n", rec_val,errno);

            //# //printf("CLK = %d\n", getClk());
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
                roundRobinCounter--;
                if (roundRobinCounter <= 0 || currentProcess->remainingTime <= 0)
                {
                    roundRobinCounter = quantum;
                    struct PCBNode *temp = currentProcess;
                    currentProcess = currentProcess->next;
                    if (temp->remainingTime <= 0)
                    {
                        deleteByID(&PCB, temp->pData->id);
                        processesCounter++;
                    }
                    if (!currentProcess)
                    {
                        currentProcess = PCB;
                    }
                }
            }
            if (currentProcess)
            {
                // @ tracing
                //printf("@@@CLK = %d: current process id = %d and PID = %d\n", getClk(), currentProcess->pData->id, currentProcess->pid);
                // //flush(stdout);
                //changed state
                if (previousId != currentProcess->pData->id)
                {
                    struct PCBNode *previousProc = searchID(PCB, previousId); // starting clk = 0 or previous deleted
                    if (previousProc)
                    {
                        stopProcess(previousProc);
                        //# //printf("Previous process ID = %d\n", previousProc->pData->id);
                    }
                    else
                    {
                        //# //printf("Previous process is already deleted\n");
                    }
                    startProcess(currentProcess);
                    // changes for next loop
                    previousId = currentProcess->pData->id;
                }
                currentProcess->remainingTime--;
                sendRem(currentProcess->remainingTime);
                if (currentProcess->remainingTime <= 0)
                {
                    //# //printf("deleted process of ID %d\n", currentProcess->pData->id);
                    if (algNum != RR) // % because it will be needed to get next one
                    {
                        deleteByID(&PCB, currentProcess->pData->id);
                        processesCounter++;
                    }
                }
                //# //printf("PCB: ");
                // printPCB(PCB);
            }
            // if (getClk() == 1)
            //     //printf("Current process is %d\n", (currentProcess ? currentProcess->pData->id : -1));

            // //printf("--- @CLK = %d-> PCB is:", getClk());
            // printPCB(PCB);
            prevClk = getClk();
        }
    }
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    clearResources(SIGINT);
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

/* ========= Round-Robin Algorithm ========= */

void RRAlgorithm()
{
    struct PCBNode *Head = PCB;
    struct PCBNode *RunningP = Head;
    processesDone = 0;
    currentProcessesNumber = 0;
    int CuTime, PrTime;
    while (processesDone < noProcesses)
    {
        RunningP = Head;
        for (int i = 0; i < currentProcessesNumber; i++)
        { // Loop on the current processes
            if (RunningP->state == ready && RunningP->hasStarted == false)
            { // First run of the process
                RunningP->hasStarted = true;
                // run process ?
            }
            else if (RunningP->state == ready && RunningP->hasStarted == true)
            { // Continue stopped process
                // Continue process
            }

            PrTime = getClk();
            CuTime = getClk();
            while ((CuTime - PrTime) < quantum)
            {
                CuTime = getClk();
                if (RunningP->remainingTime - (CuTime - PrTime) <= 0)
                {
                    RunningP->remainingTime = 0;
                    // Delete Process
                    currentProcessesNumber--;
                }
            }
            RunningP->remainingTime -= quantum;
            // Stop the Process
            RunningP = RunningP->next;
        }
    }
}
void stopProcess(struct PCBNode *process)
{
    if (process)
    {
        kill(process->pid, SIGSTOP);
        int id = process->pData->id;
        // //printf("@CLK = %d: stopping prcess of id = %d\n", getClk(), id);
    }
}
void startProcess(struct PCBNode *process)
{
    // fork or continue hasStarted
    if (!process)
    {
        return;
    }
    // fork
    if (!process->hasStarted)
    {
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
    }
    //conitnue
    else
    {
        kill(process->pid, SIGCONT);
    }
    // common
    // //printf("@CLK = %d: starting prcess of id = %d\n", getClk(), process->pData->id);
}
void sendRem(int remainingTime)
{
    mess_rem.remaining = remainingTime;
    int send_val_rem = msgsnd(downq_id_rem, &mess_rem, sizeof(mess_rem.remaining), IPC_NOWAIT);
}
void destroyRemMsg()
{
    msgctl(downq_id_rem, IPC_RMID, (struct msqid_ds *)0);
}
