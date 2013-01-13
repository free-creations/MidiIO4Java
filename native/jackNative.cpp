/*
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

#include <jni.h>
#include "javah/MidiIO4Java_Implementation_MidiJackNative.h"
#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher.\n"
#endif
#ifdef WITH_JACK
#include <sstream>
#include <string>
#include <jack/jack.h>
#include <exception>
#include <memory>
#include <atomic>

#include "portchain.hpp"
#include "port.hpp"
#include "JackInputPort.hpp"
#include "JackOutputPort.hpp"
#include "util.hpp"
#include "ControllPort.hpp"
#include "JackSystemListener.hpp"
#include "messages.hpp"


using namespace std;
static jack_client_t * clientId = nullptr;

/**
 * The "isConnected" flag indicates whether a conection to the Jack server
 * is estabished.
 */
static atomic<bool> isConnected(false);
/**
 * As long as the "isActivated" is false, Native callbacks will return
 * witout processig.
 */
static atomic<bool> isActivated(false);

typedef unique_lock<mutex> Lock;
static mutex activatedMutex;

class JackPortChain : public PortChain {
public:

  JackPortChain() :
  PortChain() {
  }
};

static unique_ptr<JackPortChain> jackPortChain = unique_ptr<JackPortChain > (new JackPortChain());

static JackSystemListener jackSystemListener;

#endif

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _isAvailable
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1isAvailable
(JNIEnv *, jclass) {
#ifdef WITH_JACK  
  return (jboolean) true;
#else
  return (jboolean) false;
#endif
}


#ifdef WITH_JACK

/****
#define STRINGIFY(x) #x

#define TOSTRING(x) STRINGIFY(x)

#define AT __FILE__ "(" TOSTRING(__LINE__) "): " 

#define THROW(message) \
  throw runtime_error(__FILE__ "(" TOSTRING(__LINE__) "): " message);
 ***/

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _isOpen
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1isOpen
(JNIEnv *, jclass) {
  return isConnected;
}

int nativeProcess(jack_nframes_t timeCodeDuration, void* arg) {
  Lock lock(activatedMutex);
  try {
    if (isActivated) {
      jack_nframes_t timeCodeStart = jack_last_frame_time(clientId);
      jackPortChain->execNativeCycle(timeCodeStart, timeCodeDuration, clientId);
    } else {
      cerr << "!!! Oh my!!! Port-chain not activated in native process\n";
    }
  } catch (...) {
    cerr << "!!! Exception in nativeProcess\n";
  }

  return 0;

}

