/*
 * Processor.h
 *
 *  Created on: 10 Jul 2014
 *      Author: irm
 */

#ifndef PROCESSOR_H_
#define PROCESSOR_H_

#ifdef __cplusplus
#include <iostream>
#include <sys/utsname.h>
#include <string>
#include <string.h>
#include "Utils.h"
#include "stdio.h"
#include "stdlib.h"
#include <math.h>

using namespace std ;

class Processor {
public:
	Processor();
	virtual ~Processor();
	const std::string& getProcArch() const;
	void setProcArch(const std::string& procArch);
	int getProcClock() const;
	void setProcClock(int procClock);
	int getProcCores() const;
	void setProcCores(int procCores);
	const std::string& getProcName() const;
	void setProcName(const std::string& procName);
	int getProcThreads() const;
	void setProcThreads(int procThreads);
	const std::string& getProcVendor() const;
	void setProcVendor(const std::string& procVendor);
	
	int printProcessorInfo();

private:
	std::string procVendor;
	std::string procName;
	std::string procArch;
	int procClock;
	int procCores;
	int procThreads;
};

extern "C" {
#endif
  void getProcInfo(char* myProcVendor, char* myProcName, char* myProcArch, int* myProcClock, int* myProcCores, int* myProcThreads);
  
#ifdef __cplusplus

}
#endif
#endif /* PROCESSOR_H_ */
