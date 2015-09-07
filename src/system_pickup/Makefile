#if defined(__INTEL_COMPILER)
MPICXX = mpiicpc
#elif defined(__xlc__)
MPICXX = mpCC_r
#else
MPICXX = mpicxx
#endif


all: check_sys

clean:
	-rm check_sys.x *.o
	
check_sys: check_sys.o ext_MPI.o Compiler.o OS.o Processor.o Utils.o
	$(MPICXX) -g -o check_sys.x check_sys.o ext_MPI.o Compiler.o OS.o Processor.o Utils.o
	
check_sys.o: ext_MPI.o Compiler.o OS.o Utils.o
	$(MPICXX) -c -g check_sys.cpp 
		
ext_MPI.o: Utils.o
	$(MPICXX) -c -g ext_MPI.cpp -I./
	
Compiler.o: Utils.o
	$(MPICXX) -c -g Compiler.cpp -I./
	
OS.o: Utils.o
	$(MPICXX) -c -g OS.cpp -I./
	
Processor.o: Utils.o
	$(MPICXX) -c -g Processor.cpp -I./
	
Utils.o:
	$(MPICXX) -c -g Utils.cpp
