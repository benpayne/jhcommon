add_library(fooservice SHARED FooService.cpp)
target_link_libraries(fooservice jhcomclient)

add_executable(listenerContainerTest listenerContainerTest.cpp )
target_link_libraries(listenerContainerTest ${JHCOMMON_LIBS} )

add_executable(eventThreadTest eventThreadTest.cpp )
target_link_libraries(eventThreadTest ${JHCOMMON_LIBS} )

add_executable(selectorTest selectorTest.cpp )
target_link_libraries(selectorTest ${JHCOMMON_LIBS} )

add_executable(timerTest timerTest.cpp )
target_link_libraries(timerTest ${JHCOMMON_LIBS} )

add_executable(loggingTest loggingTest.cpp )
target_link_libraries(loggingTest ${JHCOMMON_LIBS} )

#add_executable(sigAlrmTest sigAlrmTest.cpp )
#target_link_libraries(sigAlrmTest ${JHCOMMON_LIBS} )

add_executable(URITest URITest.cpp )
target_link_libraries(URITest ${JHCOMMON_LIBS} )

add_executable(SocketTest SocketTest.cpp )
target_link_libraries(SocketTest ${JHCOMMON_LIBS} )

add_executable(SocketTest2 SocketTest2.cpp )
target_link_libraries(SocketTest2 ${JHCOMMON_LIBS} )

add_executable(HttpTest HttpClientTest.cpp )
target_link_libraries(HttpTest ${JHCOMMON_LIBS} )

add_executable(TimeUtilsTest timeUtilsTest.cpp )
target_link_libraries(TimeUtilsTest ${JHCOMMON_LIBS} )

add_executable(FileTest FileTest.cpp )
target_link_libraries(FileTest ${JHCOMMON_LIBS} )

add_executable(loggingTest2 loggingTest2.cpp )
target_link_libraries(loggingTest2 ${JHCOMMON_LIBS} )

add_executable(allocatorTest allocatorTest.cpp )
target_link_libraries(allocatorTest ${JHCOMMON_LIBS} )

add_executable(eventAgentTest eventAgentTest.cpp )
target_link_libraries(eventAgentTest ${JHCOMMON_LIBS} )

add_executable(telnetServer TelnetServer.cpp )
target_link_libraries(telnetServer ${JHCOMMON_LIBS} )

add_executable(regexTest regexTest.cpp )
target_link_libraries(regexTest ${JHCOMMON_LIBS} )

add_executable(stringTest stringTest.cpp )
target_link_libraries(stringTest ${JHCOMMON_LIBS} )

add_executable(pathTest PathTest.cpp )
target_link_libraries(pathTest ${JHCOMMON_LIBS} )

add_executable(comServerTest comServer.cpp )
target_link_libraries(comServerTest ${JHCOMMON_LIBS} jhcomserver )

add_executable(circularBufTest CircularBufferTest.cpp )
target_link_libraries(circularBufTest ${JHCOMMON_LIBS} )


