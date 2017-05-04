#ifndef __PREDEF_H__
#define __PREDEF_H__

#if   defined(_WIN32)      \
   || defined(_WIN64)      \
   || defined(__WIN32__)   \
   || defined(__TOS_WIN__) \
   || defined(__WINDOWS__)

#  define PREDEF_OS_WINDOWS

#endif

#ifdef PREDEF_OS_WINDOWS
#  define _CRT_SECURE_NO_WARNINGS
#else
#  define USE_CUSTOM_ALIGN
#endif

#ifndef _FILE_OFFSET_BITS
#  define _FILE_OFFSET_BITS 64
#endif

#endif
