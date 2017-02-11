#include <iostream>
#include <vector>
#include <queue>
#include "Job.h"
#include "cpuScheduler.h"
#include "MemoryManager.h"
using namespace std;

vector<Job> jobTable;
long mem[100];
long currentJobRunning;
long currentJobIo;
queue<Job> readyQueue;
queue<Job> ioQueue;
cpuScheduler scheduler;
MemoryManager memory;
long drumOrCore;
long ioQueueJobIndex;

void startup ();
void Crint (long &a, long p[]);
void Svc (long &a, long p[]);
void Tro(long &a, long p[]);
void Dskint(long &a, long p[]);
long findByNumber (long jobNumber);
void addToJobTable (Job newJob);
void bookKeep (long time);
void swapper (long jobNum);
void runJob (long &a, long p[], Job toRun);
void terminateJob (long jobNumber);
void refreshJobTable ();
bool isOnIoQueue (long jobNumber);
void siodisk (long JobNum);
void siodrum (long JobNum, long JobSize, long StartCoreAddr, long TransferDir);
void remJobFromJobTable (long position);
void ioQueueJobNext();
void requestIo (long jobNumber);
void Drmint (long &a, long p[]);

//Called as soon as SOS starts, initializes currentJobRunning, currentJobIo, and drumOrCore to -1 (dummy value)
//to indicate that no job is running yet, no job is currently doing IO, and no job is being swapped in/out of memory
void startup () {
	currentJobRunning = -1;
	currentJobIo = -1;
	drumOrCore = -1;
}

//Called when a new job has entered the system - the job table is cleaned up, an instance of the Job class is created and
//added to the job table with relevant job information, the swapper is called to attempt to move job into memory, and lastly,
//dispatcher is called to run a job chosen by the STS
void Crint (long &a, long p[]) {
	bookKeep (p[5]);
	Job newJob (p);
	addToJobTable (newJob);
	//job can only go on ready queue if it is in memory
	swapper (newJob.getJobNumber());
	runJob (a, p, scheduler.scheduleCpu (readyQueue));
}

//Job needs service - firstly, the index of job in the job table is retrieved. Depending on the value of a, Svc behaves differently
//If a=5, job is requesting termination (memory is deallocated and isTerminated is set to true for specified job
//If a=6, job is requesting IO (job is placed on IO queue if queue is not empty, otherwise siodisk is called. doingIo is set to
//true in either case)
//If a=7, job is requesting to be blocked until pending IO is completed (checks if job is doing IO or is on IO queue and blocks if so)
//Lastly, dispatcher runs a new job with help of scheduler
void Svc (long &a, long p[]) {
	bookKeep(p[5]);
	long currentJobIndex = findByNumber(p[1]);
	if (currentJobIndex == -1) {
		cout << "Job is not in jobTable" << endl;
		return;
	}
	switch (a) {
		case '5':
			//Take job off ready queue, take job out of job table, deallocate memory
			terminateJob (p[1]);
			break;
		case '6':
			//Take job off ready queue, add to IO Queue, call appropriate SOS function
			requestIo (p[1]);
			break;
		case '7':
			//If job is currently doing IO or is on IO Queue, block
			if (jobTable.at(currentJobIndex).getDoingIo() || isOnIoQueue(p[1]))
				jobTable.at(currentJobIndex).setIsBlocked (true);
			break;
		}
	runJob(a, p, scheduler.scheduleCpu (readyQueue));
	return;
}

//Timer ran out: check if job is doing IO. if its not, remove it from job table, if it is, terminate job
void Tro(long &a, long p[]){
    bookKeep(p[5]);
    long jobIndex = findByNumber(p[1]);
    if(!(jobTable.at(jobIndex)).getDoingIo())
	   remJobFromJobTable(currentJobRunning);
    else
        terminateJob(p[1]);//else terminate job
    runJob (a, p, scheduler.scheduleCpu (readyQueue));
    return;
}

//swap has completed
void Drmint (long &a, long p[]){
    bookKeep(p[5]);
    long jobIndex = findByNumber(p[1]);
    //check if swapped into memory; set setInMem accordingly
    if(drumOrCore == 0)
        (jobTable.at(jobIndex)).setInMem(true);
    else if(drumOrCore == 1)
        (jobTable.at(jobIndex)).setInMem(false);
    
    runJob (a, p, scheduler.scheduleCpu (readyQueue));
    return;
}

