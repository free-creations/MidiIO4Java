/*
 * File:   processExceptionTest.hpp
 * Author: Harald Postner
 *
 * Created on Aug 27, 2012, 2:50:50 PM
 */

#ifndef PROCESSEXCEPTIONTEST_HPP
#define	PROCESSEXCEPTIONTEST_HPP

#include <cppunit/extensions/HelperMacros.h>

class processExceptionTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(processExceptionTest);

  CPPUNIT_TEST(testProcessException);
  CPPUNIT_TEST(testCause);

  CPPUNIT_TEST_SUITE_END();

public:
  processExceptionTest();
  virtual ~processExceptionTest();
  void setUp();
  void tearDown();

private:
  void testProcessException();
  void testCause();

};

#endif	/* PROCESSEXCEPTIONTEST_HPP */

