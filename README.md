# Simulated Matrix Multiplication

In this folder you will find:

* env.sh - A convenience script to set environment variables. Before building anytihng, call `source ./env.sh`
* pure/ - A pure C implementation of the matrix multiplier. Build with `make`.
* tlm/ - A SystemC + TLM 2.0 implementation. Build with `make`. Run with `make run`
* prazor-tests/ - A test suite that runs the TLM implementation as an ARM binary on the Prazor virtual platform. Build with `make`. Run with `make run`.
* utils/ - Utilities for energy monitoring and Zynq support.
* vhls/ - The Prazor virtual platform code with plugin support.

VHLS is already pre-built for Linux x86\_64, but to re-build VHLS, configure with:

	# First time commands
    aclocal
	autoconf
	automake

	# Reconfigure command
	./configure --enable-plugins

	# Rebuild
    make

As long as the environment variables point to the right places in env.sh, VHLS will build with TLM POWER 3 and spEEDO. To build without POWER 3 and spEEDO, configure with the following flags:

	./configure --enable-plugins --with-tlm-power=no --with-speedo=no

## Energy statistics

To include energy usage, build TLM simulation models with `USE_ENERGYSHIM=1 make`. On real hardware with a compatible energy probe connected, build with `USE_ENERGYSHIM=1 NATIVE=1 make` and run with `USE_ENERGYSHIM=1 CURRENTPROBE=<hostname> make run` where `<hostname>` is the network name of the probe, such as `bognor.sm.cl.cam.ac.uk`

For all energy statistics, energyshim must be compiled in. For energy statistics from a current probe, currentprobe-client must also be compiled in.

Since the `NATIVE` environment variable only affects the compilation of the energyshim, as long as matMul.o is compiled with `USE_ENERGYSHIM=1`, the object file can be re-used for compilation on real hardware for direct comparison. 
