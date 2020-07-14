#include<stdlib.h>
#include<stdio.h>
#include<limits.h>
#include<float.h>

typedef struct node {
   int data;            // Job ID index to get data from Job structure
   float priority;      // Priority value as -1 * deadline(J)
   struct node* next;   // Pointer to next node in queue
} Node;

Node* newNode(int d, float p);
int peek(Node** head);
void pop(Node** head);
void push(Node** head, int d, float p);
int isEmpty(Node** head);

struct task {
  int taskID;
  int phase;
  int period;
  int criticality;
  float wcet[2];
  float deadline;
  float virtualDeadline;
};

struct job
{
  int taskID;
  int jobID;
  int arrivalTime;
  int period;
  int criticality;
  float wcet[2];
  float executionTime;
  float actualDeadline;
  float virtualDeadline;
  float remainingTime;
  short executionStatus; // 0-not executing 1-finish execution
  int nextJobArrivalTime;
  float startTime;
  float endTime;
  float prevEndTime;
};

struct task* readTasksData(FILE *fp, int *noOfTasks);
void calculateUtilization(struct task* taskSet, int noOfTasks, float *U11, float *U22, float *U21);
void calculateHyperperiod(struct task* taskSet, int noOfTasks, int *hyperperiod);
void offlineProcessing(struct task* taskSet, int noOfTasks, float U11, float U22, float U21);
void printPreSchedulingData(struct task* taskSet, int noOfTasks, float U11, float U22, float U21, int hyperperiod);
void EDFSchedule(FILE *out, struct task* taskSet, int noOfTasks, int hyperperiod, int jobsCount[], short mode);
void calculateJobsCount(struct task* taskSet, int noOfTasks, int hyperperiod, int jobsCount[]);
