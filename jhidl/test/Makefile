TOPDIR := ../../..

TARGET_PROGS = testService

SUBDIRS = ..

SRCS_testService = test.cpp
LIBS_testService = jhcomserver

IDLDIR = .

include $(TOPDIR)/jhbuild/jhcommon.inc
include $(TOPDIR)/jhbuild/Make.Defaults
include $(TOPDIR)/jhbuild/Make.Rules

%.h : %.idl 
	$(DEFAULT_BIN_DIR)/jhidl $<

