/* -*- c++ -*- */

#define INTERFACEJRO_7_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "InterfaceJRO_7_swig_doc.i"

%{
#include "InterfaceJRO_7/InterfaceJRO_7.h"
%}


%include "InterfaceJRO_7/InterfaceJRO_7.h"
GR_SWIG_BLOCK_MAGIC2(InterfaceJRO_7, InterfaceJRO_7);
