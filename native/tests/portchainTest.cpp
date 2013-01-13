/*
 * File:   portchainTest.cpp
 * Author: Harald Postner
 *
 * Created on Sep 1, 2012, 3:54:11 PM
 */

#include "portchainTest.hpp"
#include "portchain.hpp"
#include "port.hpp"

#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <utility>
#include <vector>
#include <random>

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(portchainTest);

/**
 * A useful template function found in:
 * http://stackoverflow.com/questions/7363861/passing-a-stdunique-ptr-as-a-parameter-to-a-function
 * @param ptr
 * @return 
 */
template <typename Dst, typename Src>
std::unique_ptr<Dst> unique_dynamic_cast(std::unique_ptr<Src> && ptr) {
  Src * p = ptr.release(); // [1]
  std::unique_ptr<Dst> r(dynamic_cast<Dst*> (p)); // [2]
  if (!r) {
    ptr.reset(p); // [3]
    throw bad_cast();
  }
  return r; // [4]
}
/**
 * A mockup classes that implements the open/close and the exec...Process functionality.
 */
static int portCount = 0;

class PortMock : public Port {
public:
  int initialize_implCount = 0;
  int register_implCount = 0;
  int start_implCount = 0;
  int execJavaProcess_implCount = 0;
  int execNativeProcess_implCount = 0;
  int stop_implCount = 0;
  int uninitialize_implCount = 0;
  int unregister_implCount = 0;
  int lastCycleCount = 0;

  virtual ~PortMock() {
    if (!(isCreatedState() || isDeletableState())) {
      //CPPUNIT_FAIL("### A Port is deleted in wrong state!!!!");
      cerr << "   A Port is deleted in wrong state!!!! id=" << getId() << "\n\n";
    }
    if (getId() != PortInvalidId) {
      portCount--;
    }
  }
protected:

  PortMock(bool isOutput, long internalId) :
  Port(isOutput, internalId) {
    portCount++;
  }

  PortMock(PortMock && other) = default;

  virtual void initialize_impl(JNIEnv * env, jstring name, jobject listener)override {
    initialize_implCount++;
  }

  virtual void register_impl(void * client)override {
    register_implCount++;
  }

  virtual void start_impl()override {
    start_implCount++;
  }

  virtual void execJavaProcess_impl(JNIEnv * env, unsigned long timeCodeStart, unsigned long timeCodeDuration, bool lastCycle)override {
    execJavaProcess_implCount++;
    if (lastCycle) {
      lastCycleCount++;
    }
  }

  virtual void execNativeProcess_impl(unsigned long timeCodeStart, unsigned long timeCodeDuration, void * client)override {
    execNativeProcess_implCount++;
  }

  virtual void stop_impl()override {
    stop_implCount++;
  }

  virtual void uninitialize_impl(JNIEnv * env) override {
    uninitialize_implCount++;
  }

  virtual void unregister_impl(void * client)override {
    unregister_implCount++;
  }
};

class InputPortMock : public PortMock {
public:

  InputPortMock(long internalId) :
  PortMock(false, internalId) {
  }

  InputPortMock(InputPortMock &&) = default;


};

class OutputPortMock : public PortMock {
public:

  OutputPortMock(long internalId) :
  PortMock(true, internalId) {
  }
  OutputPortMock(OutputPortMock &&) = default;
};

static int portChainMockDestructorCount = 0;

class PortChainMock : public PortChain {
public:

  PortChainMock() :
  PortChain() {
  }

  virtual ~PortChainMock() {
    portChainMockDestructorCount++;
  }

  // redefined as public function in order to be able to test

  int findSlotForNewPort(const unique_ptr<Port>& newPort) {
    return PortChain::findSlotForNewPort(newPort);
  }
protected:


};

portchainTest::portchainTest() {
}

portchainTest::~portchainTest() {
}

void portchainTest::setUp() {
}

void portchainTest::tearDown() {
}

static long newPortId = 0;

/**
 * Testing then creation and deletion of a port-chain in its "created" state.
 */
void portchainTest::testCreateDeleteVirgin() {
  portChainMockDestructorCount = 0;
  PortChainMock* portChain = new PortChainMock();
  CPPUNIT_ASSERT_EQUAL(0, portChainMockDestructorCount);
  CPPUNIT_ASSERT(portChain->isCreatedState());

  portChain->shutdown(nullptr, nullptr);
  CPPUNIT_ASSERT(portChain->isDeletableState());
  delete portChain;
  CPPUNIT_ASSERT_EQUAL(1, portChainMockDestructorCount);
}

/**
 * Testing the creation and deletion of a port-chain in its "registered" state.
 */
