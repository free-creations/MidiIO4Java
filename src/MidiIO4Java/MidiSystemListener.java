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

/**
 *
 * @author Harald Postner
 */
public interface MidiSystemListener {

  /**
   * The "onCycleStart" event happens at the start of each cycle. The contract
   * is that this event happens before any port is processed. The calling thread
   * is the java-process-thread.
   *
   * @see {@link MidiInputPortListener#process() } and
   * {@link MidiOutputPortListener#process()}
   * @param timeCodeStart the time-tick at the start of this cycle
   * @param timeCodeDuration the number of time-ticks in this cycle
   * @param lastCycle true when this is the last cycle before shutdown. (Note:
   * in case of an emergency shutdown it may not always be possible to complete
   * a "last cycle")
   * @throws Throwable an implementation of this event handler may throw any
   * kind of exception. When such an exception is thrown the midi system will
   * shutdown. The exception emitted by this event handler will be re-thrown
   * when {@link MidiSystem#close()} is called.
   */
  public void onCycleStart(long timeCodeStart, long timeCodeDuration, boolean lastCycle) throws Throwable;
  // Signature: (JJZ)V

  /**
   * The "onCycleEnd" event happens at the end of each cycle. The contract is
   * that this event happens after all ports have been processed. The calling
   * thread is the java-process-thread.
   *
   * @see {@link MidiInputPortListener#process() } and
   * {@link MidiOutputPortListener#process()}
   * @param timeCodeStart the time-tick at the start of this cycle
   * @param timeCodeDuration the number of time-ticks in this cycle
   * @param lastCycle true when this is the last cycle before shutdown. (Note:
   * in case of an emergency shutdown it may not always be possible to complete
   * a "last cycle")
   * @throws Throwable an implementation of this event handler may throw any
   * kind of exception. When such an exception is thrown the midi system will
   * shutdown. The exception emitted by this event handler will be re-thrown
   * when {@link MidiSystem#close()} is called.
   */
  public void onCycleEnd(long timeCodeStart, long timeCodeDuration, boolean lastCycle) throws Throwable;
  // Signature: (JJZ)V

  public void onConnectionChanged() throws Throwable;
  // Signature: ()V

  public void onClose() throws Throwable;
  // Signature: ()V

  public void onOpen() throws Throwable;
  // Signature: ()V
}
