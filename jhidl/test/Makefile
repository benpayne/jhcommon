TOPDIR := ../../..

TARGET_PROGS = testService

SUBDIRS = ..
DEPS = ITest.h

SRCS_testService = test.cpp Reflect.cpp ITest.cpp
LIBS_testService = jhcomserver tinyxml
CFLAGS_testService = -DTIXML_USE_STL

INCDIRS = .. ../tinyxml
IDLDIR = .

include $(TOPDIR)/jhbuild/jhcommon.inc
include $(TOPDIR)/jhbuild/Make.Defaults
include $(TOPDIR)/jhbuild/Make.Rules

%.h : %.idl $(DEFAULT_BIN_DIR)/jhidl
	$(DEFAULT_BIN_DIR)/jhidl $<