void Dskint (long &a, long p[]){
    bookKeep(p[5]);
    ioQueueJobNext(); // set the index
    
    //if job has no more pending I/O requests, unblock it, setDoingIo to false
    if(!jobTable.at(ioQueueJobIndex).getDoingIo()){
        jobTable.at(ioQueueJobIndex).setIsBlocked (false);
        jobTable.at(ioQueueJobIndex).setDoingIo(false);
    }
       
       //if job is terminated and has no more pending I/O requests, remove it from job table
       if(((jobTable.at(ioQueueJobIndex)).getIsTerminated()) && (!(jobTable.at(ioQueueJobIndex)).getDoingIo()))
       remJobFromJobTable (ioQueueJobIndex);
       
       //remove the job from the top of the I/O queue
       ioQueue.pop();
       currentJobIo = -1;
       //as long as ioqueue is not empty, get the job index of the next job and send it to SOS
       if (!ioQueue.empty()){
           ioQueueJobNext();
           currentJobIo = jobTable.at(ioQueueJobIndex).getJobNumber();
           jobTable.at(ioQueueJobIndex).setDoingIo(true);
           siodisk(jobTable.at(ioQueueJobIndex).getJobNumber());
       }
       runJob (a, p, scheduler.scheduleCpu (readyQueue));
       return;
}

//Job with job number jobNumber is requesting IO - begins by finding its index in job table. If IO queue is empty (no jobs are
//waiting to do IO), then siodisk is called to initiate IO. Otherwise, job is pushed onto IO queue to wait for IO.
//Whether job is currently doing IO or is waiting to do IO, the doingIo member variable of the corresponding Job class is set to true
void requestIo (long jobNumber) {
	long jobIndex = findByNumber(jobNumber);
	if (jobIndex == -1) {
		cout << "Job is not in jobTable" << endl;
		return;
	}
	if (ioQueue.empty()) {
		siodisk (jobNumber);
		currentJobIo = jobNumber;
	}
	else
		ioQueue.push(jobTable.at(jobIndex));
	(jobTable.at(jobIndex)).setDoingIo(true);
}

//Job with job number jobNumber is requesting termination - begins by finding its index in job table. If jobNumber corresponds to
//a real job and the job is not waiting to do IO, the job is terminated and memory is deallocated using memory manager.
//Otherwise, if jobNumber corresponds to a real job, terminate. Job table is cleaned up afterwards.
void terminateJob (long jobNumber) {
	long jobIndex = findByNumber(jobNumber);
	if (jobNumber != -1 && !isOnIoQueue(jobNumber)) {
		(jobTable.at(jobIndex)).setIsTerminated (true);
		//DEALLOCATE MEMORY HERE!!!
		memory.removeJob(jobTable.at(jobIndex));
	}
	else if (jobNumber != -1)
		(jobTable.at(jobIndex)).setIsTerminated (true);
	refreshJobTable();
}

//Returns index of job in the job table
long findByNumber (long jobNumber) {
	for (long i = 0; i < jobTable.size(); i++) {
		if ((jobTable.at(i)).getJobNumber() == jobNumber)
			return i;
	}
	return -1;
}

//Returns true if job with job number jobNumber is on the IO queue. Otherwise, returns false.
bool isOnIoQueue (long jobNumber) {
	queue<Job> tempIoQueue = ioQueue;
	for (long i = 0; i < tempIoQueue.size(); i++) {
		if ((tempIoQueue.front()).getJobNumber() == jobNumber)
			return true;
		tempIoQueue.pop();
	}
	return false;
}

//Adds a new job to job table if not already present
void addToJobTable (Job newJob) {
	long index = newJob.getJobNumber();
	vector<Job>::iterator it = jobTable.begin();
	refreshJobTable();
	while (it != jobTable.end()) {
		if ((*it).getJobNumber() == newJob.getJobNumber())
			return;
		it++;
	}
	jobTable.push(newJob);
}
	   
void remJobFromJobTable (long position){
	memory.removeJob(jobTable.at(currentJobRunning));
	jobTable.erase(jobTable.begin()+position);
	return;
}
       
//sets the job index of the job in the front of the IO Queue
void ioQueueJobNext() {
	for(long i = 0; i < jobTable.size(); i++)
		if ((jobTable.at(i)).getJobNumber() == (ioQueue.front()).getJobNumber())
			ioQueueJobIndex = i;
	return;
}

