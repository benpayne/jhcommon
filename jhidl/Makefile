TOPDIR := ../..

TARGET_PROGS = jhidl

INSTALL = ISupports.h IFoo.h

PLATFORMS = LinuxPC Darwin

SRCS_jhidl = jhidl.c
CFLAGS_jhidl = $(shell pkg-config --cflags libIDL-2.0)
LDFLAGS_jhidl = $(shell pkg-config --libs libIDL-2.0)

%.h : %.idl jhidl
	$(DEFAULT_BIN_DIR)/jhidl $<

include $(TOPDIR)/jhbuild/Make.Defaults
include $(TOPDIR)/jhbuild/Make.Rules


