/*
 * Utils.cpp
 *
 *  Created on: 1 Jul 2014
 *      Author: irm
 */

#include "Utils.h"


using namespace std ;

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}

// Command to extract a string from the result of a system command
std::string Utils::get_system_string(const char* cmd, std::string& result,bool preserve) {
	// Open a pipe to POSIX compliant command line and execute cmd
	// *NIX only at the moment
	FILE * pipe = popen(cmd, "r");
	// May never reach this line but check for errors anyway
	if(!pipe) return "ERROR";

	char buffer[BUFF_SIZE];
	// Work through pipe and add to result string
	while(!feof(pipe)){
		if(fgets(buffer,(BUFF_SIZE - 1),pipe)!= NULL)
			result += buffer;
	}
	// Close the pipe
	// *NIX only at the moment
	pclose(pipe);

	if(!preserve){
	  replace(result.begin(), result.end(),'\n','\0');
	}

	return result;
}

bool Utils::my_exists(const std::string& filename){
	ifstream file(filename.c_str());
	return file;
}

bool Utils::stat_exists(const std::string& filename){
	struct stat buf;
	return (stat(filename.c_str(), &buf)==0);
}

//template<class T>
//std::string operator+ (std::string const &a, const T &b){
//	std::ostringstream oss;
//	oss << a << b;
//	return oss.str();
//}

