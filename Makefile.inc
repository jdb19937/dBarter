VERSION = 0.0.16
ARCH = i386-linux

SRC_DIR = .
LIB_DIR = ../lib
BIN_DIR = ../bin
INC_DIR = ../include
INST_DIR = /usr/local/dbarter

CC = gcc
CXX = g++

CFLAGS = -D_REENTRANT -DVERSION=\"$(VERSION)\" -I$(INC_DIR) -g
CXXFLAGS = $(CFLAGS)

