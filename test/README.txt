This folder contains a set of test programs for validating functionality in 
JHCOMMON.  Historically there was little focus on unit testing JHCOMMON pieces.
However today we are working to ensure that all JHCOMMON components have some
level of unit testing.  

There is a Testing Framework that all new unit test MUST use.  This is defined
in TestCase.h.  Eventually we'd like to have a single test program with the 
ability to run and set/sub-set of tests from the full suite.  Today we make
suite by building individual test programs. 

TEST PHILOSOPHY
---------------

1. What/How many tests do I need?

We aim to test every method in a class with both positive and negative test 
cases.  These tests don't test every possible outcome, but they strive to hit
major functional points.  If a method can fail (and most can) test a negative 
case.  If a single method really just calls another menthod, you should still 
test both methods.  We want the tests to stand up over time, as changes are 
made these two methods may not always rely on each other. 

2. How should I write my test cases?

Here are several factor to consider when writing the test cases.

a. Test classes in isolation.

Test the methods on a class with as little reliance on other methods in 
JH Common as possible.  This is not always possible, and don't try to devise
clever ways around this.  It is ok to rely on other classes.  Sometime you have 
to, for eaxample parts of the File class use a Path object, so you will have to
use Path in the File test.  

b. Don't go around JH Common to test.

Don't use methods from the internals of your code to test the code.  For 
example on could test the Path::getCWD work by checking that it matches the
system call getcwd.  However this assumes that getcwd exists and the whole 
point of JH common is to isolate us from this.  This method we just have to 
assume is correct.  But we can check things like: 
	getCWD().append( "../some/relative/path" ) ==
		Path( "../some/relative/path" ).makeAbsolute()
So we can check that all methods of accessing similar or the same information
thru the API return consistant information.

c. Use the test framework!!!

Build your tests in a class that derives from TestCase, use the TestSuite and
TestRunner to execute the test cases.  These classes are simple at this time
but will get more complex and powerfull as time goes on.  Putting test into this
framework means they are reusable in the future.  Or if I have to say it, 
Just do it...

d. TestCases should be self suficiant.

Do not rely on a previous test running to create any object, data, file, etc...
TestCases should be able to run in any order and TestCases should be able to 
run in isolation of any other TestCase.  

e. Creating Object Under Test.

All objects used in the test case should be created and destroyed in the 
execution of the test case.  And therefore destroyed before TestPassed is
called.  Therefore you TestCases private section will rarely have any members 
(other than the test number if you test case impliments multiple tests, and 
most do).  Also this is why most tests call TestPassed in Run after calling 
another method to execute the test.  This allows us to put items on the stack
and still meet this requierment.

f. TestCase lifetime.

Typically all test cases are allocated up from and then tests are run one at a
time.  TestCases will be destroyed immidiatly after execution.  This should not
be a factor if you follow the other guidelines here. 

g. Use Table Driven tests

As much as is practical make table driven tests.  These test cases will be 
easy to extend and there code complexity is very low.  This help reduce the
number of bugs in the test cases themselves.  You will see numberous places 
in the cases today that use tables, URI being the more prevelant.

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


