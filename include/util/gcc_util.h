#ifndef _gcc_util_h
#define _gcc_util_h 1

// Credit: https://stackoverflow.com/questions/3378560/how-to-disable-gcc-warnings-for-a-few-lines-of-code
//
#  define DIAG_STR(s) #s
#  define DIAG_JOINSTR(x,y) DIAG_STR(x ## y)

#  define DIAG_DO_PRAGMA(x) _Pragma (#x)
#  define DIAG_PRAGMA(compiler,x) DIAG_DO_PRAGMA(compiler diagnostic x)

#  if defined(__GNUC__)
#    if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#      define DISABLE_WARNING(gcc_option) DIAG_PRAGMA(GCC,push) DIAG_PRAGMA(GCC,ignored DIAG_JOINSTR(-W,gcc_option))
#      define  ENABLE_WARNING(gcc_option) DIAG_PRAGMA(GCC,pop)
#    else
#      define DISABLE_WARNING(gcc_option) DIAG_PRAGMA(GCC,ignored DIAG_JOINSTR(-W,gcc_option))
#      define  ENABLE_WARNING(gcc_option) DIAG_PRAGMA(GCC,warning DIAG_JOINSTR(-W,gcc_option))
#    endif
#  endif

// Cleaning after developers of glog 0.5.0:
#ifndef __SANITIZE_THREAD__
#define __SANITIZE_THREAD__ 0
#endif

#ifndef STRIP_LOG
#define STRIP_LOG 0
#endif

#endif
