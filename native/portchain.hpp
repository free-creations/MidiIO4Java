/*
 * File:   portchain.hpp
 *
 * Created on September 1, 2012, 2:42 PM
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


#ifndef PORTCHAIN_HPP
#define	PORTCHAIN_HPP

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
#include <atomic>
#include <memory>
#include <exception>

#include "port.hpp"
#include "util.hpp"
#include "messages.hpp"
#include "ptrEnvelope.hpp"

#define MAX_PORTS 512 // The maximum number of ports, a PortChain can manage.


using namespace std;

class PortChain {
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
   * A re-definition of std::unique_lock<mutex> for better readability.
   */
  typedef unique_lock<timed_mutex> Lock;

  /** 
   * The "stateMutex" must be acquired before changing the state of this object
   * or adding or removing ports.
   */
  mutable timed_mutex stateMutex;

  mutable timed_mutex javaMutex;

  /**
   * indicates that the next cycle will be the last one of this session.
   */
  bool lastCycle;



  /** 
   * The "onStateChanged" condition is signaled to awake waiting threads.
   * @see runJavaLoop()
   */
  condition_variable_any onStateChanged;

  /**
   * Transfers the given port into a state which is compatible with the
   * current state of the port-chain.
   * @param newPort a pointer to a port (assumed in the initialized state)
   * @param client the client pointer of the audio system.
   * @return the given port transfered into the the same state as the port-chain.
   */
  void registerAndStart(unique_ptr<Port>& newPort, void * client) const {
    if (isRegisteredState()) {
      newPort->registerAtServer(client);
    }
    if (isRunningState()) {
      newPort->registerAtServer(client);
      newPort->start();
    }

  }

  /**
   * Output ports are inserted at the end of the array.
   * Holes within the range of output ports shall be reused.
   * @return the index of a suitable slot.
   */
  int findSlotForOutputPort() {
    const int last = MAX_PORTS - 2; // MAX_PORTS - 1 is reserved
    if (portList[last].makeAccessor().isEmpty()) {
      return last;
    }
    //find an empty slot that is followed by  an output port.
    for (int i = last - 1; i >= 0; i--) {
      auto candidate = portList[i].makeAccessor();
      if (candidate.isEmpty()) {
        auto successor = portList[i + 1].makeAccessor();
        if (successor.hasItem()) {
          if (successor.get()->isOutput()) {
            return i;
          }
        }
      }
    }
    THROW("Cannot findSlotForOutputPort.");
  }

  /**
   * Output ports are inserted at the front of the array.
   * Holes within the range of input ports shall be reused.
   * @return the index of a suitable slot.
   */
  int findSlotForInputPort() {
    const int first = 1; // 0 is reserved
    if (portList[first].makeAccessor().isEmpty()) {
      return first;
    }
    //find an empty slot that is directly preceeded by an input port.
    for (int i = 1; i < MAX_PORTS; i++) {
      auto candidate = portList[i].makeAccessor();
      if (candidate.isEmpty()) {
        auto predecessor = portList[i - 1].makeAccessor();
        if (predecessor.hasItem()) {
          if (predecessor.get()->isInput()) {
            return i;
          }
        }
      }
    }
    THROW("Cannot findSlotForInputPort.");
  }

  /**
   * Find the index in portList of a port identified by its internal Id.
   * @param internalId the internal identifier to search for.
   * @return the index into the array or -1 if no suitable entry could be found.
   */
  int findSlotOfPort(long internalId) {
    for (int i = 0; i < MAX_PORTS; i++) {
      auto candidate = portList[i].makeAccessor();
      if (candidate.hasItem()) {
        if (candidate.get()->getId() == internalId) {
          // found!
          return i;
        }
      }
    }
    // not found!
    return -1;
  }

  /**
   * This function puts the current thread to sleep as long as the
   * portList is empty. As soon as a port gets
   * added to the port list the waiting thread is released and executes one
   * Java cycle. This function will also make sure
   * that the thread does not start before the state has passed the running-state
   * and immediately returns as soon as the state has reached the stopped-state.
   */
  void waitAndExecJavaCycle(JNIEnv * env) {
    {
      Lock lock(stateMutex, waitLimit);
      if (!lock.owns_lock()) {
        THROW("Timeout in waitForPorts.")
      }

      // as long as we are not started, we sleep (waiting for the state to become "running")
      while ((state == registered) || (state == initialized) || (state == created)) {
        onStateChanged.wait(lock);
      }
    }
    bool wait = true;
    while ((state == running) && (wait)) {

      auto accessor = portList[0].makeAccessor();
      if (!accessor.hasItem()) {
        THROW("No Start-Control port in port-chain.")
      }
      // if the first port (the start-control port) has done nativeCycleInit we can start the java thread
      if (!accessor.get()->isJavaToExecSubstate()) {
        this_thread::sleep_for(std::chrono::milliseconds(1));
        wait = true;
      } else {
        wait = false;
      }
    }

  }

  /**
   * This procedure implements the functionality of the "shutdown"
   * public function, but does not set any lock nor does it manage
   * the state variable, so this function can be called from within 
   * threads that have already locked the stateMutex.
   * @param env the java environment pointer for the current Java thread
   * @param client the client id of this application
   */
  void shutdown_impl(JNIEnv * env, void * client) {

    switch (state) {
      case created:
        uninitialize_impl(env); // there might be initialized ports.
        state = deletable;
        return;
      case initialized:
        uninitialize_impl(env);
        return;
      case registered:
        unregisterAtServer_impl(client);
        uninitialize_impl(env);
        return;
      case running:
        stop_impl();
        unregisterAtServer_impl(client);
        uninitialize_impl(env);
        return;
      case stopped:
        unregisterAtServer_impl(client);
        uninitialize_impl(env);
        return;
      case unregistered:
        uninitialize_impl(env);
        return;
      case deletable:
        return;
    }
  }

  /**
   * This procedure implements the functionality of the "registerAtServer"
   * public function, but does not set any lock , so this function can be called from within 
   * threads that have already locked the stateMutex.
   * @param client the client id of this application
   */
  void registerAtServer_impl(void * client) {
    if (state != initialized) {
      THROW("Cannot registerAtServer in wrong state.")
    }
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        accessor.get()->registerAtServer(client);
      }
    }
    state = registered;
  }

  /**
   * This procedure implements the functionality of the "registerAtServer"
   * public function, but does not set any lock , so this function can be called from within 
   * threads that have already locked the stateMutex.
   * @param env
   * @param startControl the start-control-port to be inserted at the very beginning of the chain
   * @param endControl the end-control-port to be inserted at the very end of the chain
   */
  void initialize_impl(JNIEnv * env, jobject jSystemListener, unique_ptr<Port> && startControl, unique_ptr<Port> && endControl) {
    if (state != created) {
      THROW("Cannot initialize in wrong state.")
    }
    if (!startControl->isInput()) {
      THROW("Start control must be an input port.")
    }
    if (!startControl->isCreatedState()) {
      THROW("Start control must be in created state.")
    }
    if (!endControl->isOutput()) {
      THROW("Start control port must be an output port.")
    }
    if (!endControl->isCreatedState()) {
      THROW("End control must be in created state.")
    }
    startControl->initialize(env, nullptr, jSystemListener);
    endControl->initialize(env, nullptr, jSystemListener);

    addPort_impl(move(startControl), 0, nullptr);
    addPort_impl(move(endControl), MAX_PORTS - 1, nullptr);

    state = initialized;
  }

  /**
   * This procedure implements the functionality of the "start"
   * public function, but does not set any lock, so this function can be called from within 
   * threads that have already locked the stateMutex.
   */
  void start_impl() {
    if (state != registered) {
      THROW("Cannot start in wrong state.")
    }
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        accessor.get()->start();
      }
    }
    state = running;
  }

  /**
   * This procedure implements the functionality of the "stop"
   * public function, but does not set any lock, so this function can be called from within 
   * threads that have already locked the stateMutex.
   */
  void stop_impl() {
    if (state != running) {
      THROW("Cannot stop in wrong state.")
    }
    // last orders please
    lastCycle = true; // if java and native  worker threads work correctly, all ports should now transit into the terminated state 
    bool forcedStop = true; // hopefully we'll set this to false in the following
    {
      // let's wait until the last port has terminated
      auto accessor = portList[MAX_PORTS - 1].makeAccessor();
      if (accessor.isEmpty()) {
        THROW("No End-Control port in port-chain.")
      }

      try {
        accessor.get()->waitForTerminatedSubstate();
        forcedStop = false; // OK, we don't need to force the ports to stop
      } catch (TimeoutException& ex) {
        forcedStop = true; // Oops, we will have to force the ports to stop
      }
    } // Accessor is freed

    // Stop all ports.
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        accessor.get()->stop(forcedStop);
      }
    }
    state = stopped;
    //wait for the java thread to end.
    Lock lock(javaMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in stop_impl.")
    }
  }

  /**
   * This procedure implements the functionality of the "unregisterAtServer"
   * public function, but does not set any lock, so this function can be called from within 
   * threads that have already locked the stateMutex.
   * @param client the client id of this application
   */
  void unregisterAtServer_impl(void * client) {
    if ((state != stopped) && (state != registered)) {
      THROW("Cannot unregisterAtServer in wrong state.")
    }
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        accessor.get()->unregisterAtServer(client);
      }
    }
    state = unregistered;
  }

  /**
   * This procedure implements the functionality of the "uninitialize"
   * public function, but does not set any lock, so this function can be called from within 
   * threads that have already locked the stateMutex.
   * @param env the java environment pointer for the current Java thread
   */
  void uninitialize_impl(JNIEnv * env) {
    if ((state != unregistered) && (state != initialized) && (state != created)) {
      THROW("Cannot un-initialize in wrong state.")
    }
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        accessor.get()->uninitialize(env);
      }
    }
    state = deletable;
  }

