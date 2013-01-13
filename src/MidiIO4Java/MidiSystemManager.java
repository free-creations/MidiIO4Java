/*
 *  Copyright 2012 Harald Postner <Harald at H-Postner.de>.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  under the License.
 */
package MidiIO4Java;


import MidiIO4Java.Implementation.MidiJackNative;
import MidiIO4Java.Implementation.MidiWindowsNative;
import java.io.File;
import java.util.ArrayList;

/**
 * The MidiSystemManager provides access to the available
 * {@link MidiSystem# Midi-Systems}.
 *
 * @author Harald Postner <Harald at H-Postner.de>
 */
public class MidiSystemManager {
  
    /**
   * The different architectures of Midi Systems.
   */
  public static enum Architecture {

    WINDOWS_MM, // ArchitectureType representing the Microsoft Multi Media Architecture.
    ALSA, //ArchitectureType representing the Advanced-Linux-Sound-Architecture Architecture
    JACK, // ArchitectureType representing the Linux Open Sound System Architecture.
    MACOSX_CORE // ArchitectureType representing the Macintosh OS-X Core Midi Architecture.
  }

  /**
   * The nativeLibLoaded flag is true if the native library has been
   * successfully loaded.
   */
  private static boolean nativeLibLoaded = false;
  /**
   * Protects the nativeLibLoaded flag from concurrent access.
   */
  private static final Object nativeLibLoadingLock = new Object();


  private static MidiSystem windowsSystemInstance = null;
  private static final Object windowsSystemCreationLock = new Object();
  private static MidiSystem jackSystemInstance = null;
  private static final Object jackSystemCreationLock = new Object();
  /**
   * Internal identifier for the MS-Windows operating system.
   */
  private static final String WIN = "win";
  /**
   * Internal identifier for the Linux operating system.
   */
  private static final String LINUX = "linux";
  /**
   * Internal identifier for the Mac OS X operating system.
   */
  private static final String MAC = "mac";

  /**
   * Load the native library. The OS specific functions are accessed through a
   * native library. This function can be used to load the library from a user
   * defined directory. To be effective, this function must be called before any
   * other function. If no library directory is given (libDir = null) the native
   * library will be searched in the Java- System directory.
   *
   * @param libDir the directory where the library is supposed to be or null if
   * the library should be fetched from the Java- System directory.
   * @throws UnsatisfiedLinkError If the native library could not be loaded
   */
  static public void loadNativeLibray(File libDir) throws UnsatisfiedLinkError {
    synchronized (nativeLibLoadingLock) {
      if (nativeLibLoaded) {
        //already loaded
        return;
      }
      if (libDir == null) {
        System.loadLibrary(getNativeLibrayName());
      } else {
        if (!libDir.exists()) {
          throw new UnsatisfiedLinkError("Directory \"" + libDir.getPath() + "\" does not exist.");
        }
        if (!libDir.isDirectory()) {
          throw new UnsatisfiedLinkError("File \"" + libDir.getPath() + "\" is not a directory.");
        }
        File libFile = new File(libDir,
                System.mapLibraryName(getNativeLibrayName()));
        if (!libFile.exists()) {
          throw new UnsatisfiedLinkError("Library \"" + libFile.getPath() + "\" does not exist.");
        }
        System.load(libFile.getAbsolutePath());
      }
      nativeLibLoaded = true;
    }
  }

  /**
   * Indicates whether the native library has been successfully loaded.
   *
   * @return true if the native library is successfully loaded
   */
  static public boolean isNativeLibLoaded() {
    synchronized (nativeLibLoadingLock) {
      return nativeLibLoaded;
    }
  }

  /**
   * Makes sure that the native library has been successfully loaded. If no
   * library has been loaded so far, this function will attempt to load the
   * library from the system path.
   *
   * @throws UnsatisfiedLinkError If the native library could not be loaded
   */
  static private void checkLoadNativeLib() throws UnsatisfiedLinkError {
    loadNativeLibray(null);
  }

  /**
   * Gets the MidiSystem object for the default Architecture.
   *
   * @return an MidiSystem object for the default Architecture.
   * @throws UnsatisfiedLinkError If the native library could not be loaded
   * @see #getMidiSystemInstance(Midi4Java.MidiSystemManager.Architecture)
   */
  public static MidiSystem getMidiSystemInstance()
          throws UnavailableException, UnsatisfiedLinkError {
    return getMidiSystemInstance(getDefaultArchitecture());
  }

