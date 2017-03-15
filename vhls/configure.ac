# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> 

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.67])
AC_INIT([vhls], [0.1])
AM_INIT_AUTOMAKE([foreign dist-bzip2 subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])

AM_PROG_AS

m4_include([ax_boost_base.m4])
m4_include([ax_check_compile_flag.m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL


# Check for C++0x and C++11 features
AX_CHECK_COMPILE_FLAG([-std=c++0x], [],
  [AC_MSG_ERROR([Your compiler does not have C++0x support])])
#AX_CHECK_COMPILE_FLAG([-std=c++11], [],
#  [AC_MSG_ERROR([Your compiler does not have C++11 support])])

# build both shared libraries and static libraries
LT_INIT
AC_ENABLE_SHARED
AC_ENABLE_STATIC

# Check for boost
AX_BOOST_BASE([1.48], 
              [], 
              [AC_MSG_ERROR([Unable to find Boost library >= 1.48])] )

# SystemC check
AC_LANG_PUSH([C++])
AC_CHECK_HEADER([systemc.h], [systemc_header_found=yes])
AS_IF([test "x$systemc_header_found" != "xyes"],
	    [AC_MSG_ERROR([Unable to find the SystemC headers])])
AC_LANG_POP([C++])

printf "checking for SystemC libraries..."
SAMPLE_SYSTEMC_PROG="#include<systemc.h>
int sc_main(int argc, char **argv) { 
sc_version(); 
}"
echo "${SAMPLE_SYSTEMC_PROG}" > test.cpp
g++ ${LDFLAGS} ${CPPFLAGS} ${CXXFLAGS} test.cpp -o test -lsystemc ${LIBS} 2> compile.log
if test $? -ne 0; then
  rm -f test.cpp test compile.log
  AC_MSG_ERROR([Unable to find the SystemC libraries])
fi
printf " yes\n"
rm -f test.cpp test compile.log

# Verilator check
#AC_ARG_VAR([VERILATOR],
#	   [Path to Verilator distribution])
#if test -z "${VERILATOR}"; then
#   AC_MSG_ERROR([Path to the verilator distribution not specified])
#fi

#CPPFLAGS+=" -I${VERILATOR}/include"

#AC_LANG_PUSH([C++])
#AC_CHECK_HEADER([verilated.h], [verilator_header_found=yes])
#AS_IF([test "x$verilator_header_found" != "xyes"],
#	    [AC_MSG_ERROR([Unable to find Verilator headers])])
#AC_CHECK_HEADER([verilated.cpp], [verilator_cpp_found=yes])
#AS_IF([test "x$verilator_cpp_found" != "xyes"],
#	    [AC_MSG_ERROR([Unable to find Verilator CPP file])])
#AC_LANG_POP([C++])

printf "checking for TLM 2 headers..."
SAMPLE_SYSTEMC_PROG="#include <systemc>
#include <tlm_generic_payload/tlm_generic_payload.h>
int sc_main(int argc, char **argv) {
}"	  
echo "${SAMPLE_SYSTEMC_PROG}" > test.cpp
g++ ${LDFLAGS} ${CPPFLAGS} ${CXXFLAGS} test.cpp -o test -lsystemc ${LIBS} 2> compile.log
if test $? -ne 0; then
   cp compile.log /tmp
   rm -f test.cpp test compile.log
   printf " nope\n" 
   AC_MSG_ERROR([Unable to find the TLM 2 headers])
fi
printf " yes\n"
rm -f test.cpp test compile.log

# Please add a comment here about underscore versus hyphen.
AC_ARG_WITH([tlm_power],
	[AC_HELP_STRING([--with-tlm-power], [Use TLM POWER 3 library @<:@default=check@:>@])],
	[],
	[with_tlm_power=check])

tlm_power_cond=false
AS_IF([test "x$with_tlm_power" != "xno"],
      [
      printf "checking for TLM POWER 3 headers..."
      SAMPLE_SYSTEMC_PROG="#include <systemc>
      #include <tlm_power>
      int sc_main(int argc, char **argv) {
      }"	  
      echo "${SAMPLE_SYSTEMC_PROG}" > test.cpp
      g++ ${LDFLAGS} ${CPPFLAGS} ${CXXFLAGS} -std=c++0x test.cpp -o test -lsystemc ${LIBS} 2> compile.log
      if test $? -ne 0; then
      	 cp compile.log /tmp
      	 rm -f test.cpp test compile.log
	 printf " nope\n"
	 if test "x$with_tlm_power" != "xcheck"; then
            AC_MSG_ERROR([Unable to find the TLM POWER 3 headers])
         fi
      else
	printf " yes\n"
        rm -f test.cpp test compile.log
          printf "checking for TLM POWER 3 libraries..."
          SAMPLE_SYSTEMC_PROG="#include <systemc>
          #include <tlm_power>
          using namespace sc_core;
          using namespace sc_pwr;
          int sc_main(int argc, char **argv) {
              pw_set_power_resolution(1, PW_fW);
              pw_set_energy_resolution(1, PW_fJ);
          }"	  
          echo "${SAMPLE_SYSTEMC_PROG}" > test.cpp
	  g++ ${LDFLAGS} ${CPPFLAGS} ${CXXFLAGS} -std=c++0x test.cpp -o test -lsystemc -ltlm-power3 ${LIBS} 2> compile.log
          if test $? -ne 0; then
   	     cp compile.log /tmp
      	     rm -f test.cpp test compile.log
	     printf " nope\n"
	     if test "x$with_tlm_power" != "xcheck"; then
	       AC_MSG_ERROR([Unable to find the TLM POWER 3 libraries])
	     fi
           else
             printf " yes\n"
             rm -f test.cpp test compile.log
	     AC_DEFINE(TLM_POWER3)
	     AC_DEFINE(PW_TLM_PAYLOAD, 3)
	     AC_SUBST(POWER_LIBRARY, ["-ltlm-power3"])
	     tlm_power_cond=true
	   fi
         fi
      ],[])

AM_CONDITIONAL([TLMPOWER3],      [test "x$tlm_power_cond" == "xtrue"])

AC_ARG_WITH([speedo],
	[AC_HELP_STRING([--with-speedo], [Use spEEDO power interface library @<:@default=check@:>@])],
	[],
	[with_speedo=check])

#The following speedo control code is probably not correct - djg - need to check it please.
speedo_cond=false

# Change spEEDO default when TLM_POWER3 is being used.
AS_IF([test "x$tlm_power_cond" == "xtrue"], [speedo_cond=true])

echo "Reporting: TLM_POWER3 included is x$tlm_power_cond"
echo "Reporting: Speedo included is x$speedo_cond"

AS_IF([test "x$with_speedo" == "xno"],
      [speedo_cond = false]
     )

AS_IF([test "x$speedo_cond" == "xtrue"],
      [AC_DEFINE(WITH_SPEEDO)]
      [])

AM_CONDITIONAL([WITH_SPEEDO], [test "x$speedo_cond" == "xtrue"])

AC_ARG_ENABLE([plugins],
	[AC_HELP_STRING([--enable-plugins], [Include plugins from the plugins folder in the build process.])],
	[AC_DEFINE([USE_PLUGINS], [1], [Define if plugins enabled.])],
	[])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([floor inet_ntoa memset mkdir socket sqrt strchr strerror strndup strtol])

AC_OUTPUT([Makefile src/Makefile src/tenos/Makefile src/bus/Makefile src/io/Makefile src/dramsim2/dist/Makefile src/dramsim2/Makefile src/or1k/verilated/Makefile src/or1k/verilated/vcpp/Makefile src/or1k/fastiss/Makefile src/memories/Makefile src/gdbrsp/Makefile src/or1k/Makefile src/x86/Makefile tests/Makefile src/arm/Makefile src/mips64/Makefile src/mips64/mips64isa/Makefile src/platform/Makefile src/platform/arm/Makefile src/platform/arm/zynq/Makefile src/platform/arm/zynq/parallella/Makefile src/platform/x86/Makefile src/platform/x86/amd/Makefile src/platform/x86/amd/phenom/Makefile src/platform/risc/Makefile src/platform/risc/or1k/Makefile])