protected:
  /**
   * List of all ports currently in use by this portChain.
   * The contract is, that at any moment every entry in this array
   * is either a null pointer or a pointer to a port that is in a state compatible
   * with the current state of the portChain.
   * Input ports are inserted from the beginning of the array.
   * Output ports are inserted to the end of the array.
   * This way input-ports are always processed before output-ports.
   * Removed ports are replaced by a null pointer. The variable
   * "portCount" is not synchronized with the
   * java worker-thread nor with the native worker-thread. As a
   * consequence these threads might see (for one round) sightly inaccurate values.
   */
  PtrEnvelope portList[MAX_PORTS];

  atomic<int> portCount;

  enum State {
    created, ///< the portchain is created.
    initialized, ///< the portchain is embeded into the java enviroment (java -call-backs have been installed)
    registered, ///< all ports are in the registered state.
    running, ///< all ports are in the running state.
    stopped, ///< all ports have stopped to process callbacks.
    unregistered, ///< all ports have unregistered from the native Midi System.
    deletable ///< the portchain has been removed from the java enviroment (java-call-backs have been freed), all ports are in the deletable state.
  };
  State state;

  PortChain() :
  state(created),
  portCount(0),
  lastCycle(false) {

  }

  /**
   * Searches an empty slot in the portList for the given port.
   * The rule is that input ports are inserted at the front of 
   * the list and output ports are inserted at the end of the
   * list. The first slot and the last slot are reserved for
   * the control ports.
   * @param newPort the given port for which a slot is searched.
   * @return an index into the portList array.
   * @throws an error if the portList is full.
   */
  int findSlotForNewPort(const unique_ptr<Port>& newPort) {
    if (newPort->isOutput()) {
      return findSlotForOutputPort();
    } else {
      return findSlotForInputPort();
    }
  }

