/*
 * Compiler.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: irm
 */

#include "Compiler.h"

using namespace std ;

/**
* @section con_compiler Compiler()
* Constructor for a Compiler object.
* 
* The constructor does the bulk of the work in interrogating the compiler 
* and returns the Vendor, Name and Version.
* 
* Currently, it uses pre-defined macros to determine which compiler is being
* used. This is no different to how it is confirmed in the Makefile but the only
* way I can see to get info at runtime is to assume that the compilers are loaded
* then it may be able to get version info.
*
*/
Compiler::Compiler() {
	// TODO Auto-generated constructor stub
#define QUOTE(X) QUOTE_(X)
#define QUOTE_(X) #X
#define CATJOIN(X,Y) X ## "." ## Y

#if defined(__INTEL_COMPILER)
#  define COMPILER_VENDOR "Intel"
#  define COMPILER_NAME "Intel"
#  define COMPILER_VERSION QUOTE(__INTEL_COMPILER)
#elif defined(__SUNPRO_C)
#  define COMPILER_VENDOR "Oracle"
#  define COMPILER_NAME "Sun"
#  define COMPILER_VERSION QUOTE(__SUNPRO_C)
#elif defined(__GNUC__)
#  define COMPILER_VENDOR "GNU"
#  define COMPILER_NAME "GCC"
#  define COMPILER_VERSION __VERSION__
#elif defined(__IBMC__)
#  define COMPILER_VENDOR "IBM"
#  define COMPILER_NAME "XLC"
#  define COMPILER_VERSION __xlc__
#elif defined(_CRAYC)
# define COMPILER_VENDOR "CRAY"
# define COMPILER_NAME "CCE"
# define COMPILER_VERSION CATJOIN(_RELEASE,_RELEASE_MINOR)
#elif defined(_PGI)
# define COMPILER_VENDOR "The Portland Group"
# define COMPILER_NAME "PGI"
# define COMPILER_VERSION __PGIC__
#elif defined(__clang__)
# define COMPILER_VENDOR "CLANG"
# define COMPILER_NAME "Clang"
# define COMPILER_VERSION CATJOIN(__clang_major__,__clang_minor__)
#else
#  define COMPILER_VENDOR "Unknown"
#  define COMPILER_NAME "Unknown"
#  define COMPILER_VERSION "Unknown"
#endif

	compilerVendor  = COMPILER_VENDOR;
	compilerName    = COMPILER_NAME;
	compilerVersion = COMPILER_VERSION;
}

/**
 * Destructor for Compiler class
 */
Compiler::~Compiler() {
	// TODO Auto-generated destructor stub
}

/**
 * @section printer_comp Compiler Printer
 * Method to print the information about the Compiler to stdout
 * 
 */
int Compiler::printCompilerInfo() {

	cout << "Compiler Info - " << endl;
	cout << "Vendor:  " <<   compilerVendor  << endl;
	cout << "Name:    "	<<	 compilerName    << endl;
	cout << "Version: " <<   compilerVersion << endl;

	return 0;
}

/*****************************************************************************/
/*****                             GETTERS                               *****/
/*****************************************************************************/

const std::string& Compiler::getCompilerName() const {
	return compilerName;
}

const std::string& Compiler::getCompilerVendor() const {
	return compilerVendor;
}

const std::string& Compiler::getCompilerVersion() const {
	return compilerVersion;
}

/*****************************************************************************/
/*****                             SETTERS                               *****/
/*****************************************************************************/

void Compiler::setCompilerName(const std::string& compilerName) {
	this->compilerName = compilerName;
}

void Compiler::setCompilerVendor(const std::string& compilerVendor) {
	this->compilerVendor = compilerVendor;
}

void Compiler::setCompilerVersion(const std::string& compilerVersion) {
	this->compilerVersion = compilerVersion;
}

/**
* @section c_interface_comp C Interface to Compiler
* This method is callable by C programs and returns the various Compiler information
*
* @param myCompVendor	Pointer to container for Vendor of Compiler
* @param myCompName		Pointer to container for Name of Compiler
* @param myCompVersion	Pointer to container for Version of Compiler
*
*/

void getCompInfo(char* myCompVendor, char* myCompName, char* myCompVersion){
    std::string vendor, name, version;
    Compiler tmpComp;
    
    vendor = tmpComp.getCompilerVendor();
    name = tmpComp.getCompilerName();
    version = tmpComp.getCompilerVersion();
    
    strcpy(myCompVendor,vendor.c_str());
    strcpy(myCompName,name.c_str());
    strcpy(myCompVersion,version.c_str());
    
}
