# Defaults

VERS  = 2.6.0
INDIR = 
INFLG = 
QADIR = 
DEBUG = 
SOCMD = all

ifneq ($(INDIR),)
  INFLG=-i $(INDIR)
endif

help:
	#
	# Installer for PMTM.
	#
	# Usage
	#
	#   make [pmtm|clean]  \
	#                [VERS=<vers>] [INDIR=<path/to/install>] [QADIR=<path/to/QA>]
	#  
	#     Required arguments:
	#       pmtm          - Perform a standalone build using current mpi/compiler modules
	#
	#       clean         - remove build files
	#
	#     Optional arguments:
	#
	#       VERS  - The version number that the library will be installed under
	#       INDIR - The path to the installation directory. 
	#               This will be expanded out to:
	#
	#                 $$(INDIR)/$$(HPC_COMPILER)/$$(HPC_MPI)/$$(VERS)
	#
	#       QADIR - The path to the QA directory (default is ../QA)
	#

pmtm:
	@ echo "================================================================"
	@ echo "Installing PMTM."
	@ echo "Version       = " $(VERS)
	@ echo "Compiler      = " $(HPC_COMPILER)
	@ echo "MPI           = " $(HPC_MPI)
	@ echo "BASEDIR       = " $(BASEDIR)
	@ echo "Installing to = " $(INSTDIR)
	@ echo "================================================================"
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERS) SVN_URL=$(SVN_URL) $(SOCMD)

serial:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERS) SVN_URL=$(SVN_URL) all SERIAL=1

test:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERS) SVN_URL=$(SVN_URL) test

testtoweb:
	$(MAKE) -C src OUT_DIR=$(INSTDIR) VERSION=$(VERS) SVN_URL=$(SVN_URL) testtoweb

docs:
	mkdir -p docs/usage_manual
	doxygen usage_manual.doxy
	cp -R docs $(INSTDIR)/

clean:
	$(MAKE) -C src clean

cleanall:
	$(MAKE) -C src cleanall INSTALL_DIR=$(INSTDIR)

cleandir:
	$(MAKE) -C src cleandir

cleandocs:
	rm -rf docs

