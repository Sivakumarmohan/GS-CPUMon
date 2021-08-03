# Makefile for CPU Monitor
# Author: Siva Mohan
# ***************************************
# Variables
#
CC = g++
CFLAGS = -Wall -Werror -g
SRC_FILES = CpuMonApp.cpp \
            Database.cpp  \
	    Mqtt.cpp      
OBJ_FILES = CpuMonApp.o \
            Database.o  \
	    Mqtt.o      
INC_FILES = `wx-config --cxxflags`
LIBS = `wx-config --libs`
LIBS := $(LIBS) -lsqlite3 -lmosquitto
CPPFLAGS = 
SHELL := /bin/bash
# ***************************************
# Targets

all: $(OBJ_FILES) 
	$(CC) -o CpuMon $(LIBS) $(OBJ_FILES)
$(OBJ_FILES): $(SRC_FILES)
	source version.sh	
	@echo VERSION_SUFFIX is ${VERSION_SUFFIX}
	$(CC) -c $(SRC_FILES) $(CFLAGS) $(CPPFLAGS) $(INC_FILES)
clean:
	rm -rf *.o CpuMon

