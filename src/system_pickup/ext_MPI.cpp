/*
 * MPI.cpp
 *
 *  Created on: 2 Jul 2014
 *      Author: irm
 */

#include "ext_MPI.h"

using namespace std ;

ext_MPI::ext_MPI() {
	// TODO Auto-generated constructor stub

//	mpiVendor  = ;
//	mpiName    = ;
//	mpiVersion = ;
	int major;
	int minor;
	int err;
	stringstream libstream,twostream;
	std::string tempStr;
#if MPI_VERSION==3
	int leng;
	char libstring[MPI_MAX_LIBRARY_VERSION_STRING];
#else
	string libstring;
#endif
	MPI_Get_version(&major, &minor);

// 	cout << major << "." << minor << endl;
	mpiStandard = "";
	mpiStandard += ( major+'0');
	mpiStandard += ".";
	mpiStandard += ( minor+'0'  );

// 	if(major == 2){
#if MPI_VERSION==2
	
	std::string mpv, mpv2,ompi,impi,mpt;
	bool mpvf,mpv2f,ompif,impif,mptf;
	
	Utils::get_system_string("which mpicxx", tempStr);
	tempStr.erase(tempStr.length()-7,7);
	
	mpvf = false;
	mpv2f = false;
	ompif = false;
	impif = false;
	
	mpv = tempStr + "mpichversion";
	mpv2 = tempStr + "mpich2version";
	ompi = tempStr + "ompi_info";
// 	impi = tempStr + "cpuinfo";
	mpt = tempStr + "mpiexec_mpt";
			
	mpvf = Utils::stat_exists(mpv);
	mpv2f = Utils::stat_exists(mpv2);
	ompif = Utils::stat_exists(ompi);
// 	impif = Utils::stat_exists(impi);
	mptf = Utils::stat_exists(mpt);
		
	if(mpvf){
// 	  cout << "In mpichversion" << endl;
	  Utils::get_system_string("mpichversion",libstring);
	}
	else if(mpv2f){
// 	  cout << "In mpich2version" << endl;
	  Utils::get_system_string("mpich2version",libstring, true);
	}
	else if(ompif){
	  Utils::get_system_string("ompi_info",libstring, true);
	}
// 	else if(impif){
// 	  Utils::get_system_string("cpuinfo",libstring, false);
// 	}
	else if(mptf){
	  Utils::get_system_string("mpiexec_mpt -v 2>&1", libstring, true);
	}
#ifdef __AIX__
	//else if( do stuff with lslpp -l ppe.poe
#endif
	else{
	  Utils::get_system_string("mpirun -V",libstring, true);
	}
	
	libstream.str(libstring.c_str());
	
#elif MPI_VERSION==3
// 	else if(major == 3){
	  err = MPI_Get_library_version(libstring, &leng);
// 	  cout << "Error output from MPI_Get_library_version is " << err << endl;
	  libstream.str(libstring);

#endif
	  
          libstream >> tempStr;
	  while(libstream){
	  	  
	  if(tempStr.compare("MPICH")==0 || tempStr.compare("MPICH2")==0 || tempStr.compare("MVAPICH2")==0){
	    mpiVendor = tempStr;
	    mpiName = tempStr;
	    libstream >> tempStr;
	    if(tempStr.compare("Version:")==0) {
	      libstream >> tempStr;
	      mpiVersion = tempStr;
	    }
	    break;
	  }
	  else if(tempStr.compare("Open")==0){
	    mpiVendor = "The Open MPI Project";
	    mpiName = "OpenMPI";
#if MPI_VERSION==3	    
	    libstream >> tempStr;
	    libstream >> tempStr;
	    tempStr.erase(0,1);
	    tempStr.erase(tempStr.length()-1,1);
	    mpiVersion = tempStr;
#else
	    while(tempStr.compare("MPI:")!=0){
	     libstream >> tempStr; 
	    }
	    libstream >> tempStr;
	    mpiVersion = tempStr;
#endif	    
	    break;
	  }
	  else if(tempStr.compare("Intel(R)")==0){
	    mpiVendor = "Intel";
	    mpiName = "IntelMPI";
            do{
	    libstream >> tempStr;
#if MPI_VERSION==3
            }while(tempStr.compare("Library")!=0);
#else
	    }while(tempStr.compare("Version")!=0);
#endif
	      mpiVersion = "";
	      libstream >> tempStr;
		mpiVersion += tempStr;
		libstream >> tempStr;
	      if(tempStr.compare("Update")==0){
		libstream >> tempStr;
	        mpiVersion += ".";
	        mpiVersion += tempStr;
	      }
	      libstream >> tempStr;
	      while(tempStr.compare("Build")!=0){
		if(libstream){
			libstream >> tempStr;
		}
		else{
			break;
		}
	      }
	      if(libstream){
	      	libstream >> tempStr;
	      	mpiVersion += "-";
	      	mpiVersion += tempStr;
	      }
	      break;
	    }
	  else if(tempStr.compare("MPT")==0){
	    mpiVendor = "SGI";
	    mpiName = "MPT";
// 	    while(!libstream.eof()){
// 	     libstream >> tempStr;
// 	     if(tempStr.compare("SGI")==0 || tempStr.compare("'SGI")==0){
// 	       libstream >> tempStr;
// 	       if(tempStr.compare("MPT")==0){
// 		  libstream >> tempStr;
// 	       }
// 	       mpiVersion = tempStr;
// 	     }
// 	    }
	    libstream >> tempStr;
	    mpiVersion = tempStr;
	    break;
	  }
	  else{
	    mpiName = "UNKNOWN";
	    mpiVendor = "UNKNOWN";
	    mpiVersion = "0";
	  }
	  libstream >> tempStr;
	}
}

ext_MPI::~ext_MPI() {
	// TODO Auto-generated destructor stub
}

int ext_MPI::printMPIInfo() {

	cout << "MPI Info - " << endl;
	cout << "MPI Vendor:  " <<   mpiVendor  << endl;
	cout << "MPI Name:    "	<<	 mpiName    << endl;
	cout << "MPI Version: " <<   mpiVersion << endl;
	cout << "MPI Standard: " << mpiStandard << endl;

	return 0;
}

const std::string& ext_MPI::getMpiName() const {
	return mpiName;
}

void ext_MPI::setMpiName(const std::string& mpiName) {
	this->mpiName = mpiName;
}

const std::string& ext_MPI::getMpiVendor() const {
	return mpiVendor;
}

void ext_MPI::setMpiVendor(const std::string& mpiVendor) {
	this->mpiVendor = mpiVendor;
}

const std::string& ext_MPI::getMpiVersion() const {
	return mpiVersion;
}

void ext_MPI::setMpiVersion(const std::string& mpiVersion) {
	this->mpiVersion = mpiVersion;
}

const std::string& ext_MPI::getMpiStandard() const {
  return mpiStandard;
}

void ext_MPI::setMpiStandard(const std::string& mpiStandard) {
  this->mpiStandard = mpiStandard;
}

void getMpiInfo(char* myMpiVendor, char* myMpiName, char* myMpiVersion, char* myMpiStandard) {
    std::string vendor, name, version, standard;
    ext_MPI tmpMPI;
    
    vendor = tmpMPI.getMpiVendor();
    name = tmpMPI.getMpiName();
    version = tmpMPI.getMpiVersion();
    standard = tmpMPI.getMpiStandard();
    
    strcpy(myMpiVendor, vendor.c_str());
    strcpy(myMpiName, name.c_str());
    strcpy(myMpiVersion, version.c_str());
    strcpy(myMpiStandard, standard.c_str());
    
}
