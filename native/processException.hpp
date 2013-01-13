/*
 * File:   processException.hpp
 *
 * Created on August 27, 2012, 12:44 PM
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


#ifndef PROCESSEXCEPTION_HPP
#define	PROCESSEXCEPTION_HPP

#include <exception>
#include <string>

#include <jni.h>
#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher.\n"
#endif

#include "util.hpp"
#include <mutex>

using std::exception;
using std::mutex;
using std::lock_guard;
using std::string;

/** ProcessException represent problems detected when running 
 * within the process loops.
 */
class ProcessException {
public:
  /**
   * Throws an appropriate Exception in the java environment.
   * @param env the java interface pointer
   */
  virtual void throwIntoJava(JNIEnv *env) = 0;
};

/** NativeProcessException represent problems detected when running the
 * native process loop.
 */
class NativeProcessException : public ProcessException {
private:
  const string _what;

public:

  /** Takes a C++ exception object describing the error.  
   * @param cause the cause of the problem in the native Thread.
   */
  explicit NativeProcessException(const exception& cause) :
  _what(cause.what()) {
  }

  /**
   * Indicates what caused the error.
   * @return a string describing the general cause of the error.  
   */
  const string& what() const {
    return _what;
  }

  /**
   * Throw the stored Exception into the java environment.
   * @param env pointer to the java environment
   */
  virtual void throwIntoJava(JNIEnv *env) {

    jclass jexceptionCls = env->FindClass("java/lang/Exception");
    if (jexceptionCls == NULL) {
      // unable to find Java class, give up..
      return;
    }
    env->ThrowNew(jexceptionCls, _what.c_str());

  }
};

/** NativeProcessException represent problems detected when running the
 * native process loop.
 */
class JavaProcessException : public ProcessException {
private:
  jthrowable _cause;
protected:
  typedef lock_guard<mutex> Lock;
  mutex thisMutex;

public:

  /** 
   * Takes a Java exception object describing the error. 
   * @param env pointer to the java environment
   * @param cause the cause of the problem in the Java Thread.
   */
  explicit JavaProcessException(JNIEnv *env, const jthrowable cause) : _cause(NULL) {
    Lock lck(thisMutex); //synchronized(this)
    //pin the exception object, so it can be used across java threads.
    _cause = static_cast<jthrowable> (env->NewGlobalRef(cause));
  }

  /**
   * Throw the stored Exception into the java environment.
   * @param env pointer to the java environment
   */
  virtual void throwIntoJava(JNIEnv *env) {
    Lock lck(thisMutex); //synchronized(this)
    if (_cause == NULL) {
      Util::throwProcessException(env, "Programming Error: Original Exception already retrieved.", NULL);
      return;
    }

    // throw the cached exception into the current Java environment
    env->Throw(_cause);
    //release the exception object, so it can be garbage collected.
    env->DeleteGlobalRef(_cause);
    _cause = NULL;
  }

};

#endif	/* PROCESSEXCEPTION_HPP */

