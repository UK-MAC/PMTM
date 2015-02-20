/*
 * Compiler.h
 *
 *  Created on: 2 Jul 2014
 *      Author: irm
 */

#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <string>
#include "Utils.h"

using namespace std ;

class Compiler {
public:
	Compiler();
	virtual ~Compiler();

	int printCompilerInfo();
	const std::string& getCompilerName() const;
	void setCompilerName(const std::string& compilerName);
	const std::string& getCompilerVendor() const;
	void setCompilerVendor(const std::string& compilerVendor);
	const std::string& getCompilerVersion() const;
	void setCompilerVersion(const std::string& compilerVersion);

private:
	std::string compilerVendor;
	std::string compilerName ;
	std::string compilerVersion ;
};

#endif /* COMPILER_H */
