/*
 * File:   ptrEnvelopeTest.hpp
 * Author: Harald Postner
 *
 * Created on Oct 18, 2012, 5:28:18 PM
 */

#ifndef PTRENVELOPETEST_HPP
#define	PTRENVELOPETEST_HPP

#include <cppunit/extensions/HelperMacros.h>

class ptrEnvelopeTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(ptrEnvelopeTest);


  CPPUNIT_TEST(testUsecount);
  CPPUNIT_TEST(testPointerMoveSemantic);
  CPPUNIT_TEST(testPointerRemoveSemantic);
  CPPUNIT_TEST(testAccessItem);

  CPPUNIT_TEST(testIsEmpty_HasItem);
  CPPUNIT_TEST(testSharedReadAccess);
  CPPUNIT_TEST(testTimeoutIn_removeItemWait);
  CPPUNIT_TEST(testTimeoutIn_setItemWait);

  CPPUNIT_TEST_SUITE_END();

public:
  ptrEnvelopeTest();
  virtual ~ptrEnvelopeTest();
  void setUp();

private:
  void testUsecount();
  void testPointerMoveSemantic();
  void testPointerRemoveSemantic();
  void testAccessItem();

  void testIsEmpty_HasItem();
  void testSharedReadAccess();
  void testTimeoutIn_removeItemWait();
  void testTimeoutIn_setItemWait();

};

#endif	/* PTRENVELOPETEST_HPP */

