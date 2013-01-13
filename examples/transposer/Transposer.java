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
package transposer;

import MidiIO4Java.CreationException;
import MidiIO4Java.MidiInputPortListener;
import MidiIO4Java.MidiOutputPortListener;
import MidiIO4Java.MidiSystem;
import MidiIO4Java.MidiSystemManager;
import MidiIO4Java.StateException;
import java.io.IOException;
import java.util.concurrent.ExecutionException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.sound.midi.InvalidMidiDataException;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.ShortMessage;

/**
 *
 * @author Harald Postner
 */
public class Transposer {

  private static MidiEvent[] events;
  private static MidiInputPortListener input =
          new MidiInputPortListener() {
            @Override
            public void process(long timeCodeStart,
                    long timeCodeDuration,
                    MidiEvent[] e,
                    boolean lastCycle) throws Throwable {
              events = e;
            }

            @Override
            public void onClose() {
            }

            @Override
            public void onOpen() {
            }
          };
  private static MidiOutputPortListener output =
          new MidiOutputPortListener() {
            @Override
            public MidiEvent[] process(long timeCodeStart,
                    long timeCodeDuration,
                    boolean lastCycle) throws Throwable {

              for (MidiEvent event : events) {
                if (event != null) {
                  if (event.getMessage() instanceof ShortMessage) {
                    ShortMessage sm = (ShortMessage) event.getMessage();
                    try {
                      if (sm.getCommand() == ShortMessage.NOTE_ON) {
                        sm.setMessage(sm.getStatus(), sm.getData1() + 12, sm.getData2());
                      }
                      if (sm.getCommand() == ShortMessage.NOTE_OFF) {
                        sm.setMessage(sm.getStatus(), sm.getData1() + 12, sm.getData2());
                      }
                    } catch (InvalidMidiDataException ex) {
                      Logger.getLogger(Transposer.class.getName()).log(Level.INFO, null, ex);
                    }
                  }
                }
              }
              return events;
            }

            @Override
            public void onClose() {
            }

            @Override
            public void onOpen() {
            }
          };

  @SuppressWarnings("SleepWhileInLoop")
  public static void main(String[] args) throws IOException, CreationException, InterruptedException, StateException, ExecutionException {
    System.out.println("Welcome to Transposer:");
    System.out.println(" Connect a keybord to the Transposer.In port.");
    System.out.println(" Connect a synthsizer to the Transposer.Out port.");
    System.out.println(" Play on your keyboard. To end, hit return key.");

    MidiSystemManager.loadNativeLibray(null);

    MidiSystem midiSystem = MidiSystemManager.getMidiSystemInstance();
    midiSystem.createInputPort("In", input);
    midiSystem.createOutputPort("Out", output);
    midiSystem.open("Transposer");

    midiSystem.start();

    while (System.in.available() == 0) {
      Thread.sleep(100);
    }

    midiSystem.close();

    System.out.println(" Bye.");
  }
}