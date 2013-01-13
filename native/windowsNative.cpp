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
/*!
 \file MidiWindowsNative.cpp
 */


#include <jni.h>
#include "javah/MidiIO4Java_Implementation_MidiWindowsNative.h"
#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher.\n"
#endif

/***
 * Indicate whether the Windows multi-media  architecture is available.
 * Class:     MidiIO4Java_Implementation_MidiWindowsNative
 * Method:    _isAvailable
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_MidiIO4Java_Implementation_MidiWindowsNative__1isAvailable
(JNIEnv *, jclass) {
#ifdef WITH_WINMM
  return (jboolean) true;
#else
  return (jboolean) false;
#endif  
}

#ifdef WITH_WINMM
#include "Util.h"

#include <Windows.h>
#include <Mmsystem.h>
//  link this file with Winmm.lib

/**
 * Class:     MidiIO4Java_Implementation_MidiWindowsNative
 * Method:    _getMidiInputPortCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiWindowsNative__1getMidiInputPortCount
(JNIEnv *, jclass) {
  return (jint) midiInGetNumDevs();
}

/**
 * Class:     MidiIO4Java_Implementation_MidiWindowsNative
 * Method:    _getMidiOutputPortCount
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_MidiIO4Java_Implementation_MidiWindowsNative__1getMidiOutputPortCount
(JNIEnv *, jclass) {
  return (jint) midiOutGetNumDevs();
}

/*
 * Class:     Midi4IOJava_Implementation_MidiWindowsNative
 * Method:    _getMidiInputPortInfo
 * Signature: (ILMidi4Java/MidiPort/Info;)LMidi4Java/MidiPort/Info;
 */
JNIEXPORT jobject JNICALL Java_MidiIO4Java_Implementation_MidiWindowsNative__1getMidiInputPortInfo
(JNIEnv * env, jclass, jint infoIndex, jobject emptyTemplate) {

  try {
    jclass emptyTemplateCls = env->GetObjectClass(emptyTemplate);
    jfieldID indexFid = Util::getFieldID(env, emptyTemplateCls, "index", "I");
    jfieldID versionFid = Util::getFieldID(env, emptyTemplateCls, "version", "Ljava/lang/String;");
    jfieldID descriptionFid = Util::getFieldID(env, emptyTemplateCls, "description", "Ljava/lang/String;");
    jfieldID vendorFid = Util::getFieldID(env, emptyTemplateCls, "vendor", "Ljava/lang/String;");
    jfieldID inputFid = Util::getFieldID(env, emptyTemplateCls, "input", "Z");
    jfieldID nameFid = Util::getFieldID(env, emptyTemplateCls, "name", "Ljava/lang/String;");

    MIDIINCAPS midiInCaps;
    MMRESULT result = midiInGetDevCaps(infoIndex, &midiInCaps, sizeof (MIDIINCAPS));
    switch (result) {
      case MMSYSERR_BADDEVICEID:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The specified device identifier is out of range.");
      case MMSYSERR_INVALPARAM:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The specified pointer or structure is invalid.");
      case MMSYSERR_NODRIVER:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The driver is not installed.");
      case MMSYSERR_NOMEM:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The system is unable to allocate or lock memory.");
      case MMSYSERR_NOERROR: break;
      default:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "Call to System function failed.");
    }


    unsigned int majorVersion = (midiInCaps.vDriverVersion & 0xFF00) >> 8;
    unsigned int minorVersion = midiInCaps.vDriverVersion & 0x00FF;

    char versionBuff [8];
    sprintf(versionBuff, "%2d.%02d", majorVersion, minorVersion);
    char unknownVendorBuff [8];
    sprintf(unknownVendorBuff, "0x%4X", midiInCaps.wMid);


    jstring namej = env->NewStringUTF(midiInCaps.szPname);
    jstring versionj = env->NewStringUTF(versionBuff);
    jstring descriptionj = env->NewStringUTF("MIDI_In");
    jstring vendorj = env->NewStringUTF(Util::getMsdnVendor(midiInCaps.wMid, unknownVendorBuff));

    env->SetIntField(emptyTemplate, indexFid, infoIndex);
    env->SetBooleanField(emptyTemplate, inputFid, true);
    env->SetObjectField(emptyTemplate, nameFid, namej);
    env->SetObjectField(emptyTemplate, versionFid, versionj);
    env->SetObjectField(emptyTemplate, descriptionFid, descriptionj);
    env->SetObjectField(emptyTemplate, vendorFid, vendorj);

    // the normal exit, now the template is not empty any more....
    return emptyTemplate;

  } catch (exception e) {
    Util::throwProcessException(env, e.what(), NULL);
  }
  return NULL; // there was an error...
}

