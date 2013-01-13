/*
 * File:   portTest.cpp
 * Author: Harald Postner
 *
 * Created on Aug 30, 2012, 8:20:19 PM
 */

#include "portTest.hpp"
#include "port.hpp"
#include <exception>
#include <thread>
#include <chrono>
#include <iostream>
#include <random>

class TestException : public std::runtime_error {
public:

  explicit
  TestException(const std::string& __arg) :
  runtime_error(__arg) {
  };

};

/**
 * A mockup class that implements the open/close and the exec...Process functionality.
 */
class PortMock : public Port {
private:
  const int initializeDuration;
  const int registerDuration;
  const int startDuration;
  const int execJavaProcessDuration;
  const int execNativeProcessDuration;
  const int stopDuration;
  const int uninitializeDuration;
  const int unregisterDuration;
public:
  int initialize_implCount;
  int register_implCount;
  int start_implCount;
  int execJavaProcess_implCount;
  int execNativeProcess_implCount;
  int stop_implCount;
  int uninitialize_implCount;
  int unregister_implCount;
  bool failOnWrongState = false;

  PortMock(bool isOutput, long internalId) :
  Port(isOutput, internalId),
  initialize_implCount(0),
  register_implCount(0),
  start_implCount(0),
  execJavaProcess_implCount(0),
  execNativeProcess_implCount(0),
  stop_implCount(0),
  uninitialize_implCount(0),
  unregister_implCount(0),
  initializeDuration(0),
  registerDuration(0),
  startDuration(0),
  execJavaProcessDuration(0),
  execNativeProcessDuration(0),
  stopDuration(0),
  uninitializeDuration(0),
  unregisterDuration(0) {
  }

  PortMock(bool isOutput,
          long internalId,
          int _initializeDuration,
          int _registerDuration,
          int _startDuration,
          int _execJavaProcessDuration,
          int _execNativeProcessDuration,
          int _stopDuration,
          int _uninitializeDuration,
          int _unregisterDuration) :
  Port(isOutput, internalId),
  initialize_implCount(0),
  register_implCount(0),
  start_implCount(0),
  execJavaProcess_implCount(0),
  execNativeProcess_implCount(0),
  stop_implCount(0),
  uninitialize_implCount(0),
  unregister_implCount(0),
  initializeDuration(_initializeDuration),
  registerDuration(_registerDuration),
  startDuration(_startDuration),
  execJavaProcessDuration(_execJavaProcessDuration),
  execNativeProcessDuration(_execNativeProcessDuration),
  stopDuration(_stopDuration),
  uninitializeDuration(_uninitializeDuration),
  unregisterDuration(_unregisterDuration) {
  }

  PortMock(PortMock &&) = default;

  PortMock(const PortMock&) = delete;

  virtual ~PortMock() {
    if (failOnWrongState) {
      if ((!isCreatedState()) && (!isDeletableState())) {
        CPPUNIT_FAIL("Port deleted in wrong state");
      }
    }
  }

private:
  bool exceptionInJava = false;
  bool exceptionInNative = false;
  bool exceptionInInitialize = false;
public:

  /**
   * Setting to true will make the execJavaProcess_impl throw an exception.
   * @param value
   */
  void setExceptionInJava(bool value) {
    exceptionInJava = value;
  }

  /**
   * Setting to true will make the initialize_impl throw an exception.
   * @param value
   */
  void setExceptionInInitialize(bool value) {
    exceptionInInitialize = value;
  }

  /**
   * Setting to true will make the execNativeProcess_impl throw an exception.
   * @param value
   */
  void setExceptionInNative(bool value) {
    exceptionInNative = value;
  }
protected:

