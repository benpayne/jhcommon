add_library(fooservice SHARED FooService.cpp)
target_link_libraries(fooservice jhcomclient)

add_executable(listenerContainerTest listenerContainerTest.cpp )
target_link_libraries(listenerContainerTest jhcommon )

add_executable(eventThreadTest eventThreadTest.cpp )
target_link_libraries(eventThreadTest jhcommon )

add_executable(selectorTest selectorTest.cpp )
target_link_libraries(selectorTest jhcommon )

add_executable(timerTest timerTest.cpp )
target_link_libraries(timerTest jhcommon )

add_executable(loggingTest loggingTest.cpp )
target_link_libraries(loggingTest jhcommon )

#add_executable(sigAlrmTest sigAlrmTest.cpp )
#target_link_libraries(sigAlrmTest jhcommon )

add_executable(URITest URITest.cpp )
target_link_libraries(URITest jhcommon )

add_executable(SocketTest SocketTest.cpp )
target_link_libraries(SocketTest jhcommon )

add_executable(SocketTest2 SocketTest2.cpp )
target_link_libraries(SocketTest2 jhcommon )

add_executable(HttpTest HttpClientTest.cpp )
target_link_libraries(HttpTest jhcommon )

add_executable(TimeUtilsTest timeUtilsTest.cpp )
target_link_libraries(TimeUtilsTest jhcommon )

add_executable(FileTest FileTest.cpp )
target_link_libraries(FileTest jhcommon )

add_executable(loggingTest2 loggingTest2.cpp )
target_link_libraries(loggingTest2 jhcommon )

add_executable(allocatorTest allocatorTest.cpp )
target_link_libraries(allocatorTest jhcommon )

add_executable(eventAgentTest eventAgentTest.cpp )
target_link_libraries(eventAgentTest jhcommon )

add_executable(telnetServer TelnetServer.cpp )
target_link_libraries(telnetServer jhcommon )

add_executable(regexTest regexTest.cpp )
target_link_libraries(regexTest jhcommon )

add_executable(stringTest stringTest.cpp )
target_link_libraries(stringTest jhcommon )

add_executable(pathTest PathTest.cpp )
target_link_libraries(pathTest jhcommon )

add_executable(comServerTest comServer.cpp )
target_link_libraries(comServerTest jhcommon jhcomserver )

add_executable(circularBufTest CircularBufferTest.cpp )
target_link_libraries(circularBufTest jhcommon )
#LDFLAGS_circularBufTest = -lpthread -lrt


