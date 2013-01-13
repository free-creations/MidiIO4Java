/*
 * File:   ptrEnvelopeTest.cpp
 * Author: Harald Postner
 *
 * Created on Oct 18, 2012, 5:28:20 PM
 */
#include <memory>
#include <thread>
#include <chrono>
#include "ptrEnvelopeTest.hpp"
#include "../ptrEnvelope.hpp"
#include "../port.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION(ptrEnvelopeTest);

static long newPortId = 1;
static int portCount = 0;

void ptrEnvelopeTest::setUp() {
  portCount = 0;
}

/**
 * A mockup class that permits to follow creation and deletion
 * by inspecting the global variable "portCount".
 */
class PortMock : public Port {
public:

  PortMock(long _portId) :
  Port(true, _portId) {
    portCount++;
    shutdown(nullptr, nullptr, false); // to avoid nagging errors on sysout "port deleted in wrong state"
  }

  virtual ~PortMock() {
    portCount--;
  }

  PortMock(PortMock &&) = default;

  PortMock(const PortMock&) = delete;

protected:

  virtual void initialize_impl(JNIEnv * env, jstring name, jobject listener)override {
  }

  virtual void register_impl(void * client)override {
  }

  virtual void start_impl()override {
  }

  virtual void execJavaProcess_impl(JNIEnv * env, unsigned long timeCodeStart, unsigned long timeCodeDuration, bool lastCycle)override {
  }

  virtual void execNativeProcess_impl(unsigned long startTimeCode, unsigned long bufferSize, void * client)override {
  }

  virtual void stop_impl()override {
  }

  virtual void uninitialize_impl(JNIEnv * env) override {
  }

  virtual void unregister_impl(void * client)override {
  }

};

ptrEnvelopeTest::ptrEnvelopeTest() {
}

ptrEnvelopeTest::~ptrEnvelopeTest() {
}

/**
 * When a new Accessor is created, the use-count must increment.
 * When an Accessor goes out of scope, the use-count must decrement.
 */
void ptrEnvelopeTest::testUsecount() {

  PtrEnvelope envelope;
  CPPUNIT_ASSERT_EQUAL(0, envelope.getUseCount());
  {
    auto accessor1 = envelope.makeAccessor();
    CPPUNIT_ASSERT_EQUAL(1, envelope.getUseCount());
    {
      auto accessor2 = envelope.makeAccessor();
      CPPUNIT_ASSERT_EQUAL(2, envelope.getUseCount());
    }
    CPPUNIT_ASSERT_EQUAL(1, envelope.getUseCount());
  }
  CPPUNIT_ASSERT_EQUAL(0, envelope.getUseCount());
}



/**
 * When an object is moved into the envelope, the original pointer
 * must become empty.
 * When an envelope goes out of scope, any object held by the
 * envelope must be deleted.
 */
