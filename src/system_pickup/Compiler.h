/**
 * @file Compiler.h
 * @author AWE Plc.
 *
 * This file defines the Compiler class that contains all the attributes and methods
 * used to interrogate the system for information about the Compiler being used
 * 
 */

#ifndef COMPILER_H
#define COMPILER_H

#ifdef __cplusplus
#include <iostream>
#include <string.h>
#include <string>
#include "Utils.h"

using namespace std ;

/** @section comp_desc Compiler Class
 * The class to contain all attriubtes and methods for obtaining information
 * about the Compiler being used and making it available for PMTM to use
 * 
 */
class Compiler {
public:
  /** @name Constructors
   * @{ */
  Compiler();
  /** @} */
  
  /** @name Destructors
   * @{ */
  virtual ~Compiler();
  /** @} */
  
  /** @name Getters
   * @{ */
  const std::string& getCompilerName() const;
  const std::string& getCompilerVendor() const;
  const std::string& getCompilerVersion() const;
  /** @} */
  
  /** @name Setters
   * @{ */
  void setCompilerName(const std::string& compilerName);
  void setCompilerVendor(const std::string& compilerVendor);
  void setCompilerVersion(const std::string& compilerVersion);
  /** @} */
  
  /** @name Printers
   * @{ */
  int printCompilerInfo();
  /** @} */
  
  
private:
  std::string compilerVendor;   /**< Which Vendor created the Compiler */
  std::string compilerName ;    /**< What is the Compiler called */
  std::string compilerVersion ; /**< What is the version of the Compiler */
};

/** @name C_Interfaces
 * @{ */
extern "C" {
#endif
  void getCompInfo(char* myCompVendor, char* myCompName, char* myCompVersion);
#ifdef __cplusplus
}
/** @} */
#endif

#endif /* COMPILER_H */