public:

  /**
   * Embeds the port-chain into the java environment. Especially this procedure
   * installs a number of call-backs into the java environment.
   * @param env the java thread calling this procedure.
   * @param jSystemListener pointer to the Java SystemListener object.
   * @param startControl a pseudo-port that performs the onCycleStart call-back
   * (must be an input-port in state "created").
   * @param endControl a pseudo-port that performs the onCycleEnd call-back
   * (must be an output-port in state "created").
   */
  void initialize(JNIEnv * env, jobject jSystemListener, unique_ptr<Port> && startControl, unique_ptr<Port> && endControl) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in initialize.")
    }
    initialize_impl(env, jSystemListener, move(startControl), move(endControl));
    onStateChanged.notify_all();
  }

  /**
   * Subscribes all ports at the Midi server.
   *
   * @param client client-identity of this application.
   */
  void registerAtServer(void * client) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in registerAtServer.")
    }
    registerAtServer_impl(client);
    onStateChanged.notify_all();
  }

  /**
   * Transit all ports into the started state.
   */
  void start() {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in start.")
    }
    start_impl();
    onStateChanged.notify_all();
  }

  /**
   * Calls the "execJavaProcess"  function of all ports.
   * This function will block  on the first port that is waiting for the native thread.
   * @param env holds the java worker thread.
   * @param  lastCycle indicates that this is the last cycle. False on normal
   * operation, true when this port is about to shutdown.
   */
  void execJavaCycle(JNIEnv * env, bool lastCycle) {
    // no lock! We rely upon the ports to manage their life cycle.
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        accessor.get()->execJavaProcess(env, lastCycle);
      }
    }
  }

  /**
   * Calls the "execNativeCycleInit()" and execNativeProcess()"  functions on all ports.
   * This function will block  on the first port that is waiting for the java thread.
   * @param env holds the java worker thread.
   */
  void execNativeCycle(unsigned long timeCodeStart, unsigned long timeCodeDuration, void * client) {
    // No lock! We rely upon the ports to manage their life cycle.

    {
      // first, let's verify that the last port (the end-control port) has finished the previous cycle
      // and if it is terminated we just return.
      auto accessor = portList[MAX_PORTS - 1].makeAccessor();
      if (accessor.hasItem()) {
        if (!accessor.get()->isRunningState()) {
          return;
        }
        if (accessor.get()->isTerminatedSubstate()) {
          return;
        }

        if ((!accessor.get()->isCycleDoneSubstate()) && (!accessor.get()->isStartedSubstate())) {
          if (accessor.get()->isRunningState()) {
            THROW("XRUN.")
          } else {
            return; //oops has been stopped meanwhile
          }
        }
      } else {
        THROW("No End-Control port in port-chain.")
      }
    }

    // initialize the new Cycle on all ports.
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        accessor.get()->execNativeCycleInit(timeCodeStart, timeCodeDuration);
      }
    }
    // perform the native work on all ports
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        // note: output ports will wait for "execJavaCycle" before
        // executing the following statement, consequently there
        // is a danger of dead-lock here.
        accessor.get()->execNativeProcess(client);
      }
    }
  }

  /**
   * Transit all ports into the stopped state. The calling thread will be blocked 
   * until the function runJava() has retured.
   */
  void stop() {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in stop.")
    }
    stop_impl();
    onStateChanged.notify_all();
    lock.unlock();

  }

  /**
   * Un-subscribes all ports at the Midi server and transit them into the 
   * "unregistered" state.
   *
   * @param client client-identity of this application.
   */
  void unregisterAtServer(void * client) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in unregisterAtServer.")
    }
    unregisterAtServer_impl(client);
    onStateChanged.notify_all();
  }

  /**
   * Remove all ports from the java environment and transit them into the 
   * "deletable" state.
   */
  void uninitialize(JNIEnv * env) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in un-initialize.")
    }
    uninitialize_impl(env);
    onStateChanged.notify_all();
  }

  bool isCreatedState() const {
    return (state == created);
  }

  bool isInitializedState() const {
    return (state == initialized);
  }

  bool isRegisteredState() const {
    return (state == registered);
  }

  bool isRunningState() const {
    return (state == running);
  }

  bool isStoppedState() const {
    return (state == stopped);
  }

  bool isUnregisteredState() const {
    return (state == unregistered);
  }

  bool isDeletableState() const {
    return (state == deletable);
  }

  /**
   * Adds a port to the chain.
   * @param newPort the port to be added.
   * @param client the client pointer (can be null as
   * long as the chain has not registered with the audio system).
   */
  void addPort(unique_ptr<Port> && newPort, void * client) {

    Lock lock(stateMutex);

    if (!static_cast<bool> (newPort)) {
      THROW("Cannot add Port from empty pointer.")
    }
    if (!newPort->isInitializedState()) {
      THROW("Attempt to add an uninitialized Port.")
    }
    if (newPort->getId() < -2) {
      THROW("Cannot add Port with invalid Id.")
    }

    if ((state != created) && (state != initialized) && (state != registered) && (state != running)) {
      THROW("Cannot add a new port when the port-chain is about to shutdown.")
    }
    if ((state != initialized) && (state != created) && (client == nullptr)) {
      THROW("Need client pointer to register new port.")
    }

    int newIdx = findSlotForNewPort(newPort);
    addPort_impl(move(newPort), newIdx, client);

    onStateChanged.notify_all();
  }
