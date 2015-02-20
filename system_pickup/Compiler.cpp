/*
 * Compiler.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: irm
 */

#include "Compiler.h"

using namespace std ;

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

Compiler::~Compiler() {
	// TODO Auto-generated destructor stub
}

int Compiler::printCompilerInfo() {

	cout << "Compiler Info - " << endl;
	cout << "Vendor:  " <<   compilerVendor  << endl;
	cout << "Name:    "	<<	 compilerName    << endl;
	cout << "Version: " <<   compilerVersion << endl;

	return 0;
}

const std::string& Compiler::getCompilerName() const {
	return compilerName;
}

void Compiler::setCompilerName(const std::string& compilerName) {
	this->compilerName = compilerName;
}

const std::string& Compiler::getCompilerVendor() const {
	return compilerVendor;
}

void Compiler::setCompilerVendor(const std::string& compilerVendor) {
	this->compilerVendor = compilerVendor;
}

const std::string& Compiler::getCompilerVersion() const {
	return compilerVersion;
}

void Compiler::setCompilerVersion(const std::string& compilerVersion) {
	this->compilerVersion = compilerVersion;
}