void portchainTest::testCreateDeleteRegistered() {
  portChainMockDestructorCount = 0;
  portCount = 0;


  PortChainMock* portChain = new PortChainMock();
  portChain->initialize(nullptr, nullptr,
          unique_ptr<InputPortMock > (new InputPortMock(-2)),
          unique_ptr<OutputPortMock > (new OutputPortMock(-1)));
  portChain->registerAtServer(nullptr);
  CPPUNIT_ASSERT(portChain->isRegisteredState());
  CPPUNIT_ASSERT_EQUAL(2, portCount);

  portChain->shutdown(nullptr, nullptr);
  CPPUNIT_ASSERT(portChain->isDeletableState());
  delete portChain;
  CPPUNIT_ASSERT_EQUAL(1, portChainMockDestructorCount);
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * Specification:
 * new input port shall be be inserted at the start,
 * the first slot and the last slot shall remain reserved for 
 * the start- and the end- control ports. 
 */
void portchainTest::testFindSlotForInputPort() {
  void * dummyClient = (void*) - 1;
  portCount = 0;
  PortChainMock* portChain = new PortChainMock();

  unique_ptr<Port> port1 = unique_ptr<InputPortMock > (new InputPortMock(newPortId++));
  port1->initialize(nullptr, nullptr, nullptr);

  // -- port 1 shall be placed at position 1 (position 0 shall remain reserved)
  int port1Pos = portChain->findSlotForNewPort(port1);
  CPPUNIT_ASSERT_EQUAL(1, port1Pos);

  portChain->addPort(move(port1), dummyClient);

  // -- port 2 at position 2
  int id2 = newPortId++;
  unique_ptr<Port> port2 = unique_ptr<InputPortMock > (new InputPortMock(id2));
  port2->initialize(nullptr, nullptr, nullptr);

  int port2Pos = portChain->findSlotForNewPort(port2);
  CPPUNIT_ASSERT_EQUAL(2, port2Pos);

  portChain->addPort(move(port2), dummyClient);

  // -- port 3 at position 3
  unique_ptr<Port> port3 = unique_ptr<InputPortMock > (new InputPortMock(newPortId++));
  port3->initialize(nullptr, nullptr, nullptr);

  int port3Pos = portChain->findSlotForNewPort(port3);
  CPPUNIT_ASSERT_EQUAL(3, port3Pos);

  portChain->addPort(move(port3), dummyClient);

  // remove port 2 at position 2
  portChain->removePort(nullptr, nullptr, id2);

  // -- port 4 at position 2 (the position of the removed port)
  unique_ptr<Port> port4 = unique_ptr<InputPortMock > (new InputPortMock(newPortId++));
  port4->initialize(nullptr, nullptr, nullptr);

  int port4Pos = portChain->findSlotForNewPort(port4);
  CPPUNIT_ASSERT_EQUAL(2, port4Pos);

  portChain->addPort(move(port4), dummyClient);

  portChain->shutdown(nullptr, dummyClient);
  delete portChain;
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * Specification:
 * new output port shall be be inserted at the end,
 */
void portchainTest::testFindSlotForOutputPort() {
  void * dummyClient = (void*) - 1;
  portCount = 0;
  PortChainMock* portChain = new PortChainMock();

  unique_ptr<Port> port1 = unique_ptr<OutputPortMock > (new OutputPortMock(newPortId++));
  port1->initialize(nullptr, nullptr, nullptr);

  // -- port 1 at position MAX_PORTS-2
  int port1Pos = portChain->findSlotForNewPort(port1);
  CPPUNIT_ASSERT_EQUAL(MAX_PORTS - 2, port1Pos);

  portChain->addPort(move(port1), dummyClient);

  // -- port 2 at position MAX_PORTS-3
  int id2 = newPortId++;
  unique_ptr<Port> port2 = unique_ptr<OutputPortMock > (new OutputPortMock(id2));
  port2->initialize(nullptr, nullptr, nullptr);

  int port2Pos = portChain->findSlotForNewPort(port2);
  CPPUNIT_ASSERT_EQUAL(MAX_PORTS - 3, port2Pos);

  portChain->addPort(move(port2), dummyClient);

  // -- port 3 at position MAX_PORTS-4
  unique_ptr<Port> port3 = unique_ptr<OutputPortMock > (new OutputPortMock(newPortId++));
  port3->initialize(nullptr, nullptr, nullptr);

  int port3Pos = portChain->findSlotForNewPort(port3);
  CPPUNIT_ASSERT_EQUAL(MAX_PORTS - 4, port3Pos);

  portChain->addPort(move(port3), dummyClient);

  // remove port 2 at position MAX_PORTS-3
  portChain->removePort(nullptr, nullptr, id2);

  // -- port 4 at position MAX_PORTS-3 (the position of the removed port)
  unique_ptr<Port> port4 = unique_ptr<OutputPortMock > (new OutputPortMock(newPortId++));
  port4->initialize(nullptr, nullptr, nullptr);

  int port4Pos = portChain->findSlotForNewPort(port4);
  CPPUNIT_ASSERT_EQUAL(MAX_PORTS - 3, port4Pos);

  portChain->addPort(move(port4), dummyClient);

  portChain->shutdown(nullptr, dummyClient);
  delete portChain;
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * Testing then creation and deletion of a port-chain that contains ports.
 * Specification: a port-chain can be deleted once it has been 
 * shut down. All ports that are currently attached to the port-chain
 * have been shut down too and will be deleted together with the port-chain.
 */
void portchainTest::testCreateDeleteWithPort() {
  void * dummyClient = (void*) - 1;
  portCount = 0;
  PortChainMock* portChain = new PortChainMock();
  portChain->initialize(nullptr, nullptr,
          unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
          unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control
  CPPUNIT_ASSERT_EQUAL(2, portCount);

  {
    auto port1 = unique_ptr<InputPortMock > (new InputPortMock(newPortId++));
    port1->initialize(nullptr, nullptr, nullptr);

    auto port2 = unique_ptr<InputPortMock > (new InputPortMock(newPortId++));
    port2->initialize(nullptr, nullptr, nullptr);
    CPPUNIT_ASSERT_EQUAL(2 + 2, portCount); //two control ports and the two test-ports just created

    portChain->addPort(move(port1), dummyClient);
    portChain->addPort(move(port2), dummyClient);

    // port1 and port2 have been moved into the Port-chain 
    // thus "port1" and "port2" will now be empty (point to null)
    CPPUNIT_ASSERT(!static_cast<bool> (port1));
    CPPUNIT_ASSERT(!static_cast<bool> (port2));
    CPPUNIT_ASSERT_EQUAL(4, portCount);
  }
  // "port1" and "port2" went out of scope, but there are still four ports in the chain
  CPPUNIT_ASSERT_EQUAL(4, portCount);

  portChain->registerAtServer(dummyClient);

  portChain->shutdown(nullptr, dummyClient);
  CPPUNIT_ASSERT(portChain->isDeletableState());
  delete portChain;

  // now the two ports in the chain should have been deleted too
  CPPUNIT_ASSERT_EQUAL(0, portCount);

}

/**
 * Testing the whole live cycle on an empty portchain.
 */
void portchainTest::testOpenClose_Empty() {
  void * dummyClient = (void*) - 1;
  unsigned long timeCodeStart = 12345;
  const unsigned long timeCodeDuration = 123;
  PortChainMock portChain;
  CPPUNIT_ASSERT(portChain.isCreatedState());

  unique_ptr<InputPortMock> startControl = unique_ptr<InputPortMock > (new InputPortMock(-2));
  unique_ptr<OutputPortMock> endControl = unique_ptr<OutputPortMock > (new OutputPortMock(-1)); //end control


  portChain.initialize(nullptr, nullptr,
          move(startControl), //start control
          move(endControl)); //end control
  CPPUNIT_ASSERT(portChain.isInitializedState());

  portChain.registerAtServer(nullptr);
  CPPUNIT_ASSERT(portChain.isRegisteredState());

  portChain.start();
  CPPUNIT_ASSERT(portChain.isRunningState());

  // we must start the java thread before calling "execNativeCycle",
  // because without a running java thread a call to "execNativeCycle"
  // will never return.
  bool javaTreadHasEnded = false;
  std::thread javaThread([&]{portChain.runJava(nullptr); javaTreadHasEnded = true;});
  javaThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  portChain.execNativeCycle(timeCodeStart, timeCodeDuration, nullptr); //1
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // ... any number of execNativeCycle here
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //2
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //3
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //4
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // we must call "stop" in its own thread, because it will wait for the last cycle
  bool stopReturned = false;
  std::thread stoppingThread([&]{portChain.stop(); stopReturned = true;});
  stoppingThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the stopping thread some time to start


  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //5
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  // this is the last cycle (the flag "lastCycle" is now set).
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //6
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  const int expectedInvokations = 6;

  CPPUNIT_ASSERT(stopReturned);
  CPPUNIT_ASSERT(javaTreadHasEnded);

  CPPUNIT_ASSERT(portChain.isStoppedState());

  portChain.unregisterAtServer(nullptr);
  CPPUNIT_ASSERT(portChain.isUnregisteredState());

  portChain.uninitialize(nullptr);
  CPPUNIT_ASSERT(portChain.isDeletableState());

  // to access the control ports, we must remove them from the port-chain this is tricky
  unique_ptr<InputPortMock> startControlAfter =
          unique_dynamic_cast<InputPortMock, Port > (portChain.removePort(nullptr, dummyClient, -2));
  unique_ptr<OutputPortMock> endControlAfter =
          unique_dynamic_cast<OutputPortMock, Port > (portChain.removePort(nullptr, dummyClient, -1));

  CPPUNIT_ASSERT(startControlAfter->isDeletableState());
  CPPUNIT_ASSERT_EQUAL(1, startControlAfter->initialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, startControlAfter->register_implCount);
  CPPUNIT_ASSERT_EQUAL(1, startControlAfter->start_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, startControlAfter->execJavaProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, startControlAfter->execNativeProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(1, startControlAfter->stop_implCount);
  CPPUNIT_ASSERT_EQUAL(1, startControlAfter->lastCycleCount);
  CPPUNIT_ASSERT_EQUAL(1, startControlAfter->uninitialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, startControlAfter->unregister_implCount);
  CPPUNIT_ASSERT(!startControlAfter->hasProcessException());

  CPPUNIT_ASSERT(endControlAfter->isDeletableState());
  CPPUNIT_ASSERT_EQUAL(1, endControlAfter->initialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, endControlAfter->register_implCount);
  CPPUNIT_ASSERT_EQUAL(1, endControlAfter->start_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, endControlAfter->execJavaProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, endControlAfter->execNativeProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(1, endControlAfter->lastCycleCount);
  CPPUNIT_ASSERT_EQUAL(1, endControlAfter->stop_implCount);
  CPPUNIT_ASSERT_EQUAL(1, endControlAfter->uninitialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, endControlAfter->unregister_implCount);
  CPPUNIT_ASSERT(!endControlAfter->hasProcessException());
}

/**
 * Testing the whole live cycle on a portchain that has
 * ports before it was opened.
 */
void portchainTest::testOpenClose_IncludedPort() {
  void * dummyClient = (void*) - 1;
  unsigned long timeCodeStart = 12345;
  const unsigned long timeCodeDuration = 123;
  PortChainMock portChain;
  CPPUNIT_ASSERT(portChain.isCreatedState());

  unique_ptr<InputPortMock> startControl = unique_ptr<InputPortMock > (new InputPortMock(-2));
  unique_ptr<OutputPortMock> endControl = unique_ptr<OutputPortMock > (new OutputPortMock(-1)); //end control


  portChain.initialize(nullptr, nullptr,
          move(startControl), //start control
          move(endControl)); //end control
  CPPUNIT_ASSERT(portChain.isInitializedState());

  long id = newPortId++;
  unique_ptr<InputPortMock> port = unique_ptr<InputPortMock > (new InputPortMock(id));
  port->initialize(nullptr, nullptr, nullptr);

  portChain.addPort(move(port), nullptr);
  CPPUNIT_ASSERT(!static_cast<bool> (port)); // assert that port now is nullptr (because of move)


  portChain.registerAtServer(nullptr);
  CPPUNIT_ASSERT(portChain.isRegisteredState());



  // we must start the java thread before calling "execNativeCycle",
  // because without a running java thread a call to "execNativeCycle"
  // will never return.
  bool javaTreadHasEnded = false;
  std::thread javaThread([&]{portChain.runJava(nullptr); javaTreadHasEnded = true;});
  javaThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.start();
  CPPUNIT_ASSERT(portChain.isRunningState());


  portChain.execNativeCycle(timeCodeStart, timeCodeDuration, nullptr); //1
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // ... any number of execNativeCycle here
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //2
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //3
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //4
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // we must call "stop" in its own thread, because it will wait for the last cycle
  bool stopReturned = false;
  std::thread stoppingThread([&]{portChain.stop(); stopReturned = true;});
  stoppingThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the stopping thread some time to start

  // when we started the stopping thread, the java thread has already started a new cycle
  // and is waiting for the native thread
  // => this cycle is executed normally (without lastCycle set).
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //5
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // this is the last cycle (the flag "lastCycle" is now set).
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //6
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  const int expectedInvokations = 6;

  CPPUNIT_ASSERT(stopReturned);
  CPPUNIT_ASSERT(javaTreadHasEnded);

  CPPUNIT_ASSERT(portChain.isStoppedState());

  portChain.shutdown(nullptr, dummyClient);


  unique_ptr<InputPortMock> removedport =
          unique_dynamic_cast<InputPortMock, Port > (portChain.removePort(nullptr, nullptr, id));

  CPPUNIT_ASSERT(static_cast<bool> (removedport)); // assert that removedport is not a nullptr

  CPPUNIT_ASSERT(removedport->isDeletableState());

  CPPUNIT_ASSERT_EQUAL(1, removedport->initialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->register_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->start_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, removedport->execJavaProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, removedport->execNativeProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->lastCycleCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->stop_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->uninitialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->unregister_implCount);

  CPPUNIT_ASSERT(!removedport->hasProcessException());
}

/**
 * Testing the whole live cycle on a portchain that gets 
 * an input-port while it is in running state.
 */
void portchainTest::testAddInputPort() {
  void * dummyClient = (void*) - 1;
  unsigned long timeCodeStart = 12345;
  const unsigned long timeCodeDuration = 123;
  PortChainMock portChain;
  CPPUNIT_ASSERT(portChain.isCreatedState());

  unique_ptr<InputPortMock> startControl = unique_ptr<InputPortMock > (new InputPortMock(-2));
  unique_ptr<OutputPortMock> endControl = unique_ptr<OutputPortMock > (new OutputPortMock(-1)); //end control


  portChain.initialize(nullptr, nullptr,
          move(startControl), //start control
          move(endControl)); //end control
  CPPUNIT_ASSERT(portChain.isInitializedState());

  portChain.registerAtServer(nullptr);
  CPPUNIT_ASSERT(portChain.isRegisteredState());

  // we must start the java thread before calling "execNativeCycle",
  // because without a running java thread a call to "execNativeCycle"
  // will never return.
  bool javaTreadHasEnded = false;
  std::thread javaThread([&]{portChain.runJava(nullptr); javaTreadHasEnded = true;});
  javaThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.start();
  CPPUNIT_ASSERT(portChain.isRunningState());


  portChain.execNativeCycle(timeCodeStart, timeCodeDuration, nullptr); //x
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //x
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  // now we add a port and do two cycles.
  long id = newPortId++;
  unique_ptr<InputPortMock> port = unique_ptr<InputPortMock > (new InputPortMock(id));
  port->initialize(nullptr, nullptr, nullptr);
  portChain.addPort(move(port), dummyClient);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //1
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //2
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // we remove the port and do again some empty cycles.
  // we must call "removePort" in its own thread, because it will wait for the last cycle
  bool removeReturned = false;
  unique_ptr<InputPortMock> removedport;
  std::thread removingThread([&]{
    removedport = unique_dynamic_cast<InputPortMock, Port > (portChain.removePort(nullptr, dummyClient, id));
    removeReturned = true;
  });
  removingThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the removing thread some time to start


  // this is the last cycle for the stopped port (the flag "lastCycle" is set).
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //3
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // this is cycle a NoOp for the stopped port
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //4
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  CPPUNIT_ASSERT(removeReturned);
  CPPUNIT_ASSERT(static_cast<bool> (removedport)); // assert that removedport is not a nullptr
  const int expectedInvokations = 3;

  // we must call "stop" in its own thread, because it will wait for the last cycle
  bool stopReturned = false;
  std::thread stoppingThread([&]{portChain.stop(); stopReturned = true;});
  stoppingThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the stopping thread some time to start


  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //5
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // this is cycle for the port-chain
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //6
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  CPPUNIT_ASSERT(stopReturned);
  CPPUNIT_ASSERT(javaTreadHasEnded);

  CPPUNIT_ASSERT(portChain.isStoppedState());

  portChain.shutdown(nullptr, dummyClient);

  CPPUNIT_ASSERT(removedport->isDeletableState());

  CPPUNIT_ASSERT_EQUAL(1, removedport->initialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->register_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->start_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, removedport->execJavaProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, removedport->execNativeProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->lastCycleCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->stop_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->uninitialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->unregister_implCount);

  CPPUNIT_ASSERT(!removedport->hasProcessException());
}

/**
 * Testing the whole live cycle on a portchain that gets 
 * an output-port while it is in running state
 * (almost identical to "testAddInputPort()").
 */
void portchainTest::testAddOutputPort() {
  void * dummyClient = (void*) - 1;
  unsigned long timeCodeStart = 12345;
  const unsigned long timeCodeDuration = 123;
  PortChainMock portChain;
  CPPUNIT_ASSERT(portChain.isCreatedState());

  unique_ptr<InputPortMock> startControl = unique_ptr<InputPortMock > (new InputPortMock(-2));
  unique_ptr<OutputPortMock> endControl = unique_ptr<OutputPortMock > (new OutputPortMock(-1)); //end control


  portChain.initialize(nullptr, nullptr,
          move(startControl), //start control
          move(endControl)); //end control
  CPPUNIT_ASSERT(portChain.isInitializedState());

  portChain.registerAtServer(nullptr);
  CPPUNIT_ASSERT(portChain.isRegisteredState());

  // we must start the java thread before calling "execNativeCycle",
  // because without a running java thread a call to "execNativeCycle"
  // will never return.
  bool javaTreadHasEnded = false;
  std::thread javaThread([&]{portChain.runJava(nullptr); javaTreadHasEnded = true;});
  javaThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.start();
  CPPUNIT_ASSERT(portChain.isRunningState());


  portChain.execNativeCycle(timeCodeStart, timeCodeDuration, nullptr); //x
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //x
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  // now we add a port and do two cycles.
  long id = newPortId++;
  unique_ptr<OutputPortMock> port = unique_ptr<OutputPortMock > (new OutputPortMock(id));
  port->initialize(nullptr, nullptr, nullptr);
  portChain.addPort(move(port), dummyClient);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //1
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //2
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // we remove the port and do again some empty cycles.
  // We must call "removePort" in its own thread, because it will wait for the last cycle
  bool removeReturned = false;
  unique_ptr<OutputPortMock> removedport;
  std::thread removingThread([&]{
    removedport = unique_dynamic_cast<OutputPortMock, Port > (portChain.removePort(nullptr, dummyClient, id));
    removeReturned = true;
  });
  removingThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the removing thread some time to start

  // this is the last cycle for the stopped port (the flag "lastCycle" is set).
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //3
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // no-op for the stopped port
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //4
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  CPPUNIT_ASSERT(removeReturned);
  CPPUNIT_ASSERT(static_cast<bool> (removedport)); // assert that removedport is not a nullptr
  const int expectedInvokations = 3;

  // we must call "stop" in its own thread
  bool stopReturned = false;
  std::thread stoppingThread([&]{portChain.stop(); stopReturned = true;});
  stoppingThread.detach();
  std::this_thread::sleep_for(std::chrono::milliseconds(10)); // give the stopping thread some time to start

  // when we started the stopping thread, the java was already waiting for the native thread
  // => this cycle is executed normally (without lastCycle set).
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //5
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  // this is the last cycle (the flag "lastCycle" is now set).
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //6
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  CPPUNIT_ASSERT(stopReturned);
  CPPUNIT_ASSERT(javaTreadHasEnded);

  CPPUNIT_ASSERT(portChain.isStoppedState());

  // it is allowed to call "execNativeCycle" on a stopped port-chain (this shall be a no-op)
  portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, nullptr); //7
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  portChain.shutdown(nullptr, dummyClient);

  CPPUNIT_ASSERT(removedport->isDeletableState());

  CPPUNIT_ASSERT_EQUAL(1, removedport->initialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->register_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->start_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, removedport->execJavaProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(expectedInvokations, removedport->execNativeProcess_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->lastCycleCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->stop_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->uninitialize_implCount);
  CPPUNIT_ASSERT_EQUAL(1, removedport->unregister_implCount);

  CPPUNIT_ASSERT(!removedport->hasProcessException());
}

class ThreadRunner {
public:

  int nativeCyclecount = 0;
  bool nativeLoopEnded = false;

  ThreadRunner() {
  }

  /**
   * Helper procedure for testProcessFlipFlop() 
   * @param port
   */
  void runNativeLoop(PortChain& portChain, void * client) {
    nativeLoopEnded = false;
    unsigned long timeCodeStart = 12345;
    const unsigned long timeCodeDuration = 123;
    while (portChain.isRunningState()) {
      nativeCyclecount++;
      portChain.execNativeCycle((timeCodeStart += timeCodeDuration), timeCodeDuration, client);
    }
    nativeLoopEnded = true;
  }
};

/**
 * Testing the portchain at full speed.
 * Specification:
 * The basic cycle should use far less than half a millisecond to execute.
 */
void portchainTest::testFullSpeed() {
  void * dummyClient = (void*) - 1;
  portCount = 0;
  {
    PortChainMock portChain;
    portChain.initialize(nullptr, nullptr,
            unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
            unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control

    unique_ptr<Port> port_i = unique_ptr<Port > (new InputPortMock(newPortId++));
    unique_ptr<Port> port_o = unique_ptr<Port > (new OutputPortMock(newPortId++));
    port_i->initialize(nullptr, nullptr, nullptr);
    port_o->initialize(nullptr, nullptr, nullptr);

    portChain.addPort(move(port_i), nullptr);
    portChain.addPort(move(port_o), nullptr);

    portChain.registerAtServer(dummyClient);



    portChain.start();
    CPPUNIT_ASSERT(portChain.isRunningState());

    ThreadRunner nativeRunner;
    thread nativeThread([&]{nativeRunner.runNativeLoop(portChain, dummyClient);});
    nativeThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // note: unlike the testOpenClose(), here we start the java thread as second.
    // Just to make sure that both variant work.
    bool javaTreadHasEnded = false;
    std::thread javaThread([&]{portChain.runJava(nullptr); javaTreadHasEnded = true;});
    javaThread.detach();

    // let it run for two seconds
    const int runningMilliSec = 2000;
    std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));


    // stop
    portChain.stop();
    CPPUNIT_ASSERT(portChain.isStoppedState());
    CPPUNIT_ASSERT(javaTreadHasEnded);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CPPUNIT_ASSERT(nativeRunner.nativeLoopEnded);

    portChain.shutdown(nullptr, dummyClient);

    // if one cycle took more than 0.5 milliseconds we issue a warning
    if (nativeRunner.nativeCyclecount < (runningMilliSec * 2)) {
      cerr << "  performance? one cycle took " << (double) runningMilliSec / (double) nativeRunner.nativeCyclecount << " milliseconds\n";
    }

    // if one cycle took more than 2 milliseconds the test fails.
    CPPUNIT_ASSERT(nativeRunner.nativeCyclecount > (int) (runningMilliSec / 2));
  }
  // the portchain is out of scope (is deleted), so all ports should be deleted too.
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * A helper class for the "testRanomAddRemovePorts()" test below.
 */
class AddRemover {
private:
  mutex portIdsMutex;
  mutex randomizerMutex;
  typedef unique_lock<mutex> Lock;
  /**
   * The list of port IDs currently in the chain.
   * Protected by portIdsMutex.
   */
  vector<long> portIds;

  /**
   * Randomizer variables. Protected by randomizerMutex.
   */
  mt19937 randomGenerator;
  uniform_int_distribution<uint32_t> portIdDist;
  uniform_int_distribution<uint32_t> timingDist;
  bernoulli_distribution boolDist;

  bool getRandomBool() {
    Lock lock(randomizerMutex);
    return boolDist(randomGenerator);
  }

  int getRandomPortIdIdx(int max) {
    Lock lock(randomizerMutex);
    return portIdDist(randomGenerator) % max;
  }

  int getRandomTime() {
    Lock lock(randomizerMutex);
    return timingDist(randomGenerator);
  }

  /**
   * pick at random one of the ports for removal.
   * @return a portID that is currently in the port chain or 
   * the invalid ID if the port-chain is empty.
   */
  long removeId() {
    Lock lock(portIdsMutex);
    if (portIds.empty()) {
      return PortInvalidId;
    } else {
      int idx = getRandomPortIdIdx(portIds.size());
      long result = portIds[idx];
      portIds.erase(portIds.begin() + idx);
      return result;
    }
  }

  /**
   * Add an ID to the list of ports currently in use.
   * @param newId
   */
  void storeId(long newId) {
    Lock lock(portIdsMutex);
    portIds.push_back(newId);
  }

  int getPortCount() {
    Lock lock(portIdsMutex);
    auto result = portIds.size();
    return result;
  }

public:
  atomic<bool> more;
  atomic<int> totalJavaCycles;
  atomic<int> totalNativeCycles;

  AddRemover() :
  more(true),
  randomGenerator(0),
  portIdDist(0, 16 * MAX_PORTS),
  timingDist(0, 25), // the waiting time in millis between two insertions respectively removals.
  boolDist(0.5),
  totalJavaCycles(0),
  totalNativeCycles(0) {
  }

public:

  /**
   * Randomly adds ports to the given port-chain.
   * @param portChain
   * @param client
   */
  void addLoop(PortChain& portChain, void * client) {
    int addcount = 0;
    int noSuccesscount = 0;
    while (more) {
      if (getPortCount() < (MAX_PORTS - 2)) {
        long newId = newPortId++;
        unique_ptr<Port> port;
        bool shallBeInput = getRandomBool();
        if (shallBeInput) {
          port = unique_ptr<Port > (new InputPortMock(newId));
        } else {
          port = unique_ptr<Port > (new OutputPortMock(newId));
        }
        port->initialize(nullptr, nullptr, nullptr);
        try {
          portChain.addPort(move(port), client);
          storeId(newId);
          addcount++;
        } catch (...) {
          noSuccesscount++;
          port->shutdown(nullptr, nullptr, true);
        }

      }
      // wait for some random time (between 0 to 25 milliseconds)
      this_thread::sleep_for(std::chrono::milliseconds(getRandomTime()));

    }
    cerr << "  testRandomAddRemovePorts: number of add-executions " << addcount << "\n";
    cerr << "  testRandomAddRemovePorts: unsuccessful add-executions " << noSuccesscount << "\n";
  }

  /**
   * Randomly removes ports from the port-chain.
   * @param portChain
   * @param client
   */
  void removeLoop(PortChain& portChain, void * client) {
    int removecount = 0;
    while (more) {
      long toRemove = removeId();
      if (toRemove != PortInvalidId) {
        unique_ptr<PortMock> removedport =
                unique_dynamic_cast<PortMock, Port > (portChain.removePort(nullptr, client, toRemove));

        CPPUNIT_ASSERT(static_cast<bool> (removedport));
        totalJavaCycles = totalJavaCycles + removedport->execJavaProcess_implCount;
        totalNativeCycles = totalNativeCycles + removedport->execNativeProcess_implCount;
        removecount++;
      }
      // wait for some random time (between 0 to 25 milliseconds)
      this_thread::sleep_for(std::chrono::milliseconds(getRandomTime()));

    }
    cerr << "  testRandomAddRemovePorts: number of remove-executions " << removecount << "\n";
  }
};

/**
 * Testing the portchain at full speed.
 * Specification:
 * The basic cycle should use far less than half a millisecond to execute.
 */
void portchainTest::testRandomAddRemovePorts() {
  void * dummyClient = (void*) - 1;
  portCount = 0;
  {
    PortChainMock portChain;
    portChain.initialize(nullptr, nullptr,
            unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
            unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control
    portChain.registerAtServer(dummyClient);

    bool javaTreadHasEnded = false;
    std::thread javaThread([&]{portChain.runJava(nullptr); javaTreadHasEnded = true;});
    javaThread.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    portChain.start();
    CPPUNIT_ASSERT(portChain.isRunningState());

    ThreadRunner nativeRunner;
    thread nativeThread([&]{nativeRunner.runNativeLoop(portChain, dummyClient);});
    nativeThread.detach();

    AddRemover addRemover;
    thread addingThread([&]{addRemover.addLoop(portChain, dummyClient);});
    thread removingThread([&]{addRemover.removeLoop(portChain, dummyClient);});

    const int runningMilliSec = 1000;
    std::this_thread::sleep_for(std::chrono::milliseconds(runningMilliSec));

    // stop adding and removing thread
    addRemover.more = false;
    addingThread.join();
    removingThread.join();


    // stop native and java thread
    portChain.stop();
    CPPUNIT_ASSERT(portChain.isStoppedState());
    CPPUNIT_ASSERT(javaTreadHasEnded);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CPPUNIT_ASSERT(nativeRunner.nativeLoopEnded);

    portChain.shutdown(nullptr, dummyClient);

    //    CPPUNIT_ASSERT(nativeRunner.nativeCyclecount > runningMilliSec);
    //    CPPUNIT_ASSERT(addRemover.totalJavaCycles > runningMilliSec);
    //    CPPUNIT_ASSERT(addRemover.totalNativeCycles > runningMilliSec);
    cerr << "  testRandomAddRemovePorts: nativeRunner.nativeCyclecount " << nativeRunner.nativeCyclecount << "\n";
    cerr << "  testRandomAddRemovePorts: addRemover.totalJavaCycles " << addRemover.totalJavaCycles << "\n";
    cerr << "  testRandomAddRemovePorts: addRemover.totalNativeCycles " << addRemover.totalNativeCycles << "\n";
  }
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * Testing that the maximum number of ports can be added
 * Specification:
 * At least (MAX_PORTS - 2) ports can be added to a port- chain.
 */
void portchainTest::testAddMaximumPorts() {
  void * dummyClient = (void*) - 1;
  portCount = 0;
  { // output ports
    PortChainMock portChain;
    portChain.initialize(nullptr, nullptr,
            unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
            unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control
    portChain.registerAtServer(dummyClient);

    for (int i = 0; i < (MAX_PORTS - 2); i++) {
      long id = newPortId++;
      unique_ptr<OutputPortMock> port = unique_ptr<OutputPortMock > (new OutputPortMock(id));
      port->initialize(nullptr, nullptr, nullptr);
      portChain.addPort(move(port), dummyClient);

    }
    CPPUNIT_ASSERT_EQUAL((MAX_PORTS), portCount);
    portChain.shutdown(nullptr, dummyClient);
  }
  CPPUNIT_ASSERT_EQUAL(0, portCount);

  { // input ports
    PortChainMock portChain;
    portChain.initialize(nullptr, nullptr,
            unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
            unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control
    portChain.registerAtServer(dummyClient);

    for (int i = 0; i < (MAX_PORTS - 2); i++) {
      long id = newPortId++;
      unique_ptr<InputPortMock> port = unique_ptr<InputPortMock > (new InputPortMock(id));
      port->initialize(nullptr, nullptr, nullptr);
      portChain.addPort(move(port), dummyClient);

    }
    CPPUNIT_ASSERT_EQUAL((MAX_PORTS), portCount);
    portChain.shutdown(nullptr, dummyClient);
  }
  CPPUNIT_ASSERT_EQUAL(0, portCount);

  { // input ports: add two, remove one
    PortChainMock portChain;
    portChain.initialize(nullptr, nullptr,
            unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
            unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control
    portChain.registerAtServer(dummyClient);

    for (int i = 0; i < (MAX_PORTS - 2); i++) {
      long id1 = newPortId++;
      auto port1 = unique_ptr<InputPortMock > (new InputPortMock(id1));
      port1->initialize(nullptr, nullptr, nullptr);
      portChain.addPort(move(port1), dummyClient);
      if (i < (MAX_PORTS - 3)) {
        long id2 = newPortId++;
        auto port2 = unique_ptr<InputPortMock > (new InputPortMock(id2));
        port2->initialize(nullptr, nullptr, nullptr);
        portChain.addPort(move(port2), dummyClient);
        portChain.removePort(nullptr, nullptr, id1);
      }

    }
    CPPUNIT_ASSERT_EQUAL((MAX_PORTS), portCount);
    portChain.shutdown(nullptr, dummyClient);
  }
  CPPUNIT_ASSERT_EQUAL(0, portCount);

  { // output  ports: add two, remove one
    PortChainMock portChain;
    portChain.initialize(nullptr, nullptr,
            unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
            unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control
    portChain.registerAtServer(dummyClient);

    for (int i = 0; i < (MAX_PORTS - 2); i++) {
      long id1 = newPortId++;
      auto port1 = unique_ptr<OutputPortMock > (new OutputPortMock(id1));
      port1->initialize(nullptr, nullptr, nullptr);
      portChain.addPort(move(port1), dummyClient);
      if (i < (MAX_PORTS - 3)) {
        long id2 = newPortId++;
        auto port2 = unique_ptr<OutputPortMock > (new OutputPortMock(id2));
        port2->initialize(nullptr, nullptr, nullptr);
        portChain.addPort(move(port2), dummyClient);
        portChain.removePort(nullptr, nullptr, id1);
      }

    }
    CPPUNIT_ASSERT_EQUAL((MAX_PORTS), portCount);
    portChain.shutdown(nullptr, dummyClient);
  }
  CPPUNIT_ASSERT_EQUAL(0, portCount);

  { // mixed ports
    bool shallBeInput = false;
    PortChainMock portChain;
    portChain.initialize(nullptr, nullptr,
            unique_ptr<InputPortMock > (new InputPortMock(-2)), //start control
            unique_ptr<OutputPortMock > (new OutputPortMock(-1))); //end control
    portChain.registerAtServer(dummyClient);

    for (int i = 0; i < (MAX_PORTS - 2); i++) {
      long id = newPortId++;
      unique_ptr<Port> port;
      if (shallBeInput) {
        port = unique_ptr<Port > (new InputPortMock(id));
      } else {
        port = unique_ptr<Port > (new OutputPortMock(id));
      }

      port->initialize(nullptr, nullptr, nullptr);
      portChain.addPort(move(port), dummyClient);
      shallBeInput = !shallBeInput;
    }


    CPPUNIT_ASSERT_EQUAL((MAX_PORTS), portCount);
    portChain.shutdown(nullptr, dummyClient);
  }

  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

