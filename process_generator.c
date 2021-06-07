#include "headers.h"
int max(int a, int b) { return ((a) > (b) ? (a) : (b)); }
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
    long mtype;
    struct processData processData;
};

struct msgbuff_nproc
{
    long mtype;
    int arrivedProccesses;
};

struct msgbuff2
{
    long mtype;
    int data[3]; //data[0] = algo, data[1] = args, data[2] = no. of processes.
};

//void clearResources(int);
void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    signal(SIGINT, SIG_DFL);
    msgctl(downq_id, IPC_RMID, (struct msqid_ds *)0);
    msgctl(upq_id, IPC_RMID, (struct msqid_ds *)0);
    destroyClk(true);
    // killpg(0,SIGKILL);
}

int main(int argc, char *argv[])
{

    // setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    signal(SIGINT, clearResources);
    key_t key_id;

    key_id = 99;
    downq_id = msgget(key_id, 0666 | IPC_CREAT);
    upq_id = msgget(key_id, 0666 | IPC_CREAT);
    struct msgbuff message;
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

    // TODO Initialization
    // 1. Read the input files.
    struct processData *prcsArray;

    // open the process file
    FILE *prcsFile;
    prcsFile = fopen(argv[1], "r");

    // count the process of the file
    int count = 0;
    char first_line[100];
    fgets(first_line, 100, prcsFile);
    while (!feof(prcsFile))
    {
        fgets(first_line, 100, prcsFile);
        // count number of proceses:
        // if(first_line[0]!='#')
        count++;
    }

    // initialize the process array with calloc
    prcsArray = (struct processData *)calloc(count, sizeof(struct processData));
    fclose(prcsFile);

    prcsFile = fopen(argv[1], "r");

    char second_line[100];
    fgets(second_line, 100, prcsFile);
    for (int i = 0; i < count; i++)
    {
        fscanf(prcsFile, "%d %d %d %d", &prcsArray[i].id, &prcsArray[i].arrivaltime, &prcsArray[i].runningtime, &prcsArray[i].priority);
    }

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int algoNum = atoi(argv[3]);
    int algoArgs;
    int pid;
    struct msgbuff2 mess;
    mess.mtype = 1;
    mess.data[0] = algoNum;
    mess.data[1] = 0;
    if (algoNum == 5)
    {
        algoArgs = atoi(argv[5]);
        mess.data[1] = algoArgs;
    }
    mess.data[2] = count - 1;
    // 3. Initiate and create the scheduler and clock processes.

    //Sending Data

    send_val = msgsnd(upq_id, &mess, sizeof(mess.data), !IPC_NOWAIT);
    // initialize the clock
    int sid = 0;
    message.mtype = 2;
    pid = fork();
    cid = pid;
    if (pid == 0)
    {
        execl("clk.out", "clk.out", (char *)NULL);
    }
    // initialize the scheduler
    pid = fork();
    sid = pid;
    if (pid == 0)
    {
        execl("scheduler.out", "scheduler.out", (char *)NULL);
    }
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int x = getClk();
    //printf("Current Time is %d\n", x);
    //flush(stdout);
    // TODO Generation Main Loop
    int totalRunningTime = 0;
    for (int y = 0; y < count - 1; y++)
    {
        totalRunningTime += prcsArray[y].runningtime;
    }
    totalRunningTime = max(totalRunningTime, prcsArray[count - 2].arrivaltime + prcsArray[count - 2].runningtime);
    //printf("total running time = %d\n", totalRunningTime);
    //flush(stdout);
    int i = 0;
    // ! to solve error: Current Time = 59: toogle the following two lines
    int prevClk = getClk();
    // printf("BBBBBBBefore entering PG: count = %d\n", count);
    while (i < count - 1) //condition that processes are all done
    // while (x <= totalRunningTime)
    {
        x = getClk();
        if (x == prevClk)
        {
            continue;
        }
        // printf("HHHHHHHHHi PG @CLK = %d\n", x);
        // entering a new second
        int arrivedProcesses = 0;
        prevClk = x;
        for (int g = i; g < count - 1; g++)
        {
            if (x == (prcsArray[g].arrivaltime))
            {
                arrivedProcesses++;
            }
        }
        struct msgbuff_nproc mess_n;
        mess_n.mtype = 7;
        mess_n.arrivedProccesses = arrivedProcesses;
        // printf("PG:: sending arrived  = %d\n",mess_n.arrivedProccesses);
        send_val = msgsnd(upq_id, &mess_n, sizeof(mess_n.arrivedProccesses), !IPC_NOWAIT);
        // printf("+++ PG: arrived processes count  = %d\n",mess_n.arrivedProccesses);
        for (int g = 0; g < arrivedProcesses; g++)
        {
            message.processData.id = prcsArray[i].id;
            message.processData.arrivaltime = prcsArray[i].arrivaltime;
            message.processData.runningtime = prcsArray[i].runningtime;
            message.processData.priority = prcsArray[i].priority;
            // ? why should I wait?
            send_val = msgsnd(upq_id, &message, sizeof(message.processData), IPC_NOWAIT);
            printf("~~PG sent process of ID = %d @clk = %d\n", message.processData.id,getClk());
            i++;
        }
        // @ delete the following
        // int y = i;
        // // for (; y < count - 1; y++) // I think send makes some delay
        // {
        //     if (x == (prcsArray[y].arrivaltime - 1))
        //     {
        //         ////printf("Process %d Started at time %d \n", prcsArray[i].id, x);
        //         // //flush(stdout);
        //         message.processData.id = prcsArray[y].id;
        //         message.processData.arrivaltime = prcsArray[y].arrivaltime;
        //         message.processData.runningtime = prcsArray[y].runningtime;
        //         message.processData.priority = prcsArray[y].priority;
        //         // ? why should I wait?
        //         send_val = msgsnd(upq_id, &message, sizeof(message.processData), IPC_NOWAIT);
        //         //printf("PG sending usr1 to Scheduler\n");

        //         kill(sid, SIGUSR1);
        //         i++;
        //     }
        // }
    }
    printf("Process Generator ending\n");
    while (1)
        ; // process generator ends earlier and this should be handled, now scheduler ends all processes
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    clearResources(SIGINT);
    return 0;
}
