/*
 * File:   messages.hpp
 *
 * Created on October 4, 2012, 2:00 PM
 * 
 * Copyright 2012 Harald Postner <Harald at free_creations.de>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef MESSAGES_HPP
#define	MESSAGES_HPP

#include <exception>
#include <string>
#include <mutex>
#include <jni.h>
#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher.\n"
#endif

using std::exception;
using std::mutex;
using std::lock_guard;
using std::string;

/** The TimeoutException is thrown when a thread has waited longer than
 * a sensible period (probably because of a dead-lock).
 *  @brief A subclasse of std::runtime_error.
 */
class TimeoutException : public std::runtime_error {
public:

  /** Takes a character string describing the error.  */
  explicit
  TimeoutException(const std::string& __arg) :
  runtime_error(__arg) {
  };

};

/** 
 * The JavaException is thrown when a call to the Java environment has 
 * failed.
 *  @brief A subclasse of std::runtime_error.
 */
class JavaException : public std::runtime_error {
private:
  jthrowable cause;
  typedef lock_guard<mutex> Lock;
  /**
   * prevents accessing the cause before the exception has been pinned.
   */
  mutable mutex thisMutex;
public:

  /**
   * Constructs a new JavaException.
   * @param env pointer to the java environment.
   * @param _cause the exception object that was thrown in the java environment.
   * @param __arg a string describing where in the C++ source code the problem was detected.
   */
  explicit
  JavaException(JNIEnv *env, const jthrowable _cause, const std::string& __arg) :
  runtime_error(__arg),
  cause(NULL) {
    Lock lck(thisMutex); //synchronized(this)
    env->ExceptionClear();
    //pin the exception object, so it can be used across java threads.
    cause = static_cast<jthrowable> (env->NewGlobalRef(_cause));
  };

  JavaException(const JavaException& other) :
  runtime_error(string(other.what())),
  thisMutex() {
    Lock lck1(thisMutex);
    Lock lck2(other.thisMutex);
    cause = other.cause;

  }

  /**
   * Throw the stored Exception into the java environment.
   * @param env pointer to the java environment
   */
  virtual void throwIntoJava(JNIEnv *env) {
    Lock lck(thisMutex); //synchronized(this)
    if (cause == NULL) {
      return; // give up
    }

    // throw the cached exception into the current Java environment
    env->Throw(cause);
    //release the exception object, so it can be garbage collected.
    env->DeleteGlobalRef(cause);
    cause = NULL;
  }

};
/**
 * Some macros to manage error messages.
 */

#define STRINGIFY(x) #x

#define TOSTRING(x) STRINGIFY(x)

#define AT __FILE__ "(" TOSTRING(__LINE__) "): " 

#define THROW(message) \
  throw std::runtime_error(__FILE__ "(" TOSTRING(__LINE__) "): " message);

#define THROW_TIMEOUT(message) \
  throw TimeoutException(__FILE__ "(" TOSTRING(__LINE__) "): " message);

#define THROW_JAVA(env, cause) \
  throw JavaException(env, cause, __FILE__ "(" TOSTRING(__LINE__) "): Java call failed.");

#endif	/* MESSAGES_HPP */

