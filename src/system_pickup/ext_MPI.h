/*
 * MPI.h
 *
 *  Created on: 2 Jul 2014
 *      Author: irm
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
//#include <boost/lexical_cast.hpp.>

using namespace std ;

class ext_MPI {
public:
	ext_MPI();
	virtual ~ext_MPI();

	int printMPIInfo();
	const std::string& getMpiName() const;
	void setMpiName(const std::string& mpiName);
	const std::string& getMpiVendor() const;
	void setMpiVendor(const std::string& mpiVendor);
	const std::string& getMpiVersion() const;
        void setMpiVersion(const std::string& mpiVersion);
        const std::string& getMpiStandard() const;
        void setMpiStandard(const std::string& mpiStandard);

private:
	std::string mpiVendor;
	std::string mpiName;
	std::string mpiVersion;
	std::string mpiStandard;
};

extern "C" {
#endif
  void getMpiInfo(char* myMpiVendor, char* myMpiName, char* myMpiVersion, char* myMpiStandard);
  
#ifdef __cplusplus
}
#endif

#endif /* EXT_MPI_H_ */
