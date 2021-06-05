#ifndef SCHDATA_H
#define SCHDATA_H
#include <stdlib.h>
#include <stdio.h>
#include "headers.h"
struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;
    int id;
};
enum ProcessState
{
    ready,
    running,
    blocked
};

enum Algorithm
{
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
void insertNode(struct PCBNode **head, struct PCBNode *Node) // like a queue
{
    if (*head == NULL)
    {
        *head = Node;
        return;
    }
    struct PCBNode *temp = *head;
    while (temp->next)
    {
        temp = temp->next;
    }
    temp->next = Node;
}
void insertNodeReversed(struct PCBNode **head, struct PCBNode *Node) // like a stack , we may not use it
{
    if (*head == NULL)
    {
        *head = Node;
        return;
    }

    struct PCBNode *temp = (*head);
    (*head) = Node;
    Node->next = temp;
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
void printPCB(struct PCBNode *head)
{
    while (head && head->pData)
    {
        printf("%d ", head->pData->id);
        head = head->next;
    }
    printf("\n");
}
int countPCB(struct PCBNode *head)
{
    int count = 0;
    while (head)
    {
        count++;
        head = head->next;
    }
    return count;
}

static bool FCFScondition(struct PCBNode *arrow, struct PCBNode *firstComing)
{
    return arrow->pData->arrivaltime < firstComing->pData->arrivaltime; // you found someone (arrow) arrived before you
}
static bool SJFcondition(struct PCBNode *arrow, struct PCBNode *shortestJob)
{
    return arrow->pData->runningtime < shortestJob->pData->runningtime; //you found someone (arrow) funs less than you
}
static bool HPFcondition(struct PCBNode *arrow, struct PCBNode *highestPrioriy)
{
    return arrow->pData->priority < highestPrioriy->pData->priority; // you found someone (arrow) of priority less than you
}
static bool SRTNcondition(struct PCBNode *arrow, struct PCBNode *shortestRemaining)
{
    return arrow->remainingTime < shortestRemaining->remainingTime; // you found someone (arrow) of remaining time less than you
}

static bool condition(int algoNum, struct PCBNode *arrow, struct PCBNode *target)
{
    switch (algoNum)
    {
    case FCFS:
        return FCFScondition(arrow, target);
        break;
    case SJF:
        return SJFcondition(arrow, target);
        break;
    case HPF:
        return HPFcondition(arrow, target);
        break;
    case SRTN:
        return SRTNcondition(arrow, target);
        break;
    default:
        break;
    }
    printf("Hi, I don't support this algorithm. bye\n");
}
struct PCBNode *findTarget(int algoNum, struct PCBNode *head)
{
    struct PCBNode *target = head;
    while (head)
    {
        if (condition(algoNum, head, target))
        {
            target = head;
        }
        head = head->next;
    }
    return target;
}
#endif
