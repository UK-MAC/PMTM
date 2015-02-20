/*
 * MPI.h
 *
 *  Created on: 2 Jul 2014
 *      Author: irm
 */

#ifndef EXT_MPI_H_
#define EXT_MPI_H_

#include <mpi.h>
#include <iostream>
#include <sstream>
#include <string>
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

private:
	std::string mpiVendor;
	std::string mpiName;
	std::string mpiVersion;
	std::string mpiStandard;
};

#endif /* EXT_MPI_H_ */