  virtual void initialize_impl(JNIEnv * env, jstring name, jobject listener)override {
    if (initializeDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(initializeDuration));
    }
    initialize_implCount++;
    if (exceptionInInitialize) {
      throw TestException("Requested exception in initialize_impl");
    }
  }

  virtual void register_impl(void * client)override {
    if (registerDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(registerDuration));
    }
    register_implCount++;
  }

  virtual void start_impl()override {
    if (startDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(startDuration));
    }
    start_implCount++;
  }

  virtual void execJavaProcess_impl(JNIEnv * env, unsigned long timeCodeStart, unsigned long timeCodeDuration, bool lastCycle)override {
    if (execJavaProcessDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(execJavaProcessDuration));
    }
    execJavaProcess_implCount++;
    if (exceptionInJava) {
      throw TestException("Requested exception in execJavaProcess_impl");
    }
  }

  virtual void execNativeProcess_impl(unsigned long timeCodeStart, unsigned long timeCodeDuration, void * client)override {
    if (execNativeProcessDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(execNativeProcessDuration));
    }
    execNativeProcess_implCount++;
    if (exceptionInNative) {
      throw TestException("Requested exception in execNativeProcess_impl");
    }
  }

  virtual void stop_impl()override {
    if (stopDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(stopDuration));
    }
    stop_implCount++;
  }

  virtual void uninitialize_impl(JNIEnv * env) override {
    if (uninitializeDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(uninitializeDuration));
    }
    uninitialize_implCount++;
  }

  virtual void unregister_impl(void * client)override {
    if (unregisterDuration != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(unregisterDuration));
    }
    unregister_implCount++;
  }
};

static long newPortId = 1;


CPPUNIT_TEST_SUITE_REGISTRATION(portTest);

void portTest::testMoveConstructor() {

  //construct port1 with a given id and put it into InitializedState
  long id1 = newPortId++;
  PortMock port1(//
          true, // isOutput,
          id1);
  port1.failOnWrongState = true;
  port1.initialize(nullptr, nullptr, nullptr);
  CPPUNIT_ASSERT(port1.isInitializedState());
  CPPUNIT_ASSERT_EQUAL(id1, port1.getId());

  //construct port1_1 (like port1)
  long id2 = newPortId++;
  PortMock port2(//
          false, // isInput,
          id2);
  port2.failOnWrongState = true;
  port2.initialize(nullptr, nullptr, nullptr);
  CPPUNIT_ASSERT(port2.isInitializedState());
  CPPUNIT_ASSERT_EQUAL(id2, port2.getId());


  // move port1 into port 1_1 
  PortMock port1_1 = std::move(port1);

  // move port2 into port 2_1 
  PortMock port2_1 = std::move(port2);

  // verify that port1_1 has got the characteristics of port1 
  CPPUNIT_ASSERT(port1_1.isInitializedState());
  CPPUNIT_ASSERT_EQUAL(id1, port1_1.getId());

  // verify that port2_1 has got the characteristics of port2
  CPPUNIT_ASSERT(port2_1.isInitializedState());
  CPPUNIT_ASSERT_EQUAL(id2, port2_1.getId());

  // very that the remains of port1 characterize a dead port
  CPPUNIT_ASSERT_EQUAL(PortInvalidId, port1.getId());
  CPPUNIT_ASSERT(port1.isDeletableState());

  // very that the remains of port2 characterize a dead port
  CPPUNIT_ASSERT_EQUAL(PortInvalidId, port2.getId());
  CPPUNIT_ASSERT(port2.isDeletableState());

  //end
  port1_1.shutdown(nullptr, nullptr, false);
  port2_1.shutdown(nullptr, nullptr, false);

}

