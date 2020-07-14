#include "header.h"

int main(int argc, char **argv)
{
  if(argc != 2)
  {
    printf("Usage: ./a.out inputFile.txt\n");
    return 0;
  }

  FILE *fp = fopen(argv[1],"r");
  FILE *out1 = fopen("schedule1.txt","w");
  FILE *out2 = fopen("schedule2.txt","w");
  if(fp == NULL)
  {
    printf("File does not exist!!\n");
    return 0;
  }

  int noOfTasks;
  float U11=0.0, U22=0.0, U21=0.0;
  int hyperperiod=0;

  struct task* taskSet;

  //read all tasks from input file
  taskSet = readTasksData(fp, &noOfTasks);

  // used to calculate avg response and waiting time of all tasks
  int jobsCount[noOfTasks];

  // calculate utilizations of all tasks at all criticality levels
  calculateUtilization(taskSet, noOfTasks, &U11, &U22, &U21);

  if(U11 > 1 || U22 > 1)
  {
    printf("Utilization greater than 1. Task set can't be scheduled...\n");
    return 0;
  }

  // calculate hyperperiod of task set
  calculateHyperperiod(taskSet, noOfTasks, &hyperperiod);

  // calculate virtual deadlines of all tasks
  offlineProcessing(taskSet, noOfTasks, U11, U22, U21);

  // calculate number of jobs per tasks - used to calculate average reponse and waiting time of tasks
  calculateJobsCount(taskSet, noOfTasks, hyperperiod, jobsCount);

  // DEBUG function to check if all calculated data is correct
  printPreSchedulingData(taskSet, noOfTasks, U11, U22, U21, hyperperiod);

  //----------------------SCHEDULING -------------------------

  //CASE 1: When all jobs execute in their LO criticality without excedding their WCET
  // 1 passed as parameter to tell the above - used while calculating execution time of jobs based on criticality
  EDFSchedule(out1, taskSet, noOfTasks, hyperperiod, jobsCount, 1);

  // CASE 2: When some job will exceed its low criticality WCET and system goes to HI criticality mode
  // 2 passed as parameter to calculate execution time and recalculate virtual relative deadline when system goes to HI criticality mode
  EDFSchedule(out2, taskSet, noOfTasks, hyperperiod, jobsCount, 2);

  //-------------------SCHEDULING ENDS--------------------------

  fclose(fp);
  fclose(out1);
  fclose(out2);
  return 0;
}
