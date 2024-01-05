# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

#include Makefile.arch

#------------------------------------------------------------------------------
DEBUG = on

# Debug mode ?
ifeq ($(DEBUG),on)
	CXXFLAGS += -DCF__DEBUG -g
endif


RC     := root-config
ifeq ($(shell which $(RC) 2>&1 | sed -ne "s@.*/$(RC)@$(RC)@p"),$(RC))
MKARCH := $(wildcard $(shell $(RC) --etcdir)/Makefile.arch)
RCONFIG := $(wildcard $(shell $(RC) --incdir)/RConfigure.h)
endif
ifneq ($(MKARCH),)
include $(MKARCH)
else
ifeq ($(ROOTSYS),)
ROOTSYS = ..
endif
include $(ROOTSYS)/etc/Makefile.arch
endif

ROOTCFLAGS    = $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTLIBS      = $(shell $(ROOTSYS)/bin/root-config --libs)
ROOTGLIBS     = $(shell $(ROOTSYS)/bin/root-config --glibs)
 
CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -Wno-deprecated
LD            = g++
LDFLAGS       = -g 
SOFLAGS       = -shared

CXXFLAGS      += $(ROOTCFLAGS)
CXX           += -I./
LIBS           = $(ROOTLIBS) 

NGLIBS         = $(ROOTGLIBS) 
NGLIBS        += -lMinuit
GLIBS          = $(filter-out -lNew, $(NGLIBS))

CXX           += -I./
OUTLIB        = ../lib
MYLIBS        = ../lib


.SUFFIXES: .cc,.C
.PREFIXES: ./lib/


DAQCLASSES = Event.o EventDict.o Hit.o HitDict.o

#=======================================================================

DaqReader.o: DaqReader.cc
	$(CXX) $(CXXFLAGS) -c -I. -o  DaqReader.o $<

Event.o: Event.cxx
	$(CXX) $(CXXFLAGS) -c -I. -o  Event.o $<

Hit.o: Hit.cxx
	$(CXX) $(CXXFLAGS) -c -I. -o  Hit.o $<

DaqReaderMain.o: DaqReaderMain.cpp
	$(CXX) $(CXXFLAGS) -c -I. -o  DaqReaderMain.o $<

EventDict.cc: Event.h EventLinkDef.h
	@echo "Generating dictionary $@..."
	@rootcint -f $@ -c $^

HitDict.cc: Hit.h HitLinkDef.h
	@echo "Generating dictionary $@..."
	@rootcint -f $@ -c $^

EventDict.o: EventDict.cc
	$(CXX) $(CXXFLAGS) -c -I. -o  EventDict.o $<

HitDict.o: HitDict.cc
	$(CXX) $(CXXFLAGS) -c -I. -o HitDict.o $<

# Controllo se esiste ../lib,, se non c'è lo creo
create_lib_dir:
	@if [ ! -d ../lib ]; then \
		mkdir -p ../lib; \
		echo "Created ../lib directory"; \
	fi

#=======================================================================
dict: EventDict.cc HitDict.cc

obj: DaqReader.o Event.o Hit.o HitDict.o EventDict.o

shared: 
	$(CXX) $(SOFLAGS) $(CXXFLAGS) $(DAQCLASSES) $(ROOTGLIBS) -o  $(OUTLIB)/libEvent.so 

all: DaqReaderMain.cpp create_lib_dir dict obj shared

	$(CXX) $(CXXFLAGS)  ./*.o -o ../Reader.bin $(GLIBS) $(OUTLIB)/libEvent.so $< $(LIBS)

#=======================================================================
clean:
	rm -f *.o
	rm -f $(OUTLIB)/*.so
	rm -f ../Reader.bin
	rm -f *Dict.*
