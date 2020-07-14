#include "header.h"

//---------------------Functions for Ready Queue----------------------
Node* newNode(int data, float priority)
{
   Node* temp = (Node*)malloc(sizeof(Node));
   temp->data = data;
   temp->priority = priority;
   temp->next = NULL;
   return temp;
}

int peek(Node** head)
{
   Node* Curr = (*head)->next;
   if(Curr==NULL)
   {
     return -1;
   }
   return Curr->data;
}

void pop(Node** head)
{
   Node* Curr = (*head)->next;
   if((Curr)==NULL)
      return;
   (*head)->next = (Curr)->next;
   free(Curr);
}


void push(Node** head, int data, float priority)
{
   Node* start = (*head);
   Node* New = newNode(data, priority);
   if ((*head)->priority > priority)
   {
      New->next = *head;
      (*head) = New;
   }
   else
   {
      while (start->next != NULL && start->next->priority <= priority)
      {
         start = start->next;
      }
      New->next = start->next;
      start->next = New;
   }
}

int isEmpty(Node** head)
{
  Node* Curr = (*head)->next;
  if(Curr == NULL)
    return 0;
  else
    return 1;
}

//----------------------End of ready queue functions---------------------
// read input file and store in task struct
struct task* readTasksData(FILE *fp, int *noOfTasks)
{
  fscanf(fp,"%d",noOfTasks);

  struct task* taskSet = malloc(*noOfTasks * sizeof(struct task));
  for(int i=0;i<*noOfTasks;i++)
  {
    fscanf(fp,"%d %d %d %f %f %f",&taskSet[i].phase, &taskSet[i].period, &taskSet[i].criticality, &taskSet[i].wcet[0], &taskSet[i].wcet[1], &taskSet[i].deadline);
    taskSet[i].taskID = i+1;
    taskSet[i].virtualDeadline = 0.0;
  }
  return taskSet;
}

// calculate utilizations of all tasks
/*
1. U11 - utilization of all low criticality tasks wrt. their LO criticality WCET
2. U22 - utilization of all high criticality tasks wrt. their HI criticality WCET
3. U21 - utilization of all high criticality tasks wrt. their LO criticality WCET
*/
void calculateUtilization(struct task* taskSet, int noOfTasks, float *U11, float *U22, float *U21)
{
  for(int i=0;i<noOfTasks;i++)
  {
    if(taskSet[i].criticality==1)
    {
      *U11 += (taskSet[i].wcet[0]/(float)taskSet[i].period);
    }
    else
    {
      *U22 += (taskSet[i].wcet[1]/(float)taskSet[i].period);
      *U21 += (taskSet[i].wcet[0]/(float)taskSet[i].period);
    }
  }
}

// helper function to calculate hyperperiod
int gcd(int a, int b)
{
  if(b==0)
    return a;
  return gcd(b, a%b);
}

// calculates hyperperiod for the task set
void calculateHyperperiod(struct task* taskSet, int noOfTasks, int *hyperperiod)
{
  *hyperperiod = taskSet[0].period;
  for(int i=1;i<noOfTasks;i++)
  {
    *hyperperiod = (taskSet[i].period * *hyperperiod) / (gcd(taskSet[i].period, *hyperperiod));
  }
}

// calculates virtual dealines of all tasks
void offlineProcessing(struct task* taskSet, int noOfTasks, float U11, float U22, float U21)
{
  float x = U21/(1-U11);
  float sum = x*U11+U22;

  if(sum <= 1)
  {
    for(int i=0;i<noOfTasks;i++)
    {
      if(taskSet[i].criticality == 2)
        taskSet[i].virtualDeadline = x * taskSet[i].deadline;
      else
        taskSet[i].virtualDeadline = taskSet[i].deadline;
    }
  }
}

// DEBUG function to print all calculated data on the terminal
void printPreSchedulingData(struct task* taskSet, int noOfTasks, float U11, float U22, float U21, int hyperperiod)
{
  printf("\n\n----------PRESCHEDULING DATA---------------\n\n");

  printf("1. NUMBER OF TASKS = %d\n\n",noOfTasks);

  printf("2. TASK SET\n");
  printf("TaskID : Phase  Period  Criticality  WCET[0]  WCET[1]  Deadline  VirtualD\n");
  for(int i=0;i<noOfTasks;i++)
  {
    printf("T%d : %d %d %d %.2f %.2f %.2f %.2f\n",taskSet[i].taskID, taskSet[i].phase, taskSet[i].period, taskSet[i].criticality, taskSet[i].wcet[0], taskSet[i].wcet[1], taskSet[i].deadline, taskSet[i].virtualDeadline);
  }
  printf("\n");

  printf("3. UTILIZATIONS\n");
  printf("U11 = %.2f, U22 = %.2f, U21 = %.2f\n\n",U11, U22, U21);

  printf("4. HYPERPERIOD = %d\n\n",hyperperiod);
}

