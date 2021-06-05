#ifndef SCHDATA_H
#define SCHDATA_H
#include <stdlib.h>
#include<stdio.h>
#include"headers.h"
struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};
enum ProcessState {
    ready,
    running,
    blocked
};

enum Algorithm {
    FCFS = 1,
    SJF,
    HPF,
    SRTN,
    RR
};

struct PCBNode
{
    int state;
    bool hasStarted;
    struct processData *pData;
    int waitingTime;
    int remainingTime;
    int startTime;
    struct PCBNode *next;
};

void insertNode(struct PCBNode **head, struct PCBNode *Node)
{
    if(*head==NULL){
        *head = Node;
        return;
    }

    else{
        struct PCBNode *temp = (*head);
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = Node;
    }
    //struct PCBNode *temp = (*head);
    //(*head)= Node;
    //Node->next = temp;
    // struct PCBNode *temp = (*head)->next;
    // (*head)->next = Node;
    // Node->next = temp;
}
//send struct PCBNode** head, ID
void deleteByID(struct PCBNode **head, int ID)
{
    struct PCBNode *temp = *head;
    if (*head == 0)
    {
        return;
    }
    if ((*head)->pData->id == ID)
    {
        *head = (*head)->next;
        free(temp);
        temp = 0;
        return;
    }
    struct PCBNode *next = temp->next;
    // asking if next is id
    while (temp && next && next->pData->id != ID)
    {
        temp = next;
        next = temp->next;
    }
    if (next) //found
    {
        temp->next = next->next;
        free(next);
        return;
    }
}
struct PCBNode *searchID(struct PCBNode *head, int ID)
{
    struct PCBNode *temp = head;
    while (temp)
    {
        if (temp->pData->id == ID)
        {
            return temp;
        }
        temp = temp->next;
    }
    return 0;
}
void destroyPCB(struct PCBNode *head)
{
    while (head)
    {
        struct PCBNode *temp = head->next;
        free(head);
        head = temp;
    }
}
void printPCB_ID(struct PCBNode*head){
    while(head && head->pData){
        printf("%d ",head->pData->id);
        head = head->next;
    }
    printf("\n");
}
int countPCB(struct PCBNode*head){
    int count = 0;
    while(head){
        count++;
        head= head->next;
    }
    return count;
}
// struct PCBNode* createPCBNode(struct processData*process,int state,bool hasStarted,struct PCBNode*next,)
struct PCBNode* findHPF(struct PCBNode*head){
    struct PCBNode* highestPrioriy = head;
    while(head){
        if(head->pData->priority>highestPrioriy->pData->priority){
            highestPrioriy = head;
        }
        head = head->next;
    }
    return highestPrioriy;
}
struct PCBNode* findSRTN(struct PCBNode*head){
    struct PCBNode* shortestRemaining = head;
    while(head){
        if(head->remainingTime < shortestRemaining->remainingTime){
            shortestRemaining = head;
        }
        head = head->next;
    }
    return shortestRemaining;
}
struct PCBNode* findFCFS(struct PCBNode*head){
    struct PCBNode* shortestRemaining = head;
    while(head){
        if(head->pData->arrivaltime < shortestRemaining->pData->arrivaltime){
            shortestRemaining = head;
        }
        head = head->next;
    }
    return shortestRemaining;
}
struct PCBNode* findSJF(struct PCBNode*head){
    struct PCBNode* shortestRemaining = head;
    while(head){
        if(head->pData->runningtime < shortestRemaining->pData->runningtime){
            shortestRemaining = head;
        }
        head = head->next;
    }
    return shortestRemaining;
}
#endif
