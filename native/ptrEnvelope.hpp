/*
 * File:   ptrEnvelope.hpp
 *
 * Created on October 18, 2012, 12:31 PM
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


#ifndef PTRENVELOPE_HPP
#define	PTRENVELOPE_HPP

#include <memory>
#include <mutex>
#include <chrono>
#include "port.hpp"
#include "messages.hpp"

/**
 * The class PtrEnvelope permits to use a "unique_ptr" in a thread save way.
 */
class PtrEnvelope {
  typedef unique_lock<mutex> Lock;
public:

  /**
   * The accessor is the only way to get access to the pointer envelopped
   * by the PtrEnvelope class. It automatically maintains a use count and
   * supports a RAII style of programming.
   */
  class Accessor {
  private:
    PtrEnvelope & owner;
  protected:
    friend class PtrEnvelope;

    /**
     * The protected constructor can only be called by the surrounding class.
     * When a new Accessor is constructed the use count of its owner
     * is automatically updated.
     * @param _owner
     */
    Accessor(PtrEnvelope& _owner) :
    owner(_owner) {
    }

  public:

    /**
     * Indicates whether the pointer holds an existing item.
     * @return false if the pointer points to nothing.
     */
    bool hasItem() const {
      return owner.hasItem();
    }

    /**
     * Indicates whether the pointer holds a null pointer
     * (the result is the inverse of "hasItem").
     * @return false if the pointer points to nothing.
     */
    bool isEmpty() const {
      return owner.isEmpty();
    }

    /**
     * Accesses the enveloped pointer.
     * @return a reference to the pointer.
     */
    const unique_ptr<Port>& get() const {
      Lock lock(owner.stateMutex);
      return owner.item;
    }

    /**
     * Destroys the Accessor and decrements the use count.
     */
    ~Accessor() {
      owner.decrementUseCount();
    }

  }; //end Accessor ------

  friend class Accessor;

public:

  /**
   * Constructs a new Pointer envelope holding an empty pointer.
   */
  PtrEnvelope() :
  maxWaitingTime(500),
  useCount(0) {
  }

  /**
   * Constructs a new Pointer envelope holding an empty pointer.
   * @param _maxWaitingTime permits to set a shorter waiting time for testing
   * purposes.
   */
  PtrEnvelope(int _maxWaitingTime) :
  maxWaitingTime(_maxWaitingTime),
  useCount(0) {
  }

  virtual ~PtrEnvelope() {
    Lock lock(stateMutex);
    if (useCount != 0) {
      // it is not wise to throw an exception in the destructor.
      // At least we can leave a message.
      cerr << "### A PtrEnvelope is deleted in wrong state!" << endl;
    }
  }
  /**
   * The copy constructor is inhibited, PtrEnvelope must be unique.
   */
  PtrEnvelope(const PtrEnvelope&) = delete;

  /**
   * The move constructor is inhibited, because we don't need it.
   */
  PtrEnvelope(PtrEnvelope &&) = delete;

private:
  const chrono::milliseconds maxWaitingTime;
  int useCount;
  mutable mutex stateMutex;
  unique_ptr<Port> item;
  /** The "onUseCountChanged" condition is signaled when the use-count changes.*/
  condition_variable_any onUseCountChanged;

  void decrementUseCount() {
    Lock lock(stateMutex);
    useCount--;
    onUseCountChanged.notify_all();
  }

public:

  /**
   * Constructs an Accessor that permits to access the pointer
   * hidden within the envelope.
   * @return a new Accessor.
   */
  Accessor makeAccessor() {
    Lock lock(stateMutex);
    useCount++;
    onUseCountChanged.notify_all();
    return Accessor(*this);
  }


  /**
   * Moves a new item into the pointer. The calling thread is blocked until
   * it has exclusive access (use count is Zero).
   * @param newItem the new item to be moved into the pointer
   * @return true if the operation succeeded false if the operation failed.
   */
  void setItemWait(unique_ptr<Port> && newItem) {
    Lock lock(stateMutex);
    if (!static_cast<bool> (newItem)) {
      THROW("Programming error: newItem is null (nothing to add).")
    }
    if (static_cast<bool> (item)) {
      THROW("Programming error: envelope not empty, cannot add a new item.")
    }
    // wait for the use-count to become zero.
    while (useCount != 0) {
      auto result = onUseCountChanged.wait_for(lock, maxWaitingTime);
      if (result == std::cv_status::timeout) {
        THROW_TIMEOUT("Timeout in setItemWait().")
      }
    }
    item = move(newItem);
  }

  /**
   * Indicates how many users currently are reading the pointer.
   * @return the number of Accessors currently existing.
   */
  int getUseCount() const {
    Lock lock(stateMutex);
    return useCount;
  }


  /**
   * Removes the item from the pointer. The calling thread is blocked until
   * it has exclusive access (use count is Zero).
   * @return the removed item 
   */
  unique_ptr<Port> removeItemWait() {
    Lock lock(stateMutex);
    if (!static_cast<bool> (item)) {
      THROW("Programming error: envelope is empty, cannot remove the item.")
    }
    // wait for the use-count to become zero.
    while (useCount != 0) {
      auto result = onUseCountChanged.wait_for(lock, maxWaitingTime);
      if (result == std::cv_status::timeout) {
        THROW_TIMEOUT("Timeout in removeItemWait().")
      }
    }
    auto result = move(item);
    return move(result);
  }

  /**
   * Indicates whether the pointer holds an existing item.
   * @return false if the pointer points to nothing.
   */
  bool hasItem() const {
    Lock lock(stateMutex);
    return static_cast<bool> (item);
  }

  /**
   * Indicates whether the envelope holds a null pointer
   * (the result is the inverse of "hasItem").
   * @return false if the pointer points to nothing.
   */
  bool isEmpty() const {
    Lock lock(stateMutex);
    return static_cast<bool> (!item);
  }
};



#endif	/* PTRENVELOPE_HPP */

