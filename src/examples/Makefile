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
###############################################################################

BUILD_DIR ?= ${HOME}/PMTM/build/$(VERSION)
OUT_DIR   ?= ${HOME}/PMTM/$(VERSION)
ifdef LOCAL
  SUB_DIR = $(HPC_SYSTEM)/$(HPC_COMPILER)/$(HPC_MPI)
endif

all:

include ../Makefile.COMMON

###############################################################################

FULL_BUILD_DIR = $(BUILD_DIR)/$(HPC_SYSTEM)/$(HPC_COMPILER)/$(HPC_MPI)/examples
PMTM_LIBDIR    = $(OUT_DIR)/lib/$(SUB_DIR)
PMTM_INCDIR    = $(OUT_DIR)/include/$(SUB_DIR)
LIB_NAME       = PMTM

PROGS = c_example.x f_example.x c_lib_example.x f_lib_example.x
PROGS := $(addprefix $(FULL_BUILD_DIR)/,$(PROGS))

OBJ = c_example.o \
      c_lib_caller_example.o \
      c_lib_example.o \
      f_example.o \
      f_lib_caller_example.o \
      f_lib_example.o
OBJ := $(addprefix $(FULL_BUILD_DIR)/,$(OBJ))

ifdef DEBUG
  FOPT_OR_DEBUG = $(FDEBUG)
  COPT_OR_DEBUG = $(CDEBUG)
  LIB_NAME      = PMTM-debug
endif

all: examples moveexamples

ifeq "$(findstring clean, $(MAKECMDGOALS))" ""
  -include $(OBJ:.o=.d)
endif

examples: $(PROGS)

INCS := $(INC)$(PMTM_INCDIR) $(INC)$(FULL_BUILD_DIR)
LIBS := $(LIBDIR)$(PMTM_LIBDIR) $(LIB)$(LIB_NAME) $(FSTDLIBS) -Xlinker -rpath -Xlinker $(PMTM_LIBDIR)

moveexamples: examples
	@ mkdir -p $(OUT_DIR)/$(SUB_DIR)/examples
	@ cp $(PROGS) $(OUT_DIR)/$(SUB_DIR)/examples/

$(FULL_BUILD_DIR)/c_example.x: c_example.c $(FULL_BUILD_DIR)/flags.h
	$(MPICC) $(CFLAGS) -g -O0 -o $@ $(INCS) c_example.c $(LIBS)

$(FULL_BUILD_DIR)/c_lib_example.x: $(FULL_BUILD_DIR)/c_lib_example.o c_lib_example.h c_lib_caller_example.c
	$(AR) rv $(FULL_BUILD_DIR)/libC_Example.a $(FULL_BUILD_DIR)/c_lib_example.o
	$(MPICC) $(CFLAGS) -g -O0 -o $@ $(INCS) c_lib_caller_example.c $(LIBS) $(LIBDIR)$(FULL_BUILD_DIR) $(LIB)C_Example

$(FULL_BUILD_DIR)/f_example.x: f_example.F90 $(FULL_BUILD_DIR)/flags.h
	$(MPIFC) $(FFLAGS) -g -O0 -o $@ $(INCS) f_example.F90 $(LIBS)

$(FULL_BUILD_DIR)/f_lib_example.x: $(FULL_BUILD_DIR)/f_lib_example.o f_lib_caller_example.f90
	$(AR) rv $(FULL_BUILD_DIR)/libF90_Example.a $(FULL_BUILD_DIR)/f_lib_example.o
	$(MPIFC) $(FFLAGS) -g -O0 -o $@ $(INCS) f_lib_caller_example.f90 $(LIBS) $(LIBDIR)$(FULL_BUILD_DIR) $(LIB)F90_Example

.PHONY: flags.h
$(FULL_BUILD_DIR)/flags.h: flags.i
	sed 's/%COMPILE_FLAGS%/$(CFLAGS) $(FFLAGS) $(FOPT_OR_DEBUG)/g' flags.i > $(FULL_BUILD_DIR)/flags.h

$(FULL_BUILD_DIR)/%.o: %.c
	$(MPICC) $(CFLAGS) $(INCS) -g -O0 -o $@ -c $<

$(FULL_BUILD_DIR)/%.o: %.f90
	$(MPIFC) $(FFLAGS) $(INCS) -g -O0 -o $@ -c $<

clean:
	rm $(FULL_BUILD_DIR)/flags.h
	rm -f $(FULL_BUILD_DIR)/*.o
	rm -f $(FULL_BUILD_DIR)/*.d
	rm -f $(FULL_BUILD_DIR)/*~
	rm -f $(FULL_BUILD_DIR)/*.pmtm
	rm -f $(FULL_BUILD_DIR)/*.x
	rm -f $(FULL_BUILD_DIR)/*.mod
	rm -f $(FULL_BUILD_DIR)/lib*.a

