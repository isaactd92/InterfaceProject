#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/lib
export PATH=/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/lib:$PATH
export LD_LIBRARY_PATH=/home/jrojas/OutOfTree-gnuradio/gr-InterfaceJRO_7/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH
test-InterfaceJRO_7 
