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
import MidiIO4Java.MidiSystemManager;
import MidiIO4Java.MidiSystemManager.Architecture;
import MidiIO4Java.StateException;
import MidiIO4Java.UnavailableException;
import java.io.IOException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ThreadFactory;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.ShortMessage;
import org.junit.After;
import org.junit.AfterClass;
import static org.junit.Assert.*;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class MidiJackNativeTest {

  final static String osName = System.getProperty("os.name").toLowerCase();

  class ProcessThreadFactory implements ThreadFactory {

    public Thread processThread = null;
    /**
     * This UncaughtExceptionHandler shall be attached to the process thread
     * thus we will be able to detect whether the process thread has thrown an
     * exception.
     */
    Thread.UncaughtExceptionHandler processThreadExceptionHandler =
            new Thread.UncaughtExceptionHandler() {
              @Override
              public void uncaughtException(Thread t, Throwable e) {
                exceptionInProcessTread = e;
              }
            };
    /**
     * This variable will receive the exception object, if an exception occurs
     * in the process thread.
     */
    public Throwable exceptionInProcessTread = null;

    public synchronized long getThreadId() {
      return processThread.getId();
    }

    @Override
    public synchronized Thread newThread(Runnable r) {
      assertNull("Testing problem: ProcessThreadFactory is not reentrant.", processThread);
      processThread = new Thread(r, "ProcessTread");
      processThread.setUncaughtExceptionHandler(processThreadExceptionHandler);

      return processThread;
    }
  }

  @BeforeClass
  public static void setUpClass() throws Exception {
  }

  @AfterClass
  public static void tearDownClass() throws Exception {
  }

  /**
   * setUp is run before each individual test.
   */
  @Before
  public void setUp() {
    // ignore tests for any OS other than Linux.
    boolean isLinux = (osName.indexOf("linux") >= 0);
    org.junit.Assume.assumeTrue(isLinux);
    if (!isLinux) {
      return;
    }
    // make sure the native library is loaded.
    // Please note, we assume that the JVM has been called with:
    // -Djava.library.path=./dist
    MidiSystemManager.loadNativeLibray(null);

    MidiJackNative instance = (MidiJackNative) MidiJackNative.getInstance();

    boolean available = instance.isAvailable();
    if (!available) {
      fail("jack support is not available (compile using WITH_JACK flag).");
    } else {
      //try to open and close the server, to make sure the server is available.
      try {
        if (!instance.isOpen()) {
          // the server remainded open (maybe because of a failed test), let's close it.
          instance.open("xxx");
        }
        assertTrue(instance.isOpen());
        instance.close();
      } catch (StateException | UnavailableException | ExecutionException ex) {
        String message = ex.getMessage() + " Is the jack server started?";
        fail(message);
      }
    }
    assertFalse(instance.isOpen()); // make sure every test case gets a closed MidiSystem

  }

  @After
  public void tearDown() {
  }

  /**
   * Test of getMidiInputPortCount method, of class MidiJackNative.
   */
  @Test
  @Ignore("Not implemeted yet")
  public void testGetMidiInputPortCount() {
    System.out.println("getMidiInputPortCount");

    MidiJackNative instance = (MidiJackNative) MidiJackNative.getInstance();
    int result = instance.getMidiInputPortCount();
    System.out.println("... InputPortCount=" + result);
    assertTrue(result >= 0);
    assertTrue(result < 100);// cannot imagine an installation with more than a hundred midi devices

  }

  /**
   * Test of getMidiPort method, of class MidiJackNative.
   */
  @Test
  @Ignore("Not implemeted yet")
  public void testGetMidiPort() {
    System.out.println("getMidiPort");

  }

  /**
   * Test of getMidiInputPortInfo method, of class MidiJackNative.
   */
  @Test
  @Ignore("Not implemeted yet")
  public void testGetMidiInputPortInfo() {
    System.out.println("getMidiInputPortInfo");

    MidiJackNative instance = (MidiJackNative) MidiJackNative.getInstance();
    int count = instance.getMidiInputPortCount();

    for (int i = 0; i < count; i++) {
      Info info = instance.getMidiInputPortInfo(i);
      assertNotNull(info);
      assertEquals(Architecture.JACK, info.getArchitecture());
      assertEquals(i, info.getIndex());
      assertTrue(info.isInput());
      assertNotNull(info.getName());
      assertNotNull(info.getVendor());
      assertNotNull(info.getDescription());
      assertNotNull(info.getVersion());
      System.out.println("..." + InfoToString(info));

    }
  }

  /**
   * Test of getMidiOutputPortInfo method, of class MidiJackNative.
   */
  @Test
  @Ignore("Not implemeted yet")
  public void testGetMidiOutputPortInfo() {
    System.out.println("getMidiOutputPortInfo");
    MidiJackNative instance = (MidiJackNative) MidiJackNative.getInstance();
    int count = instance.getMidiOutputPortCount();

    for (int i = 0; i < count; i++) {
      Info info = instance.getMidiOutputPortInfo(i);
      assertNotNull(info);
      assertEquals(Architecture.JACK, info.getArchitecture());
      assertEquals(i, info.getIndex());
      assertFalse(info.isInput());
      assertNotNull(info.getName());
      assertNotNull(info.getVendor());
      assertNotNull(info.getDescription());
      assertNotNull(info.getVersion());
      System.out.println("..." + InfoToString(info));


    }
  }

  /**
   * Test of getMidiOutputPortCount method, of class MidiJackNative.
   */
  @Test
  @Ignore("Not implemeted yet")
  public void testGetMidiOutputPortCount() {
    System.out.println("getMidiOutputPortCount");
    MidiJackNative instance = (MidiJackNative) MidiJackNative.getInstance();
    int result = instance.getMidiOutputPortCount();
    System.out.println("... OutputPortCount=" + result);
    assertTrue(result >= 0);
    assertTrue(result < 100);// cannot imagine an installation with more than a hundred midi devices

  }

  /**
   * Test of isAvailable method, of class MidiJackNative.
   */
  @Test
  @Ignore("Tested with setup")
  public void testIsAvailable() {
    System.out.println("isAvailable");

  }

  private String InfoToString(Info info) {
    String direction = "(Out)";
    if (info.isInput()) {
      direction = "(In)";
    }
    return String.format("%2d) %-5s %-25s %-35s %-25s %s",
            info.getIndex(),
            direction,
            info.getDescription(),
            info.getName(),
            info.getVendor(),
            info.getVersion());


  }

  /**
   * Test of createOutputPort method, of class MidiJackNative.
   */
  @Test
  @Ignore("see: testCreateOutputPort_onClosedSystem(),testCreateOutputPort_onSystemOpenedLater() etc.")
  public void testCreateOutputPort() {
    // This test was automatcally generated by the netbeans wizzard.
    // Alltought this test is obsolete because there are more specific tests, we leave it in. 
    // Thus we avoid the generation of a new prototype when the wizzard is run again.
  }

  /**
   * Test of createInputPort method, of class MidiJackNative.
   */
  @Test
  @Ignore("see: testCreateInputPort_onClosedSystem(),testCreateInputPort_onSystemOpenedLater() etc.")
  public void testCreateInputPort() throws Exception {
    // This test was automatcally generated by the netbeans wizzard.
    // Alltought this test is obsolete because there are more specific tests, we leave it in. 
    // Thus we avoid the generation of a new prototype when the wizzard is run again.
  }

  /**
   * Test of createInputPort method, of class MidiJackNative. Specification: It
   * must be possible to create and to close a portOut on a closed system. The
   * "onOpen" eventRecieved handler shall be called during creation, the
   * "onClose" eventRecieved handler shall be called during closing.
   */
  @Test
  public void testCreateInputPort_onClosedSystem() throws CreationException, ExecutionException {
    System.out.println("testCreateInputPort_onClosedSystem");
    InputPortListenerMock listener = new InputPortListenerMock();

    MidiSystem midiSystem = MidiJackNative.getInstance();
    assertFalse(midiSystem.isOpen());

    MidiPort port = midiSystem.createInputPort("IPortOnClosedSystem", listener);
    assertNotNull(port);

    assertFalse(port.isClosed());

    assertEquals(1, listener.onOpenCount);
    listener.onOpenCount = 0;
    assertEquals(0, listener.processCount);
    assertEquals(0, listener.onCloseCount);

    port.close();
    assertEquals(0, listener.onOpenCount);
    assertEquals(0, listener.processCount);
    assertEquals(1, listener.onCloseCount);
    assertTrue(port.isClosed());
  }

  /**
   * Test of createInputPort method, of class MidiJackNative. Specification: It
   * must be possible to create and to close a portOut on a closed system. The
   * "onOpen" eventRecieved handler shall be called during creation, the
   * "onClose" eventRecieved handler shall be called during closing.
   */
  @Test
  public void testCreateOutputPort_onClosedSystem() throws CreationException, ExecutionException {
    System.out.println("testCreateOutputPort_onClosedSystem");
    OutputPortListenerMock listener = new OutputPortListenerMock();

    MidiSystem midiSystem = MidiJackNative.getInstance();
    assertFalse(midiSystem.isOpen());

    MidiPort port = midiSystem.createOutputPort("OPortOnClosedSystem", listener);
    assertNotNull(port);

    assertFalse(port.isClosed());

    assertEquals(1, listener.onOpenCount);
    listener.onOpenCount = 0;
    assertEquals(0, listener.processCount);
    assertEquals(0, listener.onCloseCount);

    port.close();
    assertEquals(0, listener.onOpenCount);
    assertEquals(0, listener.processCount);
    assertEquals(1, listener.onCloseCount);
    assertTrue(port.isClosed());
  }

  @Test
  @Ignore("not implemetd")
  public void testCreateInputPort_onSystemOpenedLater() {
    System.out.println("testCreateInputPort_onSystemOpenedLater");
    fail("...Not yet implemented");
  }

  /**
   * Test of createInputPort method, of class MidiJackNative. Specification: It
   * must be possible to create a portOut on an open system. When portOut is
   * created the onOpen() callback of the PortListener shall be called. When
   * system is closed, the onClose() callback of the PortListener shall be
   * called.
   *
   */
  @Test
  public void testCreateInputPort_onOpenedSystem() throws CreationException, StateException, ExecutionException {
    System.out.println("testCreateInputPort_onOpenedSystem");
    InputPortListenerMock listener = new InputPortListenerMock();

    MidiSystem midiSystem = MidiJackNative.getInstance();
    midiSystem.open("testCreateInputPort");
    assertTrue(midiSystem.isOpen());

    MidiPort port = midiSystem.createInputPort("testPortOnOpenSystem", listener);
    assertNotNull(port);
    assertFalse(port.isClosed());
    assertEquals(1, listener.onOpenCount);
    listener.onOpenCount = 0;
    assertEquals(0, listener.processCount);
    assertEquals(0, listener.onCloseCount);

    midiSystem.close();
    assertTrue(port.isClosed());
    assertEquals(0, listener.onOpenCount);
    assertEquals(0, listener.processCount);
    assertEquals(1, listener.onCloseCount);

  }

  /**
   * Test of createOutputPort method, of class MidiJackNative. Specification: It
   * must be possible to create a portOut on an open system. When portOut is
   * created the onOpen() callback of the PortListener shall be called. When
   * system is closed, the onClose() callback of the PortListener shall be
   * called.
   *
   */
  @Test
  public void testCreateOutputPort_onOpenedSystem() throws CreationException, StateException, ExecutionException {
    System.out.println("testCreateOutputPort_onOpenedSystem");
    OutputPortListenerMock listener = new OutputPortListenerMock();

    MidiSystem midiSystem = MidiJackNative.getInstance();
    midiSystem.open("testCreateOutputPort");
    assertTrue(midiSystem.isOpen());

    MidiPort port = midiSystem.createOutputPort("testPortOnOpenSystem", listener);
    assertNotNull(port);
    assertFalse(port.isClosed());
    assertEquals(1, listener.onOpenCount);
    listener.onOpenCount = 0;
    assertEquals(0, listener.processCount);
    assertEquals(0, listener.onCloseCount);

    midiSystem.close();
    assertTrue(port.isClosed());
    assertEquals(0, listener.onOpenCount);
    assertEquals(0, listener.processCount);
    assertEquals(1, listener.onCloseCount);

  }

  /**
   * Test of run method, of class MidiJackNative.
   */
  @Test
  @Ignore("see: testRunEmpty(),testRun_WithInputPort() etc.")
  public void testRun() throws InterruptedException {
    // This test was automatcally generated by the netbeans wizzard.
    // Alltought this test is obsolete because of more specific tests, we leave it in 
    // to avoid the generation of a new prototype when the wizzard is run again.
  }

  /**
   * Test of run method, of class MidiJackNative on an empty system (a system
   * without ports). Specification: an open system is started by executing a new
   * thread (the process-thread) on the run() method. The process-thread will be
   * terminated by calling the close() method.
   *
   * @throws InterruptedException
   */
  @Test
  @SuppressWarnings({"CallToThreadDumpStack"})
  public void testRun_Empty() throws InterruptedException, StateException, ExecutionException {

    System.out.println("testRun_Empty");

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();
    midiSystem.open("testRun_Empty", systemListener, factory);
    assertTrue(midiSystem.isOpen());
    midiSystem.start();

    Thread.sleep(1000); // <- let it run for some cycles
    assertNotNull(factory.processThread);
    assertTrue(factory.processThread.isAlive());


    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertEquals(1, systemListener.onOpenCount);
    assertEquals(1, systemListener.onCloseCount);
    assertTrue(systemListener.onCycleStartCount > 5);
    assertEquals(systemListener.onCycleStartCount, systemListener.onCycleEndCount);
    assertEquals(1, systemListener.onCycleStart_lastCycleCount);
    assertEquals(1, systemListener.onCycleEnd_lastCycleCount);
    assertEquals(Thread.currentThread().getId(), systemListener.onOpenThreadId);
    assertEquals(Thread.currentThread().getId(), systemListener.onCloseThreadId);

    System.out.println("  Number of cycles:" + systemListener.onCycleStartCount);
    if (systemListener.xRunCount != 0) {
      System.out.println("  There were XRUNs:" + systemListener.xRunCount);
      System.out.println("  missed Cycles:" + systemListener.xRunCount2);
    }
  }

  @Test
  @SuppressWarnings({"CallToThreadDumpStack"})
  public void testRun_Empty2() throws StateException, ExecutionException {

    System.out.println("testRun_Empty2");

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();
    midiSystem.open("testRun_Empty2", systemListener, factory);
    assertTrue(midiSystem.isOpen());
    midiSystem.start();
    // close follows immediatly
    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertEquals(1, systemListener.onOpenCount);
    assertEquals(1, systemListener.onCloseCount);
    assertTrue(systemListener.onCycleStartCount >= 1);
    assertEquals(systemListener.onCycleStartCount, systemListener.onCycleEndCount);
    assertEquals(1, systemListener.onCycleStart_lastCycleCount);
    assertEquals(1, systemListener.onCycleEnd_lastCycleCount);
    assertEquals(Thread.currentThread().getId(), systemListener.onOpenThreadId);
    assertEquals(Thread.currentThread().getId(), systemListener.onCloseThreadId);

    System.out.println("  Number of cycles:" + systemListener.onCycleStartCount);
    if (systemListener.xRunCount != 0) {
      System.out.println("  There were XRUNs:" + systemListener.xRunCount);
      System.out.println("  missed Cycles:" + systemListener.xRunCount2);
    }
  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testRun_WithInputPort() throws InterruptedException, CreationException, ExecutionException {

    System.out.println("testRun_WithInputPort");
    InputPortListenerMock listener1 = new InputPortListenerMock();
    InputPortListenerMock listener2 = new InputPortListenerMock();

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testRun_WithInputPort", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createInputPort("testPort1_WithInputPort", listener1);


    midiSystem.start();
    Thread.sleep(1000);

    port1.close();
    MidiPort port2 = midiSystem.createInputPort("testPort2_WithInputPort", listener2);
    Thread.sleep(1000);

    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertTrue(port1.isClosed());
    assertEquals(1, listener1.onOpenCount);
    assertTrue(listener1.processCount > 2);
    assertEquals(1, listener1.onCloseCount);
    assertEquals(1, listener1.lastCycleCount);

    assertTrue(port2.isClosed());
    assertEquals(1, listener2.onOpenCount);
    assertTrue(listener2.processCount > 2);
    assertEquals(1, listener2.onCloseCount);
    assertEquals(1, listener2.lastCycleCount);

    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port 1 cycles:" + listener1.processCount);
    System.out.println("  Port 2 cycles:" + listener2.processCount);

  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testRun_WithOutputPort() throws InterruptedException, CreationException, ExecutionException {

    System.out.println("testRun_WithOutputPorts");
    OutputPortListenerMock listener1 = new OutputPortListenerMock();
    OutputPortListenerMock listener2 = new OutputPortListenerMock();

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testRun_WithOutputPort", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createOutputPort("testPort1_WithOutputPort", listener1);


    midiSystem.start();
    Thread.sleep(1000);

    port1.close();
    MidiPort port2 = midiSystem.createOutputPort("testPort2_WithOutputPort", listener2);
    Thread.sleep(1000);

    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertTrue(port1.isClosed());
    assertEquals(1, listener1.onOpenCount);
    assertTrue(listener1.processCount > 2);
    assertEquals(1, listener1.onCloseCount);
    assertEquals(1, listener1.lastCycleCount);

    assertTrue(port2.isClosed());
    assertEquals(1, listener2.onOpenCount);
    assertTrue(listener2.processCount > 2);
    assertEquals(1, listener2.onCloseCount);
    assertEquals(1, listener2.lastCycleCount);

    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port 1 cycles:" + listener1.processCount);
    System.out.println("  Port 2 cycles:" + listener2.processCount);

  }

  @Test
  @SuppressWarnings({"CallToThreadDumpStack", "SleepWhileInLoop"})
  public void testRun_InputOutput() throws InterruptedException, CreationException, IOException, InvalidMidiDataException, StateException, ExecutionException {
    MidiEvent event = new MidiEvent(
            new ShortMessage(ShortMessage.NOTE_ON, 0, 64, 64), 0);

    MidiEvent[] events = new MidiEvent[]{event};

    System.out.println("testRun_InpuOutput");

    OutputPortListenerMock outListener = new OutputPortListenerMock();
    outListener.eventsToSend = events;
    InputPortListenerMock inListener = new InputPortListenerMock();

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("PleaseConnectMe", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort portOut = midiSystem.createOutputPort("Out", outListener);
    MidiPort portIn = midiSystem.createInputPort("In", inListener);

    midiSystem.start();

    // ask the user to connect the ports, and wait until he acknowldges.
    System.out.println(" Connect \"Out\" port with \"In\" port.\n");

    int connectionsBefore = systemListener.onConnectionChangedCount;
    boolean userHasConnected = false;

    // wait at max 60 seconds for the user to connect the ports
    for (int i = 0; i < 600; i++) {
      if (systemListener.onConnectionChangedCount > connectionsBefore) {
        userHasConnected = true;
        break;
      }
      Thread.sleep(100);
      System.out.print(".");
    }

    if (userHasConnected) {

      // now the input-port is connected with the output-port, remember the current cycle cout
      int cyclesBefore = systemListener.onCycleStartCount;

      // let the input-port send events to the output port for 1000 millis
      Thread.sleep(1000);


      midiSystem.close();
      // calculate the number of cycles during which the input was connected with the output port.
      int connectedCycles = systemListener.onCycleStartCount - cyclesBefore;

      assertFalse(factory.processThread.isAlive());

      if (factory.exceptionInProcessTread != null) {
        factory.exceptionInProcessTread.printStackTrace();
        fail("Exception in process thread.");
      }

      assertTrue(portOut.isClosed());
      assertTrue(portIn.isClosed());
      assertEquals(1, outListener.onOpenCount);
      assertEquals(1, inListener.onOpenCount);

      assertEquals(systemListener.onCycleStartCount, outListener.processCount);
      assertEquals(systemListener.onCycleStartCount, inListener.processCount);
      // as long as input and output where connected, for every cycle
      // the input port should have received an event.
      // As the onConnectionChanged signal is not synchronised with
      // the onCycleStart signal, we must accept a small difference.
      assertTrue(connectedCycles - inListener.eventCount < 16);
      assertTrue(connectedCycles - inListener.eventCount > -16);


      assertEquals(1, outListener.onCloseCount);
      assertEquals(1, inListener.onCloseCount);
      assertNotNull(inListener.eventRecieved);

    } else {
      System.out.println(" --- Sorry could not perform this test.");
      midiSystem.close();

    }
  }

  /**
   * Test of the open and the close method, of class MidiJackNative.
   */
  @Test
  public void testOpenClose() throws StateException, UnavailableException, ExecutionException {
    System.out.println("testOpenClose");
    // The Midi System must allow to open and to close in fast succession without crashing.
    MidiSystem instance = MidiJackNative.getInstance();
    String clientName = "testOpenClose";
    for (int i = 0; i < 16; i++) {
      instance.open(clientName);
      assertTrue(instance.isOpen());
      instance.close();
      assertFalse(instance.isOpen());
    }
  }

  @Test(expected = IllegalArgumentException.class)
  public void testBadOpen() {
    System.out.println("testBadOpen");
    // The Midi System must not allow to open with a null clientName
    MidiSystem instance = MidiJackNative.getInstance();
    instance.open(null);
  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyRunningOutputPort1() throws InterruptedException, CreationException {

    System.out.println("testWithBadlyRunningOutputPort1");
    OutputPortListenerMock goodListner = new OutputPortListenerMock();
    BadOutputPortListenerMock badListener = new BadOutputPortListenerMock(false, false, true);

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testWithBadlyRunningOutputPort1", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createOutputPort("GoodPort", goodListner);


    midiSystem.start();
    Thread.sleep(100);

    MidiPort badPort = midiSystem.createOutputPort("BadPort", badListener);
    Thread.sleep(1000);

    boolean problemDetected = false;
    try {
      midiSystem.close();
    } catch (ExecutionException ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }
    assertTrue(problemDetected);
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertTrue(port1.isClosed());
    assertEquals(1, goodListner.onOpenCount);
    assertTrue(goodListner.processCount > 2);
    assertEquals(1, goodListner.onCloseCount);
    assertEquals(1, goodListner.lastCycleCount);

    assertTrue(badPort.isClosed());
    assertTrue(badListener.processCount > 2);


    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port (good) 1 cycles:" + goodListner.processCount);
    System.out.println("  Port (bad) 2 cycles:" + badListener.processCount);

  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyRunningOutputPort2() throws InterruptedException, CreationException, StateException, ExecutionException {

    System.out.println("testWithBadlyRunningOutputPort2");
    OutputPortListenerMock goodListner = new OutputPortListenerMock();
    BadOutputPortListenerMock badListener = new BadOutputPortListenerMock(false, false, true);

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testWithBadlyRunningOutputPort2", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createOutputPort("GoodPort", goodListner);
    MidiPort badPort = midiSystem.createOutputPort("BadPort", badListener);

    midiSystem.start();
    Thread.sleep(100);


    boolean problemDetected = false;
    try {
      badPort.close();
    } catch (ExecutionException ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }
    assertTrue(problemDetected);

    Thread.sleep(1000);


    // closes normally, by closing the bad port we have cleared the exception
    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertTrue(port1.isClosed());
    assertEquals(1, goodListner.onOpenCount);
    assertTrue(goodListner.processCount > 2);
    assertEquals(1, goodListner.onCloseCount);
    assertEquals(1, goodListner.lastCycleCount);

    assertTrue(badPort.isClosed());
    assertTrue(badListener.processCount > 2);


    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port (good)  cycles:" + goodListner.processCount);
    System.out.println("  Port (bad)  cycles:" + badListener.processCount);

  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyOpeningOutputPort1() throws InterruptedException, CreationException, StateException, ExecutionException {

    System.out.println("testWithBadlyOpeningOutputPort");
    OutputPortListenerMock goodListner = new OutputPortListenerMock();
    BadOutputPortListenerMock badListener = new BadOutputPortListenerMock(false, true, false);//bad open

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testWithBadlyOpeningOutputPort", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createOutputPort("GoodOutputPort", goodListner);


    midiSystem.start();
    Thread.sleep(100);


    MidiPort badPort = null;
    boolean problemDetected = false;
    try {
      badPort = midiSystem.createOutputPort("BadOutputPort", badListener);
    } catch (Throwable ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }
    assertTrue(problemDetected);

    Thread.sleep(1000);


    // should close normally, the bad port has never been insered
    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertTrue(port1.isClosed());
    assertEquals(1, goodListner.onOpenCount);
    assertTrue(goodListner.processCount > 2);
    assertEquals(1, goodListner.onCloseCount);
    assertEquals(1, goodListner.lastCycleCount);

    assertNull(badPort);
    assertEquals(0, badListener.processCount);


    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port (good) 1 cycles:" + goodListner.processCount);
    System.out.println("  Port (bad) 2 cycles:" + badListener.processCount);

  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyOpeningOutputPort2() throws InterruptedException, CreationException, StateException, ExecutionException {

    System.out.println("testWithBadlyOpeningOutputPort2");

    BadOutputPortListenerMock badListener = new BadOutputPortListenerMock(false, true, false);//bad open

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    boolean problemDetected = false;
    try {
      MidiPort badPort = midiSystem.createOutputPort("BadOutputPort", badListener);
    } catch (Throwable ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }
    midiSystem.open("testWithBadlyOpeningOutputPort2", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    midiSystem.close();
  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyRunningInputPort1() throws InterruptedException, CreationException {

    System.out.println("testWithBadlyRunningInputPort1");
    InputPortListenerMock goodListner = new InputPortListenerMock();
    BadInputPortListenerMock badListener = new BadInputPortListenerMock(false, false, true);

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testWithBadlyRunningInputPort1", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createInputPort("GoodPort", goodListner);


    midiSystem.start();
    Thread.sleep(100);

    MidiPort badPort = midiSystem.createInputPort("BadPort", badListener);
    Thread.sleep(1000);

    boolean problemDetected = false;
    try {
      midiSystem.close();
    } catch (ExecutionException ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }
    assertTrue(problemDetected);
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertTrue(port1.isClosed());
    assertEquals(1, goodListner.onOpenCount);
    assertTrue(goodListner.processCount > 2);
    assertEquals(1, goodListner.onCloseCount);
    assertEquals(1, goodListner.lastCycleCount);

    assertTrue(badPort.isClosed());
    assertTrue(badListener.processCount > 2);


    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port (good) 1 cycles:" + goodListner.processCount);
    System.out.println("  Port (bad) 2 cycles:" + badListener.processCount);

  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyOpeningInputPort() throws InterruptedException, CreationException, StateException, ExecutionException {

    System.out.println("testWithBadlyOpeningInputPort");
    InputPortListenerMock goodListner = new InputPortListenerMock();
    BadInputPortListenerMock badListener = new BadInputPortListenerMock(false, true, false);//bad open

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testWithBadlyOpeningInputPort", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createInputPort("GoodPort", goodListner);


    midiSystem.start();
    Thread.sleep(100);


    MidiPort badPort = null;
    boolean problemDetected = false;
    try {
      badPort = midiSystem.createInputPort("BadInputPort", badListener);
    } catch (Throwable ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }
    assertTrue(problemDetected);

    Thread.sleep(1000);


    // should close normally, the bad port has never been insered
    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }

    assertTrue(port1.isClosed());
    assertEquals(1, goodListner.onOpenCount);
    assertTrue(goodListner.processCount > 2);
    assertEquals(1, goodListner.onCloseCount);
    assertEquals(1, goodListner.lastCycleCount);

    assertNull(badPort);
    assertEquals(0, badListener.processCount);


    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port (good)  cycles:" + goodListner.processCount);
    System.out.println("  Port (bad)  cycles:" + badListener.processCount);

  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyClosingInputPort1() throws InterruptedException, CreationException, StateException, ExecutionException {

    System.out.println("testWithBadlyClosingInputPort");
    InputPortListenerMock goodListener = new InputPortListenerMock();
    BadInputPortListenerMock badListener = new BadInputPortListenerMock(true, false, false);//bad close

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testWithBadlyClosingInputPort", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createInputPort("GoodPort", goodListener);


    midiSystem.start();
    Thread.sleep(100);

    MidiPort badPort = midiSystem.createInputPort("BadInputPort", badListener);
    Thread.sleep(100);

    boolean problemDetected = false;
    try {
      badPort.close();
    } catch (Throwable ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }
    assertTrue(problemDetected);

    Thread.sleep(100);


    // should close normally, the bad port has nevertheless been closed
    midiSystem.close();
    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }
    assertNotNull(badPort);

    assertTrue(port1.isClosed());
    assertTrue(badPort.isClosed());
    assertEquals(1, goodListener.onOpenCount);
    assertTrue(goodListener.processCount > 2);
    assertTrue(badListener.processCount > 2);
    assertEquals(1, goodListener.onCloseCount);
    assertEquals(1, goodListener.lastCycleCount);


    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port (good)  cycles:" + goodListener.processCount);
    System.out.println("  Port (bad)  cycles:" + badListener.processCount);

  }

  @Test
  @SuppressWarnings("CallToThreadDumpStack")
  public void testWithBadlyClosingInputPort2() throws InterruptedException, CreationException, StateException, ExecutionException {

    System.out.println("testWithBadlyClosingInputPort2");
    InputPortListenerMock goodListener = new InputPortListenerMock();
    BadInputPortListenerMock badListener = new BadInputPortListenerMock(true, false, false);//bad close

    MidiSystem midiSystem = MidiJackNative.getInstance();
    SystemListenerMock systemListener = new SystemListenerMock();
    ProcessThreadFactory factory = new ProcessThreadFactory();


    midiSystem.open("testWithBadlyClosingInputPort2", systemListener, factory);
    assertTrue(midiSystem.isOpen());

    MidiPort port1 = midiSystem.createInputPort("GoodPort", goodListener);


    midiSystem.start();
    Thread.sleep(100);

    MidiPort badPort = midiSystem.createInputPort("BadInputPort", badListener);


    Thread.sleep(100);


    boolean problemDetected = false;
    try {
      midiSystem.close();
    } catch (StateException | ExecutionException ex) {
      problemDetected = true;
      System.out.println("ExecutionException correctly detected, stack trace follows:");
      ex.printStackTrace(System.out);
    }

    assertFalse(factory.processThread.isAlive());

    if (factory.exceptionInProcessTread != null) {
      factory.exceptionInProcessTread.printStackTrace();
      fail("Exception in process thread.");
    }
    assertNotNull(badPort);

    assertTrue(port1.isClosed());
    assertTrue(badPort.isClosed());
    assertEquals(1, goodListener.onOpenCount);
    assertTrue(goodListener.processCount > 2);
    assertTrue(badListener.processCount > 2);
    assertEquals(1, goodListener.onCloseCount);
    assertEquals(1, goodListener.lastCycleCount);


    System.out.println("  Number of total cycles:" + systemListener.onCycleStartCount);
    System.out.println("  Port (good)  cycles:" + goodListener.processCount);
    System.out.println("  Port (bad)  cycles:" + badListener.processCount);

  }

  /**
   * Test of close method, of class MidiJackNative.
   */
  @Test
  @Ignore("Tested with testOpenClose()")
  public void testClose() {
    System.out.println("close");
  }

  /**
   * Test of open method, of class MidiJackNative.
   */
  @Test
  @Ignore("Tested with testOpenClose()")
  public void testOpen() {
    System.out.println("open");
  }

  /**
   * Test of getInstance method, of class MidiJackNative.
   */
  @Test
  @Ignore("Tested with setUp()")
  public void testGetInstance() {
    System.out.println("getInstance");
  }

  /**
   * Test of isOpen method, of class MidiJackNative.
   */
  @Test
  @Ignore("Tested with setUp()")
  public void testIsOpen() {
    System.out.println("isOpen");
  }

  class InputPortListenerMock implements MidiInputPortListener {

    public int processCount = 0;
    public int onCloseCount = 0;
    public int onOpenCount = 0;
    public Throwable error = null;
    public MidiEvent eventRecieved = null;
    public int eventCount = 0;
    public int lastCycleCount = 0;

    @Override
    public void process(long timeCodeStart,
            long timeCodeDuration,
            MidiEvent[] events,
            boolean lastCycle) throws Throwable {

      processCount++;
      if (events == null) {
        error = new RuntimeException("InputPortListenerMock: null events");
        return;
      }
      if (events.length > 0) {
        eventCount = eventCount + events.length;
        eventRecieved = events[0];
      }
      if (lastCycle) {
        lastCycleCount++;
      }
    }

    @Override
    public void onClose() {
      onCloseCount++;
    }

    @Override
    public void onOpen() {
      onOpenCount++;
    }
  }

  class OutputPortListenerMock implements MidiOutputPortListener {

    public int processCount = 0;
    public int onCloseCount = 0;
    public int onOpenCount = 0;
    public MidiEvent[] eventsToSend = null;
    public int lastCycleCount = 0;

    @Override
    public void onClose() {
      onCloseCount++;
    }

    @Override
    public void onOpen() {
      onOpenCount++;
    }

    @Override
    public MidiEvent[] process(long timeCodeStart,
            long timeCodeDuration,
            boolean lastCycle) throws Throwable {
      processCount++;
      if (lastCycle) {
        lastCycleCount++;
      }
      return eventsToSend;
    }
  }

  class BadOutputPortListenerMock implements MidiOutputPortListener {

    public int processCount = 0;
    private final boolean badClose;
    private final boolean badOpen;
    private final boolean badProcess;

    public BadOutputPortListenerMock(boolean badClose, boolean badOpen, boolean badProcess) {
      this.badClose = badClose;
      this.badOpen = badOpen;
      this.badProcess = badProcess;
    }

    @Override
    public void onClose() {
      if (badClose) {
        throw new RuntimeException("badClosing, boom...");
      }
    }

    @Override
    public void onOpen() {
      if (badOpen) {
        throw new RuntimeException("badOpening, boom...");
      }
    }

    @Override
    public MidiEvent[] process(long timeCodeStart,
            long timeCodeDuration,
            boolean lastCycle) throws Throwable {
      processCount++;
      if (processCount > 8) {
        if (badProcess) {
          throw new RuntimeException("badProcess, boom...");
        }
      }
      return null;

    }
  }

  class BadInputPortListenerMock implements MidiInputPortListener {

    public int processCount = 0;
    private final boolean badClose;
    private final boolean badOpen;
    private final boolean badProcess;

    public BadInputPortListenerMock(boolean badClose, boolean badOpen, boolean badProcess) {
      this.badClose = badClose;
      this.badOpen = badOpen;
      this.badProcess = badProcess;
    }

    @Override
    public void onClose() {
      if (badClose) {
        throw new RuntimeException("badClosing, boom...");
      }
    }

    @Override
    public void onOpen() {
      if (badOpen) {
        throw new RuntimeException("badOpening, boom...");
      }
    }

    @Override
    public void process(long timeCodeStart,
            long timeCodeDuration,
            MidiEvent[] events,
            boolean lastCycle) throws Throwable {

      processCount++;
      if (processCount > 8) {
        if (badProcess) {
          throw new RuntimeException("badProcess, boom...");
        }
      }
    }
  }

  class SystemListenerMock implements MidiSystemListener {

    public int onCycleStartCount = 0;
    public int onCycleEndCount = 0;
    public int onConnectionChangedCount = 0;
    public int onCloseCount = 0;
    public int onOpenCount = 0;
    public int onCycleStart_lastCycleCount = 0;
    public int onCycleEnd_lastCycleCount = 0;
    private long currentTimecode = -1;
    private long currentDuration = -1;
    private long prococessThreadId;
    public long onOpenThreadId;
    public long onCloseThreadId;
    public int xRunCount = 0;
    public int xRunCount2 = 0;

    @Override
    public void onCycleStart(long timeCodeStart, long timeCodeDuration, boolean lastCycle) throws Throwable {
      onCycleStartCount++;
      if (lastCycle) {
        assertEquals(0, onCycleStart_lastCycleCount);
        onCycleStart_lastCycleCount++;
      }
      if (currentTimecode != -1) {
        long diff = timeCodeStart - currentTimecode;
        assertTrue("Timecode is not monotonic, diff=" + diff, diff >= 0);
        assertEquals(0, diff % timeCodeDuration);
        if (diff != timeCodeDuration) {
          xRunCount++;
          if (diff > 0) {
            xRunCount2++;
          }
        }
        assertEquals(prococessThreadId, Thread.currentThread().getId());
      }
      currentTimecode = timeCodeStart;
      prococessThreadId = Thread.currentThread().getId();
      currentDuration = timeCodeDuration;
    }

    @Override
    public void onCycleEnd(long timeCodeStart, long timeCodeDuration, boolean lastCycle) throws Throwable {
      onCycleEndCount++;
      if (lastCycle) {
        assertEquals(0, onCycleEnd_lastCycleCount);
        onCycleEnd_lastCycleCount++;
      }
      assertEquals(currentTimecode, timeCodeStart);
      assertEquals(prococessThreadId, Thread.currentThread().getId());
      assertEquals(currentDuration, timeCodeDuration);
    }

    @Override
    public void onConnectionChanged() throws Throwable {
      onConnectionChangedCount++;
    }

    @Override
    public void onClose() throws Throwable {
      onCloseCount++;
      onCloseThreadId = Thread.currentThread().getId();
    }

    @Override
    public void onOpen() throws Throwable {
      onOpenCount++;
      onOpenThreadId = Thread.currentThread().getId();
    }
  }
}
