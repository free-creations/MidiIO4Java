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

import MidiIO4Java.MidiPort;
import MidiIO4Java.MidiSystemManager.Architecture;
import java.util.Objects;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class InfoImpl implements MidiPort.Info {

  //  private java.lang.String name;
  //    Signature: Ljava/lang/String;
  //  private boolean input;
  //    Signature: Z
  //  private java.lang.String vendor;
  //    Signature: Ljava/lang/String;
  //  private java.lang.String description;
  //    Signature: Ljava/lang/String;
  //  private java.lang.String version;
  //    Signature: Ljava/lang/String;
  //  private int index;
  //    Signature: I  
  private String name = null;
  private boolean input = false;
  private String vendor = null;
  private String description = null;
  private String version = null;
  private int index = -1;
  private Architecture architecture = null;

  protected InfoImpl(Architecture architecture) {
    this.architecture = architecture;
  }

  protected InfoImpl(
          String name,
          boolean input,
          String vendor,
          String description,
          String version,
          int index,
          Architecture architecture) {
    this.name = name;
    this.input = input;
    this.vendor = vendor;
    this.description = description;
    this.version = version;
    this.index = index;
    this.architecture = architecture;
  }

  /**
   * Get the value of index
   *
   * @return the value of index
   */
  @Override
  public int getIndex() {
    return index;
  }

  /**
   * Obtains the name of the device.
   *
   * @return a Character string identifying the device.
   */
  @Override
  public String getName() {
    return name;
  }

  /**
   * Obtains the description of the device.
   *
   * @return a description of the device
   */
  @Override
  public String getDescription() {
    return description;
  }

  /**
   * Obtains the name of the company who supplies the device.
   *
   * @return device the vendor's name
   */
  @Override
  public String getVendor() {
    return vendor;
  }

  /**
   * Obtains the version of the device.
   *
   * @return textual version information for the device.
   */
  @Override
  public String getVersion() {
    return version;
  }

  @Override
  public String toString() {
    return "<MidiPortInfo "
            + "name=\"" + name
            + "\", input=\"" + input
            + "\", vendor=\"" + vendor
            + "\", description=\"" + description
            + "\", version=\"" + version
            + "\", index=\"" + index
            + "\", architecture=\"" + architecture
            + "\"/>";
  }

  @Override
  public boolean isInput() {
    return input;
  }

  @Override
  public Architecture getArchitecture() {
    return architecture;
  }

  private static boolean readBooleanParm(String s, String parm) {
    String value = readStringParm(s, parm);
    if (value == null) {
      throw new IllegalArgumentException("invalid  boolean \"" + parm + "\" value.");
    }
    if (value.length() == 0) {
      throw new IllegalArgumentException("invalid  boolean \"" + parm + "\" value.");
    }
    return Boolean.valueOf(value);
  }

  private static Architecture readArchitectureParm(String s, String parm) {
    String value = readStringParm(s, parm);
    if (value == null) {
      return null;
    }
    if (value.length() == 0) {
      throw new IllegalArgumentException("invalid  Architecture \"" + parm + "\" value.");
    }
    return Architecture.valueOf(Architecture.class, value);
  }

  private static int readIntParm(String s, String parm) {
    String value = readStringParm(s, parm);
    if (value == null) {
      throw new IllegalArgumentException("invalid  integer \"" + parm + "\" value.");
    }
    if (value.length() == 0) {
      throw new IllegalArgumentException("invalid  integer \"" + parm + "\" value.");
    }
    return Integer.parseInt(value);
  }

  private static String readStringParm(String s, String parm) {
    String startTag = parm + "=\"";
    int beginIndex = s.indexOf(startTag);
    if (beginIndex < 0) {
      throw new IllegalArgumentException("missing \"" + parm + "\" parameter.");
    }
    beginIndex = beginIndex + startTag.length();
    int endIndex = s.indexOf('\"', beginIndex);
    if (endIndex < 0) {
      throw new IllegalArgumentException("invalid \"" + parm + "\" value.");
    }
    String result = s.substring(beginIndex, endIndex);
    if ("null".equals(result)) {
      return null;
    } else {
      return result;
    }
  }

  public static InfoImpl fromString(String s) throws IllegalArgumentException {
    s = s.trim();
    if (!s.startsWith("<MidiPortInfo ")) {
      throw new IllegalArgumentException("missing \"MidiPortInfo\" tag.");
    }
    String name = readStringParm(s, "name");
    boolean input = readBooleanParm(s, "input");
    String vendor = readStringParm(s, "vendor");
    String description = readStringParm(s, "description");
    String version = readStringParm(s, "version");
    int index = readIntParm(s, "index");
    Architecture architecture = readArchitectureParm(s, "architecture");
    return new InfoImpl(name, input, vendor, description, version, index, architecture);
  }

  @Override
  public boolean equals(Object obj) {
    if (obj == null) {
      return false;
    }
    if (getClass() != obj.getClass()) {
      return false;
    }
    final InfoImpl other = (InfoImpl) obj;
    if (!Objects.equals(this.name, other.name)) {
      return false;
    }
    if (this.input != other.input) {
      return false;
    }
    if (!Objects.equals(this.vendor, other.vendor)) {
      return false;
    }
    if (!Objects.equals(this.description, other.description)) {
      return false;
    }
    if (!Objects.equals(this.version, other.version)) {
      return false;
    }
    if (this.index != other.index) {
      return false;
    }
    if (this.architecture != other.architecture) {
      return false;
    }
    return true;
  }

  @Override
  public int hashCode() {
    int hash = 7;
    hash = 97 * hash + Objects.hashCode(this.name);
    hash = 97 * hash + (this.input ? 1 : 0);
    hash = 97 * hash + Objects.hashCode(this.vendor);
    hash = 97 * hash + Objects.hashCode(this.description);
    hash = 97 * hash + Objects.hashCode(this.version);
    hash = 97 * hash + this.index;
    hash = 97 * hash + (this.architecture != null ? this.architecture.hashCode() : 0);
    return hash;
  }
}
