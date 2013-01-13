/*
 * Copyright 2012 Harald Postner.
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

import java.util.concurrent.ExecutionException;
import java.util.concurrent.ThreadFactory;

/**
 *
 * @author Harald Postner
 */
public interface MidiSystem  {

  /**
   * Indicates whether the particular System Architecture is available.
   *
   * @return false if the open function will fail.
   */
  public boolean isAvailable();

  /**
   * Closes the session with the Midi server.
   *
   * @throws StateException if the system is not open.
   * @throws ExecutionException when one or several ports encountered a problem,
   * the cause field will tell the details where the problem occurred.
   */
  public void close() throws StateException, ExecutionException;

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
  public MidiPort createInputPort(String name, MidiInputPortListener listener) throws CreationException;

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
  public MidiPort createOutputPort(String name, MidiOutputPortListener listener) throws CreationException;

  /**
   * Opens the session with the Midi server.
   *
   * @param clientName a name under which the server shall identify this client.
   * The server might modify this name to create a unique variant, if needed.
   * @throws StateException if an open session with this client already exists.
   * @throws UnavailableException if the session could not been opened.
   */
  public void open(String clientName) throws StateException, UnavailableException;

  /**
   * Opens the session with the Midi server.
   *
   * @param clientName a name under which the server shall identify this client.
   * The server might modify this name to create a unique variant, if needed.
   * @param listener a listener that permits to monitor the state of the system. 
   * @throws StateException if an open session with this client already exists.
   * @throws UnavailableException if the session could not been opened.
   */
  public void open(String clientName, MidiSystemListener listener) throws StateException, UnavailableException;

  /**
   * Opens the session with the Midi server.
   * 
   * @param clientName a name under which the server shall identify this client.
   * @param listener a listener that permits to monitor the state of the system. 
   * @param workerThreadFactory the factory to use when creating the worker thread.   
   * @throws StateException if an open session with this client already exists.
   * @throws UnavailableException if the session could not been opened.
   * @throws NullPointerException if threadFactory is null   
   */
    public void open(String clientName, MidiSystemListener listener,ThreadFactory processThreadFactory) throws StateException, UnavailableException;

  /**
   * Indicates whether the session with the Midi server is open.
   *
   * @return true if the session with the Midi server is currently open
   */
  public boolean isOpen();


  /**
   * 
   * @throws StateException if the system is not open.
   */
  public void start();
}