/**
 * Implements the _open method of the java class
 * MidiIO4Java.Implementation.MidiJackNative.
 * This method connects to the Jack server and registers the portchain,
 * but does not activate the Native callback (this will happen in the run method).
 * The calling thread will execute the onOpen callback on the port-listeners.
 * At the end of this procedure:
 * 1) the portchain is in state registered.
 * 2) all ports are in state registered.
 * 3) the system listener is in state activated.
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _open
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1open
(JNIEnv * env, jclass, jstring jClientName, jobject jSystemListener) {
  // Note: this procedure is NOT thread safe, it must be protected against
  // concurrent access on open() and close() at the Java side.
  try {
    if (isConnected) {
      return MidiIO4Java_Implementation_MidiJackNative_errorAlreadyOpen;
    }
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Programming Error: jackPortChain is empty.")
    }
    isActivated = false;

    clientId = nullptr;

    jack_status_t status;

    const char* cClientName = env->GetStringUTFChars(jClientName, nullptr);

    clientId = jack_client_open(cClientName, JackNoStartServer, &status);
    if (status == 0) {
      if (clientId != nullptr) {
        isConnected = true;
      }
    }
    env->ReleaseStringUTFChars(jClientName, cClientName);


    if (isConnected) {
      jack_set_process_callback(clientId, nativeProcess, nullptr);

      jackPortChain->initialize(env, jSystemListener,
              unique_ptr<ControlPort > (new ControlPort(false, string("startPort"), -1)), //start control
              unique_ptr<ControlPort > (new ControlPort(true, string("endPort"), -2))); //end control

      jackPortChain->registerAtServer(clientId);

      jackSystemListener.initialize(env, jSystemListener);
      jackSystemListener.activate(clientId);

      return MidiIO4Java_Implementation_MidiJackNative_noError;
    } else {
      return MidiIO4Java_Implementation_MidiJackNative_errorConnectionFailed;
    }
  } catch (std::exception& ex) {
    isConnected = false;
    clientId = nullptr;
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return -1; // there was an error...
}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _close
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1close
(JNIEnv * env, jclass) {
  //Note: this procedure is NOT thread safe, it must be protected against
  //concurrent access on open() and close() at the Java side.
  //Please note: even after this procedure has ended the nativeProcess-callback
  //might be invoked , therefore do not nullify the jackPortChain-pointer
  // nor the clientId here.
  try {
    if (!isConnected) {
      return MidiIO4Java_Implementation_MidiJackNative_errorNotOpen;
    }
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Programming Error: jackPortChain is empty.")
    }
    if (jackPortChain->isRunningState()) {
      jackPortChain->stop();
    }
    // disconnect the client from the jack server
    int errorDeactivate = 0;
    if (isActivated) {
      errorDeactivate = jack_deactivate(clientId);
    }
    isActivated = false;

    // close the port-chain 
    jackPortChain->shutdown(env, clientId);
    exception_ptr processException = jackPortChain->retrieveProcessException();
    isConnected = false;

    // prepare a fresh new port-chain for the next open
    {
      Lock lock(activatedMutex);
      jackSystemListener.shutdown(env, clientId);
      jackPortChain = unique_ptr<JackPortChain > (new JackPortChain());
    }

    int errorClose = jack_client_close(clientId);
    if (errorClose != 0) {
      THROW("JACK ERROR while closing client")
    }
    if (errorDeactivate != 0) {
      THROW("JACK ERROR while deactivating client")
    }

    if (processException != nullptr) {
      rethrow_exception(processException);
    }
    return MidiIO4Java_Implementation_MidiJackNative_noError;
  } catch (JavaException& ex) {
    ex.throwIntoJava(env);
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return -1; // there was an error...
}

/**
 * Fill out the given info-object with data about the given port
 * @param port
 * @param info an empty info object that should be filled in.
 * @return an info object 
 */
