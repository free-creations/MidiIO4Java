/* 
 * File:   JackInputPort.hpp
 * Author: harald
 *
 * Created on September 21, 2012, 1:09 PM
 */

#ifndef JACKINPUTPORT_HPP
#define	JACKINPUTPORT_HPP

#include <jni.h>
#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher.\n"
#endif

#ifdef WITH_JACK
#include <jack/jack.h>
#include <jack/midiport.h>
#include <string>
#include <sstream>
#include "port.hpp"
#include "messages.hpp"

using namespace std;

#define	MaxMidiEvents 255

class JackInputPort : public Port {
private:
  const string name;
  jobject javaPort;
  jmethodID onOpenMid;
  jmethodID processMid;
  jmethodID onCloseMid;
  jack_port_t* jackPort;
  /** The rawMidiData will store triplets of "status "data1 "data2" */
  jint bufferRawMidi[3 * MaxMidiEvents];
  jint bufferDeltaTimes[MaxMidiEvents];
  int bufferEventCount;
  jlong timestampDeprecated;

public:

  /**
   * 
   * @param _name
   * @param internalId
   */
  JackInputPort(const string& _name, long internalId) :
  Port(false, internalId),
  name(_name),
  javaPort(NULL),
  onOpenMid(NULL),
  processMid(NULL),
  onCloseMid(NULL),
  jackPort(nullptr),
  bufferEventCount(0) {
  }

  JackInputPort(JackInputPort && other) = default;

  JackInputPort(const JackInputPort & other) = delete;

  virtual ~JackInputPort() {

  }

protected:

  /**
   * 1) Store the pointer to the listener (this will exclude it from garbage collection).
   * 2) cache the method-identifiers of the listeners methods.
   * 3) execute the listeners onOpen method.
   * @param env
   * @param 
   * @param _javaPort a reference to the listener. It is a Java object of 
   * class MidiIO4Java.Implementation.MidiJackNative$MidiInputPort
   */
  virtual void initialize_impl(JNIEnv * env, jstring /*name*/, jobject _javaPort)override {
    // pin the java-port
    javaPort = env->NewGlobalRef(_javaPort);
    if (javaPort == NULL) {
      THROW("Call to NewGlobalRef function failed.")
    }
    // --- cache the method IDs of the callback functions in the java port.
    jclass javaPortClass = env->GetObjectClass(javaPort);

    if (javaPortClass == NULL) {
      THROW("MidiInputPortListener class not found.")
    }
    onOpenMid = env->GetMethodID(javaPortClass, "onOpen", "()V");
    processMid = env->GetMethodID(javaPortClass, "process", "(JJZ[I[I)V");
    onCloseMid = env->GetMethodID(javaPortClass, "onClose", "()V");
    if ((onOpenMid == NULL) || (processMid == NULL) || (onCloseMid == NULL)) {
      THROW("Method-identifier not found.")
    }
    // --- call javaPort.onOpen()
    env->CallVoidMethod(javaPort, onOpenMid);
    jthrowable jexception = env->ExceptionOccurred();
    if (jexception != NULL) {
      THROW_JAVA(env, jexception)
    }
  }

  virtual void register_impl(void * client)override {
    if (client == nullptr) {
      THROW("Client was NULL.")
    }
    jack_client_t * jackClient = static_cast<jack_client_t *> (client);

    jackPort = jack_port_register(jackClient, name.c_str(), JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
    if (jackPort == nullptr) {
      ostringstream ost;
      ost << AT "JACK error creating port (" << name << ").";
      throw runtime_error(ost.str());
    }
  }

  virtual void start_impl()override {
  }

  virtual void execJavaProcess_impl(JNIEnv * env, unsigned long timeCodeStart, unsigned long timeCodeDuration, bool lastCycle)override {
    if (bufferEventCount > MaxMidiEvents) {
      THROW("Buffer overflow.")
    }
    jintArray rawEvents = env->NewIntArray(3 * bufferEventCount);
    jintArray deltaTimes = env->NewIntArray(bufferEventCount);

    if ((rawEvents == NULL) || (deltaTimes == NULL)) {
      THROW("Out of memory.")
    }
    env->SetIntArrayRegion(rawEvents, 0, 3 * bufferEventCount, bufferRawMidi);
    env->SetIntArrayRegion(deltaTimes, 0, bufferEventCount, bufferDeltaTimes);

    // call Java method wit java-signature:
    // "public void process(long timeCodeStart, long timeCodeDuration, boolean lastCycle,int[] rawEvents, int[] deltaTimes)throws Throwable "
    env->CallVoidMethod(javaPort, processMid,
            (jlong) timeCodeStart,
            (jlong) timeCodeDuration,
            (jboolean) lastCycle,
            rawEvents,
            deltaTimes);

    jthrowable jexception = env->ExceptionOccurred();
    if (jexception != NULL) {
      THROW_JAVA(env, jexception)
    }
  }

  virtual void execNativeProcess_impl(unsigned long timeCodeStart, unsigned long timeCodeDuration, void * client)override {
    if (jackPort == nullptr) {
      THROW("jackPort was NULL.")
    }

    bufferEventCount = 0;
    void* jackBuffer = jack_port_get_buffer(jackPort, timeCodeDuration);
    int jackEventCount = jack_midi_get_event_count(jackBuffer);
    for (int i = 0; i < jackEventCount; ++i) {

      jack_midi_event_t jackEvent;
      int error = jack_midi_event_get(&jackEvent, jackBuffer, i);
      if (error == 0) {
        if (jackEvent.size == 3) {
          if (bufferEventCount >= MaxMidiEvents) {
            THROW("Buffer overflow.")
          }
          int rawIdx = 3 * bufferEventCount;
          bufferDeltaTimes[bufferEventCount] = jackEvent.time;
          bufferRawMidi[rawIdx] = static_cast<jint> (jackEvent.buffer[0]);
          bufferRawMidi[rawIdx + 1] = static_cast<jint> (jackEvent.buffer[1]);
          bufferRawMidi[rawIdx + 2] = static_cast<jint> (jackEvent.buffer[2]);
          bufferEventCount++;
        }
      } else {
        /** @Todo better error handling.*/
        THROW("Error retrieving Midi Events.")
      }
    }


  }

  virtual void stop_impl()override {
  }

  virtual void unregister_impl(void * client)override {
    if (client == nullptr) {
      THROW("Client was NULL.")
    }
    if (jackPort == nullptr) {
      THROW("jackPort was NULL.")
    }

    jack_client_t * jackClient = static_cast<jack_client_t *> (client);

    int err = jack_port_unregister(jackClient, jackPort);
    if (err != 0) {
      THROW("JACK error while unregistering port.")
    }
    jackPort = nullptr;
  }

  /**
   * 1) execute the listeners onClose method.
   * 2) Free the pointer to the listener (this will include it into garbage collection).
   * @param env
   */
  virtual void uninitialize_impl(JNIEnv * env) override {
    if ((javaPort == NULL) || (onCloseMid == NULL)) {
      THROW("Invalid null pointer.")
    }
    env->CallVoidMethod(javaPort, onCloseMid);
    env->DeleteGlobalRef(javaPort);
    javaPort = NULL;
    onOpenMid = NULL;
    processMid = NULL;
    onCloseMid = NULL;
  }


};


#endif	/* WITH_JACK */
#endif	/* JACKINPUTPORT_HPP */

