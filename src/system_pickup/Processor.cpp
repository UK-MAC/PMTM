/*
 * Processor.cpp
 *
 *  Created on: 10 Jul 2014
 *      Author: irm
 */

#include "Processor.h"

using namespace std ;

Processor::Processor() {
	// TODO Auto-generated constructor stub
  
#if defined(__AIX__)
  
#elif defined(__bg__)
  
#elif defined(__linux__)
  
  int sibs,cores;
  double clock;
  stringstream libstream;
  std::string tempStr,libstring;
  
  std::string tempstrint;
  Utils::get_system_string("awk '/siblings/ {print $3}' /proc/cpuinfo | sort -u",tempstrint);
  //sibs = std::stoi(tempstrint);
  sibs = strtol(tempstrint.c_str(),NULL,0);
  tempstrint.clear();
  Utils::get_system_string("awk '/cpu cores/ {print $4}' /proc/cpuinfo | sort -u",tempstrint);
  //cores = std::stoi(tempstrint);
  cores = strtol(tempstrint.c_str(),NULL,0);
  procThreads = sibs / cores ;

  tempstrint.clear();
  Utils::get_system_string("grep 'core id' /proc/cpuinfo |sort -u|wc -l",tempstrint);
//  procCores = std::stoi(tempstrint);
  procCores = strtol(tempstrint.c_str(),NULL,0);
  
  tempstrint.clear();
  Utils::get_system_string("cat /proc/cpuinfo | grep 'cpu MHz' | sort -u|sed 's/[^0-9\\.]//g'",tempstrint);
  clock = atof(tempstrint.c_str());
  procClock = (int) round(clock);
  
  Utils::get_system_string("uname -m",procArch);
  Utils::get_system_string("grep vendor_id /proc/cpuinfo|awk '{print $3}'|sort -u",procVendor);
  if(procVendor.find("Intel")!=string::npos){
   procVendor = "Intel"; 
   procName = "";
   //Utils::get_system_string("grep 'model name' /proc/cpuinfo|sort -u|awk '{print $5\"_\"$7}'",procName);
   Utils::get_system_string("grep 'model name' /proc/cpuinfo | sort -u",libstring);
   libstream.str(libstring.c_str());
   libstream >> tempStr;
   while(tempStr.compare("Intel(R)")!=0){
     libstream >> tempStr;
   }
   while(tempStr.compare("@")!=0){
    if(tempStr.compare("Intel(R)")!=0 && tempStr.compare("CPU")!=0){
     procName += tempStr;
     procName += "_";
    }
    libstream >> tempStr; 
   }
   procName.erase(procName.length()-1,1);
  }
  else if(procVendor.compare("AMD")!=string::npos){
   procVendor = "AMD";
   Utils::get_system_string("grep 'model name' /proc/cpuinfo|sort -u|awk '{print $6\"_\"$7$8}'",procName);
  }
#else
  	cout << "Unsupported Operating System. Data collected is likely to be erroneous" << endl;
#endif

}

Processor::~Processor() {
	// TODO Auto-generated destructor stub
}

int Processor::printProcessorInfo(){
	cout << "Processor Info - " << endl;
	cout << "Vendor: " << procVendor << endl;
	cout << "Name: " << procName << endl;
	cout << "Arch: " << procArch << endl;
	cout << "Cores: " << procCores << endl;
	cout << "Threads: " << procThreads << endl;
	cout << "Clock rate: " << procClock << endl;
	return 0;
}

const std::string& Processor::getProcArch() const {
	return procArch;
}

void Processor::setProcArch(const std::string& procArch) {
	this->procArch = procArch;
}

int Processor::getProcClock() const {
	return procClock;
}

void Processor::setProcClock(int procClock) {
	this->procClock = procClock;
}

int Processor::getProcCores() const {
	return procCores;
}

void Processor::setProcCores(int procCores) {
	this->procCores = procCores;
}

const std::string& Processor::getProcName() const {
	return procName;
}

void Processor::setProcName(const std::string& procName) {
	this->procName = procName;
}

int Processor::getProcThreads() const {
	return procThreads;
}

void Processor::setProcThreads(int procThreads) {
	this->procThreads = procThreads;
}

const std::string& Processor::getProcVendor() const {
	return procVendor;
}

void Processor::setProcVendor(const std::string& procVendor) {
	this->procVendor = procVendor;
}

void getProcInfo(char* myProcVendor, char* myProcName, char* myProcArch, int* myProcClock, int* myProcCores, int* myProcThreads){
  std::string vendor, name, arch;
  Processor tmpProc;
  
  vendor = tmpProc.getProcVendor();
  name = tmpProc.getProcName();
  arch = tmpProc.getProcArch();
  *myProcClock = tmpProc.getProcClock();
  *myProcCores = tmpProc.getProcCores();
  *myProcThreads = tmpProc.getProcThreads();
  
  strcpy(myProcVendor,vendor.c_str());
  strcpy(myProcName,name.c_str());
  strcpy(myProcArch,arch.c_str());

}
