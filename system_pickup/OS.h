/*
 * OS.h
 *
 *  Created on: 30 Jun 2014
 *      Author: irm
 */

#ifndef OS_H
#define OS_H

#include <iostream>
#include <sys/utsname.h>
#include <string>
#include "Utils.h"


using namespace std ;

class OS {
public:
	OS();
	virtual ~OS();
	const std::string& getOsKernel() const;
	void setOsKernel(const std::string& osKernel);
	const std::string& getOsName() const;
	void setOsName(const std::string& osName);
	const std::string& getOsVendor() const;
	void setOsVendor(const std::string& osVendor);
	const std::string& getOsVersion() const;
	void setOsVersion(const std::string& osVersion);

	int printOSInfo();

private:
std::string osVendor;
std::string osName;
std::string osVersion;
std::string osKernel;

};

#endif /* OS_H */
