/*
 * File:   util.hpp
 *
 * Created on August 25, 2012, 4:20 PM
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


#ifndef UTIL_HPP
#define	UTIL_HPP

#include <jni.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <chrono>

using namespace std;

#ifdef WITH_WINMM
#include <Windows.h>
#include <Mmsystem.h>
#include <mmreg.h>
#endif

#ifndef JNI_VERSION_1_2
#error "Needs Java version 1.2 or higher.\n"
#endif

class Util {
public:


  /**
   * Create a Java-Exception of type MidiProcessException.
   */
  static jthrowable makeProcessException(JNIEnv * env, const string& cppMessage,
          jthrowable cause) {
    jclass cls = env->FindClass("MidiIO4Java/MidiProcessException");
    if (cls == NULL) {
      return NULL; //give up
    }
    jmethodID cid =
            env->GetMethodID(cls, "<init>",
            "(Ljava/lang/String;Ljava/lang/Throwable;)V");
    if (cid == NULL) {
      return NULL; //give up
    }
    jstring jmessage = env->NewStringUTF(cppMessage.c_str());
    jthrowable exception = (jthrowable) env->NewObject(cls, cid,
            (jobject) jmessage,
            (jobject) cause);
    env->DeleteLocalRef(cls);
    return exception;
  }

  /**
   * Raise an Exception of type MidiProcessException in the Java environment.
   */
  static void throwProcessException(JNIEnv * env, const string& cppMessage,
          jthrowable cause) {
    jthrowable exception = makeProcessException(env, cppMessage, cause);
    if (exception == NULL) {
      return; //give up
    }
    env->Throw(exception);
    env->DeleteLocalRef(exception);
  }

  /**
   * Returns the JNI Field ID for an instance field.
   * Raise an Exception if the field cannot be found;
   */
  static jfieldID getFieldID(JNIEnv * env, jclass clazz, const char *name,
          const char *sig) {
    jfieldID result = env->GetFieldID(clazz, name, sig);
    if (result == NULL) {
      string message = "No such field: Name(" + string(name) + ")";
      throw std::runtime_error(message);
    }
    return result;
  }

