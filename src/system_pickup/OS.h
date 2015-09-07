/**
 * @file OS.h
 * @author AWE Plc.
 *
 * This file defines the OS class that contains all the attributes and methods
 * used to interrogate the underlying operating system and report it back to 
 * PMTM.
 * 
 */

#ifndef OS_H
#define OS_H


#ifdef __cplusplus
#include <iostream>
#include <sys/utsname.h>
#include <string>
#include <string.h>
#include "Utils.h"

using namespace std ;

/** @section os_desc OS Class
 * The class to contain all attributes and methods for interrogating the
 * underlying Operating System and making it available for PMTM to use.
 * 
 */
class OS {
public:
  /** @name Constructors
   * @{ */
	OS();
  /** @} */
  
  /** @name Destructors
   * @{ */
	virtual ~OS();
  /** @} */
  
  const std::string& getOsKernel() const;
  const std::string& getOsName() const;
  const std::string& getOsVendor() const;
  const std::string& getOsVersion() const;
  const std::string& getOsNode() const;
  /** @} */
  
  /** @name Setters
   * @{ */
  void setOsKernel(const std::string& osKernel);
  void setOsName(const std::string& osName);
  void setOsVendor(const std::string& osVendor);
  void setOsVersion(const std::string& osVersion);
  void setOsNode(const std::string& osNode);
  /** @} */
  
  /** @name Printers
   * @{ */
  int printOSInfo();
  /** @} */

private:
  std::string osVendor;   /**< Which Vendor creates the OS */
  std::string osName;     /**< What is the OS called */
  std::string osVersion;  /**< What version is this */
  std::string osKernel;   /**< What kernel is used */
  std::string osNode;     /**< What is the name of the node this rank is running on */

};

/** @name C_Interfaces
 * @{ */
extern "C" {
#endif
  void getOSInfo(char* myOsVendor, char* myOsName, char* myOsVersion, char* myOsKernel, char* myOsNode);
#ifdef __cplusplus
}
/** @} */
#endif
#endif /* OS_H */
