/**
 * @file Processor.h
 * @author AWE Plc.
 *
 *  This file defines the Processor Class that contains all the attributes and methods
 * used to interrogate the underlying Processor and report it back to PMTM.
 *
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

/** @section proc_desc Processor Class
* The class to contain all attributes and methods for interrogating the 
* underlying Processor and making it available for PMTM to use.
*
*/
class Processor {
public:
  /** @name Constructors
   * @{ */
	Processor();
  /** @} */
  
  /** @name Destructors
   * @{ */
	virtual ~Processor();
  /** @} */
  
  /** @name Getters
  * @{ */
	const std::string& getProcVendor() const;
	const std::string& getProcName() const;
	const std::string& getProcArch() const;
	int getProcClock() const;
	int getProcCores() const;
	int getProcThreads() const;
  /** @} */
  
  /** @name Setters
   * @{ */
	void setProcVendor(const std::string& procVendor);
	void setProcName(const std::string& procName);
	void setProcArch(const std::string& procArch);
	void setProcClock(int procClock);
	void setProcCores(int procCores);
	void setProcThreads(int procThreads);
  /** @} */
  
  /** @name Printers
   * @{ */	
	int printProcessorInfo();
  /** @} */

private:
	std::string procVendor;		/**< Which Vendor created the Processor */
	std::string procName;		/**< What is the name of Processor */
	std::string procArch;		/**< What architecture is the Processor */
	int procClock;				/**< What is the clock speed of the Processor in MHz */
	int procCores;				/**< How many physical cores does the Processor have */
	int procThreads;			/**< How many threads does each Processor have */
};

/** @name C_Interfaces
 * @{ */
extern "C" {
#endif
  void getProcInfo(char* myProcVendor, char* myProcName, char* myProcArch, int* myProcClock, int* myProcCores, int* myProcThreads);
#ifdef __cplusplus
}
/** @} */
#endif
#endif /* PROCESSOR_H_ */