private:

  /**
   * This procedure implements the functionality of the "addPort"
   * public function, but does not set any lock , so this function can be called from within 
   * threads that have already locked the stateMutex.
   * @param newPort the port to be added.
   * @param newIdx the slot where the port shall be inserted.
   * @param client the client pointer (can be null as
   * long as the chain has not registered with the audio system).
   */
  void addPort_impl(unique_ptr<Port> && newPort, int newIdx, void * client) {

    registerAndStart(newPort, client);

    // try to insert the new port into the given slot, if the slot is for too long an exception is thrown.

    portList[newIdx].setItemWait(move(newPort));

  }

public:

  unique_ptr<Port> removePort(JNIEnv * env, void * client, long internalId) {
    Lock lock(stateMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in removePort.")
    }
    int removeIdx = findSlotOfPort(internalId);
    //throw a runtime exception if not found
    if (removeIdx < 0) {
      THROW("Cannot removePort, port not found.")
    }

    if (portList[removeIdx].isEmpty()) {
      THROW("Programming error: port has no item.")
    }

    {
      // shutdown the port and free the Accessor afterwards
      auto accessor = portList[removeIdx].makeAccessor();
      accessor.get()->shutdown(env, client, false);
    }
    // try to remove the given port. If the port is locked for too long an exception is thrown.
    auto portToRemove = portList[removeIdx].removeItemWait();

    if (!static_cast<bool> (portToRemove)) {
      THROW("Programming error: portToRemove is null.")
    }

    portCount--;
    onStateChanged.notify_all();

    return portToRemove;
  }

  /**
   * runs the java thread.
   * The calling thread will not return as long as the port-chain is in running state.
   * @param env holds the java worker thread.
   */
  void runJava(JNIEnv * env) {
    Lock lock(javaMutex, waitLimit);
    if (!lock.owns_lock()) {
      THROW("Timeout in runJava.")
    }

    // for the first cycle, we avoid spinning in an empty loop (and waisting recources) by waiting.
    waitAndExecJavaCycle(env);

    bool more = true;

    while ((state == running) && (more)) {

      auto accessor = portList[0].makeAccessor();
      if (!accessor.hasItem()) {
        THROW("No Start-Control port in port-chain.")
      }
      // if the first port (the start-control port) has terminated we'll end the java thread
      if (accessor.get()->isTerminatedSubstate()) {
        more = false;
      } else {
        execJavaCycle(env, lastCycle);
      }
    }
  }

  /**
   * Puts the portchain and all its ports into the deletable state.
   * @param env
   * @param client
   */
  void shutdown(JNIEnv * env, void * client) {
    Lock lock(stateMutex, waitLimit);
    onStateChanged.notify_all(); // make sure the java thread gets released whatever happens
    if (!lock.owns_lock()) {
      //force shutdown
      shutdown_impl(env, client);
      THROW("Timeout in shutDown.")
    }
    shutdown_impl(env, client);
    lock.unlock();

  }

  /**
   * Indicates whether a port with the given identity is is currently hooked into the portchain.
   * @param internalId the identifier to search for
   * @return true is the searched port is currently part of the portchain.
   */
  bool portExists(long internalId) {
    Lock lock(stateMutex);
    return findSlotOfPort(internalId) != -1;

  }

  void waitForCycleDone() {

    auto accessor = portList[MAX_PORTS - 1].makeAccessor();
    if (accessor.isEmpty()) {
      THROW("No End-Control port in port-chain.")
    }
    accessor.get()->waitForCycleDone2();

  }

  /**
   * Searches the ports for process exceptions. Returns the first exception.
   * @return an exception pointer pointing. If no exception was found
   * the pointer will be empty.
   */
  exception_ptr retrieveProcessException() {
    for (auto &entry : portList) {
      auto accessor = entry.makeAccessor();
      if (accessor.hasItem()) {
        if (accessor.get()->hasProcessException()) {
          return move(accessor.get()->getProcessException());
        }
      }
    }
    return nullptr;

  }

};



#endif	/* PORTCHAIN_HPP */

