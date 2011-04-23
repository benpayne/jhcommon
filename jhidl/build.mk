ifndef LOADED_nntp/build.mk
LOADED_nntp/build.mk := 1

DIR=jhcommon/jhidl

INSTALL = ISupports.h IFoo.h

TARGET_PROGS = jhidl

SRCS_jhidl = jhidl.cpp DumpCPlusPlus.cpp DumpXML.cpp
CFLAGS_jhidl = $(shell pkg-config --cflags libIDL-2.0)
LDFLAGS_jhidl = $(shell pkg-config --libs libIDL-2.0) -ltinyxml

%.h : %.idl jhidl
	$(DEFAULT_BIN_DIR)/jhidl $<

include $(TOPSRCDIR)/jhbuild/jhcommon.inc
include $(TOPSRCDIR)/jhbuild/rules.mk
include $(TOPSRCDIR)/jhcommon/build.mk

endif