#ifdef WITH_WINMM

  static const char* getMsdnMidiOutTechnology(WORD wTechnology) {
    switch (wTechnology) {

      case MOD_MIDIPORT: return "MIDI_out hardware port";
      case MOD_SYNTH: return "Synthesizer";
      case MOD_SQSYNTH:return "Square wave synthesizer";
      case MOD_FMSYNTH: return "FM synthesizer";
      case MOD_MAPPER: return "Microsoft MIDI mapper";
      case MOD_WAVETABLE: return "Hardware wavetable synthesizer";
      case MOD_SWSYNTH: return "Software synthesizer";
      default:return "MIDI_out";
    }
  }

  /**
   * 
   * @param vDriverVersion
   * @param unknownVendor
   * @return 
   * @see http://msdn.microsoft.com (search for "Manufacturer Identifiers")
   */
  static const char* getMsdnVendor(MMVERSION vDriverVersion, const char* unknownVendor) {
    switch (vDriverVersion) {
      case MM_GRAVIS: return "Advanced Gravis Computer Technology, Ltd.";
      case MM_ANTEX: return "Antex Electronics Corporation";
      case MM_APPS: return "APPS Software";
      case MM_ARTISOFT: return "Artisoft, Inc.";
      case MM_AST: return "AST Research, Inc.";
      case MM_ATI: return "ATI Technologies, Inc.";
      case MM_AUDIOFILE: return "Audio, Inc.";
      case MM_APT: return "Audio Processing Technology";
      case MM_AUDIOPT: return "Audio Processing Technology";
      case MM_AURAVISION: return "Auravision Corporation";
      case MM_AZTECH: return "Aztech Labs, Inc.";
      case MM_CANOPUS: return "Canopus, Co., Ltd.";
      case MM_COMPUSIC: return "Compusic";
      case MM_CAT: return "Computer Aided Technology, Inc.";
      case MM_COMPUTER_FRIENDS: return "Computer Friends, Inc.";
      case MM_CONTROLRES: return "Control Resources Corporation";
      case MM_CREATIVE: return "Creative Labs, Inc.";
      case MM_DIALOGIC: return "Dialogic Corporation";
      case MM_DOLBY: return "Dolby Laboratories, Inc.";
      case MM_DSP_GROUP: return "DSP Group, Inc.";
      case MM_DSP_SOLUTIONS: return "DSP Solutions, Inc.";
      case MM_ECHO: return "Echo Speech Corporation";
      case MM_ESS: return "ESS Technology, Inc.";
      case MM_EVEREX: return "Everex Systems, Inc.";
      case MM_EXAN: return "EXAN, Ltd.";
      case MM_FUJITSU: return "Fujitsu, Ltd.";
      case MM_IOMAGIC: return "I/O Magic Corporation";
      case MM_ICL_PS: return "ICL Personal Systems";
      case MM_OLIVETTI: return "Ing. C. Olivetti & C., S.p.A.";
      case MM_ICS: return "Integrated Circuit Systems, Inc.";
      case MM_INTEL: return "Intel Corporation";
      case MM_INTERACTIVE: return "InterActive, Inc.";
      case MM_IBM: return "International Business Machines";
      case MM_ITERATEDSYS: return "Iterated Systems, Inc.";
      case MM_LOGITECH: return "Logitech, Inc.";
      case MM_LYRRUS: return "Lyrrus, Inc.";
      case MM_MATSUSHITA: return "Matsushita Electric Corporation of America";
      case MM_MEDIAVISION: return "Media Vision, Inc.";
      case MM_METHEUS: return "Metheus Corporation";
      case MM_MELABS: return "microEngineering Labs";
      case MM_MICROSOFT: return "Microsoft Corporation";
      case MM_MOSCOM: return "MOSCOM Corporation";
      case MM_MOTOROLA: return "Motorola, Inc.";
      case MM_NMS: return "Natural MicroSystems Corporation";
      case MM_NCR: return "NCR Corporation";
      case MM_NEC: return "NEC Corporation";
      case MM_NEWMEDIA: return "New Media Corporation";
      case MM_OKI: return "OKI";
      case MM_OPTI: return "OPTi, Inc.";
      case MM_ROLAND: return "Roland Corporation";
      case MM_SCALACS: return "SCALACS";
      case MM_EPSON: return "Seiko Epson Corporation, Inc.";
      case MM_SIERRA: return "Sierra Semiconductor Corporation";
      case MM_SILICONSOFT: return "Silicon Software, Inc.";
      case MM_SONICFOUNDRY: return "Sonic Foundry";
      case MM_SPEECHCOMP: return "Speech Compression";
      case MM_SUPERMAC: return "Supermac Technology, Inc.";
      case MM_TANDY: return "Tandy Corporation";
      case MM_KORG: return "Toshihiko Okuhura, Korg, Inc.";
      case MM_TRUEVISION: return "Truevision, Inc.";
      case MM_TURTLE_BEACH: return "Turtle Beach Systems";
      case MM_VAL: return "Video Associates Labs, Inc.";
      case MM_VIDEOLOGIC: return "VideoLogic, Inc.";
      case MM_VITEC: return "Visual Information Technologies, Inc.";
      case MM_VOCALTEC: return "VocalTec, Inc.";
      case MM_VOYETRA: return "Voyetra Technologies";
      case MM_WANGLABS: return "Wang Laboratories";
      case MM_WILLOWPOND: return "Willow Pond Corporation";
      case MM_WINNOV: return "Winnov, LP";
      case MM_XEBEC: return "Xebec Multimedia Solutions Limited";
      case MM_YAMAHA: return "Yamaha Corporation of America";
      default:return unknownVendor;
    }
  }
#endif

};


#endif	/* UTIL_HPP */

