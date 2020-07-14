### Assumptions

1. There can only be one job per task in the ready queue.
2. period is always less than or equal deadline.
3. All tasks are in-phase tasks.
4. All jobs of all tasks are independent of each other.
5. There is no arrival time jitter.

### Input file structure

1. phase (integer)
2. period (int)
3. criticality (int) - can be either 1 (for LO tasks) or 2(for HI jobs)
4. WCET (float) - 2 values (LO criticality WCET, HI criticality WCET)
5. relative deadline (float)


### To compile:

1. Before compiling - executable permissions should be enabled for all files.

2. Simply run the make command in the directory where Makefile is located.

### To run the program:

1. Type the command ./driver.o inputFileName.txt

### Output Files Generated:

1. schedule1.txt

	-Contains the final schedule for case 1 of execution (when all jobs run in their LO criticality wcet.


	Contains all output statistics:
	- Number of context switches
	- Number of preemptions
	- Min, max and avg response time for all tasks
	- Absolute and relative response time jitter
	- Min, max and avg waiting times


2. schedule2.txt
	-Contains the final schedule for case 2 of execution (when all jobs run in LO criticality wcet and one of them exceeds its LO WCET and the system changes to HI criticality mode.

	Contains all output statistics:
	- Number of context switches
	- Number of preemptions
	- Min, max and avg response time for all tasks
	- Absolute and relative response time jitter
	- Min, max and avg waiting times
