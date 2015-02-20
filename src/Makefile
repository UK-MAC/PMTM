###############################################################################
#                                                                             #
#              ######  ##### ##  ## ##   ##  ####  ######  ####               #
#                ##   ##     ##  ## ###  ## ##  ## ##     ##  ##              #
#                ##   ##     ###### ## # ## ###### ####   ######              #
#                ##   ##     ##  ## ##  ### ##  ## ##     ##  ##              #
#              ######  ##### ##  ## ##   ## ##  ## ###### ##  ##              #
#                                                                             #
# Makefile for the Ichnaea Performance Modelling Timing Module                #
#                                                                             #
#  For best use set HPC_SYSTEM, HPC_COMPILER and HPC_MPI to meaningful values #
#  then change BUILD_DIR and OUT_DIR to where you would like build and        #
#  install ICHNAEA                                                            #
#                                                                             #
#  Setting of Flags and other key compiler information is set in              #
#  Makefile.COMMON                                                            #
#                                                                             #
# History                                                                     #
# Version : When       : Who        : What                                    #
# 0.1     : 16/09/2010 : H          : Created                                 #
# 0.2     : 17/09/2010 : H          : Change test programs to use libraries   #
#         :            :            :  instead of linking against the objects #
# 0.3     : 15/03/2013 : M          : Change Makefile to build in a           #
#                                      standalone manner.                     #
#                                                                             #
###############################################################################

BUILD_DIR ?= $(HOME)/PMTM/build
OUT_DIR   ?= $(HOME)/PMTM
ifdef LOCAL
  SUB_DIR = $(HPC_SYSTEM)/$(HPC_COMPILER)/$(HPC_MPI)
endif

all:

include ./Makefile.COMMON

###############################################################################

FULL_BUILD_DIR = $(BUILD_DIR)/$(HPC_SYSTEM)/$(HPC_COMPILER)/$(HPC_MPI)
PMTM_LIBDIR    = $(OUT_DIR)/lib/$(SUB_DIR)
PMTM_INCDIR    = $(OUT_DIR)/include/$(SUB_DIR)
PMTM_DOCDIR    = $(OUT_DIR)/docs
ifdef WITH_OMP
LIB_NAME       = PMTM_openmp
else
LIB_NAME       = PMTM
endif
FULL_LIB_NAME  = $(PMTM_LIBDIR)/lib$(LIB_NAME).a
FULL_SO_NAME   = $(PMTM_LIBDIR)/lib$(LIB_NAME).so

LIB_OBJS    = $(FULL_BUILD_DIR)/pmtm.o \
              $(FULL_BUILD_DIR)/pmtm_internal.o \
              $(FULL_BUILD_DIR)/PMTM.o \
              $(FULL_BUILD_DIR)/linux_timers.o
ifdef PMTM_HW_COUNTERS
  LIB_OBJS += $(FULL_BUILD_DIR)/hardware_counters.o
endif
CHEADERS    = pmtm.h
FMODULES    = $(FULL_BUILD_DIR)/pmtm.mod

MODULE_NAME = pmtm

ifdef DEBUG
  FOPT_OR_DEBUG = $(FDEBUG)
  COPT_OR_DEBUG = $(CDEBUG)
  LIB_NAME      = PMTM-debug
endif

MCH_DEFS  = $(CDEF)MACHINE_VENDOR=$(PMTM_MACHINE_VENDOR) \
            $(CDEF)MACHINE_NAME=$(PMTM_MACHINE_NAME)
PROC_DEFS = $(CDEF)PROC_VENDOR=$(PMTM_PROC_VENDOR) \
            $(CDEF)PROC_NAME=$(PMTM_PROC_NAME) \
            $(CDEF)PROC_ARCH=$(PMTM_PROC_ARCH) \
			$(CDEF)PROC_CLOCK=$(PMTM_PROC_CLOCK) \
            $(CDEF)PROC_CORES=$(PMTM_PROC_CORES) \
            $(CDEF)PROC_THREADS=$(PMTM_PROC_THREADS)
