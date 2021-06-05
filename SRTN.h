#ifndef SRTN_H
#define SRTN_H
#include"headers.h"
#include"schedulerData.h"

void SRTNAlgorithm(struct PCBNode* head){
    printPCB_ID(head);
    printf("Count of Processes in the PCB is %d\n",countPCB(head));
}

#endif
