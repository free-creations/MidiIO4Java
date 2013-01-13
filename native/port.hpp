/*
 * File:   port.hpp
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


#ifndef PORT_HPP
#define	PORT_HPP

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
#include <chrono>
#include <stdexcept>
#include <exception>
#include "messages.hpp"
#include "util.hpp"

/**
 * A value for the internalId that is used to mark ports a being dead 
 * (the remains of a move transaction).
 */
#define PortInvalidId  -100000L

using namespace std;

/**
 * An input-port is responsible to transport data from a native-thread to
 * the Java-callback-thread, an output port does it the other way round.
 * <p>
 * This is done by copying the data into a local storage in one thread and 
 * than providing this local storage to the other thread. A port deals
 * with at least three threads; the "native worker thread", the "java worker thread"
 * and the administrative thread (the thread that opens and closes a port).
 * The task of the port class is to synchronize these threads.
 * </p>
 * <p>
 * The base class "port" provides the infrastructure needed by the 
 * input and the output ports. In particular the mechanics to flip
 * between threads is implemented in this class, whereas functions
 * that need a JNIEnv pointer or that access the Native audio system
 * are factored out in virtual functions left to be implemented in subclasses.
 * </p>
 */
class Port {
private:
  /**
   * The waitLimit is the longest time we are going to wait for a thread
   * to become active. If this time limit is exceeded, we assume some
   * fundamental problem like a deadlock and we crash on
   * an exception to insure that the application does not freeze
   * without giving any indication about the nature of the problem.
   */
  const int waitLimitSeconds = 10;
  const chrono::seconds waitLimit = chrono::seconds(waitLimitSeconds);

  /**
   * A time that is for sure longer than a cycle.
   */
  const chrono::milliseconds maxWaitingTime = chrono::milliseconds(500);

  /**
   * A unique identifier.
   */
  long internalId;

  /**
   * "Lock" is a shorthand for "unique_lock<mutex>".
   */
  typedef unique_lock<timed_mutex> Lock;

  /** 
   * The "stateMutex" must be acquired before accessing the state of this port.
   */
  mutable timed_mutex stateMutex;


  /** The "onStateChanged" condition is signaled to awake waiting threads.*/
  mutable condition_variable_any onStateChanged;

  /** The "lastCycle" flag is set to true when we are about to perform the 
   * last cycle before shutting down. */
  bool lastCycle;

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
      case created: return;
      case initialized:
        uninitialize_impl(env);
        return;
      case registered:
        unregister_impl(client);
        uninitialize_impl(env);
        return;
      case running:
        stop_impl();
        unregister_impl(client);
        uninitialize_impl(env);
        return;
      case stoppedOnError:
        unregister_impl(client);
        uninitialize_impl(env);
        return;
      case stopped:
        unregister_impl(client);
        uninitialize_impl(env);
        return;
      case unregistered:
        uninitialize_impl(env);
        return;
      case deletable:
        return;
    }
  }