#COMP_DEFS = $(CDEF)COMPILER_VENDOR=$(PMTM_COMPILER_VENDOR) \
#            $(CDEF)COMPILER_NAME=$(PMTM_COMPILER_NAME) \
#            $(CDEF)COMPILER_VERSION=$(PMTM_COMPILER_VERSION)
MPI_DEFS  = $(CDEF)MPI_NAME=$(PMTM_MPI_NAME)
MPI_DEFS  = $(CDEF)MPI_VENDOR=$(PMTM_MPI_VENDOR) \
            $(CDEF)MPI_NAME=$(PMTM_MPI_NAME) \
            $(CDEF)MPI_LIB_VER=$(PMTM_MPI_VERSION)
SYS_DEFS  = $(CDEF)SYSTEM_VENDOR=$(PMTM_OS_VENDOR) \
            $(CDEF)SYSTEM_NAME=$(PMTM_OS_NAME) \
            $(CDEF)SYSTEM_VERSION=$(PMTM_OS_VERSION) \
            $(CDEF)SYSTEM_KERNEL=$(PMTM_OS_KERNEL)

FFLAGS   += $(FFPIC) 
CFLAGS   += $(CFPIC) -g $(F2C_INC) $(CDEF)$(F2C_OPT) $(INC).\
            $(MCH_DEFS) $(PROC_DEFS) $(COMP_DEFS) $(MPI_DEFS) $(SYS_DEFS)

ifdef WITH_OMP
FFLAGS   += -g -O0 -openmp
CFLAGS   += -g -O0 -openmp
endif

ifdef HPC_MPI="serial"
FFLAGS   += $(FDEF)SERIAL
CFLAGS   += $(CDEF)SERIAL
endif

ifdef PMTM_HW_COUNTERS
  CFLAGS += $(CDEF)HW_COUNTERS $(LIB)papi
endif
CXXFLAGS += $(INC)catch -g -O0

-include $(FULL_BUILD_DIR)/F2C_conf

all: $(FULL_BUILD_DIR) lib config
	
#LD_LIBRARY_PATH=$(PMTM_LIBDIR):$$LD_LIBRARY_PATH $(MPI_RUN) $(MPI_NPS)4 ./tests.x;
test: FFLAGS += $(FDEBUG)
test: CFLAGS += $(CDEBUG)
test: $(FULL_BUILD_DIR)/tests.x $(FULL_BUILD_DIR)/ftests.x
	@ echo "Starting Tests for $(HPC_SYSTEM) $(HPC_COMPILER) $(HPC_MPI)"
	@ echo "Date: `date`"
	@ echo
	@ echo "Core Tests:"
	@ echo
	@ cd $(FULL_BUILD_DIR); \
	    if [ "$(HPC_MPI)" != "serial" ]; \
	    then \
	        $(MPI_RUN) $(MPI_NPS)4 ./tests.x; \
	        declare -i rank=0; \
	        while [ $$rank -lt 4 ]; \
	        do \
	            file="tests_rank_$${rank}.out"; \
	            cat $${file} | awk "{ print \"Rank $${rank}: \" \$$0 }"; \
	            rank+=1; \
	        done; \
	    else \
		LD_LIBRARY_PATH=$(PMTM_LIBDIR):$$LD_LIBRARY_PATH ./tests.x; \
	    fi
	@ echo
	@ echo "Fortran Wrapper Tests:"
	@ echo
	@ cd $(FULL_BUILD_DIR); \
		export NEW_PFUNIT="./ftests.x"; \
		export PMTM_DATA_STORE=; \
		export PMTM_KEEP_LOCAL_COPY=1; \
		./ftests.x; \
		rm fortran_tests_*.pmtm
	@ echo
	@ echo "Tests Complete"

docs: $(FULL_BUILD_DIR)/Doxyfile
	@-mkdir -p $(PMTM_DOCDIR)
	cd $(FULL_BUILD_DIR) && doxygen
	cd $(FULL_BUILD_DIR)/docs/latex && $(MAKE) all
	@ cp -f $(FULL_BUILD_DIR)/docs/latex/refman.pdf $(PMTM_DOCDIR)/

