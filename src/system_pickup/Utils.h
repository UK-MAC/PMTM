/*
 * Utils.h
 *
 *  Created on: 1 Jul 2014
 *      Author: irm
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

class Utils {
public:
	Utils();
	virtual ~Utils();
	static std::string get_system_string(const char * cmd, std::string& result,bool preserve = false);
	static bool my_exists(const std::string& filename);
	static bool stat_exists(const std::string& filename);
//	std::string operator+ (std::string const &a, int b);
	static const int BUFF_SIZE = 128;
};

#endif /* UTILS_H */