jobject completeInfo(JNIEnv * env, const jack_port_t* port, jobject info) {
  if (clientId == nullptr) {
    return nullptr;
  }

  try {
    const char* portNameC = jack_port_name(port);
    const char* portTypeC = jack_port_type(port);
    bool portIsMine = jack_port_is_mine(clientId, port);
    bool isJackInput = jack_port_flags(port) & JackPortIsInput;
    bool isMineInput;

    if (portIsMine)
      isMineInput = isJackInput;
    else
      isMineInput = !isJackInput;

    jclass infoCls = env->GetObjectClass(info);
    jfieldID indexFid = Util::getFieldID(env, infoCls, "index", "I");
    jfieldID versionFid = Util::getFieldID(env, infoCls, "version", "Ljava/lang/String;");
    jfieldID descriptionFid = Util::getFieldID(env, infoCls, "description", "Ljava/lang/String;");
    jfieldID vendorFid = Util::getFieldID(env, infoCls, "vendor", "Ljava/lang/String;");
    jfieldID inputFid = Util::getFieldID(env, infoCls, "input", "Z");
    jfieldID nameFid = Util::getFieldID(env, infoCls, "name", "Ljava/lang/String;");

    jstring portNameJ = env->NewStringUTF(portNameC);
    jstring versionJ = env->NewStringUTF("0.0");
    jstring descriptionJ = env->NewStringUTF(portTypeC);
    jstring vendorJ = env->NewStringUTF("Jack Audio");

    env->SetIntField(info, indexFid, -1);
    env->SetBooleanField(info, inputFid, isMineInput);
    env->SetObjectField(info, nameFid, portNameJ);
    env->SetObjectField(info, versionFid, versionJ);
    env->SetObjectField(info, descriptionFid, descriptionJ);
    env->SetObjectField(info, vendorFid, vendorJ);

    // the normal exit, now the template is not empty any more....
    return info;
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return nullptr; // there was an error...

}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _getMidiInputPortCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1getMidiInputPortCount
(JNIEnv *, jclass) {
  if (clientId == nullptr) {
    //synchronized on java side
    return 0;
  }
  int count = 0;

  // List of available ports (note our inputs are Jack's outputs)
  const char **ports = jack_get_ports(clientId, nullptr, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput);

  if (ports == nullptr) return 0;
  while (ports[count] != nullptr)
    count++;

  jack_free(ports);

  return count;
}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _getMidiOutputPortCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1getMidiOutputPortCount
(JNIEnv *, jclass) {
  if (clientId == nullptr) {
    //synchronized on java side
    return 0;
  }
  int count = 0;

  // List of available ports (note our outputs are Jack's inputs)
  const char **ports = jack_get_ports(clientId, nullptr, JACK_DEFAULT_MIDI_TYPE, JackPortIsInput);

  if (ports == nullptr) return 0;
  while (ports[count] != nullptr)
    count++;

  jack_free(ports);

  return count;
}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _getMidiInputPortInfo
 * Signature: (ILMidiIO4Java/Implementation/InfoImpl;)LMidiIO4Java/MidiPort/Info;
 */
JNIEXPORT jobject JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1getMidiInputPortInfo
(JNIEnv * env, jclass, jint infoIndex, jobject emptyTemplate) {

  if (clientId == nullptr) {
    //synchronized on java side
    return nullptr;
  }

  ostringstream ost;
  string retStr("");

  try {

    // List of available ports
    const char **ports = jack_get_ports(clientId, nullptr,
            JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput);

    // Check port validity
    if (ports == nullptr) {
      THROW("Call to System function failed.")
    }
    int count = 0;
    while (ports[count] != nullptr)
      count++;
    if ((infoIndex >= count) || (infoIndex < 0)) {
      ost << AT "The 'infoIndex' argument (" << infoIndex << ") is invalid.";
      throw runtime_error(ost.str());
    }

    if (ports[infoIndex] == nullptr) {
      ost << AT "The 'infoIndex' argument (" << infoIndex << ") is invalid.";
      throw runtime_error(ost.str());
    } else retStr.assign(ports[infoIndex]);
    jack_free(ports);


    jclass emptyTemplateCls = env->GetObjectClass(emptyTemplate);
    jfieldID indexFid = Util::getFieldID(env, emptyTemplateCls, "index", "I");
    jfieldID versionFid = Util::getFieldID(env, emptyTemplateCls, "version", "Ljava/lang/String;");
    jfieldID descriptionFid = Util::getFieldID(env, emptyTemplateCls, "description", "Ljava/lang/String;");
    jfieldID vendorFid = Util::getFieldID(env, emptyTemplateCls, "vendor", "Ljava/lang/String;");
    jfieldID inputFid = Util::getFieldID(env, emptyTemplateCls, "input", "Z");
    jfieldID nameFid = Util::getFieldID(env, emptyTemplateCls, "name", "Ljava/lang/String;");

    jstring namej = env->NewStringUTF(retStr.c_str());
    jstring versionj = env->NewStringUTF("0.0");
    jstring descriptionj = env->NewStringUTF("MIDI_In");
    jstring vendorj = env->NewStringUTF("Jack Audio");

    env->SetIntField(emptyTemplate, indexFid, infoIndex);
    env->SetBooleanField(emptyTemplate, inputFid, true);
    env->SetObjectField(emptyTemplate, nameFid, namej);
    env->SetObjectField(emptyTemplate, versionFid, versionj);
    env->SetObjectField(emptyTemplate, descriptionFid, descriptionj);
    env->SetObjectField(emptyTemplate, vendorFid, vendorj);

    // the normal exit, now the template is not empty any more....
    return emptyTemplate;
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return nullptr; // there was an error...
}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _getMidiOutputPortInfo
 * Signature: (ILMidiIO4Java/Implementation/InfoImpl;)LMidiIO4Java/MidiPort/Info;
 */
JNIEXPORT jobject JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1getMidiOutputPortInfo
(JNIEnv * env, jclass, jint infoIndex, jobject emptyTemplate) {

  if (clientId == nullptr) {
    //synchronized on java side
    return nullptr;
  }

  ostringstream ost;
  string portNameC("");

  try {

    // List of available ports
    const char **ports = jack_get_ports(clientId, nullptr,
            JACK_DEFAULT_MIDI_TYPE, JackPortIsInput);

    // Check port validity
    if (ports == nullptr) {
      THROW("Call to System function failed.")
    }
    int count = 0;
    while (ports[count] != nullptr)
      count++;
    if ((infoIndex >= count) || (infoIndex < 0)) {
      ost << AT "The 'infoIndex' argument (" << infoIndex << ") is invalid.";
      throw runtime_error(ost.str());
    }

    if (ports[infoIndex] == nullptr) {
      ost << "MidiJackNative.cpp: the 'infoIndex' argument (" << infoIndex << ") is invalid.";
      throw runtime_error(ost.str());
    } else portNameC.assign(ports[infoIndex]);
    jack_free(ports);


    jclass emptyTemplateCls = env->GetObjectClass(emptyTemplate);
    jfieldID indexFid = Util::getFieldID(env, emptyTemplateCls, "index", "I");
    jfieldID versionFid = Util::getFieldID(env, emptyTemplateCls, "version", "Ljava/lang/String;");
    jfieldID descriptionFid = Util::getFieldID(env, emptyTemplateCls, "description", "Ljava/lang/String;");
    jfieldID vendorFid = Util::getFieldID(env, emptyTemplateCls, "vendor", "Ljava/lang/String;");
    jfieldID inputFid = Util::getFieldID(env, emptyTemplateCls, "input", "Z");
    jfieldID nameFid = Util::getFieldID(env, emptyTemplateCls, "name", "Ljava/lang/String;");

    jstring portNameJ = env->NewStringUTF(portNameC.c_str());
    jstring versionJ = env->NewStringUTF("0.0");
    jstring descriptionJ = env->NewStringUTF("MIDI_Out");
    jstring vendorJ = env->NewStringUTF("Jack Audio");

    env->SetIntField(emptyTemplate, indexFid, infoIndex);
    env->SetBooleanField(emptyTemplate, inputFid, false);
    env->SetObjectField(emptyTemplate, nameFid, portNameJ);
    env->SetObjectField(emptyTemplate, versionFid, versionJ);
    env->SetObjectField(emptyTemplate, descriptionFid, descriptionJ);
    env->SetObjectField(emptyTemplate, vendorFid, vendorJ);

    // the normal exit, now the template is not empty any more....
    return emptyTemplate;
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return nullptr; // there was an error...
}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _createOutputPort
 * Signature: (JLMidiIO4Java/Implementation/InfoImpl;Ljava/lang/String;LMidiIO4Java/Implementation/MidiJackNative/MidiOutputPort;)I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1createOutputPort
(JNIEnv * env, jclass, jlong portID, jobject emptyTemplate, jstring portNameJ, jobject javaPort) {
  // Note: this procedure is NOT thread safe, it must be protected against
  // concurrent access on open() and close() at the Java side.
  try {
    if (portNameJ == nullptr) {
      THROW("Port-name is null.")
    }
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Port-chain NULL pointer exception.")
    }

    const char * portNameC = env->GetStringUTFChars(portNameJ, nullptr);

    unique_ptr<Port> newPort =
            unique_ptr<JackOutputPort > (new JackOutputPort(string(portNameC), portID));
    newPort->initialize(env, portNameJ, javaPort);
    env->ReleaseStringUTFChars(portNameJ, portNameC);

    jackPortChain->addPort(move(newPort), clientId);


    /**@ToDo fill-in the template...*/
    return MidiIO4Java_Implementation_MidiJackNative_noError;

  } catch (JavaException& ex) {
    ex.throwIntoJava(env);
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return -1; // there was an error...

}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _createInputPort
 * Signature: (JLMidiIO4Java/Implementation/InfoImpl;Ljava/lang/String;LMidiIO4Java/Implementation/MidiJackNative/JackMidiPort;)I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1createInputPort
(JNIEnv * env, jclass, jlong portID, jobject emptyTemplate, jstring portNameJ, jobject javaPort) {
  // Note: this procedure is NOT thread safe, it must be protected against
  // concurrent access on open() and close() at the Java side.
  try {
    if (portNameJ == nullptr) {
      THROW("Port-name is null.")
    }
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Port-chain NULL pointer exception.")
    }

    const char * portNameC = env->GetStringUTFChars(portNameJ, nullptr);

    unique_ptr<Port> newPort =
            unique_ptr<JackInputPort > (new JackInputPort(string(portNameC), portID));
    newPort->initialize(env, portNameJ, javaPort);
    env->ReleaseStringUTFChars(portNameJ, portNameC);

    jackPortChain->addPort(move(newPort), clientId);


    /**@ToDo fill-in the template...*/
    return MidiIO4Java_Implementation_MidiJackNative_noError;

  } catch (JavaException& ex) {
    ex.throwIntoJava(env);
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return -1; // there was an error...
}

/**
 * Close a Port. It is assumed that the given portId belongs to a port hooked
 * into the current portchain. The given portId is searched in the portchain.
 * The peer class is removed from the portchain, than this peer is closed and
 * destroyed.
 *
 * Implements: MidiIO4Java.Implementation.MidiJackNative._closePort
 * @param env pointer to calling the Java thread.
 * @param internalPortId the internal identifier of the port 
 * @return 0 if the port could successfully be closed; a negative error code
 * if something went wrong.
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1closePort
(JNIEnv * env, jclass, jlong internalPortId) {

  try {
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Port-chain NULL pointer exception.")
    }

    unique_ptr<Port> removedPort = move(jackPortChain->removePort(env, clientId, internalPortId));
    if (removedPort->hasProcessException()) {
      rethrow_exception(removedPort->getProcessException());
    }
    return MidiIO4Java_Implementation_MidiJackNative_noError;

  } catch (JavaException& ex) {
    ex.throwIntoJava(env);
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  } catch (...) {
    return MidiIO4Java_Implementation_MidiJackNative_errorClosingPort; // there was an error...
  }
  return MidiIO4Java_Implementation_MidiJackNative_errorClosingPort; // there was an error...
}

/**
 * Indicates whether the port given by the internalPortId can be considered
 * to be closed.
 * The given portId is searched in the portchain; if it is not found, it is
 * assumed that a port has been closed and removed.
 * implements: MidiIO4Java.Implementation.MidiJackNative._isClosedPort
 * @param env pointer to calling the Java thread.
 * @param internalPortId the internal identifier of the port 
 * @return false if a port with the given id was found.
 */
JNIEXPORT jboolean JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1isClosedPort
(JNIEnv * env, jclass, jlong internalPortId) {
  try {
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Port-chain NULL pointer exception.")
    }
    return !jackPortChain->portExists(internalPortId);
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
  return true; // in case of an error
}

/**
 * This will start the processing.
 * The calling thread will be blocked until close() is executed.
 * @param env the java environment pointer
 * @param ignored
 */
JNIEXPORT void JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1run
(JNIEnv * env, jclass) {
  int err = 0;
  try {
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Port-chain NULL pointer exception.")
    }
    if (!jackPortChain->isRegisteredState()) {
      THROW("Cannot run; port-chain in wrong state.")
    }
    if (!isConnected) {
      THROW("Cannot run; not connected.")
    }
    if (isActivated) {
      THROW("Cannot run; already activated.")
    }
    {
      Lock lock(activatedMutex);
      //start the Native callback loop
      jackPortChain->start();
      err = jack_activate(clientId);
      if (err != 0) {
        THROW("Could not activate client.");
      }
      isActivated = true;
    }
    // start the java callback loop.
    jackPortChain->runJava(env); // No return until stop() has executed!!!!

  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
}

/*
 * Class:     MidiIO4Java_Implementation_MidiJackNative
 * Method:    _waitForCycleDone
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_MidiIO4Java_Implementation_MidiJackNative__1waitForCycleDone
(JNIEnv * env, jclass) {
  try {
    if (!static_cast<bool> (jackPortChain)) {
      THROW("Port-chain NULL pointer exception.")
    }
    jackPortChain->waitForCycleDone();
  } catch (std::exception& ex) {
    Util::throwProcessException(env, ex.what(), nullptr);
  }
}

#endif // with Jack




