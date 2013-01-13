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
package MidiIO4Java;


import MidiIO4Java.MidiSystemManager.Architecture;
import java.io.File;
import static org.junit.Assert.*;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class MidiSystemManagerTest {

  final static String osName = System.getProperty("os.name").toLowerCase();
  final static boolean isWin = (osName.indexOf("windows") >= 0);
  final static boolean isLinux = (osName.indexOf("linux") >= 0);
  final static boolean isMac = (osName.indexOf("mac") >= 0);

  public MidiSystemManagerTest() {
  }

  /**
   * Test of loadNativeLibray method, of class MidiSystemManager. These
   * functions can only be tested in the "BeforeClass" method, because
   * once the library is successfully loaded, nothing happens anymore.
   */
  @BeforeClass
  public static void testLoadNativeLibray() {
    System.out.println("loadNativeLibray");
    // calling "loadNativeLibray" with an inexisting path
    // must throw an exception
    boolean thrown = false;
    try {
      MidiSystemManager.loadNativeLibray(new File("thisDirectorydoesNotExist"));
    } catch (Throwable ex) {
      thrown = true;
    }
    assertTrue(thrown);

    // calling "loadNativeLibray"  with null
    // should always work in this test configuration.
    // The library should be loaded from the system library path.
    // (please  note that we have set in project/properties/Run
    //-Djava.library.path=./dist)
    MidiSystemManager.loadNativeLibray(null);
    assertTrue(MidiSystemManager.isNativeLibLoaded());
  }

  /**
   * Test of getMidiSystemInstance method, of class MidiSystemManager.
   */
  @Test
  public void testGetMidiSystemInstance_0args() throws Exception {
    System.out.println("getMidiSystemInstance");
    // calling getMidiSystemInstance() should return
    // the default system which should not be null.
    MidiSystem instance = MidiSystemManager.getMidiSystemInstance();
    assertNotNull(instance);

  }

  /**
   * Test of getMidiSystemInstance method, of class MidiSystemManager.
   */
  @Test
  public void testGetMidiSystemInstance_MidiSystemFactoryArchitecture() throws Exception {
    System.out.println("getMidiSystemInstance");
    // calling getMidiSystemInstance(Architecture) for the default Architecture should always work.
    Architecture defaultArchitecture = MidiSystemManager.getDefaultArchitecture();
    MidiSystem instance = MidiSystemManager.getMidiSystemInstance(defaultArchitecture);
    assertNotNull(instance);
  }

  /**
   * Test of getAvailableArchitectures method, of class MidiSystemManager.
   */
  @Test
  public void testGetAvailableArchitectures() {
    System.out.println("getAvailableArchitectures");

    Architecture[] result = MidiSystemManager.getAvailableArchitectures();
    //there should at least be 1 system available on every plattform.
    assertTrue(result.length > 0);

  }

  /**
   * Test of isNativeLibLoaded method, of class MidiSystemManager.
   */
  @Test
  @Ignore("already tested in: testLoadNativeLibray")
  public void testIsNativeLibLoaded() {
  }

  /**
   * Test of isArchitecturesAvailable method, of class MidiSystemManager.
   */
  @Test
  public void testIsArchitecturesAvailable() {
    System.out.println("isArchitecturesAvailable");
    // we assume, that on windows the windowsMM architecture is always available
    if (isWin) {
      assertTrue(MidiSystemManager.isArchitecturesAvailable(Architecture.WINDOWS_MM));
      assertFalse(MidiSystemManager.isArchitecturesAvailable(Architecture.JACK));
    }
    // we assume, that on linux the jack architecture is always available
    if (isLinux) {
      assertFalse(MidiSystemManager.isArchitecturesAvailable(Architecture.WINDOWS_MM));
      assertTrue(MidiSystemManager.isArchitecturesAvailable(Architecture.JACK));
    }

  }

  /**
   * Test of getDefaultArchitecture method, of class MidiSystemManager.
   */
  @Test
  public void testGetDefaultArchitecture() {
    System.out.println("getDefaultArchitecture");
    // we assume, that on windows, the windowsMM architecture is the default.
    if (isWin) {
      assertEquals(Architecture.WINDOWS_MM, MidiSystemManager.getDefaultArchitecture());

    }
    // we assume, that on linux, the jack architecture is the default.
    if (isLinux) {
      assertEquals(Architecture.JACK, MidiSystemManager.getDefaultArchitecture());
    }
  }
}
