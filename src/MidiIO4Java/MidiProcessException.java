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

/**
 * Exception thrown when a request could not be processed due to an internal
 * error.
 *
 * @author Harald Postner <Harald at free_creations.de>
 */
public class MidiProcessException extends RuntimeException {

  /**
   * Constructs a new exception with null as its detail message.
   */
  MidiProcessException() {
    super();
  }

  /**
   * Constructs a new exception with the specified cause, using the message of
   * "cause" as its detail message.
   *
   * @param cause the cause
   */
  MidiProcessException(Throwable cause) {
    super(cause);
  }

  /**
   * Constructs a new exception with the specified detail message.
   * @param message the detail message. 
   */
  MidiProcessException(String message) {
    super(message);
  }

  /**
   * Constructs a new exception with the specified detail message and cause. 
   * @param message the detail message 
   * @param cause the cause
   */
  MidiProcessException(String message,
          Throwable cause) {
    super(message, cause);
  }
}