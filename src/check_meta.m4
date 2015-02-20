/* This file is generated from check_meta.m4, do not manually edit. */

define(`CHECK',`dnl
#ifndef $1
#  error $1 must be defined
#endif')dnl

CHECK(`MACHINE_VENDOR')
CHECK(`MACHINE_NAME')

CHECK(`PROC_VENDOR')
CHECK(`PROC_NAME')
CHECK(`PROC_ARCH')
CHECK(`PROC_CLOCK')
CHECK(`PROC_CORES')
CHECK(`PROC_THREADS')

CHECK(`OS_VENDOR')
CHECK(`OS_NAME')
CHECK(`OS_VERSION')
CHECK(`OS_KERNEL')

CHECK(`MPI_NAME')
CHECK(`MPI_VENDOR')
CHECK(`MPI_LIB_VER')