examples: all
	$(MAKE) -C examples

$(FULL_BUILD_DIR)/F2C_conf:
	@ echo "subroutine test" > tmp.f90; \
	  echo "end subroutine" >> tmp.f90; \
	  $(MPIFC) -o tmp.o -c tmp.f90; \
	  case `nm tmp.o | cut -d' ' -f3` in \
	      test_) F2C_OPT=F2C_UNDERSCORE;; \
	      test__) F2C_OPT=F2C_DOUBLEUNDERSCORE;; \
	      TEST) F2C_OPT=F2C_CAPS;; \
	      TEST_) F2C_OPT=F2C_CAPSUNDERSCORE;; \
	      *) F2C_OPT=F2C_NOCHANGE;; \
	  esac; \
	  echo "F2C_OPT = $$F2C_OPT" > $(FULL_BUILD_DIR)/F2C_conf; \
	  echo "Set F2C_OPT=$$F2C_OPT"; \
	  rm tmp.f90 tmp.o;

ifeq "$(findstring clean, $(MAKECMDGOALS))" ""
  -include $(FULL_BUILD_DIR)/tests.d $(FULL_BUILD_DIR)/pmtm.d
endif

$(FULL_BUILD_DIR):
	@ echo "Creating build directory"
	@-mkdir -p $(FULL_BUILD_DIR)
	@-mkdir -p $(FULL_BUILD_DIR)/examples

ifdef NOSHARED
lib: $(FULL_LIB_NAME)
else
lib: $(FULL_LIB_NAME) #$(FULL_SO_NAME)
endif

.PHONY: config
config:
	@ sed -e "s@%VERSION%@$(VERSION)@" \
		-e "s@%SYSTEM%@$(HPC_SYSTEM)@" \
		-e "s@%ARCH%@$(HPC_ARCH)@" \
		-e "s@%COMPILER%@$(HPC_COMPILER)@" \
		-e "s@%MPI%@$(HPC_MPI)@" \
		-e "s@%SVN_URL%@$(SVN_URL)@" \
		-e "s@%SVN_REV%@$(shell svn info $(SVN_URL) | grep Revision | cut -d' ' -f2)@" \
		-e "s@%DATE%@$(shell date)@" \
		-e "s@%SRC_DIR%@$(PWD)@" \
		-e "s@%BUILD_DIR%@$(FULL_BUILD_DIR)@" \
		-e "s@%CC%@$(CC)@" \
		-e "s@%CC_VERSION%@$(shell $(CCVERSION))@" \
		-e "s@%CFLAGS%@$(CFLAGS)@" \
		-e "s@%FC%@$(FC)@" \
		-e "s@%FC_VERSION%@$(shell $(FCVERSION))@" \
		-e "s@%FFLAGS%@$(FFLAGS)@" \
		-e "s@%F2C_DIR%@$(F2C_DIR)@" \
		-e "s@%F2C_DEF%@$(F2C_OPT)@" \
		lib$(LIB_NAME).config.template > $(PMTM_LIBDIR)/lib$(LIB_NAME).config

$(PMTM_LIBDIR):
	@-mkdir -p $(PMTM_LIBDIR)
	
$(PMTM_INCDIR): $(FMODULES) $(CHEADERS)
	@-mkdir -p $(PMTM_INCDIR)
	@ echo "Copying include files to $(PMTM_INCDIR)"
	@ cp -f $(FMODULES) $(CHEADERS) $(PMTM_INCDIR)

$(FULL_LIB_NAME): $(PMTM_LIBDIR) $(PMTM_INCDIR) $(LIB_OBJS)
	$(AR) rv $@ $(LIB_OBJS)
	@ echo
	@ echo "Library lib$(LIB_NAME).a built in $(PMTM_LIBDIR)"
	@ echo

$(FULL_SO_NAME): $(PMTM_LIBDIR) $(PMTM_INCDIR) $(LIB_OBJS)
	$(MPICC) $(CSHARED) $(CFLAGS) -o $@ $(LIB_OBJS)
	@ echo
	@ echo "Library lib$(LIB_NAME).so built in $(PMTM_LIBDIR)"
	@ echo