protected:

  /**
   * The time-code indicating when the current cycle started.
   */
  unsigned long timeCodeStart;

  /**
   * The length of one cycle.
   */
  unsigned long timeCodeDuration;

  enum State {
    created, ///< the port is created.
    initialized, ///< the port is embeded in the java environment.
    registered, ///< the port has registered with the native Midi System.
    running, ///< the port is processing callbacks from Java and from Native side.
    stoppedOnError, ///< the port has stopped to process callbacks because of an error.
    stopped, ///< the port has stopped to process callbacks.
    unregistered, ///< the port has unregistered from the native Midi System.
    deletable ///< the port has detached from the java environment and can safely be deleted.
  };

  /**
   * The sub-states for the running state.
   */
  enum RunningSubState {
    started, ///< the running state has just been entered,
    javaToExec, ///< the Java thread is requested to execute, the Native thread must wait.
    nativeToExec, ///< the Native thread is requested to execute, the Java thread must wait.
    cycleDone, ///< A complete cycle has been excuted.
    nativeToTerminate, ///< the Native thread should terminate the last cycle (only output ports).
    terminated, ///< the running state is terminated.
    none ///< subState is not applicable (main state is not "running").
  };

  /**
   * The current main state.
   */
  State state;

  /**
   * The sub-state when in running state.
   */
  RunningSubState substate;

  /**
   * Indicates whether this is an output port or an input port.
   * An output port shuffles data from the java side to the native side.
   * Therefore the java process comes first than comes the native process.
   * For an input port it is the other way round.
   */
  const bool output;


  /**
   * Stores a pointer to the exception object when problem occurs in the Java or 
   * the native thread.
   */
  exception_ptr processException;


  /**
   * This function shall register the
   * given Java-listener-object and than call the "onOpen" callback function of 
   * this object. The thread calling into this function shall not
   * be the Midi/Audio-thread because onOpen might execute some
   * lengthy initializations. The contract is that the Java callback function
   * (in execJavaProcess_impl) in is not invoked before the "onOpen" returns.
   * The implementation is deferred to a subclass.
   *
   * @param env pointer to the java environment.
   * @param name a unique name for this port.
   * @param listener a java listener that calls-back MidiInputPortListener or
   * MidiOutputPortListener
   */
  virtual void initialize_impl(JNIEnv * env, jstring name, jobject listener) = 0;

  /**
   * This function shall subscribe this port at the Midi server.
   * The implementation is deferred to a subclass.
   *
   * @param client client identity of this application.
   */
  virtual void register_impl(void * client) = 0;

  /**
   * This function can be used to customize the start function in a subclass.
   */
  virtual void start_impl() = 0;

  /**
   * This function can be used to customize the stop function in a subclass.
   */
  virtual void stop_impl() = 0;
  /**
   * This function shall call the "process" callback function of the associated
   * Java listener object. The contract is that the Java "process" callback function
   * is called only on ports in "running" state. The Java "process" callback will not be 
   * invoked before the "onOpen" returns and will not be invoked after
   * "onClose" has been invoked.
   * The implementation is deferred to a subclass.
   */
  virtual void execJavaProcess_impl(JNIEnv * env, unsigned long timeCodeStart, unsigned long timeCodeDuration, bool lastCycle) = 0;

  /**
   * This function shall access the native audio system. The contract is 
   * that the native audio system is called only on open ports. 
   * The implementation is deferred to a subclass.
   */
  virtual void execNativeProcess_impl(unsigned long timeCodeStart, unsigned long timeCodeDuration, void * client) = 0;

  /**
   * This function shall undo what initialize has done, it shall call
   * the "onClose" callback function of the associated
   * Java listener object. The contract is that the "onClose" callback 
   * is not invoked before the Java callback function (in execJavaProcess_impl) returns.
   * The implementation is deferred to a subclass.
   */
  virtual void uninitialize_impl(JNIEnv * env) = 0;

  /**
   * This function shall un-subscribe this port at the Midi server.
   * The implementation is deferred to a subclass.
   *
   * @param client client identity of this application.
   */
  virtual void unregister_impl(void * client) = 0;

  /**
   * Creates a port. 
   * @param isOutput is true on output ports (the java thread will be first to run),
   * is false on input ports (the native thread will be first to run).
   * @param _internalId a unique identifier for each port
   */
  Port(bool _isOutput, long _internalId) :
  output(_isOutput),
  internalId(_internalId),
  state(created),
  substate(none),
  lastCycle(false) {
  }

  /**
   * The move constructor, move the state from the other port into this.
   * @param other the port which shall be moved into this.
   */
  Port(Port && other) :
  internalId(other.internalId),
  processException(),
  onStateChanged(),
  output(other.output),
  state(created),
  stateMutex(),
  substate(none) {
    Lock lock(other.stateMutex); // we must wait until "other" is not busy.
    //take over the internal state of the other port
    processException = move(other.processException);
    state = move(other.state);
    substate = move(other.substate);

    // invalidate the remains of the other port
    other.internalId = PortInvalidId;
    other.state = deletable;
    other.substate = none;
  }


