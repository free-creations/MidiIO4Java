/*
 * File:   systemListener.hpp
 *
 * Created on August 22, 2012, 11:04 AM
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


#ifndef JACKSYSTEMLISTENER_HPP
#define	JACKSYSTEMLISTENER_HPP

#include <jni.h>
#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher."
#endif

#ifdef __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 \
                + __GNUC_MINOR__ * 100 \
                + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40702
#error "Needs GCC 4.7.2 or better."
#endif
#endif

#include <condition_variable>
#include <mutex>
#include <thread>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "messages.hpp"
#include "util.hpp"


using namespace std;

static bool ignoreCallback = true;

/**
 * The system-listener catches call-backs from the jack server and translates them
 * into java call-backs.
 * </p>
 * <p>
 * The implementation is very similar to the port class.
 * </p>
 */
class JackSystemListener {
private:


  jobject systemListener;
  jmethodID onConnectionChangedMid;
  JavaVM* jvm;

  /**
   * "Lock" is a shorthand for "unique_lock<mutex>".
   */
  typedef unique_lock<mutex> Lock;

  /** 
   * The "stateMutex" must be acquired before accessing the 
   * state of this system-listener.
   */
  mutable mutex stateMutex;

  /**
   * This procedure implements the functionality of the "shutdown"
   * public function, but does not set any lock nor does it manage
   * the state variable, so this function can be called from within 
   * threads that have already locked the stateMutex.
   * @param env
   * @param client
   */
  void shutdown_impl(JNIEnv * env, void * client) {

    switch (state) {
      case uninitialized: return;
      case initialized:
        uninitialize_impl(env);
        return;
      case activated:
        deactivate_impl(client);
        uninitialize_impl(env);
        return;

      case deactivateted:
        uninitialize_impl(env);
        return;
    }
  }

  enum State {
    uninitialized, ///< the system-listener is either just created or it has been shutdown (and can be re-initialized).
    initialized, ///< the system-listener is embeded in the java environment.
    activated, ///< the system-listener has registered with the native Midi System.
    deactivateted ///< the system-listener has unregistered from the native Midi System.
  };


  /**
   * The current main state.
   */
  State state;


  exception_ptr nativeProcessException;

  /**
   * This function will register the
   * given Java-listener-object 
   * 
   * @param env pointer to the java environment.
   * @param listener a java listener that calls-back MidiSystemListener
   * Java class: "public interface MidiIO4Java.MidiSystemListener "
   */
  void initialize_impl(JNIEnv * env, jobject _listener) {
    // pin the listener
    systemListener = env->NewGlobalRef(_listener);
    if (systemListener == NULL) {
      THROW("Call to NewGlobalRef function failed.")
    }
    // --- cache the method IDs of the callback functions provided by the systemListener.
    jclass listenerClass = env->GetObjectClass(systemListener);

    if (listenerClass == NULL) {
      THROW("MidiSystemListener class not found.")
    }
    onConnectionChangedMid = env->GetMethodID(listenerClass, "onConnectionChanged", "()V");

    if ((onConnectionChangedMid == NULL)) {
      THROW("Method-identifier not found.")
    }
    // --- cache a pointer to the java virtual machine. This pointer remains
    // valid across any thread.
    jint errorCode = env->GetJavaVM(&jvm);
    if (errorCode != 0) {
      THROW("Attaching the JVM pointer failed.")
    }
  }

  /**
   * This function will subscribe this system-listener at the Midi server.
   *
   * @param client client identity of this application.
   */
  void activate_impl(void * client) {

    jack_client_t * jackClient = static_cast<jack_client_t *> (client);

    int err = jack_set_port_connect_callback(jackClient, onPortConnect, this);
    if (err != 0) {
      THROW("jack_set_port_connect_callback failed.")
    }
    ignoreCallback = false;
  }

  /**
   * This function shall undo what initialize has done.
   */
  void uninitialize_impl(JNIEnv * env) {
    if (systemListener == NULL) {
      THROW("Invalid null pointer.")
    }
    // free the listener
    env->DeleteGlobalRef(systemListener);
    systemListener = NULL;
    onConnectionChangedMid = NULL;


  }

  /**
   * This function shall un-subscribe this system-listener at the Midi server.
   *
   * @param client client identity of this application.
   */
  void deactivate_impl(void * client) {
    // There is no function to undo "jack_set_port_connect_callback".
    // So we leave the callback function active. Once "deactivate" further callbacks
    // will be ignored because "onPortConnect_impl" will always check 
    // the state variable before accessing the java listener.
    ignoreCallback = true;
  }

