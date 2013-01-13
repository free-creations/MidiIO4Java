/*
 * File:   portTest.hpp
 * Author: Harald Postner
 *
 * Created on Aug 30, 2012, 8:20:18 PM
 */

#ifndef PORTTEST_HPP
#define	PORTTEST_HPP

#include <cppunit/extensions/HelperMacros.h>

class portTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(portTest);

  CPPUNIT_TEST(testMoveConstructor);
  CPPUNIT_TEST(testMoveConstructorOnBusyPort);
  CPPUNIT_TEST(testFullLiveCicle_Input);
  CPPUNIT_TEST(testFullLiveCicle_Output);
  CPPUNIT_TEST(testProcessFlipFlopAtMaxSpeed_Output);
  CPPUNIT_TEST(testProcessFlipFlopAtMaxSpeed_Input);
  CPPUNIT_TEST(testBadNativeProcess);
  CPPUNIT_TEST(testBadJavaProcess);
  CPPUNIT_TEST(testBadOpen);
  CPPUNIT_TEST(testRandomTiming);
  //  CPPUNIT_TEST(testTimeoutExceptionInStop);

  CPPUNIT_TEST_SUITE_END();


private:
  void testMoveConstructor();
  void testMoveConstructorOnBusyPort();
  void testFullLiveCicle_Input();
  void testFullLiveCicle_Output();
  void testProcessFlipFlopAtMaxSpeed(bool isOutput);
  void testProcessFlipFlopAtMaxSpeed_Output();
  void testProcessFlipFlopAtMaxSpeed_Input();
  void testBadNativeProcess();
  void testBadJavaProcess();
  void testBadOpen();
  void testRandomTiming();
  void doTestRandomTiming(//
          bool isOutput,
          int initializeDuration,
          int registerDuration,
          int startDuration,
          int execJavaProcessDuration,
          int execNativeProcessDuration,
          int stopDuration,
          int uninitializeDuration,
          int unregisterDuration);

  void testTimeoutExceptionInStop();

  void runFullLiveCicle(bool nativeFirst, bool javaLast, bool outputPort);
};

#endif	/* PORTTEST_HPP */

