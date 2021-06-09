#ifndef MEMORYDATA_H
#define MEMORYDATA_H

#include "headers.h"
#include "schedulerData.h"

char memory[1024];
int memAlgNum;
int totalAllocatedMemory;
int lastAllocatedPosition;

enum MemAlgorithm
{
    FF = 1,
    NF,
    BF,
    BSA
};

void writeToMemory(int start, int finish)
{
    if (finish < start)
    {
        for (int i=0; i<=finish; i++)
        {
            memory[i] = '1';
        }
        for (int i=start; i<1024; i++)
        {
            memory[i] = '1';
        }
        totalAllocatedMemory+=(finish+1) + (1024-start);
        return;
    }
    for (int i=start; i<= finish; i++)
    {
        memory[i] = '1';
    }
    totalAllocatedMemory += finish+1-start;
    
}

bool FFAlgorithm(struct PCBNode* process)
{
    int start=0;
    int finish=0;
    int size = process->pData->memsize;
    for (int i=0; i<1024; i++)
    {
        if (memory[i] == '0')
        {
            finish=i;
        }
        else
        {
            start=i+1;
            finish=i+1;
        }
        if (finish+1-start == size)
        {
            writeToMemory(start, finish);
            process->memoryStart=start;
            process->memoryFinish=finish;
            return true;
        }
    }
    return false;    
}

bool NFAlgorithm(struct PCBNode* process)
{
    int start=lastAllocatedPosition+1;
    int finish=lastAllocatedPosition+1;
    int size = process->pData->memsize;
    int i=lastAllocatedPosition+1;
    int k=0;
    while(k<1024)
    {
        if (memory[i] == '0')
        {
            finish=i;
        }
        else
        {
            start=(i+1)%1024;
            finish=(i+1)%1024;
        }
        if (finish > start && finish+1-start == size || finish < start && (1024-start+finish+1) == size)
        {
            writeToMemory(start, finish);
            process->memoryStart=start;
            process->memoryFinish=finish;
            lastAllocatedPosition=finish;
            return true;
        }
        i=(i+1)%1024;
        k++;
    }
    return false;    
}

bool BFAlgorithm(struct PCBNode* process)
{
    int start=0;
    int finish=0;
    int bestStart=0;
    int bestEnd=1024;
    int size = process->pData->memsize;
    bool foundPartition=false;
    for (int i=0; i<1024; i++)
    {
        if (memory[i] == '0')
        {
            finish=i;
        }
        else
        {
            start=i+1;
            finish=i+1;
        }
        if ((i==1023 || memory[i+1] == '1') && finish+1-start >= size && finish-start < bestEnd-bestStart)
        {
            printf("FOUND PARTITION FROM %d to %d\n", start,finish);
            bestStart = start;
            bestEnd = finish;
            foundPartition=true;
        }
    }
    
    if (!foundPartition || bestEnd+1-bestStart < size)
        return false;    
    writeToMemory(bestStart, bestStart+size-1);
    process->memoryStart=bestStart;
    process->memoryFinish=bestStart+size-1;
    return true;
}

bool BSAAlgorithm(struct PCBNode* process)
{
    int size=process->pData->memsize;
    int powers[] = {16,32,64,128,256,512,1024};
    int poweredSize=0;
    for (int i=0; i<7; i++)
    {
        if (size < powers[i])
        {
            poweredSize = powers[i];
            break;
        }
    }
    for (int i=0; i<1024; i=i+poweredSize)
    {
        bool fragmentFound=true;
        for (int j=i; j<i+poweredSize; j++)
        {
            if (memory[j] == '1')
            {
                fragmentFound=false;
                break;
            }
        }
        if (fragmentFound)
        {
            writeToMemory(i, i+poweredSize-1);
            process->memoryStart=i;
            process->memoryFinish=i+poweredSize-1;
            return true;
        }
    }
    return false;

}

void initializeMemory()
{
    for (int i=0; i<1024; i++)
    {
        memory[i] = '0';
    }
    totalAllocatedMemory=0;
    lastAllocatedPosition=-1;
}

void freeMemory(struct PCBNode* process)
{
    int start=process->memoryStart;
    int finish=process->memoryFinish;
    if (finish < start)
    {
        for (int i=0; i<=finish; i++)
        {
            memory[i] = '0';
        }
        for (int i=start; i<1024; i++)
        {
            memory[i] = '0';
        }
        totalAllocatedMemory-=(finish+1) + (1024-start);
        return;
        
    }
    for (int i=start; i<= finish; i++)
    {
        memory[i] = '0';
    }
    totalAllocatedMemory-=finish+1-start;
}

bool allocateMemory(struct PCBNode* process)
{
    
    if (process->pData->memsize > 1024-totalAllocatedMemory)
        return false;
    
    if (memAlgNum == FF)
    {
        return FFAlgorithm(process);
    }
    else if (memAlgNum == NF)
    {
        return NFAlgorithm(process);
    }
    else if (memAlgNum == BF)
    {
        return BFAlgorithm(process);
    }
    else if (memAlgNum == BSA)
    {
        return BSAAlgorithm(process);
    }

}




#endif
