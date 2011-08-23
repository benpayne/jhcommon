add_library(jhcommon SHARED Allocator.cpp AppArgs.cpp CircularBuffer.cpp Condition.cpp
		     EventDispatcher.cpp EventQueue.cpp EventThread.cpp FdReaderWriter.cpp
		     File.cpp HttpAgent.cpp HttpHeader.cpp HttpHeaderBase.cpp
		     HttpRequest.cpp HttpResponse.cpp JetHead.cpp MulticastSocket.cpp
		     Mutex.cpp Path.cpp Regex.cpp Selector.cpp Socket.cpp
		     Thread.cpp Timer.cpp TimerManager URI.cpp jh_memory.cpp logging.cpp)
		     
add_library(jhcomserver SHARED ComponentManager.cpp ComponentManagerUtils.cpp)
target_link_libraries(jhcomserver jhcommon ${JHCOM_LIBS} )

add_library(jhcomclient SHARED ComponentManager.cpp ComponentManagerUtils.cpp)
target_link_libraries(jhcomclient jhcommon ${JHCOM_LIBS} )

