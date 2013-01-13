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

import MidiIO4Java.CreationException;
import MidiIO4Java.MidiInputPortListener;
import MidiIO4Java.MidiOutputPortListener;
import MidiIO4Java.MidiPort;
import MidiIO4Java.MidiPort.Info;
import MidiIO4Java.MidiSystem;
import MidiIO4Java.MidiSystemListener;
import MidiIO4Java.MidiSystemManager.Architecture;
import MidiIO4Java.StateException;
import MidiIO4Java.UnavailableException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.ShortMessage;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class MidiJackNative implements MidiSystem {

  /**
   * error code of native functions when the function succeeded.
   */
  static final int noError = 0;
  static final int errorAlreadyOpen = -1;
  static final int errorNotOpen = -2;
  static final int errorConnectionFailed = -3;
  static final int errorClosingPort = -4;
  private static final Architecture thisArchitecture = Architecture.JACK;
  private static final Object openCloseLock = new Object();
  private ThreadFactory processThreadFactory = Executors.defaultThreadFactory();
  /**
   * Every port will get its own internal identifier. This variable stores the
   * identifier to be used for the next new port and must be incremented each
   * time a port is created. Identifiers of deleted ports shall not be reused.
   */
  private static long newPortID = 0;

  private static native boolean _isAvailable();

  private static native boolean _isOpen();

  private static native int _createInputPort(long portID, InfoImpl emptyTemplate, String name, MidiInputPort port);

  private static native int _createOutputPort(long portID, InfoImpl emptyTemplate, String name, MidiOutputPort port);

  private static native Info _getMidiInputPortInfo(int index, InfoImpl emptyTemplate);

  private static native Info _getMidiOutputPortInfo(int index, InfoImpl emptyTemplate);

  private static native int _getMidiInputPortCount();

  private static native int _getMidiOutputPortCount();

  private static native int _open(String clientName, MidiSystemListener listener);

  private static native void _run();

  /**
   * Indicates whether the portchain is processing native callbacks. If the
   * portchain is about to start, the calling thread will be blocked until the
   * portchain has reached a definitive state.
   *
   * @return
   */
  private static native void _waitForCycleDone();

  /**
   * Native implementation of close(). See: "jackNative.cpp"
   *
   * @return one of the above defined error codes.
   */
  private static native int _close();

  /**
   * Close a Port. It is assumed that the given portId belongs to a port hooked
   * into the current portchain. The given portId is searched in the portchain.
   * The peer class is removed from the portchain than the peer is closed and
   * destroyed.
   *
   * Implemented in: jackNative.cpp
   *
   * @param env pointer to calling the Java thread.
   * @param internalPortId the internal identifier of the port
   * @return 0 if the port could successfully be closed; a negative error code
   * if something went wrong.
   */
  private static native int _closePort(long portId);

  private static native boolean _isClosedPort(long portId);
  static private MidiJackNative instance = new MidiJackNative();
  private boolean isRunnable = false;

  private MidiJackNative() {
  }

  static public MidiJackNative getInstance() {
    return instance;
  }

  public Info getMidiInputPortInfo(int index) {
    assumeOpen();
    InfoImpl emptyTemplate = new InfoImpl(thisArchitecture);
    return _getMidiInputPortInfo(index, emptyTemplate);
  }

  public Info getMidiOutputPortInfo(int index) {
    assumeOpen();
    InfoImpl emptyTemplate = new InfoImpl(thisArchitecture);
    return _getMidiOutputPortInfo(index, emptyTemplate);
  }

  public int getMidiInputPortCount() {
    assumeOpen();
    return _getMidiInputPortCount();
  }

  public int getMidiOutputPortCount() {
    assumeOpen();
    return _getMidiOutputPortCount();
  }

  /**
   * Currently this function returns true if the native library has been
   * compiled with Jack support.
   */
  @Override
  public boolean isAvailable() {
    return _isAvailable();
  }

  private void assumeAvailable() {
    if (!isAvailable()) {
      throw new UnavailableException("Jack Audio is not available");
    }
  }

  private void assumeOpen() {
    assumeAvailable();
    if (!isOpen()) {
      throw new StateException("Jack Audio is not open");
    }
  }

  @Override
  public boolean isOpen() {
    synchronized (openCloseLock) {
      return _isOpen();
    }
  }

  /**
   * Creates a new output-port for this client. An output-port is an object used
   * for moving Midi-data out of this client and make it available to other
   * applications.
   *
   * @param name non-empty short name for the new port (not including the
   * leading "client_name:"). Must be unique among all ports owned by this
   * client.
   * @param listener the listener will receive call-backs when the system is
   * ready to accept the next buffer of Midi Data.
   * @return the newly created port.
   * @throws CreationException if the creation fails.
   */
  @Override
  public MidiPort createOutputPort(String name, MidiOutputPortListener listener)
          throws CreationException {
    if (listener == null) {
      throw new IllegalArgumentException("listner shall not be null.");
    }
    if (name == null) {
      throw new IllegalArgumentException("name shall not be null.");
    }
    synchronized (openCloseLock) {
      assumeAvailable();
      InfoImpl template = new InfoImpl(thisArchitecture);
      long thisPortID = newPortID;
      newPortID++;
      MidiOutputPort port = new MidiOutputPort(thisPortID, listener, template, name);
      int err = _createOutputPort(thisPortID, template, name, port);
      if (err < 0) {
        throw new RuntimeException("Error(" + err + ") while creating an OutputPort.");
      }
      return port;
    }
  }

  /**
   * Creates a new input-port for this client. An input-port is an object used
   * for moving Midi-data from other applications into client.
   *
   * @param name non-empty short name for the new port (not including the
   * leading "client_name:"). Must be unique among all ports owned by this
   * client.
   * @param listener the listener will receive call-backs when the system has
   * new Midi Data.
   * @return the newly created port.
   * @throws CreationException if the creation fails.
   */
  @Override
  public MidiPort createInputPort(String name, MidiInputPortListener listener)
          throws CreationException {
    if (listener == null) {
      throw new IllegalArgumentException("listner shall not be null.");
    }
    if (name == null) {
      throw new IllegalArgumentException("name shall not be null.");
    }
    synchronized (openCloseLock) {
      assumeAvailable();
      InfoImpl template = new InfoImpl(thisArchitecture);
      long thisPortID = newPortID;
      newPortID++;
      MidiInputPort port = new MidiInputPort(thisPortID, listener, template, name);
      int err = _createInputPort(thisPortID, template, name, port);
      if (err < 0) {
        throw new RuntimeException("Error(" + err + ") while creating an InputPort.");
      }
      return port;
    }
  }

  @Override
  public void close() throws ExecutionException {
    synchronized (openCloseLock) {
      isRunnable = false;
      assumeAvailable();
      int error = noError;
      try {
        error = _close();
      } catch (Throwable th) {
        throw new ExecutionException("Error in process thread", th);
      }
      switch (error) {
        case noError:
          return;
        case errorNotOpen:
          throw new StateException("Attempt to close the Jack-Audio Server twice, but was not open.");
        default:
          throw new RuntimeException("Error(" + error + ") while disconnecting from the Jack-Audio Server.");
      }
    }
  }

  @Override
  public void open(String clientName, MidiSystemListener listener, ThreadFactory processThreadFactory) throws StateException, UnavailableException {
    if (clientName == null) {
      throw new IllegalArgumentException("clientName may not be null.");
    }
    if (processThreadFactory == null) {
      throw new NullPointerException();
    }
    synchronized (openCloseLock) {
      assumeAvailable();
      this.processThreadFactory = processThreadFactory;
      int error = _open(clientName, listener);
      switch (error) {
        case noError:
          isRunnable = true;
          return;
        case errorAlreadyOpen:
          throw new StateException("Attempt to open the Jack-Audio Server twice.");
        case errorConnectionFailed:
          throw new RuntimeException("Could not connect to the Jack-Audio Server.");
        default:
          throw new RuntimeException("Could not open the Jack-Audio Server (" + error + ")");
      }
    }
  }

  @Override
  public void open(String clientName, MidiSystemListener listener) throws StateException, UnavailableException {
    ThreadFactory newThreadFactory = Executors.defaultThreadFactory();
    open(clientName, listener, newThreadFactory);
  }

  @Override
  public void open(String clientName) throws StateException, UnavailableException {
    MidiSystemListener dummyListener = new MidiSystemListener() {
      @Override
      public void onConnectionChanged() {
      }

      @Override
      public void onClose() {
      }

      @Override
      public void onOpen() throws Throwable {
      }

      @Override
      public void onCycleStart(long timeCodeStart, long timeCodeDuration, boolean lastCycle) throws Throwable {
      }

      @Override
      public void onCycleEnd(long timeCodeStart, long timeCodeDuration, boolean lastCycle) throws Throwable {
      }
    };
    open(clientName, dummyListener);
  }

  @Override
  public void start() {
    synchronized (openCloseLock) {
      if (!isRunnable) {
        throw new StateException("Cannot run, system is not runnable.");
      }
      Thread processThread = processThreadFactory.newThread(
              new Runnable() {
                @Override
                public void run() {
                  _run();
                }
              });
      processThread.start();
      _waitForCycleDone();
    }
  }

  private static class MidiInputPort implements MidiPort {

    final long portId;
    final InfoImpl info;
    final String name;
    final MidiInputPortListener listener;

    protected MidiInputPort(long portId, MidiInputPortListener listener, InfoImpl info, String name) {
      this.listener = listener;
      this.portId = portId;
      this.info = info;
      this.name = name;
    }

    @Override
    public Info getPortInfo() {
      return info;
    }

    @Override
    public void close() throws ExecutionException {
      int error = noError;
      try {
        error = _closePort(portId);
      } catch (Throwable th) {
        throw new ExecutionException("Error in process thread", th);
      }
      switch (error) {
        case noError:
          return;
        case errorClosingPort:
          throw new RuntimeException("Could not close port \"" + name + "\".");
        default:
          throw new RuntimeException("Error (" + error + ") while closing port \"" + name + "\".");
      }
    }

    @Override
    public boolean isClosed() {
      return _isClosedPort(portId);
    }

    /**
     * Process callback of an input port.
     *
     * @param timeCodeStart
     * @param timeCodeDuration
     * @param lastCycle
     * @param rawEvents
     * @param deltaTimes
     * @throws Throwable
     */
    public void process(long timeCodeStart,
            long timeCodeDuration,
            boolean lastCycle,
            int[] rawEvents,
            int[] deltaTimes)
            throws Throwable {

      //Signature: (JJZ[I[I)V


      int eventCount = deltaTimes.length;
      if (eventCount * 3 != rawEvents.length) {
        throw new RuntimeException("Array length missmatch.");
      }
      MidiEvent[] events = new MidiEvent[eventCount];
      for (int i = 0; i < eventCount; i++) {
        ShortMessage message = new ShortMessage();
        int rawIdx = 3 * i;
        message.setMessage(
                rawEvents[rawIdx], // status
                rawEvents[rawIdx + 1], // data1
                rawEvents[rawIdx + 2]); // data2

        MidiEvent event = new MidiEvent(message, deltaTimes[i]);
        events[i] = event;
      }
      listener.process(timeCodeStart, timeCodeDuration, events, lastCycle);
    }

    // Signature: ()V
    public void onClose() throws Throwable {
      listener.onClose();
    }

    // Signature: ()V
    public void onOpen() throws Throwable {
      listener.onOpen();
    }
  }

  private static class MidiOutputPort implements MidiPort {

    final long portId;
    final InfoImpl info;
    final String name;
    final MidiOutputPortListener listener;

    protected MidiOutputPort(long portId, MidiOutputPortListener listener, InfoImpl info, String name) {
      this.listener = listener;
      this.portId = portId;
      this.info = info;
      this.name = name;
    }

    @Override
    public Info getPortInfo() {
      return info;
    }

    @Override
    public void close() throws ExecutionException {
      int error = noError;
      try {
        error = _closePort(portId);
      } catch (Throwable th) {
        throw new ExecutionException("Error in process thread", th);
      }
      switch (error) {
        case noError:
          return;
        case errorClosingPort:
          throw new RuntimeException("Could not close port \"" + name + "\".");
        default:
          throw new RuntimeException("Error (" + error + ") while closing port \"" + name + "\".");
      }
    }

    @Override
    public boolean isClosed() {
      return _isClosedPort(portId);
    }

    /**
     * Process callback of an output port.
     *
     * @param timeCodeStart the time-tick at the start of this cycle
     * @param timeCodeDuration the number of time-ticks in this process-cycle
     * (the value is equal to the number of audio-frames per period).
     * @param rawEventsOut an array that should be filled with midi data.
     * @param deltaTimesOut an array that should be filled with the delta times
     * corresponding to the rawEvents.
     * @param eventSizeOut an array that should be filled with the sizes of the
     * rawEvents.
     * @return the number of events generated in this cycle
     * @throws Throwable
     */
    public int process(long timeCodeStart,
            long timeCodeDuration,
            boolean lastCycle,
            int[] rawEventsOut,
            int[] deltaTimesOut,
            int[] eventSizeOut)
            throws Throwable {
      // Signature: (JJZ[I[I[I)I

      int maxEvents = eventSizeOut.length;

      if (deltaTimesOut.length != maxEvents) {
        throw new IllegalArgumentException("Array length missmatch.");
      }
      if (rawEventsOut.length != 3 * maxEvents) {
        throw new IllegalArgumentException("Array length missmatch.");
      }

      // ask the listener to produce new events
      MidiEvent[] midiEvents = listener.process(timeCodeStart, timeCodeDuration, lastCycle);

      // transfer the event-data into the output arrays
      int eventCount = 0;
      if (midiEvents != null) {
        for (MidiEvent event : midiEvents) {
          // We are currently only treating short messages
          if (event.getMessage() instanceof ShortMessage) {
            if (eventCount >= maxEvents) {
              throw new RuntimeException("Too many midi events, array overflow.");
            }
            // handle delta time
            int deltaTime = (int) event.getTick();
            if (deltaTime < 0) {
              throw new IllegalArgumentException("Negative delta-time " + deltaTime + " in " + event);
            }
            if (deltaTime >= timeCodeDuration) {
              throw new IllegalArgumentException("Invalid delta-time " + deltaTime + " in " + event);
            }
            deltaTimesOut[eventCount] = deltaTime;

            // Handle the message
            ShortMessage message = (ShortMessage) event.getMessage();
            eventSizeOut[eventCount] = message.getLength();

            int rawEventBase = 3 * eventCount;
            rawEventsOut[rawEventBase] = message.getStatus();
            rawEventsOut[rawEventBase + 1] = message.getData1();
            rawEventsOut[rawEventBase + 2] = message.getData2();

            // increment event count
            eventCount++;
          }
        }
      }
      return eventCount;
    }

    // Signature: ()V
    public void onClose() throws Throwable {
      listener.onClose();
    }

    // Signature: ()V
    public void onOpen() throws Throwable {
      listener.onOpen();
    }
  }
}
