//============================================================================
// Name        : check_sys.cpp
// Author      : Iain Miller
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <mpi.h>
#include <iostream>
#include "OS.h"
#include "Compiler.h"
#include "ext_MPI.h"
#include "Processor.h"

using namespace std;

int main(int argc, char* argv[]) {

	MPI_Init(&argc, &argv);

	OS Bob;
	Compiler Sue;
	ext_MPI Kate;
	Processor June;

	Bob.printOSInfo();
	Sue.printCompilerInfo();
	Kate.printMPIInfo();
	June.printProcessorInfo();

	MPI_Finalize();

	return 0;
}
