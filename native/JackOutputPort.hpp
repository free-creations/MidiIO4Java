/* 
 * File:   JackOutputPort.hpp
 * Author: harald
 *
 * Created on September 21, 2012, 1:09 PM
 */

#ifndef JACKOUTPUTPORT_HPP
#define	JACKOUTPUTPORT_HPP

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

class JackOutputPort : public Port {
private:
  const string name;
  jobject javaPort;
  jmethodID onOpenMid;
  jmethodID processMid;
  jmethodID onCloseMid;
  jack_port_t* jackPort;
  /** The rawMidiData will store "integer-triplets" of "status "data1 "data2" */
  jint bufferRawMidi[3 * MaxMidiEvents];
  jint bufferDeltaTimes[MaxMidiEvents];
  jint bufferEventSizes[MaxMidiEvents];
  int bufferEventCount;

  /** the java arrays will be used to transfer the "integer-triplets" into the java environments*/
  jintArray javaRawMidi;
  jintArray javaDeltaTimes;
  jintArray javaEventSizes;

  jlong timestampDeprecated;
  jack_nframes_t jackBufferSizeDeprecated;

public:

  /**
   * 
   * @param _name
   * @param internalId
   */
  JackOutputPort(const string& _name, long internalId) :
  Port(true, internalId),
  name(_name),
  javaPort(NULL),
  onOpenMid(NULL),
  processMid(NULL),
  onCloseMid(NULL),
  jackPort(nullptr),
  bufferEventCount(0),
  javaRawMidi(NULL),
  javaDeltaTimes(NULL),
  javaEventSizes(NULL) {
  }

  JackOutputPort(JackOutputPort && other) = default;

  JackOutputPort(const JackOutputPort & other) = delete;

  virtual ~JackOutputPort() {

  }

protected:

  /**
   * 1) Store the pointer to the listener (this will exclude it from garbage collection).
   * 2) cache the method-identifiers of the listeners methods.
   * 3) prepare a number of Java Arrays that we'll use to transfer data from Java to native
   * 4) execute the listeners onOpen method.
   * @param env the java environment pointer
   * @param name is ignored (the name is given in the constructor)
   * @param _javaPort a reference to the listener. It is a Java object of 
   * class MidiIO4Java.Implementation.MidiJackNative$MidiOutputPort
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
      THROW("MidiOutputPortListener class not found.")
    }
    onOpenMid = env->GetMethodID(javaPortClass, "onOpen", "()V");
    processMid = env->GetMethodID(javaPortClass, "process", "(JJZ[I[I[I)I");
    onCloseMid = env->GetMethodID(javaPortClass, "onClose", "()V");
    if ((onOpenMid == NULL) || (processMid == NULL) || (onCloseMid == NULL)) {
      THROW("Method-identifier not found.")
    }

    //----- prepare the buffers to transfer the raw-data from java to native
    javaRawMidi = static_cast<jintArray> (env->NewGlobalRef(env->NewIntArray(3 * MaxMidiEvents)));
    javaDeltaTimes = static_cast<jintArray> (env->NewGlobalRef(env->NewIntArray(MaxMidiEvents)));
    javaEventSizes = static_cast<jintArray> (env->NewGlobalRef(env->NewIntArray(MaxMidiEvents)));
    if ((javaRawMidi == NULL) || (javaDeltaTimes == NULL) || (javaEventSizes == NULL)) {
      THROW("Out of memory.")
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

    jackPort = jack_port_register(jackClient, name.c_str(), JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
    if (jackPort == nullptr) {
      ostringstream ost;
      ost << AT "JACK error creating port (" << name << ").";
      throw runtime_error(ost.str());
    }

  }

  virtual void start_impl()override {
  }

  virtual void execJavaProcess_impl(JNIEnv * env, unsigned long timeCodeStart, unsigned long timeCodeDuration, bool lastCycle)override {


    // obtain Midi events from java listener. 
    // java signature :"public int process(long timeCodeStart, long timeCodeDuration, boolean lastCycle, int[] rawEventsOut,int[] deltaTimesOut,int[] eventSizeOut)throws Throwable"
    bufferEventCount = env->CallIntMethod(javaPort, processMid,
            (jlong) timeCodeStart,
            (jlong) timeCodeDuration,
            (jboolean) lastCycle,
            javaRawMidi, // java signature: int[] rawEventsOut,
            javaDeltaTimes, // java signature: int[] deltaTimesOut,
            javaEventSizes); // java signature: int[] eventSizeOut
    jthrowable jexception = env->ExceptionOccurred();
    if (jexception != NULL) {
      //THROW_JAVA(env, jexception)
      bufferEventCount = 0; /**@ToDo consider to write "all-sounds-off" to the buffer**/
      THROW_JAVA(env, jexception)
    }

    // transfer raw midi events from java arrays into native arrays.
    env->GetIntArrayRegion(javaRawMidi, 0, 3 * MaxMidiEvents, bufferRawMidi);
    env->GetIntArrayRegion(javaDeltaTimes, 0, MaxMidiEvents, bufferDeltaTimes);
    env->GetIntArrayRegion(javaEventSizes, 0, MaxMidiEvents, bufferEventSizes);
  }

  virtual void execNativeProcess_impl(unsigned long timeCodeStart, unsigned long timeCodeDuration, void * client)override {
    if (jackPort == nullptr) {
      THROW("jackPort is NULL.")
    }

    void* jackBuffer = jack_port_get_buffer(jackPort, timeCodeDuration);
    jack_midi_clear_buffer(jackBuffer);

    int rawMidiIdx = 0;
    int offset = 0;
    for (int i = 0; i < bufferEventCount; i++) {
      int eventSize = bufferEventSizes[i];
      if (bufferDeltaTimes[i] < offset) {
        THROW("Midi-Event was out of order.")
      }
      offset = bufferDeltaTimes[i];
      jack_midi_data_t* eventBuffer = jack_midi_event_reserve(jackBuffer, offset, eventSize);
      if (eventBuffer == NULL) {
        THROW("Not enough space to write Midi Events.")
      }
      for (int j = 0; j < eventSize; j++) {
        eventBuffer[j] = static_cast<jack_midi_data_t> (bufferRawMidi[rawMidiIdx + j]);
      }
      rawMidiIdx = rawMidiIdx + 3;
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
    env->DeleteGlobalRef(javaRawMidi);
    env->DeleteGlobalRef(javaDeltaTimes);
    env->DeleteGlobalRef(javaEventSizes);
    javaPort = NULL;
    onOpenMid = NULL;
    processMid = NULL;
    onCloseMid = NULL;
    javaRawMidi = NULL;
    javaDeltaTimes = NULL;
    javaEventSizes = NULL;
  }


};


#endif	/* WITH_JACK */
#endif	/* JACKOUTPUTPORT_HPP */