// function to calculate random value betwwen min and max - used while calculating actual execution time
float float_rand( float min, float max )
{
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );
}

// DEBUG function - prints data of all jobs on terminal
void printJobList(struct job* jobList, int noOfTasks)
{
  printf("Job (Task) : ArrivalTime   WCET[0] WCET[1] ExecutionTime   ActualDeadline    VirtualDeadline    RemainingTime    NextJobArrivalTime\n");
  for(int i=0;i<noOfTasks;i++)
  {
    printf("J%d (T%d) : %d %.2f \t\t %.2f \t\t %.2f \t\t %.2f \t\t %.2f \t\t %.2f \t\t%d\n", jobList[i].jobID, jobList[i].taskID, jobList[i].arrivalTime, jobList[i].wcet[0], jobList[i].wcet[1], jobList[i].executionTime, jobList[i].actualDeadline, jobList[i].virtualDeadline, jobList[i].remainingTime, jobList[i].nextJobArrivalTime);
  }
}

// DEBUG function - prints all jobs currently in ready queue along with remaining time for execution completion on terminal
void printQueue(Node** head, struct job* jobList){
  Node* temp = (*head)->next;
  printf("\nStart of Queue:-\n");
  while(temp!=NULL){
    printf("J%d (%.2f)", jobList[temp->data].jobID, jobList[temp->data].remainingTime);
    temp = temp->next;
  }
  printf("\nEnd of Queue ------\n");
}

// Deletes all low criticality jobs from ready queue when criticality level changes
void delete(Node** head, struct job* jobList)
{
  Node* temp = (*head)->next;
  Node* prev = *head;
  if((temp)==NULL)
    return;
  while(temp != NULL)
  {
    if(jobList[temp->data].criticality == 1)
    {
      prev->next = temp->next;
      free(temp);
      temp = prev->next;
    }
    else
    {
      prev = temp;
      temp = temp->next;
    }
  }
}


float min(float a, float b)
{
  if(a < b)
    return a;
  return b;
}

// inserts job in jobList struct when time is initially 0 - executed only at the start
/*
Variables:
mode - to check weather scheduler is run for case 1(when all jobs run in LO criticality) or case 2(when some job may exceed its LO criticality WCET leading to criticality level change)
criticalityChange - used in case 2 to denote that system has changed its criticality level


1. Execution time calculation
  - if system executes in case 1 or when system executes in case 2 but system criticality has not changed, then use WCET[0] for execution time calculations
  - else use WCET[1] for execution time calculation

2. Virtual Deadline calculation
  - when no criticalityChange, then virtual actual deadline calculated from virtual deadline of tasks
  - else, virtual relative deadline calculated from relative deadline of tasks
*/
void initialJobInsert(struct task task1, struct job* queue, int cnt, int i, short mode, short criticalityChange)
{
  queue[i].taskID = task1.taskID;
  queue[i].jobID = cnt;
  queue[i].arrivalTime = task1.phase;
  queue[i].period = task1.period;
  queue[i].criticality = task1.criticality;
  queue[i].wcet[0] = task1.wcet[0];
  queue[i].wcet[1] = task1.wcet[1];

  if(mode==1 || (mode==2 && queue[i].criticality==1))
    queue[i].executionTime = task1.wcet[0] * float_rand(0.5, 1.0);
  else
    queue[i].executionTime = task1.wcet[1] * float_rand(0.3, 1.0);

  queue[i].actualDeadline = queue[i].arrivalTime + task1.deadline;

  if(criticalityChange == 0)
    queue[i].virtualDeadline = queue[i].arrivalTime + task1.virtualDeadline;
  else
    queue[i].virtualDeadline = queue[i].arrivalTime + task1.deadline;

  queue[i].remainingTime = queue[i].executionTime;
  queue[i].executionStatus = 0;
  queue[i].nextJobArrivalTime = queue[i].arrivalTime + queue[i].period;
  queue[i].prevEndTime = 0;
}

