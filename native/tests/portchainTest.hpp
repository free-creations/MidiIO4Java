/*
 * File:   portchainTest.hpp
 * Author: Harald Postner
 *
 * Created on Sep 1, 2012, 3:54:10 PM
 */

#ifndef PORTCHAINTEST_HPP
#define	PORTCHAINTEST_HPP

#include <cppunit/extensions/HelperMacros.h>

class portchainTest : public CPPUNIT_NS::TestFixture {
  CPPUNIT_TEST_SUITE(portchainTest);

  CPPUNIT_TEST(testCreateDeleteVirgin);
  CPPUNIT_TEST(testCreateDeleteRegistered);
  CPPUNIT_TEST(testCreateDeleteWithPort);

  CPPUNIT_TEST(testFindSlotForInputPort);
  CPPUNIT_TEST(testFindSlotForOutputPort);


  CPPUNIT_TEST(testOpenClose_Empty);
  CPPUNIT_TEST(testOpenClose_IncludedPort);
  CPPUNIT_TEST(testAddInputPort);
  CPPUNIT_TEST(testAddOutputPort);
  CPPUNIT_TEST(testFullSpeed);
  CPPUNIT_TEST(testRandomAddRemovePorts);
  CPPUNIT_TEST(testAddMaximumPorts);

  CPPUNIT_TEST_SUITE_END();

public:
  portchainTest();
  virtual ~portchainTest();
  void setUp();
  void tearDown();

private:
  void testCreateDeleteVirgin();
  void testCreateDeleteRegistered();
  void testCreateDeleteWithPort();

  void testFindSlotForInputPort();
  void testFindSlotForOutputPort();

  void testOpenClose_Empty();
  void testOpenClose_IncludedPort();
  void testAddInputPort();
  void testAddOutputPort();
  void testFullSpeed();
  void testRandomAddRemovePorts();
  void testAddMaximumPorts();



};

#endif	/* PORTCHAINTEST_HPP */

