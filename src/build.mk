ifndef LOADED_jhcommon/src/build.mk
LOADED_jhcommon/src/build.mk := 1

DIR := jhcommon/src

TARGET_LIBS := libjhcommon libjhcomserver libjhcomclient

INCDIRS_$(DIR) := $(TOPSRCDIR)/$(DIR)/../include

#Only if JH_GCHEAP is not no should we enable GCHEAP
ifneq ($(JH_GCHEAP), no)
CFLAGS_PROG_$(DIR) += -DGCHEAP_ENABLED
endif

ifeq ($(PLATFORM),Darwin)
CFLAGS_PROG_$(DIR) += -DPLATFORM_DARWIN
endif

$(DIR)_JH_COMMON_SRCS = CircularBuffer.cpp Thread.cpp \
	EventQueue.cpp Selector.cpp Socket.cpp File.cpp \
	EventThread.cpp EventDispatcher.cpp Timer.cpp jh_memory.cpp \
	AppArgs.cpp URI.cpp JetHead.cpp FdReaderWriter.cpp \
	HttpHeaderBase.cpp HttpHeader.cpp HttpRequest.cpp HttpResponse.cpp \
	HttpAgent.cpp logging.cpp MulticastSocket.cpp \
	Allocator.cpp Condition.cpp Mutex.cpp

SRCS_libjhcommon := $($(DIR)_JH_COMMON_SRCS)

SRCS_libjhcommonmd := $($(DIR)_JH_COMMON_SRCS)
CFLAGS_libjhcommonmd := -DUSE_WRAPPERS

SRCS_libjhcommongb := $($(DIR)_JH_COMMON_SRCS)
CFLAGS_libjhcommongb := -DUSE_GUARD_BANDS -DUSE_WRAPPERS

SRCS_libjhcomserver := ComponentManager.cpp ComponentManagerUtils.cpp

SRCS_libjhcomclient := ComponentManagerClient.cpp ComponentManagerUtils.cpp

include $(TOPSRCDIR)/jhbuild/rules.mk

endif
