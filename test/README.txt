This folder contains a set of test programs for validating functionality in 
JHCOMMON.  Historically there was little focus on unit testing JHCOMMON pieces.
However today we are working to ensure that all JHCOMMON components have some
level of unit testing.  

There is a Testing Framework that all new unit test MUST use.  This is defined
in TestCase.h.  Eventually we'd like to have a single test program with the 
ability to run and set/sub-set of tests from the full suite.  Today we make
suite by building individual test programs. 

TESTS TODAY
-----------

In each section there is a [X], where X is G for a test using the new framework
and that runs properly and is fairly complete in coverage.  S is for "so-so", it
uses the new framework but either doesn't work or coverage needs improvement.
N is for needs improvment, these test may work fine but do not use the test
framework.  B is for "bad" these test don't use the framework and either don't
run or have bad coverage.

1. allocatorTest [G]

2. circularBufTest [G]

3. comServerTest [B] - need coverage on removing services and needs new framework 

4. eventAgentTest [B] - should look at selector test and maybe merge.

5. eventThreadTest [B] - same as above.

6. FileTest [G]

7. HttpTest [S] - very limited test set, relies on external websites and files.

8. listenerContainerTest [N] - testing looks good, but requiers standard input to run

9. loggingTest [B] - not clear what this test is for, some testing of function_name_fixup.

10. loggingTest2 [S] - uses framework but does not validate anything.  

11. regexTest [G]

12. selectorTest [S] - need to check if eventing style, is "new" eventing.

13. sigAlrmTest [B] - not sure what in JHCOMMON this is testing.

14. SocketTest [B] - uses framework but never calls TestFailed and doesn't run.

15. SocketTest2 [S] - bad formating and need to check coverage.  But it runs.

16. stringTest [G]

17. telnetServer [B] - not a test program, needs to be somewhere else.

18. timerTest [N] - seems complete, but may need more.

19. TimeUtilsTest [S] - could use more coverage.

20. URITest [G]