void portTest::testMoveConstructorOnBusyPort() {

  //construct port1 with a given id and put it into InitializedState
  long id = newPortId++;
  PortMock port1(//
          true, // isOutput,
          id, // internalId,
          500, // _initializeDuration, << initialization will take long time
          0, // _registerDuration,
          0, // _startDuration, 
          0, // _execJavaProcessDuration,
          0, // _execNativeProcessDuration,
          0, // _stopDuration,
          0, // _uninitializeDuration,
          0); // _unregisterDuration) 


  // this will take a while to execute....
  std::thread iniThread([&]{port1.initialize(nullptr, nullptr, nullptr);});
  std::this_thread::sleep_for(std::chrono::milliseconds(20)); // just to make sure that iniThread runs...

  // the move should block until the iniThread has ended.
  PortMock port2 = std::move(port1);
  iniThread.join();

  // verify that port2 has got the characteristics of port1 (after initialization has finished)
  CPPUNIT_ASSERT(port2.isInitializedState());

  CPPUNIT_ASSERT_EQUAL(id, port2.getId());
  CPPUNIT_ASSERT_EQUAL(1, port2.initialize_implCount);


  // very that the remains of port1 characterize a dead port
  CPPUNIT_ASSERT_EQUAL(PortInvalidId, port1.getId());
  CPPUNIT_ASSERT(port1.isDeletableState());

  //end
  port2.shutdown(nullptr, nullptr, false);

}

/**
 * Test the full live-cycle on an input port.
 */
void portTest::testFullLiveCicle_Input() {

  long id = newPortId++;
  bool isOutputPort = false;
  PortMock port(isOutputPort, id);
  CPPUNIT_ASSERT_EQUAL(id, port.getId());
  CPPUNIT_ASSERT(port.isCreatedState());
  CPPUNIT_ASSERT(port.isNoneSubstate());

  port.initialize(nullptr, nullptr, nullptr);
  CPPUNIT_ASSERT(port.isInitializedState());

  port.registerAtServer(nullptr);
  CPPUNIT_ASSERT(port.isRegisteredState());

  port.start();
  CPPUNIT_ASSERT(port.isRunningState());
  CPPUNIT_ASSERT(port.isStartedSubstate());

  // now lets execute two process-cycles
  int expectedNativeInvokations = 2;
  int expectedJavaInvokations = 2;

  port.execNativeCycleInit(123, 100);
  CPPUNIT_ASSERT(port.isNativeToExecSubstate());

  port.execNativeProcess(nullptr);
  CPPUNIT_ASSERT(port.isJavaToExecSubstate());

  port.execJavaProcess(nullptr, false);
  CPPUNIT_ASSERT(port.isCycleDoneSubstate());

  port.execNativeCycleInit(223, 100);
  CPPUNIT_ASSERT(port.isNativeToExecSubstate());

  port.execNativeProcess(nullptr);
  CPPUNIT_ASSERT(port.isJavaToExecSubstate());

  port.execJavaProcess(nullptr, true); //<< last cycle
  CPPUNIT_ASSERT(port.isTerminatedSubstate());

  port.stop(false);
  CPPUNIT_ASSERT(port.isStoppedState());
  CPPUNIT_ASSERT(port.isNoneSubstate());

  port.unregisterAtServer(nullptr);
  CPPUNIT_ASSERT(port.isUnregisteredState());

  port.uninitialize(nullptr);
  CPPUNIT_ASSERT(port.isDeletableState());

  CPPUNIT_ASSERT_EQUAL(1, port.initialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.register_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.start_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedJavaInvokations, port.execJavaProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedNativeInvokations, port.execNativeProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.stop_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.uninitialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.unregister_implCount);

  port.shutdown(nullptr, nullptr, false);
  CPPUNIT_ASSERT(!port.hasProcessException());
  CPPUNIT_ASSERT(port.isDeletableState());

}

/**
 * Test the full live-cycle on an output port.
 */
