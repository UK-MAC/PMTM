/*
 * @file Processor.cpp
 * @author AWE plc.
 *
 *  
 */

#include "Processor.h"

using namespace std ;

/** @section con_proc Processor()
* Constructor for a Processor Object.
*
* The constructor does the bulk of the work in interrogating the Processor
* and returns the Vendor, Name, Architecture, Clock speed, Cores and threads 
* in the processor.
*
* @TODO Create parser for PowerPC
* @TODO Figure out how to get information about attached devices such as GPUs
*
*/
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

/**
 * Destructor for OS class
 */
Processor::~Processor() {
	// TODO Auto-generated destructor stub
}


/**
 * @section printer_proc Processor Printer
 * Method to print the information about the Processor to stdout
 * 
 */
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

/*****************************************************************************/
/*****                             GETTERS                               *****/
/*****************************************************************************/

const std::string& Processor::getProcVendor() const {
  return procVendor;
}

const std::string& Processor::getProcName() const {
  return procName;
}

const std::string& Processor::getProcArch() const {
	return procArch;
}

int Processor::getProcClock() const {
  return procClock;
}

int Processor::getProcCores() const {
  return procCores;
}

int Processor::getProcThreads() const {
  return procThreads;
}

/*****************************************************************************/
/*****                             SETTERS                               *****/
/*****************************************************************************/

void Processor::setProcVendor(const std::string& procVendor) {
  this->procVendor = procVendor;
}

void Processor::setProcName(const std::string& procName) {
  this->procName = procName;
}

void Processor::setProcArch(const std::string& procArch) {
	this->procArch = procArch;
}

void Processor::setProcClock(int procClock) {
	this->procClock = procClock;
}

void Processor::setProcCores(int procCores) {
	this->procCores = procCores;
}

void Processor::setProcThreads(int procThreads) {
	this->procThreads = procThreads;
}

/**
* @section c_interface_proc C Interface to Processor
* This method is callable by C programs and returns the Various Processor information
*
* @param myProcVendor   Pointer to container for Vendor of Processor
* @param myProcName     Pointer to container for Name of Processor
* @param myProcArch     Pointer to container for the Architecture of Processor
* @param myProcClock    Pointer to container for Clock speed of Processor
* @param myProcCores    Pointer to container for number of physical cores on Processor
* @param myProcThreads  Pointer to container for number of threads on a core
*
*/
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
