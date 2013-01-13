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

import javax.sound.midi.MidiEvent;

/**
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public interface MidiInputPortListener {

  /**
   * The "process" event is the main event in every process cycle. The calling
   * thread is the java-process-thread.
   *
   * @see {@link MidiInputPortListener#process() } and
   * {@link MidiOutputPortListener#process()}
   * @param timeCodeStart the time-tick at the start of this cycle
   * @param timeCodeDuration the number of time-ticks in this cycle
   * @param events a list of MidiEvents that shall be processed in this
   * cycle. The timestamps of the midi events are relative to the timeCodeStart.
   * @param lastCycle true when this is the last cycle before shutdown. (Note:
   * in case of an emergency shutdown it may not always be possible to complete
   * a "last cycle")
   * @throws Throwable an implementation of this event handler may throw any
   * kind of exception. When such an exception is thrown the midi system will
   * shutdown. The exception emitted by this event handler will be re-thrown
   * when {@link MidiSystem#close()} is called.
   */
  public void process(long timeCodeStart, long timeCodeDuration,
          MidiEvent[] events, boolean lastCycle) throws Throwable;

  public void onClose() throws Throwable;

  public void onOpen() throws Throwable;
}
