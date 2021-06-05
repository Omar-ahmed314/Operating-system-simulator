#include "headers.h"

/* Modify this file as needed*/
int remainingtime;
int downq_id, rec_val;
struct msgbuffRem
{
    int mtype;
    int remaining;
};
int main(int agrc, char *argv[])
{
    initClk();
    key_t key_id;
    key_id = 167;

    downq_id = msgget(key_id, 0666 | IPC_CREAT);
    if (downq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    struct msgbuffRem mess;
    rec_val = msgrcv(downq_id, &mess, sizeof(mess.remaining), 0, !IPC_NOWAIT);
    if (rec_val == -1)
    {
        printf("error in receiving value\n");
    }
    remainingtime = mess.remaining;
    while (remainingtime > 0)
    {
        rec_val = msgrcv(downq_id, &mess, sizeof(mess.remaining), 0, !IPC_NOWAIT);
        if (rec_val == -1)
        {
            printf("error in receiving value\n");
        }
        remainingtime = mess.remaining;
    }
    destroyClk(false);

    return 0;
}
