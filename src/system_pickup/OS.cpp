/**
 * @file OS.cpp
 * @author AWE Plc.
 *
 *
 */

#include "OS.h"

using namespace std ;

/**
 * @section con_os OS()
 * Constuctor for an OS object.
 * 
 * The constructor does the bulk of the work in interrogating the operating system
 * and returns the Vendor, Name, Version and Kernel. 
 * 
 * It also returns the node it is being run on, but this only works if the object 
 * is constructed on all processors, which is not ideal.
 * 
 * @TODO Create code to interrogate AIX system - Needed 
 * @TODO Create code to interrogate BlueGene system - May not need
 * @TODO Create code to interrogate UNIX system - Linux code may be good enough
 * @TODO Create code to interrogate MacOS system - For dev use only
 * @TODO Create code to interrogate BSD system - Can use Linux code?
 * @TODO Create code to interrogate Solaris system - May not need
 * @TODO Create code to interrogate Cygwin system - Only needed if decided to support this usage
 * @TODO Create code to interrogate Windows system - Only needed if windows support required
 * @TODO Create code to interrogate DOS system - Only needed if DOS support required
 */
OS::OS() {
  
/**
 * The uname command is a POSIX command, so should be available in most 
 * operating systems except for Windows. And as we do not yet support 
 * Windows yet, it is OK not to ifdef the usage of uname
 */
	struct utsname sysinfo;
	std::string tempStr;

	uname(&sysinfo);

	osNode   = sysinfo.nodename;
	osKernel = sysinfo.release;
	
#if defined(__AIX__)
	// Stub for code to find AIX information
	
#elif defined(__bg__)
//	// Stub for code to find BlueGene information
#elif defined(__linux__)
	// Stub for code to find Linux information
/**
 * For Linux there are two basic options:
 * 
 * 1. lsb_release
 * 2. Read from /proc
 * 
 * In the former case everything is much simpler but unfornately it is not always
 * installed on every system (@note Be wary as it can be installed on a Login node
 * but then not be installed on a compute node, this is more likely to happen when
 * a lightweight compute OS is used).
 * 
 * In the latter case only Red Hat and SuSE are supported at the moment, with other
 * Linux distributions being designated as "Unknown" and the version and kernel
 * set to the same thing.
 */
	Utils::get_system_string("which lsb_release", tempStr);
	
	if(Utils::stat_exists(tempStr)){
	
	
	        Utils::get_system_string("lsb_release -i -s", osVendor);
	        Utils::get_system_string("lsb_release -r -s", osVersion);
	        if(osVersion.find(".") == string::npos){
		        std::string patch;
		        Utils::get_system_string("cat /etc/*-release | awk '/PATCHLEVEL/ {print $3}'", patch);
		        osVersion += ".";
		        osVersion += patch;
	        }
	        Utils::get_system_string("lsb_release -d -s", osName);
	        while(osName.find('"') != string::npos){
	                osName.erase(osName.find('"'),1);
	        }
	        
	        int ob;
	        ob = osName.find("(");
	        if(ob != string::npos){
	                osName.erase((ob-1),string::npos);
	        }
	}
	else{
	        Utils::get_system_string("cat /proc/version", tempStr);
	        
	        
	        if((tempStr.find("Red Hat") != string::npos) || (tempStr.find("redhat") != string::npos)){
	                osVendor = "Red Hat";
	                Utils::get_system_string("cat /etc/redhat-release | head -1", osName);
	                tempStr = osName;
	                int fn,ln;
	                fn = tempStr.find_first_of("0123456789");
	                ln = tempStr.find_last_of("0123456789");
	                osVersion = tempStr.substr(fn,((ln-fn)+1));
	        }
	        else if((tempStr.find("SUSE") != string::npos) || (tempStr.find("suse") != string::npos)){
	                osVendor = "SuSE";
	                Utils::get_system_string("cat /etc/SuSE-release | head -1", osName);
	                std::string patch;
	                Utils::get_system_string("cat /etc/SuSE-release | awk '/VERSION/ {print $3}'", osVersion);
	                osVersion += ".";
	                Utils::get_system_string("cat /etc/*-release | awk '/PATCHLEVEL/ {print $3}'", patch);
		        osVersion += patch;
	                
	        }
	        else {
	                osVendor = "Unknown";
	                osName = "Linux";
	                osVersion = osKernel;
	                cout << "Found Unknown Linux distribution, setting version number to kernel revision" << endl;
	        }
	        int ob;
	        ob = osName.find("(");
	        if(ob != string::npos){
	                osName.erase((ob-1),string::npos);
	        }
	}

#elif defined(__unix__)
	// Stub for code to find *NIX information

#elif defined(__APPLE__ && __MACH__)
//	// Stub for code to find MacOS information
#elif defined(__bsdi__)
//	// Stub for code to find BSD information
#elif defined(__sun)
//	// Stub for code to find Solaris information
#elif defined(__CYGWIN__)
//	// Stub for code to find CYGWIN information
#elif defined(__WIN32)
//	// Stub for code to find Windows information
#elif defined(__MSDOS__)
//	// Stub for code to find MSDOS information
#else
	cout << "Unsupported Operating System. Data collected is likely to be erroneous" << endl;
#endif

}

