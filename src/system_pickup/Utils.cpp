/**
 * @file Utils.cpp
 * @author AWE Plc.
 *
 *  This file defines a number of utilities that are commonly used within the auto
 * system checker
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
/** @section extract_string String Extractor
* Utility to return a string from a command run outside the program on the command line
*
* @param com 		The command to run outside the program
* @param preserve	Indicator of whether to preserve endlines 
* @return result	The output from the command
*
*/
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

/** @section my_exists
* Utility to discover whether a file exists or not
*
* @param filename	Name of file to check 
*
*/
bool Utils::my_exists(const std::string& filename){
	ifstream file(filename.c_str());
	return file;
}

/** @section stat_exists
* Utility to discover whether a file exists or not
*
* @param filename	Name of the file to check
*/
bool Utils::stat_exists(const std::string& filename){
	struct stat buf;
	return (stat(filename.c_str(), &buf)==0);
}

