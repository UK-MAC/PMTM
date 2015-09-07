//============================================================================
// Name        : trial.c
// Author      : Iain Miller
// Version     :
// Copyright   : Your copyright notice
// Description : Trial of calling C++ from C
//============================================================================

// #include <mpi.h>
// #include <iostream>
#include <stdio.h>
#include "OS.h"
#include "Compiler.h"
#include "ext_MPI.h"
#include "Processor.h"

// using namespace std;

int main(int argc, char* argv[]) {

	char vendor[1024],name[1024],version[1024],kernel[1024],node[1024];
        char compVendor[1024],compName[1024],compVersion[1024];
        char mpiVendor[1024],mpiName[1024],mpiVersion[1024],mpiStandard[1024];
        char procVendor[1024],procName[1024],procArch[1024];
        int procClock,procCores,procThreads;
        
        getOSInfo(vendor,name,version,kernel,node);
        
        printf("My Vendor is: \t%s\n", vendor);
        printf("My Name is: \t%s\n", name);
        printf("My Version is: \t%s\n", version);
        printf("My Kernel is: \t%s\n", kernel);
        printf("My Node is: \t%s\n", node);
        
        getCompInfo(compVendor,compName,compVersion);
        
        printf("Compiler Info - \n" );
        printf("Compiler Vendor: \t%s\n", compVendor);
        printf("Compiler Name: \t%s\n", compName);
        printf("Compiler Version: \t%s\n", compVersion);
        
        getMpiInfo(mpiVendor,mpiName,mpiVersion,mpiStandard);
        
        printf("MPI Info - \n"  );
        printf("MPI Vendor:  \t%s\n",    mpiVendor  );
        printf("MPI Name:    \t%s\n",        mpiName    );
        printf("MPI Version: \t%s\n",    mpiVersion );
        printf("MPI Standard: \t%s\n",  mpiStandard );
        
        getProcInfo(procVendor,procName,procArch,&procClock,&procCores,&procThreads);
        
        printf("Processor Info - \n");
        printf("Vendor: \t%s\n", procVendor);
        printf("Name: \t%s\n", procName);
        printf("Arch: \t%s\n", procArch);
        printf("Cores: \t%d\n", procCores);
        printf("Threads: \t%d\n", procThreads);
        printf("Clock rate: \t%d\n", procClock);

	return 0;
}
