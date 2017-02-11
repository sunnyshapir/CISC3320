#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <iostream>
using namespace std;


class MemoryManager{

public: 
    
    
    int memoryChunk= 100;
    int arrayOfJobs[memoryChunk]; 


    MemoryManager(){
        initJobsArray();
    }
    
    

// Initialize all chunks of memory to 0  
void initJobsArray(){
	for (int i = 0; i < memoryChunk; i++){
		arrayOfJobs[i]=0;
	}
}
    

// Returns the amount of free memory chunks
int getFreeMemory(){
    
	int blocks = 0;
    
	for (int i = 0; i < memoryChunk; i++){
		if (arrayOfJobs[i] == 0){
			blocks++;
		}
	}

	blocks = blocks*1000;
    
    return  blocks;
}

// Tests if Job will fit into memory
bool jobFit(Job &j){
    if (j.getJobSize() < getFreeMemory()){
		return true;
	} else {
		return false;
	}
}


    
//Print Memory Management Table
void printMemoryTable(){
	for (int i =0; i <memoryChunk; i++){
		cout << "Place in Memory | " << i << " | Job Number: | " <<  arrayOfJobs[i] << endl;
	}
}
    
    
// Small Error in code ** still working on**
// Jobs are placed into memory but in certain cases 
// the job will not place jobNumber into the last slot of memory it will consume
    
// Places Job Into Memory
void placeJob(Job &j){

	int counter;

    // Store size and jobNumber
	int sizeOfJob = j.getJobSize();
	int jId = j.getJobNumber();

    // Pointer to beginning of jobs array
	int *n = (int*)&arrayOfJobs[0];

	for (int i=0; i<memoryChunk-sizeOfJob; i++){

		counter =0;
        
        // If current array position doesnt have jobNumber
		if (arrayOfJobs[i] == 0){
            
			n = (int*)&arrayOfJobs[i];
            
            // Loop to find contiguous memory slots
			while (*n==0 && counter!=sizeOfJob){
				counter++;
				n++;
			}
            
            // If there is enough contiguous memory, place job into array
			if (counter == sizeOfJob){
                
                    std::cout << "Job #"<< j.getJobNumber() << " may fit, beginning with index " << i << " and ending at " << i + (counter-1) <<endl;

                    // Set memory location of job
                    j.setMemoryLocation(i);

                    // Set pointer to first memory address locaction  of job
                    n = (int*)&arrayOfJobs[i];

                    // Places jobNumber into contiguous memory slots
                    for (int j=i-1; j <(i+(counter-1));j++){
                        *n = jId;
                        n++;
                    }

                    // Set pointer to null
                    n=NULL;

                    // Print memory Table
                    printMemoryTable();
                    
                    // Set in memory for job to ture
                    j.setInMem(true);
                
                    // Break out of loop
                    break;
			
                }  else   {
                    
                    // Go to next iteration of loop
                    continue;
			     }
		      } else {
			         std::cout << "Job with id: " << j.getJobNumber() << " cannot be placed into memory @ " << i <<endl;
			         continue;
		      }
	}
    
    // Set pointer to null
    n=NULL;
}
    
// Removes job from memory 
void removeJob(Job &j){
    
    // Counter to keep track of memory locations deleted in array 
	int counter =0;
    
    // Pointer to the memory location of the job in the array 
	int *pointer = (int*)&arrayOfJobs[j.getMemoryLocation()];

    // Test if job is in memory
    if (j.getInMem()){
           
        // Compare counter to jobSize
        while (counter != j.getJobSize()){
           
            // Set memory location to 0
            // Removes jobID# from current array location
            *pointer=0;
            
            // Increment pointer to memory location of the job within the array
            pointer++;
            
            // Increment counter 
            counter++;
	   }
        // Set pointer to null
        pointer = NULL;
    }
    
    // Take job out of memory
    j.setInMem(false);

}
   
    
};

#endif