  void throwCannot(const string& attemptedAction, int lineNumber, State state) {

    string stateStr;
    switch (state) {
      case uninitialized: stateStr = " in uninitialized state.";
        break;
      case initialized:stateStr = " in initialized state.";
        break;
      case activated:stateStr = " in registered state.";
        break;
      case deactivateted:stateStr = " in unregistered state.";
        break;
    }
    ostringstream errorMessage;
    errorMessage << __FILE__ << "(" << lineNumber << "): Cannot " << attemptedAction << stateStr;
    throw runtime_error(errorMessage.str());
  }

  /**
   * This  function that is called whenever a port is connected or disconnected.
   * Note: All "notification events" are received in a separate non RT thread,
   *
   * @param a one of two ports connected or disconnected. 
   * @param b one of two ports connected or disconnected.
   * @param connect 	non-zero if ports were connected zero if ports were disconnected. 
   * @param _self pointer to the current JackSystemListener object.
   */
  static void onPortConnect(jack_port_id_t a, jack_port_id_t b, int connect, void *_self) {
    if (ignoreCallback) {
      return;
    }
    if (_self != nullptr) {
      JackSystemListener* self = static_cast<JackSystemListener*> (_self);
      try {
        self->onPortConnect_impl(a, b, connect);
      } catch (...) {
        self->nativeProcessException = current_exception();
      }
    }
  }

  void onPortConnect_impl(jack_port_id_t a, jack_port_id_t b, int connect) {
    Lock lock(stateMutex);

    JNIEnv * env;
    jint errCode = jvm->AttachCurrentThread((void**)&env, NULL);
    if (errCode != 0) {
      THROW("AttachCurrentThread failed.")
    }

    env->CallVoidMethod(systemListener, onConnectionChangedMid);

    /** @TODO error handling when java call fails*/

    jvm->DetachCurrentThread();
  }

public:

  /**
   * Creates a system-listener. 
   */
  JackSystemListener() :
  state(uninitialized),
  systemListener(NULL),
  onConnectionChangedMid(NULL),
  jvm(NULL) {
  }
  /**
   * The move constructor is inhibited.
   */
  JackSystemListener(JackSystemListener && other) = delete;

  /**
   * The copy constructor is inhibited, system-listeners must be unique.
   */
  JackSystemListener(const JackSystemListener&) = delete;

  virtual ~JackSystemListener() {
    if (state != uninitialized) {
      // it is not wise to throw an exception in the destructor.
      // At least we can leave a message.
      cerr << "### A Port is deleted in wrong state!!!!" << endl;
    }
  }

  /**
   * Initializes this system-listener for use. Once a system-listener is initialized it is
   * capable to cooperate with the Java environment.
   * @param env pointer to the java thread 
   */
  void initialize(JNIEnv * env, jobject listener) {
    Lock lock(stateMutex);
    if (state != uninitialized) {
      throwCannot("initialize", __LINE__, state);
    }
    initialize_impl(env, listener);
    state = initialized;
  }

  /**
   * Subscribes this system-listener at the Midi server.
   * Tell the JACK server to call the onPortConnect function whenever a
   * port is connected or disconnected.
   * NOTE: this function cannot be called while the client is activated
   * (after jack_activate has been called.)
   *
   * @param client client-identity of this application.
   */
  void activate(void * client) {
    Lock lock(stateMutex);
    if (state != initialized) {
      throwCannot("activate", __LINE__, state);
    }
    activate_impl(client);
    state = activated;
  }

  /**
   * Un-subscribes this system-listener at the Midi server.
   *
   * @param client client-identity of this application.
   */
  void deactivate(void * client) {
    Lock lock(stateMutex);
    if (state != activated) {
      throwCannot("deactivate", __LINE__, state);
    }
    deactivate_impl(client);
    state = deactivateted;
  }

  /**
   * This function shall undo what initialize has done, it shall call
   * the "onClose" callback function of the associated
   * Java listener object. The contract is that the "onClose" callback 
   * is not invoked before the Java callback function (in execJavaProcess_impl) returns.
   * The implementation is deferred to a subclass.
   */
  void uninitialize(JNIEnv * env) {
    Lock lock(stateMutex);
    if ((state != deactivateted) && (state != initialized)) {
      throwCannot("un-initialize", __LINE__, state);
    }
    uninitialize_impl(env);
    state = uninitialized;
  }

  void shutdown(JNIEnv * env, void * client) {
    Lock lock(stateMutex);
    shutdown_impl(env, client);
    state = uninitialized;
  }

  bool isUninitializedState() const {

    return (state == uninitialized);
  }

  bool isInitializedState() const {

    return (state == initialized);
  }

  bool isActivatedState() const {

    return (state == activated);
  }

  bool isDeactivatedState() const {

    return (state == deactivateted);
  }

  /**
   * Returns true if the system-listener has encountered an exception in one of its worker
   * threads.
   * @return true if processException has been set.
   */
  bool hasProcessException() const {
    return static_cast<bool> (nativeProcessException);
  }


};



#endif	/* JACKSYSTEMLISTENER_HPP */