private:

  void throwCannot(const string& attemptedAction, int lineNumber, State state) {
    throwCannot(attemptedAction, lineNumber, state, none);
  }

  void throwCannot(const string& attemptedAction, int lineNumber, State state, RunningSubState substate) {

    string stateStr;
    switch (state) {
      case created: stateStr = " in created state.";
        break;
      case initialized:stateStr = " in initialized state.";
        break;
      case registered:stateStr = " in registered state.";
        break;
      case running:stateStr = " in running state.";
        break;
      case stoppedOnError:stateStr = " in stopped-on-error state.";
        break;
      case stopped:stateStr = " in stopped state.";
        break;
      case unregistered:stateStr = " in unregistered state.";
        break;
      case deletable:stateStr = " in deletable state.";
        break;
      default:stateStr = " in unknown state";
    }

    string subStateStr;

    switch (substate) {
      case started: subStateStr = " (sub-state: started)";
        break;
      case javaToExec: subStateStr = " (sub-state: javaToExec)";
        break;
      case nativeToExec: subStateStr = " (sub-state: nativeToExec)";
        break;
      case cycleDone: subStateStr = " (sub-state: cycleDone)";
        break;
      case nativeToTerminate: subStateStr = " (sub-state: nativeToTerminate)";
        break;
      case terminated: subStateStr = " (sub-state: terminated)";
        break;
      case none: subStateStr = "(sub-state: none)";
        break;
      default:subStateStr = " (sub-state: ???)";

    }

    ostringstream errorMessage;
    errorMessage << __FILE__ << "(" << lineNumber << "):Port(" << internalId
            << ") Cannot "
            << attemptedAction
            << stateStr
            << subStateStr;
    throw runtime_error(errorMessage.str());
  }

  /**
   * This procedure is called when a problem in the native or the java thread occurred.
   * @param cause an exception pointer
   */
  void emergencyStop(exception_ptr && cause) {
    stop_impl();
    state = stoppedOnError;
    substate = none;
    setProcessException(move(cause));
    onStateChanged.notify_all();
  }

  /**
   * sets the process exception. If there is already a process exception
   * the new exception object will be discarded, only the first exception will
   * be kept.
   * @param ex an exception pointer
   */
  void setProcessException(exception_ptr && ex) {
    if (!processException) {
      processException = move(ex);
    }
  }

