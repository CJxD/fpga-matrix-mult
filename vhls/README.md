Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

Steps to build the PRAZOR/VHLS simulator:

First you need to create configure script in order to build simulator.
You can do this by changing directory to vhls and invoking autoreconf. Once
autoreconf has finished you should have configure script in vhls directory.

To build simulator cd to vhls and get everything set up such that
./configure and make work ok. The steps to set everything up are as follows:

(1) Add boost library >= 1.48
Setup a path to BOOST using something like
export BOOST_ROOT=/usr/local/boost/boost_1_48_0

# If boost is not built you will need to cd to it and do something like
  $ chmod +x ./bootstrap.sh 
  $ chmod +x tools/build/v2/engine/*.sh
  $ ./boostrap.sh
  $ ./b2
# Note you may need to chmod +x some of the .sh files to make it build.


(2) Install SystemC library ( generally in usr/local), if it cannot be found by ./configure then
use something like 

export   SYSTEMC=/usr/local/systemc/systemc-2.3.0
export   LDFLAGS="-L$SYSTEMC/lib-linux64   -L/usr/local/lib"
export   CXXFLAGS="-I$SYSTEMC/include"


If there are problems make sure you have compiled and installed SystemC for your architecture. You might need to make a symlink
as follows
 $ cd $SYSTEMC
 $ make install
 $ ln -s lib-linux64 lib-x86_64


(3) Build tlm-power3 library (separate distribution) - if you dont want to install it in /use/local then adjust
the paths in step 4 accordingly. You do not need to 'install' it at all if you adjust your TLM_POWER3 setting for the following step to reflect where it resides (e.g. in your own folder).
	* autoreconf
	* LDFLAGS="-L/usr/local/lib-linux64" CXXFLAGS="-I/usr/local/include/tlm_core/tlm_2" ./configure --prefix=/usr/local
	* make
	* make install


The prazor simulator is supposed to compile with and without TLM_POWER3 installed, but
it might currently fail on some dependancies - insufficient ifdefing - without it. 
TLM_POWER3 adds overhead to the execution of the simulator which is not always wanted.  The hamming
distance tracking for mode 3 adds can add on a considerable overhead if the confidence switcher
is set to high accuracy. 
Without it you will need definitions of the following macros
#define  POWER3(X)            /* nothing */
typedef int pw_customer_acct; /*use a simple int as a placeholder */
#define PW_TLM3(X)            /* nothing */
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;



If TLM_POWER3 is not found or if "--with-tlm-power=no" is passed to ./configure it will be disabled. But you need
to make sure you unset TLM_POWER3 in your shell environment since the same identifier is used by the C++ preprocessor
to direct compilation.

(4) Build simulator in vhls directory using something like this (bash syntax used)
  autoreconf
  export TLM_POWER3=/usr/local/lib/tlm-power3
  export LDFLAGS="-L$SYSTEMC/lib-linux64 -L/usr/local/lib -L$TLM_POWER3/src/.libs"
  export CXXFLAGS="-I$SYSTEMC/include/ -I$BOOST_ROOT -I$SYSTEMC/include/tlm_core/tlm_2 -I$TLM_POWER3/include -g -O2"
  ./configure
  make 
  (optionally) make install

If you require adding your own TLM models to platform targets such as the Zynq or OR1K, use ./configure --enable-plugins rather than just ./configure. This will allow you to specify your own TLM models in the src/plugins directory (currently only supports header files). See the examples and modify the plugins.h header to include your plugin. You must add the following definitions to your own TLM models:

  #ifdef USE_PLUGINS
  #define PLUGIN_NAME "<your_plugin_name>"
  #define PLUGIN_MODULE <systemc_module_name>
  #endif

Rather then install it you can simply implement your hardware configuration that you want to simulate in the platform directy and the pass to the binary library that was built in .libs directory of your implementation using the -platform flag.

(5) Run any example from the images directory.  

You should find some example invocations in the Makefiles in the images folders. The DRAM simulator requires a pair of args for its configuration files and you need an ELF binary to load into the DRAM.  Ther emay be some precompiled binaries in the images folder.  The binary you want depends on the architecture you chose: mips, x86, openrisc, arm etc..

You will separately need binutils and gcc for the architecture you chose for when you recompile or make your own binary images. You will have the x86_64 standard binutils on your workstation/laptop most likely from building the simulator and general C++ programming.

$ cd images/hello-world/
$ file hello
    hello: ELF 32-bit MSB executable, OpenRISC, version 1 (SYSV), statically linked, not stripped
$ ls -l hello
-rwxrwxr-x. 1 djg11 djg11 343653 Sep  2 08:25 hello
$ /home/djg11/d320/prazor/trunk/vhls/src/vhls -platform $(VHLS)/src/platform/risc/or1k/.libs/libor1k.so -dram-system-ini /home/djg11/d320/prazor/trunk/vhls/src/dramsim2/dist/system.ini.example  -dram-device /home/djg11/d320/prazor/trunk/vhls/src/dramsim2/dist/ini/DDR3_micron_8M_8B_x16_sg15.ini -cores 1 -name vv -image ./hello -- red yellow green blue well done 
  ... an Xwindow console may pop up if X is enabled
$ cat vv.the_top.uart0.txt 
Hello World
$



END