//updates jobs parameters when current job of some task has finished executing and next job of same task arrives
/*
Variables:
mode - to check weather scheduler is run for case 1(when all jobs run in LO criticality) or case 2(when some job may exceed its LO criticality WCET leading to criticality level change)
criticalityChange - used in case 2 to denote that system has changed its criticality level


1. Execution time calculation
  - if system executes in case 1 or when system executes in case 2 but system criticality has not changed, then use WCET[0] for execution time calculations
  - else use WCET[1] for execution time calculation

2. Virtual Deadline calculation
  - when no criticalityChange, then virtual actual deadline calculated from virtual deadline of tasks
  - else, virtual relative deadline calculated from relative deadline of tasks

3. Also, insert updated job in ready queue
*/
void updateJobs(struct task* taskSet, struct job* jobList, int noOfTasks, int time, int *cnt, Node* queue, short mode, short criticalityChange)
{
  for(int i=0;i<noOfTasks;i++)
  {
    if(jobList[i].nextJobArrivalTime == time)
    {
      jobList[i].jobID = *cnt;
      jobList[i].arrivalTime = time;

      if(mode==1 || (mode==2 && jobList[i].criticality==1))
        jobList[i].executionTime = taskSet[jobList[i].taskID -1].wcet[0] * float_rand(0.5, 1.0);
      else
        jobList[i].executionTime = taskSet[jobList[i].taskID -1].wcet[1] * float_rand(0.3, 1.0);

      jobList[i].actualDeadline = jobList[i].arrivalTime + taskSet[jobList[i].taskID -1].deadline;

      if(criticalityChange == 0)
        jobList[i].virtualDeadline = jobList[i].arrivalTime + taskSet[jobList[i].taskID -1].virtualDeadline;
      else
        jobList[i].virtualDeadline = jobList[i].arrivalTime + taskSet[jobList[i].taskID -1].deadline;

      jobList[i].remainingTime = jobList[i].executionTime;
      jobList[i].executionStatus = 0;
      jobList[i].nextJobArrivalTime = jobList[i].arrivalTime + jobList[i].period;
      jobList[i].prevEndTime = jobList[i].endTime;

      // push jobs in queue depending on mode and criticalityChange values
      if(mode==1 || (mode==2 && criticalityChange == 0))
        push(&queue, i, jobList[i].virtualDeadline);

      else if(mode==2 && criticalityChange == 1)
      {
        if(jobList[i].criticality == 2)
          push(&queue, i, jobList[i].virtualDeadline);
      }
      (*cnt)++;
    }
  }
}

// finds the next minimum arrival time from all the jobs in the jobList
/*
1. if system runs in CASE 1 or in CASE 2 with no criticalityChange, then update normally.
2. if system runs in CASE 2 and criticalityChange has occured, then only use HI criticality jobs to update the variables
*/
int findNextMinJobArrivalTime(struct job* jobList, int noOfTasks, short mode, short criticalityChange)
{
  int min = INT_MAX;
  for(int i=0;i<noOfTasks;i++)
  {
    if(mode==2 && criticalityChange==1 && jobList[i].criticality==2)
    {
      if(jobList[i].nextJobArrivalTime < min)
        min = jobList[i].nextJobArrivalTime;
    }
    else if(mode==1 || (mode==2 && criticalityChange==0))
    {
      if(jobList[i].nextJobArrivalTime < min)
        min = jobList[i].nextJobArrivalTime;
    }
  }
  return min;
}

// prints all parameters into FILE
void printStatistics(FILE *out, int noOfTasks, float maxResponseTime[], float minResponseTime[],
float avgResponseTime[], float maxEndTime[], float minWaitingTime[],
float maxWaitingTime[], float avgWaitingTime[], int contextSwitch,
int preemptionCount, int jobsCount[])
{
    fprintf(out,"\n\n----------------------------OUTPUT STATISTICS---------------------------\n\n");
    fprintf(out,"1. TOTAL CONTEXT SWITCHES = %d\n",contextSwitch);
    fprintf(out,"2. TOTAL PREEMPTION COUNT = %d\n",preemptionCount);

    fprintf(out,"\n3. RESPONSE TIMES\n");
    fprintf(out,"TASK ID\tMAX RESPONSE TIME\tMIN RESPONSE TIME\tAVG RESPONSE TIME\n");
    for(int i=0;i<noOfTasks;i++)
    {
      if(minResponseTime[i] == FLT_MAX)
      {
        continue;
      }
      fprintf(out,"%d \t %.2f \t\t\t %.2f \t\t\t %.2f \n",i+1,maxResponseTime[i],minResponseTime[i], avgResponseTime[i]/jobsCount[i]);
    }

    fprintf(out,"\n4. ABSOLUTE AND RELATIVE RESPONSE TIME JITTER\n");
    fprintf(out,"TASK ID\tABS. JITTER\tREL. JITTER\n");
    for(int i=0;i<noOfTasks;i++)
    {
      if(minResponseTime[i] == FLT_MAX)
      {
        continue;
      }
      fprintf(out,"%d \t %.2f \t\t %.2f \n",i+1,maxResponseTime[i]-minResponseTime[i], maxEndTime[i]);
    }

    fprintf(out,"\n5. WAITING TIMES\n");
    fprintf(out,"TASK ID\tMAX WAITING TIME\tMIN WAITING TIME\tAVG WAITING TIME\n");
    for(int i=0;i<noOfTasks;i++)
    {
      if(minWaitingTime[i] == FLT_MAX)
      {
        continue;
      }
      fprintf(out,"%d \t %.2f \t\t\t %.2f \t\t\t %.2f \n",i+1,maxWaitingTime[i],minWaitingTime[i], avgWaitingTime[i]/jobsCount[i]);
    }
}

