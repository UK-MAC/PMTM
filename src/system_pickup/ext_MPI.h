/**
 * @file ext_MPI.h
 * @author AWE Plc.
 *
 * This file defines the ext_MPI class that contains all the attributes and methods
 * used to interrogate the underlying MPI implementation and report it back to PMTM.
 *
 */

#ifndef EXT_MPI_H_
#define EXT_MPI_H_

#ifdef __cplusplus
#include <mpi.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include "Utils.h"
#include <ctype.h>
#include <stdlib.h>

using namespace std ;

/** @section mpi_desc ext_MPI Clas
* The class to contain all attributes and methods for interrogating the 
* underlying the MPI implementation and making it available for PMTM to
* use.
* 
*/
class ext_MPI {
public:
  /** @name Constructors
   * @{ */
	ext_MPI();
  /** @} */
  
  /** @name Destructors
   * @{ */
	virtual ~ext_MPI();
  /** @} */

  
  /** @name Getters
  * @{ */
	const std::string& getMpiName() const;
	const std::string& getMpiVendor() const;
	const std::string& getMpiVersion() const;
    const std::string& getMpiStandard() const;
  /** @} */
  
  /** @name Setters
   * @{ */
	void setMpiName(const std::string& mpiName);
	void setMpiVendor(const std::string& mpiVendor);
    void setMpiVersion(const std::string& mpiVersion);
    void setMpiStandard(const std::string& mpiStandard);
  /** @} */
  
  /** @name Printers
   * @{ */
	int printMPIInfo();
  /** @} */

private:
	std::string mpiVendor;		/**< Which Vendor creates the MPI */
	std::string mpiName;		/**< What is the MPI is called */
	std::string mpiVersion;		/**< What version of the MPI is this */
	std::string mpiStandard;	/**< What standard does the MPI follow */
};

/** @name C_Interfaces
 * @{ */
extern "C" {
#endif
  void getMpiInfo(char* myMpiVendor, char* myMpiName, char* myMpiVersion, char* myMpiStandard);
  
#ifdef __cplusplus
}
/** @} */
#endif

#endif /* EXT_MPI_H_ */