$(FULL_BUILD_DIR)/tests.x: tests.cpp lib
	$(MPICXX) $(CFLAGS) $(CXXFLAGS) -o $@ $< -L$(PMTM_LIBDIR) -l$(LIB_NAME) $(FSTDLIBS) -lrt

$(FULL_BUILD_DIR)/ftests.x: tests.F90 lib
	export PFUNIT=$(PFUNIT_DIR); \
		$(PFUNIT_DIR)/bin/wrapTest tests.F90 $(FULL_BUILD_DIR)/tests_wrap.F90
	$(PFUNIT_DIR)/bin/extract $< > $(FULL_BUILD_DIR)/suite_list
	cpp -P -I$(PFUNIT_DIR)/include tests.F90 > $(FULL_BUILD_DIR)/tests.f90
	cpp -P -I$(PFUNIT_DIR)/include $(FULL_BUILD_DIR)/tests_wrap.F90 > $(FULL_BUILD_DIR)/tests_wrap.f90
	$(MPIFC) $(FFLAGS) -o $(FULL_BUILD_DIR)/tests.o -c $(FULL_BUILD_DIR)/tests.f90 \
		-I$(PMTM_INCDIR) -I$(PFUNIT_DIR)/mod
	$(MPIFC) $(FFLAGS) -o $(FULL_BUILD_DIR)/tests_wrap.o -c $(FULL_BUILD_DIR)/tests_wrap.f90 \
		-I$(PMTM_INCDIR) -I$(PFUNIT_DIR)/mod
	cpp -P $(PFUNIT_DIR)/include/driver.F90 -I$(FULL_BUILD_DIR) -I$(PFUNIT_DIR)/include -DHAS_CONCATENATION_OPERATOR \
		> $(FULL_BUILD_DIR)/driver.f90
	$(MPIFC) $(FFLAGS) -o $@ $(FULL_BUILD_DIR)/driver.f90 $(FULL_BUILD_DIR)/tests.o $(FULL_BUILD_DIR)/tests_wrap.o \
		-I$(PMTM_INCDIR) -I$(PFUNIT_DIR)/mod -L$(PMTM_LIBDIR) -l$(LIB_NAME) -L$(PFUNIT_DIR)/lib -lpfunit -lrt

$(FULL_BUILD_DIR)/pmtm.mod: $(FULL_BUILD_DIR)/PMTM.o

$(FULL_BUILD_DIR)/%.o: %.F90
	$(MPIFC) $(FFLAGS) $(FMODDIR)$(FULL_BUILD_DIR) -o $@ -c $<

$(FULL_BUILD_DIR)/%.o: %.c
	$(MPICC) $(CFLAGS) -o $@ -c $<

$(FULL_BUILD_DIR)/%.o: %.cpp
	$(MPICXX) $(CFLAGS) $(CXXFLAGS) -o $@ -c $<

$(FULL_BUILD_DIR)/%.d: %.c $(FULL_BUILD_DIR) $(FULL_BUILD_DIR)/F2C_conf
	$(MPICXX) $(CFLAGS) $(CXXDEPENDENCY) $< > $@

$(FULL_BUILD_DIR)/%.d: %.cpp $(FULL_BUILD_DIR) $(FULL_BUILD_DIR)/F2C_conf
	$(MPICXX) $(CFLAGS) $(CXXFLAGS) $(CXXDEPENDENCY) $< > $@

$(FULL_BUILD_DIR)/Doxyfile: $(FULL_BUILD_DIR)
	@ cp -f Doxyfile $(FULL_BUILD_DIR)/

clean:
	rm -rf $(FULL_BUILD_DIR)
	if [ -z "`ls $(BUILD_DIR)`" ]; then rmdir ../build; fi

cleaner:
	rm -rf $(PMTM_LIBDIR) $(PMTM_INCDIR)

cleanest: clean cleaner

cleanall:
	rm -rf $(INSTALL_DIR)