// main scheduler functions
/*
mode parameter used to denote weather system running in CASE 1 or CASE 2
CASE 1 (mode = 1): System runs completly in LO criticality
CASE 2 (mode = 2): System runs initially in LO critcality then changes to HI criticality
*/
void EDFSchedule(FILE *out, struct task* taskSet, int noOfTasks, int hyperperiod, int jobsCount[], short mode)
{
  struct job* jobList = malloc(noOfTasks * sizeof(struct job)); // to store all job parameters of jobs currently in ready queue
  Node* queue = newNode(-1,0); // ready queue

  float old_t=0.0, t=0.0;
  float decisionPoint;
  int cnt = 1;
  int curr;
  short criticalityChange = 0; // 0 - no criticality change, 1 - criticality change occured
  int contextSwitch = 0, preemptionCount = 0;

  // statistics calculation variables declaration and initialization
  float maxResponseTime[noOfTasks];
  float minResponseTime[noOfTasks];
  float avgResponseTime[noOfTasks];
  float initialEndTime[noOfTasks];
  float maxEndTime[noOfTasks];
  float minWaitingTime[noOfTasks];
  float maxWaitingTime[noOfTasks];
  float avgWaitingTime[noOfTasks];

  for(int i=0;i<noOfTasks;i++)
  {
    maxResponseTime[i] = 0;
    minResponseTime[i] = FLT_MAX;
    avgResponseTime[i] = 0;
    initialEndTime[i] = 0;
    maxEndTime[i] = 0;
    maxWaitingTime[i] = 0;
    minWaitingTime[i] = FLT_MAX;
    avgWaitingTime[i] = 0;
  }

  // initial insertion of jobs in queue and jobList when t=0.0
  for(int i=0;i<noOfTasks;i++)
  {
    if(taskSet[i].phase == 0.0)
    {
      initialJobInsert(taskSet[i], jobList, cnt, i, mode, criticalityChange);
      push(&queue, i, jobList[i].virtualDeadline);
      cnt++;
    }
  }

  int nextMinJobArrivalTime = findNextMinJobArrivalTime(jobList, noOfTasks, mode, criticalityChange);

  printf("\n");

  printJobList(jobList, noOfTasks);


  printf("\n\n-------------START OF SCHEDULING--------------\n\n");
  fprintf(out,"---------------START OF SCHEDULING-----------------\n\n");

  while(1)
  {
    printQueue(&queue, jobList);
    curr = peek(&queue); // get job from start of ready queue

    // criticality level change occured
    if(jobList[curr].executionTime > jobList[curr].wcet[0] && jobList[curr].executionTime <= jobList[curr].wcet[1])
    {
      criticalityChange = 1;
      delete(&queue, jobList);
      printQueue(&queue, jobList);
      nextMinJobArrivalTime = findNextMinJobArrivalTime(jobList, noOfTasks, mode, criticalityChange);
      curr = peek(&queue);
    }

    // calculates the next decision point based on minimum of virtual deadline, execution time and next job arrival time of job at the head of ready queue
    decisionPoint = min(jobList[curr].nextJobArrivalTime, min(jobList[curr].virtualDeadline, t+jobList[curr].remainingTime));

    // job executes completly without any new job arriving in between - no preemption occurs
    if(decisionPoint == t+jobList[curr].remainingTime && decisionPoint <= nextMinJobArrivalTime)
    {
      if(jobList[curr].executionTime == jobList[curr].remainingTime)
        jobList[curr].startTime = t;

      t += jobList[curr].remainingTime;
      fprintf(out, "%.2f - %.2f  :  J%d (T%d)\n", old_t, t, jobList[curr].jobID, jobList[curr].taskID);
      jobList[curr].remainingTime = 0;
      jobList[curr].endTime = t;
      jobList[curr].executionStatus = 1;

      // ----------------statistics calculations ----------------
      // max, min, avg response time of task
      if(maxResponseTime[jobList[curr].taskID -1] < (jobList[curr].endTime - jobList[curr].arrivalTime))
        maxResponseTime[jobList[curr].taskID -1]  = jobList[curr].endTime - jobList[curr].arrivalTime;

      if(minResponseTime[jobList[curr].taskID -1] > (jobList[curr].endTime - jobList[curr].arrivalTime))
        minResponseTime[jobList[curr].taskID -1]  = jobList[curr].endTime - jobList[curr].arrivalTime;

      avgResponseTime[jobList[curr].taskID -1] += (jobList[curr].endTime - jobList[curr].arrivalTime);

      // relative response time jitter calculation
      if(jobList[curr].prevEndTime = 0) // first job of that task
      {
        initialEndTime[jobList[curr].taskID -1] = jobList[curr].prevEndTime;
      }
      else
      {
        if(maxEndTime[jobList[curr].taskID -1] < jobList[curr].endTime - jobList[curr].prevEndTime)
          maxEndTime[jobList[curr].taskID -1] = jobList[curr].endTime - jobList[curr].prevEndTime;
      }
      if(jobList[curr].nextJobArrivalTime >= hyperperiod) // last job of that task
      {
        if(maxEndTime[jobList[curr].taskID -1] < jobList[curr].endTime - initialEndTime[jobList[curr].taskID -1])
          maxEndTime[jobList[curr].taskID -1] = jobList[curr].endTime - initialEndTime[jobList[curr].taskID -1];
      }

      // max, min, avg waiting time
      int totalWaitingTime = jobList[curr].endTime - (jobList[curr].arrivalTime + jobList[curr].executionTime);
      if(maxWaitingTime[jobList[curr].taskID -1] < totalWaitingTime)
        maxWaitingTime[jobList[curr].taskID -1] = totalWaitingTime;

      if(minWaitingTime[jobList[curr].taskID -1] > totalWaitingTime)
        minWaitingTime[jobList[curr].taskID -1] = totalWaitingTime;

      avgWaitingTime[jobList[curr].taskID -1] += totalWaitingTime;

      //--------------statistics calculations end-----------------------

      old_t = t;
      ++contextSwitch;
      pop(&queue);
    }

    // new job arrived before current job completion - premeption may or may not occur
    else if(decisionPoint == t+jobList[curr].remainingTime && decisionPoint > nextMinJobArrivalTime)
    {
      if(jobList[curr].executionTime == jobList[curr].remainingTime)
        jobList[curr].startTime = t;

      t = nextMinJobArrivalTime;
      fprintf(out, "%.2f - %.2f  :  J%d (T%d)\n",old_t, t, jobList[curr].jobID, jobList[curr].taskID);
      jobList[curr].remainingTime -= (t-old_t);
      ++preemptionCount;
      old_t = t;
      ++contextSwitch;

      updateJobs(taskSet, jobList, noOfTasks, nextMinJobArrivalTime, &cnt, queue, mode, criticalityChange);
      nextMinJobArrivalTime = findNextMinJobArrivalTime(jobList, noOfTasks, mode, criticalityChange);
    }

    // no job in ready queue and next job arrival is later - IDLE time
    if(isEmpty(&queue)==0 && nextMinJobArrivalTime < hyperperiod && t < nextMinJobArrivalTime)
    {
      t = nextMinJobArrivalTime;
      fprintf(out, "%.2f - %.2f  :  IDLE\n",old_t, t);
      old_t = t;
      updateJobs(taskSet, jobList, noOfTasks, nextMinJobArrivalTime, &cnt, queue, mode, criticalityChange);
      nextMinJobArrivalTime = findNextMinJobArrivalTime(jobList, noOfTasks, mode, criticalityChange);
    }

    // break condition at the end of schedule
    else if(isEmpty(&queue)==0 && nextMinJobArrivalTime >= hyperperiod)
    {
      printf("\n\n-------------END OF SCHEDULING--------------\n\n");
      printStatistics(out, noOfTasks, maxResponseTime, minResponseTime,
      avgResponseTime, maxEndTime, minWaitingTime, maxWaitingTime,
      avgWaitingTime, contextSwitch, preemptionCount, jobsCount);
      break;
    }
  }
}

// to help in calculating average waiting and response time
void calculateJobsCount(struct task* taskSet, int noOfTasks, int hyperperiod, int jobsCount[])
{
  //int count=0;
  for(int i=0;i<noOfTasks;i++)
  {
    jobsCount[i] = (hyperperiod / taskSet[i].period);
    //count += (hyperperiod / taskSet[i].period);
  }
  //printf("TOTAL JOBS = %d\n",count);
}
