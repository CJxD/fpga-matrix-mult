export VHLS=/home/hc475/P35/sim


export CLTEACH=/usr/groups/han/clteach
export BOOST=$CLTEACH/boost/boost_1_48_0
export BOOST_ROOT=$BOOST
export SYSTEMC=$CLTEACH/systemc/systemc-current
export TLM_POWER3=$CLTEACH/tlm-power3
export LDFLAGS="-L$SYSTEMC/lib-linux64 -L/usr/local/lib -L$TLM_POWER3/src/.libs"
export CXXFLAGS="-I$SYSTEMC/include/ -I$BOOST_ROOT -I$SYSTEMC/include/tlm_core/tlm_2 -I$TLM_POWER3/include -g -O2"
export TARCH=ARM32

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/groups/han/clteach/tlm-power3/lib-linux64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SYSTEMC/lib-linux64

