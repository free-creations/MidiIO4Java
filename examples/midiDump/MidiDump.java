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
package midiDump;

import MidiIO4Java.CreationException;
import MidiIO4Java.MidiInputPortListener;
import MidiIO4Java.MidiPort;
import MidiIO4Java.MidiSystem;
import MidiIO4Java.MidiSystemManager;
import MidiIO4Java.StateException;
import java.io.IOException;
import java.util.concurrent.ExecutionException;
import javax.sound.midi.MidiEvent;
import javax.sound.midi.MidiMessage;
import javax.sound.midi.ShortMessage;

/**
 *
 * @author Harald Postner
 */
public class MidiDump {

  private static MidiInputPortListener listener =
          new MidiInputPortListener() {
            @Override
            public void process(long timeCodeStart,
                    long timeCodeDuration,
                    MidiEvent[] events,
                    boolean lastCycle) throws Throwable {

              if (events.length == 0) {
                return;
              }
              System.out.println(" --- Processing frame(" + timeCodeStart + ") with " + events.length + " events.");
              for (MidiEvent event : events) {
                long tick = event.getTick();
                MidiMessage m = event.getMessage();
                if (m instanceof ShortMessage) {
                  ShortMessage sm = (ShortMessage) m;
                  System.out.println("    " + tick
                          + " Midi; status(" + sm.getStatus()
                          + ") command(" + sm.getCommand()
                          + ") data1(" + sm.getData1()
                          + ") data2(" + sm.getData2()
                          + ").");
                } else {
                  System.out.println("    " + tick + " un-recognized message.");
                }
              }
            }

            @Override
            public void onClose() {
              System.out.println(" --- port closed.");
            }

            @Override
            public void onOpen() {
              System.out.println(" --- port opened.");

            }
          };

  @SuppressWarnings("SleepWhileInLoop")
  public static void main(String[] args) throws IOException, CreationException, InterruptedException, StateException, ExecutionException {
    System.out.println("Welcome to Midi-dump:");
    System.out.println(" Connect a midi device to the port.");
    System.out.println(" To end, hit return key.");

    MidiSystemManager.loadNativeLibray(null);

    MidiSystem midiSystem = MidiSystemManager.getMidiSystemInstance();
    MidiPort port = midiSystem.createInputPort("Port", listener);
    midiSystem.open("MidiDump");

    midiSystem.start();

    while (System.in.available() == 0) {
      Thread.sleep(100);
    }

    midiSystem.close();


    System.out.println(" Bye.");
  }
}
