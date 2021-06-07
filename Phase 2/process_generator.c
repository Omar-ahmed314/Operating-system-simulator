#include "headers.h"
int max(int a, int b) { return ((a) > (b) ? (a) : (b)); }
struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int memsize;
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
    int data[4]; //data[0] = algo, data[1] = args, data[2] = no. of processes., data[3] = mem algo
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
        fscanf(prcsFile, "%d %d %d %d %d", &prcsArray[i].id, &prcsArray[i].arrivaltime, &prcsArray[i].runningtime, &prcsArray[i].priority, &prcsArray[i].memsize);
    }

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    int algoNum = atoi(argv[3]);
    int algoArgs;
    int memAlg = atoi(argv[5]);
    int pid;
    struct msgbuff2 mess;
    mess.mtype = 1;
    mess.data[0] = algoNum;
    mess.data[1] = 0;
    if (algoNum == 5)
    {
        algoArgs = atoi(argv[5]);
        mess.data[1] = algoArgs;
        memAlg = atoi(argv[7]);
    }
    mess.data[2] = count - 1;
    mess.data[3] = memAlg;
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
    //printf("total running time = %d\n", totalRunningTime);
    int i = 0;
    int prevClk = getClk();
    while (i < count - 1) //condition that processes are all done
    {
        x = getClk();
        if (x == prevClk)
        {
            continue;
        }
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
        send_val = msgsnd(upq_id, &mess_n, sizeof(mess_n.arrivedProccesses), IPC_NOWAIT);
        // printf("+++ PG: arrived processes count  = %d\n",mess_n.arrivedProccesses);
        for (int g = 0; g < arrivedProcesses; g++)
        {
            message.processData.id = prcsArray[i].id;
            message.processData.arrivaltime = prcsArray[i].arrivaltime;
            message.processData.runningtime = prcsArray[i].runningtime;
            message.processData.priority = prcsArray[i].priority;
            message.processData.memsize = prcsArray[i].memsize;
            // ? why should I wait?
            send_val = msgsnd(upq_id, &message, sizeof(message.processData), IPC_NOWAIT);
            printf("~~PG sent process of ID = %d @clk = %d\n", message.processData.id,getClk());
            i++;
        }
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
