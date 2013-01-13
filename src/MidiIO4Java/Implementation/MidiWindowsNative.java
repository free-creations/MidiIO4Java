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
package MidiIO4Java.Implementation;

import MidiIO4Java.MidiInputPortListener;
import MidiIO4Java.MidiOutputPortListener;
import MidiIO4Java.MidiPort;
import MidiIO4Java.MidiPort.Info;
import MidiIO4Java.MidiSystem;
import MidiIO4Java.MidiSystemListener;
import MidiIO4Java.MidiSystemManager.Architecture;
import MidiIO4Java.StateException;
import MidiIO4Java.UnavailableException;
import java.util.concurrent.ThreadFactory;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class MidiWindowsNative implements MidiSystem {

  private static final Architecture thisArchitecture = Architecture.WINDOWS_MM;

  private static native Info _getMidiInputPortInfo(int index, InfoImpl emptyTemplate);

  private static native Info _getMidiOutputPortInfo(int index, InfoImpl emptyTemplate);

  private static native int _getMidiInputPortCount();

  private static native int _getMidiOutputPortCount();

  private static native boolean _isAvailable();  
  
  private static native boolean _isOpen();
  /**
   * the one and only instance is constructed at load time 
   * (to my knowledge this is thread save).
   */
  static private MidiWindowsNative instance = new MidiWindowsNative();

  /**
   * Constructor is private (singleton pattern)
   */
  private MidiWindowsNative() {
  }

  static public MidiWindowsNative getInstance() {
    return instance;
  }

  public Info getMidiInputPortInfo(int index) {
    InfoImpl emptyTemplate = new InfoImpl(thisArchitecture);
    return _getMidiInputPortInfo(index, emptyTemplate);
  }

  public Info getMidiOutputPortInfo(int index) {
    InfoImpl emptyTemplate = new InfoImpl(thisArchitecture);
    return _getMidiOutputPortInfo(index, emptyTemplate);
  }

  public int getMidiInputPortCount() {
    return _getMidiInputPortCount();
  }

  public int getMidiOutputPortCount() {
    return _getMidiOutputPortCount();
  }

  @Override
  public boolean isAvailable() {
    return _isAvailable();
  }

  @Override
  public MidiPort createOutputPort(String name, MidiOutputPortListener listener) {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public MidiPort createInputPort(String name, MidiInputPortListener listener) {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public void close() {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public void open(String clientName) {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public boolean isOpen() {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public void open(String clientName, MidiSystemListener listener) throws StateException, UnavailableException {
    throw new UnsupportedOperationException("Not supported yet.");
  }


  @Override
  public void start()  {
    throw new UnsupportedOperationException("Not supported yet.");
  }

  @Override
  public void open(String clientName, MidiSystemListener listener, ThreadFactory processThreadFactory) throws StateException, UnavailableException {
    throw new UnsupportedOperationException("Not supported yet.");
  }
}
