/* 
 * File:   ControllPort.hpp
 * Author: harald
 *
 * Created on September 21, 2012, 1:09 PM
 */

#ifndef CONTROLLPORT_HPP
#define	CONTROLLPORT_HPP

#include <jni.h>
#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher.\n"
#endif


#include <string>
#include <sstream>
#include "port.hpp"
#include "messages.hpp"

using namespace std;

/**
 * Control ports are used internally in the port-chain. Control ports
 * are not seen externally and cannot be accessed through the java
 * interface. In a port-chain there are two control-ports; one at the
 * beginning, the "start-port" and one at the end of the chain, the
 * "end-port". The tasks of these ports are:
 * <ul>
 * <li>provide events for the MidiSystemListener
 * (onCycleStart, onCycleEnd, onClose, onOpen)</li>
 * <li>Synchronize the Java process-thread with the native callback
 * on an empty port-chain. Thus preventing the Java tread from
 * endlessly spinning in an empty loop wasting process resources.</li>
 */
class ControlPort : public Port {
private:
  const string name;
  jobject systemListener;
  jmethodID onOpenMid;
  jmethodID onCycleMid;
  jmethodID onCloseMid;

  /** the timecode of the next java buffer */
  jlong timeCodeStartDeprecated;
  jack_nframes_t timeCodeDurationDeprecated;

public:

  /**
   * Creates a new control port.
   * @param _isEndPort true if this port will be inserted at the end
   * of the port-chain, false if the port will be inserted at the beginning
   * of the port-chain. Note: the end-port is internally an output port,
   * the start-port is internally an input port.
   * @param _name an arbitrary name for this pseudo port.
   * @param internalId by convention the start-port will get the ID -1
   * and the end-port shall get the ID -2.
   */
  ControlPort(bool _isEndPort, const string& _name, long internalId) :
  Port(_isEndPort, internalId),
  name(_name),
  systemListener(NULL),
  onOpenMid(NULL),
  onCycleMid(NULL),
  onCloseMid(NULL) {
  }

  ControlPort(ControlPort && other) = default;

  ControlPort(const ControlPort & other) = delete;

  virtual ~ControlPort() {

  }

protected:

  /**
   * 1) Store the pointer to the listener (this will exclude it from garbage collection).
   * 2) cache the method-identifiers of the listeners methods.
   * 3) prepare a number of Java Arrays that we'll use to transfer data from Java to native
   * 4) execute the listeners onOpen method.
   * @param env the java environment pointer
   * @param name is ignored (the name is given in the constructor)
   * @param _javaPort a reference to the listener.
   * java class: public interface MidiIO4Java.MidiSystemListener 
   */
  virtual void initialize_impl(JNIEnv * env, jstring /*name*/, jobject _systemListener)override {
    // pin the java-port
    systemListener = env->NewGlobalRef(_systemListener);
    if (systemListener == NULL) {
      THROW("Call to NewGlobalRef function failed.")
    }
    // --- cache the method IDs of the callback functions in the java port.
    jclass listenerClass = env->GetObjectClass(systemListener);

    if (listenerClass == NULL) {
      THROW("MidiSystemListener class not found.")
    }
    onOpenMid = env->GetMethodID(listenerClass, "onOpen", "()V");
    if (isInput()) {
      // Note: it might not be very clean to map two different java procedures
      // to one and the same identifier. But as the two procedures
      // must have the same signature, this works nicely and avoids
      // to have to make the distinction between input- and output- port in
      // the execJavaProcess_impl function.
      onCycleMid = env->GetMethodID(listenerClass, "onCycleStart", "(JJZ)V");
    } else {
      onCycleMid = env->GetMethodID(listenerClass, "onCycleEnd", "(JJZ)V");
    }
    onCloseMid = env->GetMethodID(listenerClass, "onClose", "()V");
    if ((onOpenMid == NULL) || (onCycleMid == NULL) || (onCloseMid == NULL)) {
      THROW("Method-identifier not found.")
    }

    // --- the startControl shall call MidiSystemListener.onOpen()
    if (isInput()) {
      env->CallVoidMethod(systemListener, onOpenMid);
    }
  }

  virtual void register_impl(void * client)override {
    if (client == nullptr) {
      THROW("Client was NULL.")
    }
    jack_client_t * jackClient = static_cast<jack_client_t *> (client);

  }

  virtual void start_impl()override {
  }

  virtual void execJavaProcess_impl(JNIEnv * env, unsigned long timeCodeStart, unsigned long timeCodeDuration, bool lastCycle)override {

    // perform call-backs
    env->CallVoidMethod(systemListener, onCycleMid,
            (jlong)timeCodeStart, // java signature: long timestamp,
            (jlong)timeCodeDuration, // java signature: long timeCodeDuration
            (jboolean)lastCycle); // java signature: boolean lastCycle

  }

  virtual void execNativeProcess_impl(unsigned long _timeCodeStart, unsigned long _timeCodeDuration, void * client)override {
  }

  virtual void stop_impl()override {
  }

  virtual void unregister_impl(void * client)override {
  }

  /**
   * 1) execute the listeners onClose method.
   * 2) Free the pointer to the listener (this will include it into garbage collection).
   * @param env
   */
  virtual void uninitialize_impl(JNIEnv * env) override {
    if ((systemListener == NULL) || (onCloseMid == NULL)) {
      THROW("Invalid null pointer.")
    }
    // --- the endControl shall call MidiSystemListener.onClose()
    if (isOutput()) {
      env->CallVoidMethod(systemListener, onCloseMid);
    }
    env->DeleteGlobalRef(systemListener);
    systemListener = NULL;
    onOpenMid = NULL;
    onCycleMid = NULL;
    onCloseMid = NULL;
  }
};



#endif	/* JACKOUTPUTPORT_HPP */

