TOPDIR := ../..

SUBDIRS = ../src

TARGET_PROGS = eventThreadTest selectorTest timerTest comServerTest \
	loggingTest listenerContainerTest sigAlrmTest circularBufTest \
	URITest SocketTest HttpTest TimeUtilsTest \
	SocketTest2 FileTest pathTest loggingTest2 allocatorTest eventAgentTest \
	telnetServer regexTest stringTest 

TARGET_LIBS = libfooservice

SRCS_listenerContainerTest = listenerContainerTest.cpp
SRCS_eventThreadTest = eventThreadTest.cpp
SRCS_selectorTest = selectorTest.cpp
SRCS_timerTest = timerTest.cpp
SRCS_loggingTest = loggingTest.cpp
SRCS_sigAlrmTest = sigAlrmTest.cpp
SRCS_URITest = URITest.cpp
SRCS_SocketTest = SocketTest.cpp
SRCS_SocketTest2 = SocketTest2.cpp
SRCS_HttpTest = HttpClientTest.cpp
SRCS_TimeUtilsTest = timeUtilsTest.cpp
SRCS_FileTest = FileTest.cpp
SRCS_loggingTest2 = loggingTest2.cpp
SRCS_allocatorTest = allocatorTest.cpp
SRCS_eventAgentTest = eventAgentTest.cpp
SRCS_telnetServer = TelnetServer.cpp
SRCS_regexTest = regexTest.cpp
SRCS_stringTest = stringTest.cpp
SRCS_pathTest = PathTest.cpp

SRCS_comServerTest = comServer.cpp
LIBS_comServerTest = jhcomserver
#LDFLAGS_comServerTest = -ldl

SRCS_libfooservice = FooService.cpp
LIBS_libfooservice = jhcomclient

SRCS_circularBufTest = CircularBufferTest.cpp
LDFLAGS_circularBufTest = -lpthread -lrt

BIN = test

include $(TOPDIR)/jhbuild/jhcommon.inc
include $(TOPDIR)/jhbuild/Make.Defaults
include $(TOPDIR)/jhbuild/Make.Rules