void portTest::testFullLiveCicle_Output() {

  long id = newPortId++;
  bool isOutputPort = true;
  PortMock port(isOutputPort, id);
  CPPUNIT_ASSERT_EQUAL(id, port.getId());
  CPPUNIT_ASSERT(port.isCreatedState());
  CPPUNIT_ASSERT(port.isNoneSubstate());

  port.initialize(nullptr, nullptr, nullptr);
  CPPUNIT_ASSERT(port.isInitializedState());

  port.registerAtServer(nullptr);
  CPPUNIT_ASSERT(port.isRegisteredState());

  port.start();
  CPPUNIT_ASSERT(port.isRunningState());
  CPPUNIT_ASSERT(port.isStartedSubstate());

  // now lets execute two process-cycles
  int expectedNativeInvokations = 2;
  int expectedJavaInvokations = 2;

  // cycle 1
  port.execNativeCycleInit(123, 100);
  CPPUNIT_ASSERT(port.isJavaToExecSubstate());

  port.execJavaProcess(nullptr, false);
  CPPUNIT_ASSERT(port.isNativeToExecSubstate());

  port.execNativeProcess(nullptr);
  CPPUNIT_ASSERT(port.isCycleDoneSubstate());

  // here we call "stop" in its own thread and let it wait for the last cycle to execute.
  bool stopReturned = false;
  std::thread stoppingThread([&]{port.stop(false); stopReturned = true;});
  stoppingThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // cycle 2 (the last cycle)
  port.execNativeCycleInit(223, 100);
  CPPUNIT_ASSERT(port.isJavaToExecSubstate());

  port.execJavaProcess(nullptr, false); //<< "last cycle" not set, but stop called above
  CPPUNIT_ASSERT(port.isNativeToTerminateSubstate());

  port.execNativeProcess(nullptr);
  CPPUNIT_ASSERT(port.isTerminatedSubstate());
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  CPPUNIT_ASSERT(stopReturned);
  CPPUNIT_ASSERT(port.isStoppedState());
  CPPUNIT_ASSERT(port.isNoneSubstate());

  port.unregisterAtServer(nullptr);
  CPPUNIT_ASSERT(port.isUnregisteredState());

  port.uninitialize(nullptr);
  CPPUNIT_ASSERT(port.isDeletableState());

  CPPUNIT_ASSERT_EQUAL(1, port.initialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.register_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.start_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedJavaInvokations, port.execJavaProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedNativeInvokations, port.execNativeProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.stop_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.uninitialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, port.unregister_implCount);

  port.shutdown(nullptr, nullptr, false);
  CPPUNIT_ASSERT(!port.hasProcessException());
  CPPUNIT_ASSERT(port.isDeletableState());

}

class ThreadRunner {
public:
  bool more = true;
  int javaCyclecount = 0;
  int nativeCyclecount = 0;

  /**
   * Helper procedure for testProcessFlipFlop() 
   * @param port
   */
  void runJavaLoop(Port& port) {
    while (more) {
      javaCyclecount++;
      port.execJavaProcess(nullptr, false);
    }
    port.execJavaProcess(nullptr, true);
  }

  /**
   * Helper procedure for testProcessFlipFlop() 
   * @param port
   */
  void runNativeLoop(Port& port) {
    unsigned long timeCodeStart = 0;
    const unsigned long timeCodeDuration = 255;
    while (port.isRunningState()) {
      nativeCyclecount++;
      port.execNativeCycleInit(timeCodeStart, timeCodeDuration);
      port.execNativeProcess(nullptr);
      port.waitForCycleDone();
      timeCodeStart += timeCodeDuration;
    }
  }
};

/**
 * When calling execJavaProcess and execNativeProcess on two separate
 * threads the two processes must flip processing. The number of 
 * invocations shall not differ by more than one.
 */
