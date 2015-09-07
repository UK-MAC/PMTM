/**
 * @file Utils.h
 * @author AWE Plc.
 *
 */
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <sys/stat.h>

using namespace std ;

/** @section utils_desc Utils Class
* A utility class that contains a number of small methods to do common tasks
*
*/
class Utils {
public:
  /** @name Constructors
   * @{ */
	Utils();
  /** @} */
  
  /** @name Destructors
   * @{ */
	virtual ~Utils();
  /** @} */

  /** @name Utilities
  * @{ */
	static std::string get_system_string(const char * cmd, std::string& result,bool preserve = false);
	static bool my_exists(const std::string& filename);
	static bool stat_exists(const std::string& filename);
  /** @} */

  /** @name Sizes
  * @{} */
	static const int BUFF_SIZE = 128;
  /** @} */
};

#endif /* UTILS_H */