/*
 * Class:     Midi4IOJava_Implementation_MidiWindowsNative
 * Method:    _getMidiOutputPortInfo
 * Signature: (ILMidi4Java/MidiPort/Info;)LMidi4Java/MidiPort/Info;
 */
JNIEXPORT jobject JNICALL Java_MidiIO4Java_Implementation_MidiWindowsNative__1getMidiOutputPortInfo
(JNIEnv * env, jclass, jint infoIndex, jobject emptyTemplate) {

  try {
    jclass emptyTemplateCls = env->GetObjectClass(emptyTemplate);
    jfieldID indexFid = Util::getFieldID(env, emptyTemplateCls, "index", "I");
    jfieldID versionFid = Util::getFieldID(env, emptyTemplateCls, "version", "Ljava/lang/String;");
    jfieldID descriptionFid = Util::getFieldID(env, emptyTemplateCls, "description", "Ljava/lang/String;");
    jfieldID vendorFid = Util::getFieldID(env, emptyTemplateCls, "vendor", "Ljava/lang/String;");
    jfieldID inputFid = Util::getFieldID(env, emptyTemplateCls, "input", "Z");
    jfieldID nameFid = Util::getFieldID(env, emptyTemplateCls, "name", "Ljava/lang/String;");

    MIDIOUTCAPS midiOutCaps;
    MMRESULT result = midiOutGetDevCaps(infoIndex, &midiOutCaps, sizeof (MIDIOUTCAPS));
    switch (result) {
      case MMSYSERR_BADDEVICEID:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The specified device identifier is out of range.");
      case MMSYSERR_INVALPARAM:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The specified pointer or structure is invalid.");
      case MMSYSERR_NODRIVER:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The driver is not installed.");
      case MMSYSERR_NOMEM:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "The system is unable to allocate or lock memory.");
      case MMSYSERR_NOERROR: break;
      default:
        throw std::runtime_error("MidiWindowsNative.cpp: \"midiInGetDevCaps\", "
                "Call to System function failed.");
    }


    unsigned int majorVersion = (midiOutCaps.vDriverVersion & 0xFF00) >> 8;
    unsigned int minorVersion = midiOutCaps.vDriverVersion & 0x00FF;

    char versionBuff [8];
    sprintf(versionBuff, "%2d.%02d", majorVersion, minorVersion);
    char unknownVendorBuff [8];
    sprintf(unknownVendorBuff, "0x%4X", midiOutCaps.wMid);

    jstring namej = env->NewStringUTF(midiOutCaps.szPname);
    jstring versionj = env->NewStringUTF(versionBuff);
    jstring descriptionj = env->NewStringUTF(Util::getMsdnMidiOutTechnology(midiOutCaps.wTechnology));
    jstring vendorj = env->NewStringUTF(Util::getMsdnVendor(midiOutCaps.wMid, unknownVendorBuff));

    env->SetIntField(emptyTemplate, indexFid, infoIndex);
    env->SetBooleanField(emptyTemplate, inputFid, false);
    env->SetObjectField(emptyTemplate, nameFid, namej);
    env->SetObjectField(emptyTemplate, versionFid, versionj);
    env->SetObjectField(emptyTemplate, descriptionFid, descriptionj);
    env->SetObjectField(emptyTemplate, vendorFid, vendorj);

    // the normal exit, now the template is not empty any more....
    return emptyTemplate;

  } catch (exception e) {
    Util::throwProcessException(env, e.what(), NULL);
  }
  return NULL; // there was an error...
}
#endif
