#ifndef JOB_H_
#define JOB_H_

#include <iostream>
using namespace std;

class Job {
	public:
		Job (long p[]) {
			jobNumber = p[1];
			jobPriority = p[2];
			jobSize = p[3];
			maxCPU = p[4];
			currentTime = p[5];
			memoryLocation = -1;
			inMem = false;
			doingIo = false;
			isBlocked = false;
			isTerminated = false;
		}

		Job () {
			jobNumber = -1;
			jobPriority = NULL;
			jobSize = NULL;
			maxCPU = NULL;
			currentTime = NULL;
			//must get memory location from memory handler
			memoryLocation = -1;
			inMem = false;
			doingIo = false;
			isBlocked = false;
			isTerminated = false;
		}

		long getJobNumber () {
			return jobNumber;
		}

		void setJobNumber (long number) {
			jobNumber = number;
		}

		long getJobPriority () {
			return jobPriority;
		}

		void setJobPriority (long priority) {
			jobPriority = priority;
		}

		long getJobSize () {
			return jobSize;
		}

		void setJobSize (long size) {
			jobSize = size;
		}

		long getMaxCPU () {
			return maxCPU;
		}

		void setMaxCPU (long max) {
			maxCPU = max;
		}

		long getCurrentTime () {
			return currentTime;
		}

		void setCurrentTime (long current) {
			currentTime = current;
		}

		void setMemoryLocation (long address) {
			memoryLocation = address;
		}

		long getMemoryLocation () {
			return memoryLocation;
		}

		void setInMem (bool val) {
			inMem = val;
		}

		bool getInMem () {
			return inMem;
		}

		void setDoingIo (bool val) {
			doingIo = val;
		}

		bool getDoingIo () {
			return doingIo;
		}

		void setIsBlocked (bool val) {
			isBlocked = val;
		}

		bool getIsBlocked () {
			return isBlocked;
		}

		void setIsTerminated (bool val) {
			isTerminated = val;
		}

		bool getIsTerminated () {
			return isTerminated;
		}

	private:
		long jobNumber;
		long jobPriority;
		long jobSize;
		long maxCPU;
		long currentTime;
		long memoryLocation;
		bool inMem;
		bool doingIo;
		bool isBlocked;
		bool isTerminated;
};

#endif /* JOB_H_ */
