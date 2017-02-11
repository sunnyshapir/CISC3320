#ifndef CPUSCHEDULER_H_
#define CPUSCHEDULER_H_
#include <iostream>
#include <vector>
#include "Job.h"
using namespace std;

class cpuScheduler {
	public:
	//STS: First Come, First Served using ready queue
		Job scheduleCpu (queue<Job> readyQueue) {
			queue<Job> ready = readyQueue;
			Job toRun = NULL;
			while (!ready.empty()) {
				if ((ready.front()).getInMem())
					if (!(ready.front()).getIsBlocked() && !(ready.front()).getDoingIo()) {
						toRun = ready.front();
						break;
					}
				ready.pop();
			}
			return toRun;
		}
};

#endif /* CPUSCHEDULER_H_ */