  /**
   * Gets an MidiSystem object for the requested Architecture. For each
   * Architecture type only one and only one MidiSystemXXXXXXNative-object
   * (where XXX stands for the  architecture type) will be
   * initialized (singleton pattern). Subsequent calls to getMidiSystemInstance
   * with the same ArchitectureType will return the same object.
   *
   * @param ArchitectureType represents the type of Architecture requested.
   * @return a MidiSystem object for the requested Architecture.
   * @throws UnavailableException If the requested Architecture is not
   * available
   * @throws UnsatisfiedLinkError If the native library could not be loaded
   */
  public static MidiSystem getMidiSystemInstance(Architecture ArchitectureType)
          throws UnavailableException, UnsatisfiedLinkError {

    checkLoadNativeLib();
    switch (ArchitectureType) {
      case WINDOWS_MM:
        return getWindowsSystemInstance();
      case JACK:
        return getJackSystemInstance();
      default:
        throw new UnavailableException("Not yet implemented");
    }
  }

  private static MidiSystem getWindowsSystemInstance() throws UnavailableException {
    synchronized (windowsSystemCreationLock) {
      if (windowsSystemInstance == null) {
        windowsSystemInstance = MidiWindowsNative.getInstance();
      }
      if (!windowsSystemInstance.isAvailable()) {
        throw new UnavailableException("Windows Multi-Media not available.");
      }
      return windowsSystemInstance;
    }
  }

  private static MidiSystem getJackSystemInstance() throws UnavailableException {
    synchronized (jackSystemCreationLock) {
      if (jackSystemInstance == null) {
        jackSystemInstance = MidiJackNative.getInstance();
      }
      if (!jackSystemInstance.isAvailable()) {
        throw new UnavailableException("Jack-Audio not available.");
      }
      return jackSystemInstance;
    }
  }

  /**
   * Evaluate which Midi Architectures are currently available.
   *
   * @return an array with an entry for each available Architecture.
   */
  public static Architecture[] getAvailableArchitectures() {
    ArrayList<Architecture> result = new ArrayList<>();
    for (Architecture arch : Architecture.values()) {
      if (isArchitecturesAvailable(arch)) {
        result.add(arch);
      }
    }
    Architecture[] resultArray = new Architecture[result.size()];
    return result.toArray(resultArray);
  }

  /**
   * Evaluate whether a given Midi Architecture is currently available.
   *
   * @param a the given Midi Architecture
   * @return true if the given architecture is available, false otherwise.
   */
  @SuppressWarnings("UseSpecificCatch")
  public static boolean isArchitecturesAvailable(Architecture a) {
    try {
      getMidiSystemInstance(a);
      return true;
    } catch (Exception ignored) {
      return false;
    }
  }

  /**
   * Selects the preferred Midi Architectures among all available Architectures.
   *
   * @return the default Architecture type.
   */
  public static Architecture getDefaultArchitecture() {
    switch (getOsName()) {
      case WIN:
        return Architecture.WINDOWS_MM;
      case LINUX:
        return Architecture.JACK;
      default:
        throw new RuntimeException("Unsupported Operating System \"" + System.getProperty("os.name") + "\".");
    }
  }

  /**
   * Determine the name of the operating system.
   *
   * @return either "win" or "linux" or "mac".
   */
  private static String getOsName() {
    String osName = System.getProperty("os.name").toLowerCase();
    if (osName.indexOf("windows") > -1) {
      return WIN;
    }
    if (osName.indexOf("linux") > -1) {
      return LINUX;
    }
    if (osName.indexOf("mac") > -1) {
      return MAC;
    }
    throw new RuntimeException("Unsupported Operating System \"" + System.getProperty("os.name") + "\".");
  }

  /**
   * Determine if we are running on a 64 bit or a 32 bit version of JRE.
   *
   * @return the string "32" or "64".
   */
  private static String getJreBitness() {
    //let's try with "sun.arch.data.model"
    String sun_arch_data_model = System.getProperty("sun.arch.data.model");
    if ("64".equals(sun_arch_data_model)) {
      return "64";
    }
    if ("32".equals(sun_arch_data_model)) {
      return "32";
    }
    //"sun.arch.data.model" did not succeed let's try with "os.arch"
    String os_arch = System.getProperty("os.arch");
    if (os_arch.endsWith("64")) {
      return "64";
    }
    //nothing succeeded let's assume 32 bit
    return "32";
  }

  private static String getNativeLibrayName() {
    final String name = ProjectInfo.PROJECT_NAME + "-"
            + getOsName() + getJreBitness() + "-"
            + ProjectInfo.VERSION_MAJOR + "."
            + ProjectInfo.VERSION_MINOR + "."
            + ProjectInfo.VERSION_PATCH;
    return name;
  }
}
