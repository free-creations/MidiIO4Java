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
import java.util.concurrent.ExecutionException;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public interface MidiPort {

  /**
   * The info structure is used to identify a specific port. The info object can
   * be persisted as a String.
   */
  public static interface Info {

    /**
     * Indicates to which architecture this port belongs to.
     *
     * @return the architecture.
     */
    public Architecture getArchitecture();

    /**
     * Get the value of index
     *
     * @return the value of index
     */
    public int getIndex();

    /**
     * Obtains the name of the device.
     *
     * @return a Character string identifying the device.
     */
    public String getName();

    /**
     * Obtains the description of the device.
     *
     * @return a description of the device
     */
    public String getDescription();

    /**
     * Obtains the name of the company who supplies the device.
     *
     * @return device the vendor's name
     */
    public String getVendor();

    /**
     * Obtains the version of the device.
     *
     * @return textual version information for the device.
     */
    public String getVersion();

    public boolean isInput();

    @Override
    public String toString();
  }

  /**
   * Obtains information about the device.
   *
   * @return device info
   */
  public Info getPortInfo();

  /**
   * Closes the device, indicating that the device should now release any system
   * resources it is using. The callback of port- listener will be stopped and
   * the listeners onClose method will be invoked. Once a port is closed it
   * cannot be re-opened.
   *
   * @throws ExecutionException when the port encountered a problem in
   * processing the call-backs, the cause field will tell the details where the
   * problem occurred.
   */
  public void close() throws ExecutionException;

  /**
   * Reports whether the device has been closed. A port can be closed for one of
   * three reasons: <ol> <li>The device has explicitly been closed by the port's
   * close() function.</li> <li>The MidiSystem been closed.</li> <li>The
   * Process() callback has thrown an exception.</li> </ol> A closed port can
   * not be reopened.
   *
   * @return true if the device is closed, otherwise false
   */
  public boolean isClosed();
}
