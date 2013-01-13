/*
 * File:   processExceptionTest.cpp
 * Author: Harald Postner
 *
 * Created on Aug 27, 2012, 2:50:51 PM
 */

#include "processExceptionTest.hpp"
#include "processException.hpp"
#include <exception>
#include <string>


CPPUNIT_TEST_SUITE_REGISTRATION(processExceptionTest);

processExceptionTest::processExceptionTest() {
}

processExceptionTest::~processExceptionTest() {
}

void processExceptionTest::setUp() {
}

void processExceptionTest::tearDown() {
}

class TestException : public std::exception {
private:
  const string _message;
public:

  TestException(const std::string& message) : _message(message) {
  }

  virtual const char* what() const throw () {
    return _message.c_str();
  }

};

/**
 * Creation of a new NativeProcessException must not throw an error.
 */
void processExceptionTest::testProcessException() {
  const std::string message = "Test Cause";
  const TestException cause(message);
  NativeProcessException* ne = new NativeProcessException(cause);
  CPPUNIT_ASSERT(ne != NULL);
}

void processExceptionTest::testCause() {
  const std::string message = "Test Cause";
  const TestException cause(message);
  NativeProcessException* ne = new NativeProcessException(cause);
  CPPUNIT_ASSERT_EQUAL(message, ne->what());
}