//Tracks state of job at beginning of interrupt by tracking time that interrupt occurred
void bookKeep (long time) {
	if (currentJobRunning != -1) {
		long timeSpent = time - (jobTable.at(currentJobRunning)).getCurrentTime();
		(jobTable.at(currentJobRunning)).setCurrentTime((jobTable.at(currentJobRunning)).getCurrentTime() + timeSpent);
		//(jobTable.at(currentJobRunning)).setMaxCpu((jobTable.at(currentJobRunning)).getMaxCpu() - timeSpent);
	}
}

//Dispatcher: sets CPU registers and runs job
void runJob (long &a, long p[], Job toRun) {
	if (readyQueue.empty() || toRun.getJobNumber() == -1)
		a = 1;
	else {
		a = 2;
		p[2] = toRun.getMemoryLocation();
		p[3] = toRun.getJobSize();
		p[4] = 50;
		currentJobRunning = toRun.getJobNumber();
		if ((readyQueue.front()).getJobNumber() == toRun.getJobNumber())
			readyQueue.pop();
	}
	return;
}

//Cleans up the job table by removing any job that has been terminated and attempts to find space in memory for a job waiting to run
void refreshJobTable () {
	for (long i = 0; i < jobTable.size(); i++) {
		vector<Job>::iterator it = jobTable.begin();
		if ((jobTable.at(i)).getIsTerminated())
			(jobTable.erase(it + i));
	}
	
	//Finding space without memory queue
	vector<Job>::iterator it = jobTable.begin();
	while (it != jobTable.end()) {
		if (!(*it).getInMem() && memory.jobFit(*it))
			if (!(*it).getIsBlocked() && !(*it).getDoingIo() && !(*it).getIsTerminated())
				swapper((*it).getJobNumber());
		it++;
	}
}

//Swapper determines which job to move into memory from the drum by first checking if job fits in memory and making sure that the
//job is not in memory already. If criteria are met, swapper places job in memory, adds the job to the ready queue, calls siodrum
//to initiate transfer, and drumOrCore is set to 0 so that Drmint could use this information to set inMem variable in corresponding
//Job instance for the job that was sent to the swapper.
//If there is no room for the job in memory, the Job member variable inMem is set to false and the job must wait for a room to free up
//Otherwise, the job must move from memory to the drum - swapper makes sure the job is in memory but not doing IO/waiting to do IO,
//and calls siodrum to initiate transfer in appropriate direction as well as removes job from memory. drumOrCore is set to 1 for use by Drmint.
void swapper (long jobNum) {
	long jobIndex = findByNumber(jobNum);
	if (jobIndex == -1) {
		cout << "Job is not in jobTable" << endl;
		return;
	}
	refreshJobTable ();
	//Swap requested from drum (jobTable) into memory
	if (memory.jobFit(jobTable.at(jobIndex)) && !(jobTable.at(jobIndex)).getInMem()) {
		drumOrCore = 0; //drum to core
		memory.placeJob(jobTable.at(jobIndex));
		readyQueue.push(jobTable.at(jobIndex));
		siodrum ((jobTable.at(jobIndex)).getJobNumber(), (jobTable.at(jobIndex)).getJobSize(), (jobTable.at(jobIndex)).getMemoryLocation(), drumOrCore);
		//(jobTable.at(jobIndex)).setInMem(true);
	}
	//Note that job is not in memory and wait for memory to free up
	else if (!memory.jobFit(jobTable.at(jobIndex)) && !(jobTable.at(jobIndex)).getInMem()) {
		(jobTable.at(jobIndex)).setInMem(false);
	}
	//Swap requested from memory to drum
	else {
		if (!(jobTable.at(jobIndex)).getDoingIo() && (jobTable.at(jobIndex)).getInMem()) {
			drumOrCore = 1; //core to drum
			memory.removeJob(jobTable.at(jobIndex));
			//(jobTable.at(jobIndex)).setInMem(false);
			(jobTable.at(jobIndex)).setMemoryLocation(-1);
			siodrum ((jobTable.at(jobIndex)).getJobNumber(), (jobTable.at(jobIndex)).getJobSize(), (jobTable.at(jobIndex)).getMemoryLocation(), drumOrCore);
		}
	}
}