void portTest::testProcessFlipFlopAtMaxSpeed(bool isOutput) {
  PortMock port(//
          isOutput,
          newPortId++, // internalId,
          0, // _initializeDuration,
          0, // _registerDuration,
          0, // _startDuration, 
          0, // _execJavaProcessDuration,
          0, // _execNativeProcessDuration,
          0, // _stopDuration,
          0, // _uninitializeDuration,
          0); // _unregisterDuration) 

  ThreadRunner runner;

  port.initialize(nullptr, nullptr, nullptr);
  port.registerAtServer(nullptr);
  port.start();

  runner.more = true;
  std::thread javaThread([&]{runner.runJavaLoop(port);});
  std::thread nativeThread([&]{runner.runNativeLoop(port);});

  const int runningMilliSec = 20;
  std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));
  CPPUNIT_ASSERT(port.isRunningState());


  runner.more = false;
  // some time for the java thread...
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  port.stop(false);
  javaThread.join();
  nativeThread.join();

  CPPUNIT_ASSERT(port.isStoppedState());

  port.unregisterAtServer(nullptr);
  port.uninitialize(nullptr);

  CPPUNIT_ASSERT(!port.hasProcessException());

  // One round should not have taken longer than 0.05 milliseconds
  CPPUNIT_ASSERT(port.execJavaProcess_implCount > runningMilliSec * 20);
  CPPUNIT_ASSERT(port.execNativeProcess_implCount > runningMilliSec * 20);

  // the number of java invocations must equal the number of native invocations.
  CPPUNIT_ASSERT_EQUAL(port.execNativeProcess_implCount, port.execJavaProcess_implCount);

  port.shutdown(nullptr, nullptr, false);
  CPPUNIT_ASSERT(!port.hasProcessException());
  CPPUNIT_ASSERT(port.isDeletableState());

}

void portTest::testProcessFlipFlopAtMaxSpeed_Output() {
  testProcessFlipFlopAtMaxSpeed(true);
}

void portTest::testProcessFlipFlopAtMaxSpeed_Input() {
  testProcessFlipFlopAtMaxSpeed(false);
}

/**
 * When an exception occurs in the native thread, the exception should be trapped
 * and the port should stop itself.
 */
void portTest::testBadNativeProcess() {

  PortMock port(//
          true, // isOutput,
          newPortId++);

  ThreadRunner runner;

  port.initialize(nullptr, nullptr, nullptr);
  port.registerAtServer(nullptr);
  port.start();

  runner.more = true;
  bool javaEnded = false;
  bool nativeEnded = false;
  std::thread javaThread([&]{runner.runJavaLoop(port); javaEnded = true;});
  javaThread.detach();
  std::thread nativeThread([&]{runner.runNativeLoop(port); nativeEnded = true;});
  nativeThread.detach();

  const int runningMilliSec = 20;
  std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));
  CPPUNIT_ASSERT(port.isRunningState());

  port.setExceptionInNative(true); // Boom!!!
  std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));
  CPPUNIT_ASSERT(port.isStoppedOnErrorState());
  CPPUNIT_ASSERT(!javaEnded); // the java thread must now be free wheeling. 
  //(note: the native thread will stop because the "ThreadRunner" checks on "isRunningState()")


  runner.more = false;
  // give the java thread some time to terminate.
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  CPPUNIT_ASSERT(javaEnded);
  CPPUNIT_ASSERT(nativeEnded);

  port.stop(false);



  CPPUNIT_ASSERT(port.isStoppedState());
  CPPUNIT_ASSERT(port.hasProcessException());

  port.shutdown(nullptr, nullptr, false);
  CPPUNIT_ASSERT(port.isDeletableState());
  try {
    std::rethrow_exception(port.getProcessException());
  } catch (const TestException& e) {
    cerr << " ...Expected exception successfully thrown:" << e.what() << endl;
  } catch (...) {
    CPPUNIT_FAIL("Unexpected Exception.");
  }
}

/**
 * When an exception occurs during the opening of a port, the exception should be thrown
 * and the port should transit into the deletable state.
 */
