#define QUOTE(X) QUOTE_(X)
#define QUOTE_(X) #X

#if defined(__INTEL_COMPILER)
#  define COMPILER_VENDOR "Intel"
#  define COMPILER_NAME "Intel"
#  define COMPILER_VERSION QUOTE(__INTEL_COMPILER)
#elif defined(__SUNPRO_C)
#  define COMPILER_VENDOR "Oracle"
#  define COMPILER_NAME "Sun"
#  define COMPILER_VERSION QUOTE(__SUNPRO_C)
#elif defined(__GNUC__)
#  define COMPILER_VENDOR "GNU"
#  define COMPILER_NAME "GCC"
#  define COMPILER_VERSION __VERSION__
#elif defined(__IBMC__)
#  define COMPILER_VENDOR "IBM"
#  define COMPILER_NAME "XLC"
#  define COMPILER_VERSION __xlc__
#else
#  define COMPILER_VENDOR "Unknown"
#  define COMPILER_NAME "Unknown"
#  define COMPILER_VERSION "Unknown"
#endif

#include "check_meta.inc"
