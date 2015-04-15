# serial 1 flagrtb.m4
AC_DEFUN([AX_CHECK_FLAGRTB],
[AC_PREREQ([2.65])dnl
AC_ARG_WITH([flagrtb],
            AC_HELP_STRING([--with-flagrtb=DIR],
                           [Location of flagRTB headers/libs (/usr/local)]),
            [FLAGRTBDIR="$withval"],
            [FLAGRTBDIR=/usr/local])

orig_LDFLAGS="${LDFLAGS}"
LDFLAGS="${orig_LDFLAGS} -L${FLAGRTBDIR}/lib"
AC_CHECK_LIB([flagrtb], [flagrtbInit],
             # Found
             AC_SUBST(FLAGRTB_LIBDIR,${FLAGRTBDIR}/lib),
             # Not found there, check FLAGRTBDIR
             AS_UNSET(ac_cv_lib_flagrtb_flagrtbInit)
             LDFLAGS="${orig_LDFLAGS} -L${FLAGRTBDIR}"
             AC_CHECK_LIB([FLAGRTB], [flagrtbInit],
                          # Found
                          AC_SUBST(FLAGRTB_LIBDIR,${FLAGRTBDIR}),
                          # Not found there, error
                          AC_MSG_ERROR([flagRTB library not found])))
LDFLAGS="${orig_LDFLAGS}"

AC_CHECK_FILE([${FLAGRTBDIR}/include/flagrtb.h],
              # Found
              AC_SUBST(FLAGRTB_INCDIR,${FLAGRTBDIR}/include),
              # Not found there, check FLAGRTBDIR
              AC_CHECK_FILE([${FLAGRTBDIR}/flagrtb.h],
                            # Found
                            AC_SUBST(FLAGRTB_INCDIR,${FLAGRTBDIR}),
                            # Not found there, error
                            AC_MSG_ERROR([FLAGRTB.h header file not found])))
])

dnl Calls AX_CHECK_FLAGRTB and then checks for and uses flagrtbinfo to define the
dnl following macros in config.h:
dnl
dnl   FLAGRTB_NSTATION   - Number of dual-pol(!) stations per flagRTB instance
dnl   FLAGRTB_NFREQUENCY - Number of frequency channels per flagRTB instance
dnl   FLAGRTB_NTIME      - Number of time samples per freqency channel per flagRTB
dnl                     instance
dnl
AC_DEFUN([AX_CHECK_FLAGRTBINFO],
[AC_PREREQ([2.65])dnl
AX_CHECK_FLAGRTB
AC_CHECK_FILE([${FLAGRTBDIR}/bin/flagrtbinfo],
              # Found
              AC_SUBST(FLAGRTB_BINDIR,${FLAGRTBDIR}/bin),
              # Not found there, check FLAGRTBDIR
              AC_CHECK_FILE([${FLAGRTBDIR}/flagrtbinfo],
                            # Found
                            AC_SUBST(FLAGRTB_BINDIR,${FLAGRTBDIR}),
                            # Not found there, error
                            AC_MSG_ERROR([flagrtbinfo program not found])))

AC_DEFINE_UNQUOTED([FLAGRTB_NSTATION],
                   [`${FLAGRTB_BINDIR}/flagrtbinfo | sed -n '/Number of stations: /{s/.*: //;p}'`],
                   [Number of stations == Ninputs/2])

AC_DEFINE_UNQUOTED([FLAGRTB_NFREQUENCY],
                   [`${FLAGRTB_BINDIR}/flagrtbinfo | sed -n '/Number of frequencies: /{s/.*: //;p}'`],
                   [Number of frequency channels per flagRTB instance])

AC_DEFINE_UNQUOTED([FLAGRTB_NTIME],
                   [`${FLAGRTB_BINDIR}/flagrtbinfo | sed -n '/time samples per GPU integration: /{s/.*: //;p}'`],
                   [Number of time samples (i.e. spectra) per flagRTB integration])
])
