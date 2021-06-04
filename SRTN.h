#ifndef SRTN_H
#define SRTN_H
#include"headers.h"
#include"schedulerData.h"
#include<stdio.h>
void SRTN_func(struct PCBNode* head){
    printf("SRTN printing:");
    printPCB_ID(head);
}

#endif
