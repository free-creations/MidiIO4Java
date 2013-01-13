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

import MidiIO4Java.MidiPort.Info;
import MidiIO4Java.MidiSystemManager;
import MidiIO4Java.MidiSystemManager.Architecture;
import static org.junit.Assert.*;
import org.junit.*;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class MidiWindowsNativeTest {

  final static String osName = System.getProperty("os.name").toLowerCase();

  public MidiWindowsNativeTest() {
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
    // ignore tests for any OS other than MS-Windows.
    org.junit.Assume.assumeTrue(osName.indexOf("windows") >= 0);
    // make sure the native library is loaded.
    // Please note, we assume that the JVM has been called with:
    // -Djava.library.path=./dist
    MidiSystemManager.loadNativeLibray(null);
  }

  @After
  public void tearDown() {
  }

  /**
   * Test of getMidiInputPortCount method, of class MidiWindowsNative.
   */
  @Test
  public void testGetMidiInputPortCount() {
    System.out.println("getMidiInputPortCount");

    MidiWindowsNative instance = MidiWindowsNative.getInstance();
    int result = instance.getMidiInputPortCount();
    System.out.println("... InputPortCount=" + result);
    assertTrue(result >= 0);
    assertTrue(result < 100);// cannot imagine an installation with more than a hundred midi devices

  }

  /**
   * Test of getMidiOutputPortCount method, of class MidiWindowsNative.
   */
  @Test
  public void testGetMidiOutputPortCount() {
    System.out.println("getMidiOutputPortCount");

    MidiWindowsNative instance = MidiWindowsNative.getInstance();
    int result = instance.getMidiOutputPortCount();
    System.out.println("... OutputPortCount=" + result);
    assertTrue(result > 0); //
    assertTrue(result < 100);// cannot imagine an installation with more than a hundred midi devices

  }

  /**
   * Test of testgetMidiInputPortInfo method, of class MidiWindowsNative.
   */
  @Test
  public void testGetMidiInputPortInfo() {
    System.out.println("testGetMidiInputPortInfo");

    MidiWindowsNative instance = MidiWindowsNative.getInstance();
    int count = instance.getMidiInputPortCount();

    for (int i = 0; i < count; i++) {
      Info info = instance.getMidiInputPortInfo(i);
      assertNotNull(info);
      assertEquals(Architecture.WINDOWS_MM ,info.getArchitecture());
      assertEquals(i, info.getIndex());
      assertTrue(info.isInput());
      assertNotNull(info.getName());
      assertNotNull(info.getVendor());
      assertNotNull(info.getDescription());
      assertNotNull(info.getVersion());
      System.out.println(
              String.format("...%2d) %-25s %-35s %-25s %s",
              info.getIndex(),
              info.getDescription(),
              info.getName(),
              info.getVendor(),
              info.getVersion()));

    }

  }

  /**
   * Test of getMidiOutputPortInfo method, of class MidiWindowsNative.
   */
  @Test
  public void testGetMidiOutputPortInfo() {
    System.out.println("testGetMidiOutputPortInfo");

    MidiWindowsNative instance = MidiWindowsNative.getInstance();
    int count = instance.getMidiOutputPortCount();

    for (int i = 0; i < count; i++) {
      Info info = instance.getMidiOutputPortInfo(i);
      assertNotNull(info);
      assertEquals(info.getIndex(), i);
      assertFalse(info.isInput());
      assertNotNull(info.getName());
      assertNotNull(info.getVendor());
      assertNotNull(info.getDescription());
      assertNotNull(info.getVersion());
      System.out.println(
              String.format("...%2d) %-25s %-35s %-25s %s",
              info.getIndex(),
              info.getDescription(),
              info.getName(),
              info.getVendor(),
              info.getVersion()));

    }
  }
}