void portTest::testBadOpen() {

  PortMock port(//
          true, // isOutput,
          newPortId++);

  port.setExceptionInInitialize(true);

  try {
    port.initialize(nullptr, nullptr, nullptr);
  } catch (const TestException& e) {
    cerr << " ...Expected exception successfully thrown:" << e.what() << endl;
  } catch (...) {
    CPPUNIT_FAIL("Unexpected Exception.");
  }
  // on a badly opened port, execJavaProcess() should should be a no-op.
  port.execJavaProcess(NULL,false);
  CPPUNIT_ASSERT_EQUAL(0, port.execJavaProcess_implCount);
  
 // CPPUNIT_ASSERT(port.isDeletableState());
  


}

/**
 * When an exception occurs in the java thread, the exception should be trapped
 * and the port should stop itself.
 */
void portTest::testBadJavaProcess() {
  PortMock port(//
          true, // isOutput,
          newPortId++);

  ThreadRunner runner;

  port.initialize(nullptr, nullptr, nullptr);
  port.registerAtServer(nullptr);
  port.start();

  runner.more = true;
  bool javaEnded = false;
  bool nativeEnded = false;
  std::thread javaThread([&]{runner.runJavaLoop(port); javaEnded = true;});
  javaThread.detach();
  std::thread nativeThread([&]{runner.runNativeLoop(port); nativeEnded = true;});
  nativeThread.detach();


  const int runningMilliSec = 20;
  std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));
  CPPUNIT_ASSERT(port.isRunningState());

  port.setExceptionInJava(true); // Boom!!!
  std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));
  CPPUNIT_ASSERT(port.isStoppedOnErrorState());
  CPPUNIT_ASSERT(!javaEnded); // the java thread must now be free wheeling. 
  //(note: the native thread will stop because the "ThreadRunner" checks on "isRunningState()")


  runner.more = false;
  // give the java thread some time to terminate.
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  CPPUNIT_ASSERT(javaEnded);
  CPPUNIT_ASSERT(nativeEnded);

  port.stop(false);

  CPPUNIT_ASSERT(port.isStoppedState());
  CPPUNIT_ASSERT(port.hasProcessException());
  port.shutdown(nullptr, nullptr, false);
  CPPUNIT_ASSERT(port.isDeletableState());

  try {
    std::rethrow_exception(port.getProcessException());
  } catch (const TestException& e) {
    cerr << " ...Expected exception successfully thrown:" << e.what() << endl;
  } catch (...) {
    CPPUNIT_FAIL("Unexpected Exception.");
  }
}

/**
 * Helper procedure for the "testRandomTiming".
 * @param isOutput do we test an output or input port
 * @param initializeDuration the time in milliseconds that the "initialize" procedure should take.
 * @param registerDuration the time in milliseconds that the "register" procedure should take.
 * @param startDuration the time in milliseconds that the "start" procedure should take.
 * @param execJavaProcessDuration the time in milliseconds that the "java worker thread" procedure should take.
 * @param execNativeProcessDuration the time in milliseconds that the "native worker thread" procedure should take.
 * @param stopDuration the time in milliseconds that the "stop" procedure should take.
 * @param uninitializeDuration the time in milliseconds that the "un-initialize" procedure should take.
 * @param unregisterDuration the time in milliseconds that the "un-register" procedure should take.
 */