/**
 * Destructor for OS class
 */
OS::~OS() {
	// TODO Auto-generated destructor stub
}

/**
 * @section printer_os OS Printer
 * Method to print the information about the OS to stdout
 * 
 */
int OS::printOSInfo(){

	cout << "Operating System Info - " << endl;
	cout << "Vendor:  " <<   osVendor  << endl;
	cout << "Name:    "	<<	 osName    << endl;
	cout << "Version: " <<   osVersion << endl;
	cout << "Kernel:  " <<   osKernel  << endl;
	return 0;
}

/*****************************************************************************/
/*****                             GETTERS                               *****/
/*****************************************************************************/

const std::string& OS::getOsKernel() const {
	return osKernel;
}

const std::string& OS::getOsName() const {
  return osName;
}

const std::string& OS::getOsVendor() const {
  return osVendor;
}

const std::string& OS::getOsVersion() const {
  return osVersion;
}

const std::string& OS::getOsNode() const {
  return osNode;
}

/*****************************************************************************/
/*****                             SETTERS                               *****/
/*****************************************************************************/

void OS::setOsKernel(const std::string& osKernel) {
	this->osKernel = osKernel;
}

void OS::setOsName(const std::string& osName) {
	this->osName = osName;
}

void OS::setOsVendor(const std::string& osVendor) {
	this->osVendor = osVendor;
}

void OS::setOsVersion(const std::string& osVersion) {
	this->osVersion = osVersion;
}

void OS::setOsNode(const std::string& osNode) {
        this->osNode = osNode;
}

/**
 * @section c_interface_os C Interface to OS
 * This method is callable by C programs and returns the various OS information
 * 
 * @param myOsVendor    Pointer to container for Vendor of OS
 * @param myOsName      Pointer to container for Name of OS
 * @param myOsVersion   Pointer to container for version number of OS
 * @param myOsKernel    Pointer to container for the kernel used by OS
 * @param myOsNode      Pointer to container for the node on which this method is called
 * 
 * 
 */
void getOSInfo(char* myOsVendor, char* myOsName, char* myOsVersion, char* myOsKernel, char* myOsNode){
    std::string vendor, name, version, kernel, node;
    OS tmpOS;
    
    vendor = tmpOS.getOsVendor();
    name = tmpOS.getOsName();
    version = tmpOS.getOsVersion();
    kernel = tmpOS.getOsKernel();
    node = tmpOS.getOsNode();
    
    strcpy(myOsVendor,vendor.c_str());
    strcpy(myOsName,name.c_str());
    strcpy(myOsVersion,version.c_str());
    strcpy(myOsKernel,kernel.c_str());
    strcpy(myOsNode,node.c_str());
}