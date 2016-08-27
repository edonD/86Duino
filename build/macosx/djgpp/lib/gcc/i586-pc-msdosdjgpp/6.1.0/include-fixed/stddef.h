/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"/usr/local/djgpp-610/i586-pc-msdosdjgpp/sys-include/stddef.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */

/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_stddef_h_
#define __dj_include_stddef_h_

#ifdef __cplusplus
namespace std {
  extern "C" {
#endif

#include <sys/djtypes.h>

/* Some programs think they know better... */
#undef NULL
#if (__GNUC__ >= 4) && defined(__cplusplus)
#  define NULL          __null
#elif defined(__cplusplus)
#  define NULL          0
#else
#  define NULL          ((void*)0)
#endif

#ifdef __cplusplus
#define offsetof(s_type, mbr) ((std::size_t) &((s_type *)0)->mbr)
#else
#define offsetof(s_type, mbr) ((size_t) &((s_type *)0)->mbr)
#endif

#ifndef _PTRDIFF_T
#if !defined(_GCC_PTRDIFF_T)
#define _GCC_PTRDIFF_T
typedef __PTRDIFF_TYPE__ ptrdiff_t;
#endif

#define _PTRDIFF_T
#endif

#ifndef _SIZE_T
__DJ_size_t
#define _SIZE_T
#endif

#ifndef _WCHAR_T
__DJ_wchar_t
#define _WCHAR_T
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
  }
}
#endif

#endif /* !__dj_include_stddef_h_ */


#if defined(__cplusplus)
#ifndef __dj_via_cplusplus_header_

using std::ptrdiff_t;
using std::size_t;

#endif /* !__dj_via_cplusplus_header_ */
#endif /* __cplusplus */
