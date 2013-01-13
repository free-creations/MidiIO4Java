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

import MidiIO4Java.MidiSystemManager.Architecture;
import static org.junit.Assert.*;
import org.junit.Ignore;
import org.junit.Test;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class InfoImplTest {

  public InfoImplTest() {
  }

  /**
   * Test of getIndex method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testGetIndex() {
    System.out.println("getIndex");
  }

  /**
   * Test of getName method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testGetName() {
    System.out.println("getName");
  }

  /**
   * Test of getDescription method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testGetDescription() {
    System.out.println("getDescription");
  }

  /**
   * Test of getVendor method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testGetVendor() {
    System.out.println("getVendor");
  }

  /**
   * Test of getVersion method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testGetVersion() {
    System.out.println("getVersion");
  }

  /**
   * Test of toString method, of class InfoImpl.
   */
  @Test
  public void testToString() {
    System.out.println("testToString");
    InfoImpl instance = new InfoImpl(
            "aName",
            true,
            "aVendor",
            "aDesc",
            "aVersion",
            123,
            Architecture.WINDOWS_MM);
    String expResult = "<MidiPortInfo name=\"aName\", "
            + "input=\"true\", "
            + "vendor=\"aVendor\", "
            + "description=\"aDesc\", "
            + "version=\"aVersion\", "
            + "index=\"123\", "
            + "architecture=\"WINDOWS_MM\"/>";
    String result = instance.toString();
    assertEquals(expResult, result);
    System.out.println("..." + result);
  }

  /**
   * Test of isInput method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testIsInput() {
    System.out.println("isInput");
  }

  /**
   * Test of getArchitecture method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testGetArchitecture() {
    System.out.println("getArchitecture");
  }

  /**
   * Test of hashCode method, of class InfoImpl.
   */
  @Test
  @Ignore("Trivial")
  public void testHashCode() {
    System.out.println("hashCode");
  }

  /**
   * Test of fromString method, of class InfoImpl.
   */
  @Test
  public void testFromString() {
    System.out.println("testFromString");
    String s = "<MidiPortInfo name=\"XaNameY\", "
            + "input=\"true\", "
            + "vendor=\"\", " // test for empty params
            + "description=\"null\", " // test for null strings
            + "version=\"aVersion\", "
            + "index=\"123\", "
            + "architecture=\"WINDOWS_MM\"/>";
    InfoImpl expResult = new InfoImpl(
            "XaNameY",
            true,
            "",
            null,
            "aVersion",
            123,
            Architecture.WINDOWS_MM);
    InfoImpl result = InfoImpl.fromString(s);
    assertEquals(expResult, result);
  }

  /**
   * Test of equals method, of class InfoImpl.
   */
  @Test
  public void testEquals() {
    System.out.println("testEquals");

    InfoImpl instance = new InfoImpl(
            "aName",
            true,
            "aVendor",
            "aDesc",
            "aVersion",
            123,
            Architecture.WINDOWS_MM);

    Object nullObj = null;
    Object arbitrayObj = new Object();

    InfoImpl infoEqual = new InfoImpl(
            "aName",
            true,
            "aVendor",
            "aDesc",
            "aVersion",
            123,
            Architecture.WINDOWS_MM);

    InfoImpl infoOtherName = new InfoImpl(
            "aNameX",
            true,
            "aVendor",
            "aDesc",
            "aVersion",
            123,
            Architecture.WINDOWS_MM);
    InfoImpl infoOtherInput = new InfoImpl(
            "aName",
            false,
            "aVendor",
            "aDesc",
            "aVersion",
            123,
            Architecture.WINDOWS_MM);

    InfoImpl infoOtherVendor = new InfoImpl(
            "aName",
            true,
            "aVendorX",
            "aDesc",
            "aVersion",
            123,
            Architecture.WINDOWS_MM);

    InfoImpl infoOtherDesc = new InfoImpl(
            "aName",
            true,
            "aVendor",
            "aDescX",
            "aVersion",
            123,
            Architecture.WINDOWS_MM);

    InfoImpl infoOtherVersion = new InfoImpl(
            "aName",
            true,
            "aVendor",
            "aDesc",
            "aVersionX",
            123,
            Architecture.WINDOWS_MM);

    InfoImpl infoOtherIndex = new InfoImpl(
            "aName",
            true,
            "aVendor",
            "aDesc",
            "aVersion",
            1234,
            Architecture.WINDOWS_MM);
    InfoImpl infoOtherArchitecture = new InfoImpl(
            "aName",
            true,
            "aVendor",
            "aDesc",
            "aVersion",
            123,
            Architecture.JACK);

    assertFalse(instance.equals(nullObj));
    assertFalse(instance.equals(arbitrayObj));
    assertTrue(instance.equals(infoEqual));
    assertFalse(instance.equals(infoOtherName));
    assertFalse(instance.equals(infoOtherInput));
    assertFalse(instance.equals(infoOtherVendor));
    assertFalse(instance.equals(infoOtherDesc));
    assertFalse(instance.equals(infoOtherVersion));
    assertFalse(instance.equals(infoOtherIndex));
    assertFalse(instance.equals(infoOtherArchitecture));
  }
}
