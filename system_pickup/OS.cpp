/*
 * OS.cpp
 *
 *  Created on: 30 Jun 2014
 *      Author: irm
 */

#include "OS.h"

using namespace std ;

OS::OS() {
	// TODO Auto-generated constructor stub
	struct utsname sysinfo;

	uname(&sysinfo);

//	osVendor = sysinfo.sysname;
//	osName   = sysinfo.nodename;
//	osVersion = sysinfo.version;
	osKernel = sysinfo.release;

#if defined(__AIX__)
	// Stub for code to find AIX information
	
#elif defined(__bg__)
//	// Stub for code to find BlueGene information
#elif defined(__linux__)
	// Stub for code to find Linux information
	Utils::get_system_string("lsb_release -i -s", osVendor);
	Utils::get_system_string("lsb_release -r -s", osVersion);
	if(osVersion.find(".") == string::npos){
		std::string patch;
		Utils::get_system_string("cat /etc/*-release | awk '/PATCHLEVEL/ {print $3}'", patch);
		osVersion += ".";
		osVersion += patch;
	}
	Utils::get_system_string("lsb_release -d -s", osName);

#elif defined(__unix__)
	// Stub for code to find *NIX information

//#elif defined(__APPLE__ && __MACH__)
//	// Stub for code to find MacOS information
//#elif defined(__bsdi__)
//	// Stub for code to find BSD information
//#elif defined(__sun)
//	// Stub for code to find Solaris information
//#elif defined(__CYGWIN__)
//	// Stub for code to find CYGWIN information
//#elif defined(__WIN32)
//	// Stub for code to find Windows information
//#elif defined(__MSDOS__)
//	// Stub for code to find MSDOS information
#else
	cout << "Unsupported Operating System. Data collected is likely to be erroneous" << endl;
#endif

}

OS::~OS() {
	// TODO Auto-generated destructor stub
}




int OS::printOSInfo(){

	cout << "Operating System Info - " << endl;
	cout << "Vendor:  " <<   osVendor  << endl;
	cout << "Name:    "	<<	 osName    << endl;
	cout << "Version: " <<   osVersion << endl;
	cout << "Kernel:  " <<   osKernel  << endl;
	return 0;
}

const std::string& OS::getOsKernel() const {
	return osKernel;
}

void OS::setOsKernel(const std::string& osKernel) {
	this->osKernel = osKernel;
}

const std::string& OS::getOsName() const {
	return osName;
}

void OS::setOsName(const std::string& osName) {
	this->osName = osName;
}

const std::string& OS::getOsVendor() const {
	return osVendor;
}

void OS::setOsVendor(const std::string& osVendor) {
	this->osVendor = osVendor;
}

const std::string& OS::getOsVersion() const {
	return osVersion;
}

void OS::setOsVersion(const std::string& osVersion) {
	this->osVersion = osVersion;
}