public:



  /**
   * The copy constructor is inhibited, ports must be unique.
   */
  Port(const Port&) = delete;

  virtual ~Port() {
    if ((state != created) && (state != deletable)) {
      // it is not wise to throw an exception in the destructor.
      // At least we can leave a message.
      cerr << "### A Port is deleted in wrong state!" << endl;
    }
  }

  /**
   * Initializes this port for use. Once a port is initialized it is
   * capable to cooperate with the Java environment.
   * @param env pointer to the java thread that is about to create a port
   * @param name a unique name for this port
   * @param listener the java listener which will be called back 
   */
  void initialize(JNIEnv * env, jstring name, jobject listener) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in initialize.")
    }
    if (state != created) {
      throwCannot("initialize", __LINE__, state);
    }
    initialize_impl(env, name, listener);
    state = initialized;
    onStateChanged.notify_all();
  }

  /**
   * Subscribes this port at the Midi server.
   * The subscription may fail if the name given in initialize is not unique.
   *
   * @param client client-identity of this application.
   */
  void registerAtServer(void * client) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in registerAtServer.")
    }
    if (state != initialized) {
      throwCannot("register", __LINE__, state);
    }
    register_impl(client);
    state = registered;
    onStateChanged.notify_all();
  }

  /**
   * When this event is received, the port will participates in the next cycle.
   */
  void start() {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in start.")
    }
    if (state != registered) {
      throwCannot("start", __LINE__, state);
    }
    start_impl();
    state = running;
    substate = started;
    onStateChanged.notify_all();
  }

  /**
   * Calls the "process" callback function of the associated
   * Java listener object. 
   * @param env holds the java worker thread which shall perform the callback.
   * @param lastCycle indicates that this is the last cycle. False on normal
   * operation, true when this port is about to shutdown.
   */
  void execJavaProcess(JNIEnv * env, bool _lastCycle) {

    Lock lock(stateMutex, waitLimit);
    try {
      if (!lock.owns_lock()) {
        THROW("Timeout in execJavaProcess.")
      }
      if (state != running) {
        return;
      }
      if ((substate == started) || (substate == terminated) || (substate == nativeToTerminate)) {
        return;
      }


      // as long as we are in the "waitingForJava"- state, we will wait for the state to change.
      while ((substate != javaToExec) && (state == running)) {
        onStateChanged.wait(lock);
        // if the state has changes to something unexpected, we exit..
        if ((state != running) || (substate == terminated) || (substate == nativeToTerminate)) {
          return; //throwCannot("execJavaProcess", __LINE__, state, substate);
        }
      }


      // OK let's do the work.
      lastCycle = lastCycle || _lastCycle;
      execJavaProcess_impl(env, timeCodeStart, timeCodeDuration, lastCycle);

      // awake the native process
      if (lastCycle) {
        // last cycle -> terminate the session
        if (isOutput()) {
          // on output ports the native thread must do the last actions of the session.
          substate = nativeToTerminate;
        } else {
          // on an input port the session ends with the java process
          substate = terminated;
        }
      } else {
        if (isOutput()) {
          // on output ports the native thread must follow the java thread
          substate = nativeToExec;
        } else {
          // on an input port the cycle ends with the java process
          substate = cycleDone;
        }
      }
      onStateChanged.notify_all();

    } catch (...) {
      // something went wrong: capture the exception and stop the port
      emergencyStop(move(current_exception()));
    }
  }

  /**
   * The native thread initiates with this event a new cycle.
   * @param timeCodeStart the time code value to be used for the java and the native processes
   * @param timeCodeDuration the duration to be used for the java and the native processes
   */
  void execNativeCycleInit(unsigned long _timeCodeStart, unsigned long _timeCodeDuration) {
    Lock lock(stateMutex);
    try {
      if (state != running) {
        return;
      }
      if (substate == terminated) {
        return;
      }

      if ((substate != cycleDone) && (substate != started)) {
        throwCannot("execNativeCycleInit", __LINE__, state, substate);
      }

      // 1) store the time-code values for latter use (by java and native thread)
      timeCodeStart = _timeCodeStart;
      timeCodeDuration = _timeCodeDuration;

      // 2) determine whether native or java has to execute next.
      if (isInput()) {
        substate = nativeToExec;
      } else {
        substate = javaToExec;
      }
      onStateChanged.notify_all();

    } catch (...) {
      // something went wrong: capture the exception and stop the port
      emergencyStop(move(current_exception()));
    }
  }

  /**
   * The calling thread will be blocked in "running" state until the "cycleDone" sub-state is reached.
   */
  void waitForCycleDone() {
    Lock lock(stateMutex);
    // as long as we are not in the "cycleDone"- state, we will wait for the state to change.
    while ((state == running) && (substate != cycleDone) && (substate != terminated)) {
      onStateChanged.wait(lock);
    }
  }

  /**
   * Access the native audio system. This procedure shall be run in the 
   * "native worker thread" of the audio system callback.
   */
  void execNativeProcess(void * client) {
    Lock lock(stateMutex, waitLimit);
    try {
      if (!lock.owns_lock()) {
        THROW("Timeout in execNativeProcess.")
      }
      if (state != running) {
        return;
      }
      if ((substate == started) || (substate == terminated)) {
        return;
      }
      while ((state == running) && (substate != nativeToExec) && (substate != nativeToTerminate)) {
        onStateChanged.wait(lock);
        // if the state has changes to something unexpected, we throw an error.
        if ((substate == terminated) || (substate == cycleDone) || (substate == started)) {
          throwCannot("execNativeProcess", __LINE__, state, substate);
        }
      }

      // was our port closed while waiting?
      if (state != running) {
        return;
      }

      if (isInput()) {
        if ((substate == nativeToTerminate)) {
          // an input port never processes the nativeToTerminate state
          throwCannot("execNativeProcess on Input", __LINE__, state, substate);
        }
      }


      // OK let's do the work.
      execNativeProcess_impl(timeCodeStart, timeCodeDuration, client);

      if (isInput()) {
        // on input port: awake the java process
        substate = javaToExec;
      } else {
        // on output port:  terminate the session or terminate the cycle
        if (substate == nativeToTerminate) {
          substate = terminated;
        } else {
          substate = cycleDone;
        }
      }
      onStateChanged.notify_all();

    } catch (...) {
      // something wrong: capture the exception and stop the port
      emergencyStop(move(current_exception()));
    }
  }

  /**
   * Stop the worker processes. Once the "stop" procedure has executed 
   * the execNativeProcess, execNativeCycleInit and execJavaProcess will
   * will become empty operations.
   * @param force forces stopping without waiting for the port to terminate.
   */
  void stop(bool force) {
    Lock lock(stateMutex);

    if (state == stoppedOnError) {
      state = stopped;
      onStateChanged.notify_all();
      return;
    }

    if (((state != running)) && (state != registered)) {
      throwCannot("stop", __LINE__, state, substate);
    }

    lastCycle = true;

    // unless "force" is set, we'll wait for max. 500 milliseconds to get the port terminated.
    while ((!force) && (state == running) && (substate != terminated) && (substate != none)) {
      auto result = onStateChanged.wait_for(lock, maxWaitingTime);
      if (result == std::cv_status::timeout) {
        force = true;
      }
    }

    if ((substate != terminated) && (substate != none)) {
      emergencyStop(make_exception_ptr(runtime_error(AT "Port did not terminate.")));
    } else {
      stop_impl();
    }
    state = stopped;
    substate = none;
    onStateChanged.notify_all();
  }

  /**
   * Un-subscribes this port at the Midi server.
   *
   * @param client client-identity of this application.
   */
  void unregisterAtServer(void * client) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in unregisterAtServer.")
    }
    if ((state != stopped) && (state != registered)) {
      throwCannot("unregister", __LINE__, state);
    }
    unregister_impl(client);
    state = unregistered;
    onStateChanged.notify_all();
  }

  /**
   * This function shall undo what initialize has done, it shall call
   * the "onClose" callback function of the associated
   * Java listener object. The contract is that the "onClose" callback 
   * is not invoked before the Java callback function (in execJavaProcess_impl) returns.
   * The implementation is deferred to a subclass.
   */
  void uninitialize(JNIEnv * env) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in uninitialize.")
    }
    if ((state != unregistered) && (state != initialized)) {
      throwCannot("un-initialize", __LINE__, state);
    }
    uninitialize_impl(env);
    state = deletable;
    onStateChanged.notify_all();
  }

  void shutdown(JNIEnv * env, void * client, bool force) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in shutdown.")
    }
    lastCycle = true;

    try {
      // unless "force" is set, we'll wait for max. 500 milliseconds to get the port terminated.
      while ((!force) && (state == running) && (substate != terminated) && (substate != none)) {
        auto result = onStateChanged.wait_for(lock, maxWaitingTime);
        if (result == std::cv_status::timeout) {
          force = true;
        }
      }


      shutdown_impl(env, client);

    } catch (...) {
      setProcessException(current_exception());
    }

    state = deletable;
    substate = none;
    onStateChanged.notify_all();

  }

  bool isCreatedState() const {
    Lock lock(stateMutex);
    return (state == created);
  }

  bool isInitializedState() const {
    Lock lock(stateMutex);
    return (state == initialized);
  }

  bool isRegisteredState() const {
    Lock lock(stateMutex);
    return (state == registered);
  }

  bool isRunningState() const {
    Lock lock(stateMutex);
    return (state == running);
  }

  bool isStoppedState() const {
    Lock lock(stateMutex);
    return (state == stopped);
  }

  bool isStoppedOnErrorState() const {
    Lock lock(stateMutex);
    return (state == stoppedOnError);
  }

  bool isUnregisteredState() const {
    Lock lock(stateMutex);
    return (state == unregistered);
  }

  /**
   * Indicates that the port has detached from the java environment and 
   * can be deleted.
   * Note: albeit one can be sure that no other thread is
   * currently using this object when the function returns true, the deleting 
   * tread must nevertheless be sure that no other thread can re-aquire the port
   * while the port is being deleted.
   * @return true if the port can be deleted.
   */
  bool isDeletableState() const {
    Lock lock(stateMutex);

    return (state == deletable);
  }

  /**
   * Indicates that the port is running and  in the "started" sub-state.
   * (this function locks the state-mutex, not be used inside a function that
   * has already acquired the state-mutex.
   * @return true if the port is in the "started" sub-state.
   */
  bool isStartedSubstate() const {
    Lock lock(stateMutex);
    return (substate == started);
  }

  /**
   * Indicates that the port is running and  in the "JavaToExec" sub-state.
   * (this function locks the state-mutex, not be used inside a function that
   * has already acquired the state-mutex.
   * @return true if the port is in the "javaToExec" sub-state.
   */
  bool isJavaToExecSubstate() const {
    Lock lock(stateMutex);
    return (substate == javaToExec);
  }

  /**
   * Indicates that the port is running and  in the "CycleDone" sub-state.
   * (this function locks the state-mutex, not be used inside a function that
   * has already acquired the state-mutex.
   * @return true if the port is in the "cycleDone" sub-state.
   */
  bool isCycleDoneSubstate() const {
    Lock lock(stateMutex);
    return (substate == cycleDone);
  }

  /**
   * Not synchronized; only for test. 
   * @return true if the port is in the "nativeToExec" sub-state.
   */
  bool isNativeToExecSubstate() const {
    Lock lock(stateMutex);
    return (substate == nativeToExec);
  }

  /**
   * @return true if the port is in the "nativeToTerminate" sub-state.
   */
  bool isNativeToTerminateSubstate() const {
    Lock lock(stateMutex);
    return (substate == nativeToTerminate);
  }

  /**
   * @return true if the port is in the "terminated" sub-state.
   */
  bool isTerminatedSubstate() const {
    Lock lock(stateMutex);
    return (substate == terminated);
  }

  /**
   * Blocks the calling thread until the port is in terminated sub-state.
   * @throws TimeoutException if the waiting time exceeds a predefined limit.
   */
  void waitForTerminatedSubstate() const {
    Lock lock(stateMutex);
    if (state != running) {
      return;
    }
    while (substate != terminated) {
      auto result = onStateChanged.wait_for(lock, maxWaitingTime);
      if (result == std::cv_status::timeout) {
        THROW_TIMEOUT("Timeout in waitForTerminatedSubstate().")
      }
      if (state != running) {
        return; // state has become "stopped" while waiting
      }
    }
  }

  /**
   * Blocks the calling thread until the port is in cycleDone sub-state.
   * @Todo merge this with waitForCycleDone()
   * @throws TimeoutException if the waiting time exceeds a predefined limit.
   */
  void waitForCycleDone2() const {
    Lock lock(stateMutex);
    if (state > running) {
      return;
    }
    while (substate != cycleDone) {
      auto result = onStateChanged.wait_for(lock, maxWaitingTime);
      if (result == std::cv_status::timeout) {
        THROW_TIMEOUT("Timeout in waitForCycleDone2().")
      }
      if (state > running) {
        return; // state has become "stopped" while waiting
      }
    }
  }

  /**
   * @return true if the port is in the "none" sub-state.
   */
  bool isNoneSubstate() const {
    Lock lock(stateMutex);
    return (substate == none);
  }

  bool isOutput() const {
    return output;
  }

  bool isInput() const {
    return !output;
  }

  /**
   * Returns true if the port has encountered an exception in one of its worker
   * threads.
   * @return true if processException has been set.
   */
  bool hasProcessException() const {
    return static_cast<bool> (processException);
  }

  /**
   * Permits to access the process exception.
   * @return a reference to the process exception pointer.
   */
  exception_ptr& getProcessException() {
    return processException;
  }

  long getId() const {
    return internalId;
  }

};



#endif	/* PORT_HPP */