void portTest::doTestRandomTiming(//
        bool isOutput,
        int initializeDuration,
        int registerDuration,
        int startDuration,
        int execJavaProcessDuration,
        int execNativeProcessDuration,
        int stopDuration,
        int uninitializeDuration,
        int unregisterDuration) {

  std::cerr << " portTest::doTestRandomTiming("
          << isOutput << ", "
          << initializeDuration << ", "
          << registerDuration << ", "
          << startDuration << ", "
          << execJavaProcessDuration << ", "
          << execNativeProcessDuration << ", "
          << stopDuration << ", "
          << uninitializeDuration << ", "
          << unregisterDuration << ")\n";

  PortMock port(//
          isOutput,
          newPortId++, // internalId,
          initializeDuration,
          registerDuration,
          startDuration,
          execJavaProcessDuration,
          execNativeProcessDuration,
          stopDuration,
          uninitializeDuration,
          unregisterDuration);

  ThreadRunner runner;

  port.initialize(nullptr, nullptr, nullptr);
  port.registerAtServer(nullptr);
  port.start();

  runner.more = true;
  bool javaThreadEnded = false;
  bool nativeThreadEnded = false;
  std::thread javaThread([&]{runner.runJavaLoop(port); javaThreadEnded = true;});
  std::thread nativeThread([&]{runner.runNativeLoop(port); nativeThreadEnded = true;});
  javaThread.detach();
  nativeThread.detach();

  // lets do twenty (or more) rounds
  const int runningMilliSec = 20 * (execJavaProcessDuration + execNativeProcessDuration) + 10;
  std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));


  // tell the java thread to do a last round
  runner.more = false;
  //wait for the last round to finish
  std::this_thread::sleep_for(std::chrono::milliseconds(execJavaProcessDuration + execNativeProcessDuration + 10));
  port.stop(false);
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); //give the native thread some time to terminate
  CPPUNIT_ASSERT(javaThreadEnded);
  CPPUNIT_ASSERT(nativeThreadEnded);


  port.unregisterAtServer(nullptr);
  port.uninitialize(nullptr);

  // There should at least be twenty rounds
  CPPUNIT_ASSERT(port.execJavaProcess_implCount >= 20);
  CPPUNIT_ASSERT(port.execNativeProcess_implCount >= 20);

  CPPUNIT_ASSERT_EQUAL(port.execNativeProcess_implCount, port.execJavaProcess_implCount);

  port.shutdown(nullptr, nullptr, false);
  CPPUNIT_ASSERT(!port.hasProcessException());
  CPPUNIT_ASSERT(port.isDeletableState());

}

/**
 * This test is similar to "testProcessFlipFlopAtMaxSpeed".
 * This time the the processes will have random timing , thus we might
 * discover dead locks and the like.
 */
void portTest::testRandomTiming() {

  mt19937 rng(0); // the Mersenne Twister random generator
  uniform_int_distribution<uint32_t> idist(0, 25);
  bernoulli_distribution bdist(0.5);

  for (int i = 0; i < 10; i++) {
    doTestRandomTiming(//
            bdist(rng), //bool isOutput,
            idist(rng), // int initializeDuration,
            idist(rng), // int registerDuration,
            idist(rng), // int startDuration,
            idist(rng), // int execJavaProcessDuration,
            idist(rng), // int execNativeProcessDuration,
            idist(rng), // int stopDuration,
            idist(rng), // int uninitializeDuration,
            idist(rng)); // int unregisterDuration) 
  }
}

/**
 * This test verifies that when the waiting-time exceeds the "crashTimeout"
 * an exception is thrown.
 */
void portTest::testTimeoutExceptionInStop() {
  std::cerr << "\nportTest::testTimeoutExceptionInStop(Please be patient...)\n";
  PortMock port(//
          true, // isOutput,
          newPortId++, // internalId,
          0, // _initializeDuration,
          0, // _registerDuration,
          20000, // _startDuration, <<< the start duration is longer than the crashTimeout
          0, // _execJavaProcessDuration,
          0, // _execNativeProcessDuration,
          0, // _stopDuration,
          0, // _uninitializeDuration,
          0); // _unregisterDuration) 

  port.initialize(nullptr, nullptr, nullptr);
  port.registerAtServer(nullptr);


  // do the start procedure in a separate thread 
  //(this thread will take an exceptionally long time, simulating a problem
  // when starting the port)
  std::thread timeWaisterThread([&]{port.start();});
  timeWaisterThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // << make sure the new thread has started

  // now the stop-procedure is expected to die in timeout
  std::cerr << "\nmessage \" A Port is deleted in wrong state\" is expected... \n";
  CPPUNIT_ASSERT_THROW(port.stop(false), std::runtime_error);

}