void ptrEnvelopeTest::testPointerMoveSemantic() {

  auto originalPtr = unique_ptr<Port > (new PortMock(newPortId++));
  CPPUNIT_ASSERT_EQUAL(1, portCount);
  {
    PtrEnvelope envelope;
    CPPUNIT_ASSERT(envelope.isEmpty());
    envelope.setItemWait(move(originalPtr));
    CPPUNIT_ASSERT(envelope.hasItem());
    // still one port
    CPPUNIT_ASSERT_EQUAL(1, portCount);
    // ... but it belongs to the envelope; originalPtr should be empty.
    CPPUNIT_ASSERT(originalPtr == false);
  }
  // envelope went out of scope; the port should also be deleted
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * When an object is removed from the envelope, the envelope
 * must become empty.
 */
void ptrEnvelopeTest::testPointerRemoveSemantic() {
  portCount = 0;
  long id = newPortId++;
  auto originalPtr = unique_ptr<Port > (new PortMock(id));
  CPPUNIT_ASSERT_EQUAL(1, portCount);
  {
    PtrEnvelope envelope;
    envelope.setItemWait(move(originalPtr));
    CPPUNIT_ASSERT(envelope.hasItem());
    // still one port
    CPPUNIT_ASSERT_EQUAL(1, portCount);

    // we remove the item
    auto receivingPtr = envelope.removeItemWait();
    // the envelope should be empty by now.
    CPPUNIT_ASSERT(envelope.isEmpty());
    // the receiving pointer should not be null by now.
    CPPUNIT_ASSERT(receivingPtr != false);
    // the receiving pointer should point to the initial port.
    CPPUNIT_ASSERT_EQUAL(id, receivingPtr->getId());
    // still one port
    CPPUNIT_ASSERT_EQUAL(1, portCount);
  }
  // receivingPtr went out of scope; the port should also be deleted
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * It shall be possible to access the item through the Accessor.
 * It shall be impossible to change the pointer through the Accessor.
 */
void ptrEnvelopeTest::testAccessItem() {

  long id1 = newPortId++;

  PtrEnvelope envelope;

  // lets shift a new item into the envelope. It has an identity given by id1.
  envelope.setItemWait(unique_ptr<Port > (new PortMock(id1)));

  auto accessor = envelope.makeAccessor();

  // lets verify that the item pointed by the Accessor is well the item we just made.
  CPPUNIT_ASSERT_EQUAL(id1, accessor.get()->getId());

  // uncommenting the following line must give a compiler error!
  //unique_ptr<Port> bastard = move(accessor.get());

  // also uncommenting the following line must give a compiler error!
  //accessor.get() = unique_ptr<Port> (new PortMock(id1));

}

/**
 * Specification:
 * isEmpty() shall return true on an empty envelope.
 * isEmpty() shall return false on an envelope pointing to an existing item.
 */
void ptrEnvelopeTest::testIsEmpty_HasItem() {


  PtrEnvelope envelope;
  portCount = 0;

  CPPUNIT_ASSERT(envelope.makeAccessor().isEmpty());
  CPPUNIT_ASSERT(!envelope.makeAccessor().hasItem());
  CPPUNIT_ASSERT_EQUAL(0, portCount);

  unique_ptr<Port > port = unique_ptr<Port > (new PortMock(newPortId++));
  envelope.setItemWait(move(port));

  CPPUNIT_ASSERT(!envelope.makeAccessor().isEmpty());
  CPPUNIT_ASSERT(envelope.makeAccessor().hasItem());
  CPPUNIT_ASSERT_EQUAL(1, portCount);

  envelope.removeItemWait();
  CPPUNIT_ASSERT(envelope.makeAccessor().isEmpty());
  CPPUNIT_ASSERT(!envelope.makeAccessor().hasItem());
  CPPUNIT_ASSERT_EQUAL(0, portCount);

}

/**
 * Helper procedure that holds an Accessor for about 100 milliseconds.
 * @param envelope the envelope on which the thread should grasp an Accessor.
 * @param hasItem is true the procedure will periodically verify that 
 * the envelope still has an item; if false the procedure will periodically verify that 
 * the envelope still is empty.
 */
void accessAndHoldPointer(PtrEnvelope& envelope, bool hasItem) {
  auto accessor = envelope.makeAccessor();
  for (int i = 0; i < 33; i++) {
    CPPUNIT_ASSERT(envelope.getUseCount() > 0);
    CPPUNIT_ASSERT_EQUAL(accessor.hasItem(), hasItem);
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
  }
}

/**
 * Verify the shared read access on an envelope.
 * Specification: several threads shall be able to access the pointer simultaneously.
 * 
 */
void ptrEnvelopeTest::testSharedReadAccess() {
  {
    PtrEnvelope envelope;
    portCount = 0;

    unique_ptr<Port > port = unique_ptr<Port > (new PortMock(newPortId++));
    envelope.setItemWait(move(port));

    std::thread readAccessTread1([&]{accessAndHoldPointer(envelope, true);});
    std::thread readAccessTread2([&]{accessAndHoldPointer(envelope, true);});
    std::thread readAccessTread3([&]{accessAndHoldPointer(envelope, true);});
    std::thread readAccessTread4([&]{accessAndHoldPointer(envelope, true);});

    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    CPPUNIT_ASSERT_EQUAL(4, envelope.getUseCount());
    readAccessTread1.join();
    readAccessTread2.join();
    readAccessTread3.join();
    readAccessTread4.join();
    CPPUNIT_ASSERT_EQUAL(0, envelope.getUseCount());

  }
  CPPUNIT_ASSERT_EQUAL(0, portCount);
}

/**
 * It shall be impossible to change the pointer as long as an Accessor is in use.
 * specification: removeItemWait() will fail with a "timeoutException" if
 * the envelope remains locked for a period longer than the predefined 
 * "maxWaitingTime".
 */
void ptrEnvelopeTest::testTimeoutIn_removeItemWait() {

  PtrEnvelope envelope(5); // we set the maxWaitingTime time to 5 milliseconds to speed up the test.
  //PtrEnvelope envelope; // the default waiting time of half a second (be patient when testing)
  portCount = 0;

  unique_ptr<Port > port = unique_ptr<Port > (new PortMock(newPortId++));
  envelope.setItemWait(move(port));
  CPPUNIT_ASSERT(envelope.hasItem());

  std::thread readAccessTread([&]{accessAndHoldPointer(envelope, true);});
  std::this_thread::sleep_for(std::chrono::milliseconds(5)); // make sure the thread has started

  bool timeoutDetected = false;
  bool unexpectedException = false;
  try {
    // provoke a timeout (the readAccessTread holds blocks the Accessor for more than maxWaitingTime) 
    envelope.removeItemWait();
  } catch (TimeoutException& ex) {
    timeoutDetected = true;
    std::cerr << "Timeout detected:" << ex.what() << std::endl;
  } catch (...) {
    unexpectedException = true;
  }
  readAccessTread.join();
  CPPUNIT_ASSERT(timeoutDetected);
  CPPUNIT_ASSERT(!unexpectedException);

  // verify that the envelope still holds the port
  CPPUNIT_ASSERT_EQUAL(1, portCount);
  CPPUNIT_ASSERT(envelope.hasItem());

}

/**
 * It shall be impossible to change the pointer as long as an Accessor is in use.
 * specification: setItemWait() will fail with a "timeoutException" if
 * the envelope remains locked for a period longer than the predefined 
 * "maxWaitingTime".
 */
void ptrEnvelopeTest::testTimeoutIn_setItemWait() {

  PtrEnvelope envelope(5); // we set the maxWaitingTime time to 5 milliseconds to speed up the test.
  //PtrEnvelope envelope; // the default waiting time of half a second (be patient when testing)
  portCount = 0;

  unique_ptr<Port > port = unique_ptr<Port > (new PortMock(newPortId++));
  CPPUNIT_ASSERT_EQUAL(1, portCount);


  std::thread readAccessTread([&]{accessAndHoldPointer(envelope, false);});
  std::this_thread::sleep_for(std::chrono::milliseconds(5)); // make sure the thread has started

  bool timeoutDetected = false;
  bool unexpectedException = false;
  try {
    // provoke a timeout (the readAccessTread holds blocks the Accessor for more than maxWaitingTime) 
    envelope.setItemWait(move(port));
  } catch (TimeoutException& ex) {
    timeoutDetected = true;
    std::cerr << "Timeout detected:" << ex.what() << std::endl;
  } catch (...) {
    unexpectedException = true;
  }
  readAccessTread.join();

  // that's what we are looking for.
  CPPUNIT_ASSERT(timeoutDetected);
  CPPUNIT_ASSERT(!unexpectedException);

  // verify that the original pointer still holds the port
  CPPUNIT_ASSERT_EQUAL(1, portCount);
  CPPUNIT_ASSERT(port != false);
  CPPUNIT_ASSERT(envelope.isEmpty());

}