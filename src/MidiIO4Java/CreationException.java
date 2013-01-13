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
 * This error is thrown when the Midi System cannot create
 * a requested object.
 * @author Harald Postner
 */
public class CreationException extends Exception {

  /**
   * Constructs a new exception with null as its detail message.
   */
  public CreationException() {
    super();
  }

  /**
   * Constructs a new exception with the specified detail message.
   *
   * @param message the detail message.
   */
  public CreationException(String message) {
    super(message);
  }

  /**
   * Constructs a new exception with the specified detail message and cause.
   *
   * @param message the detail message
   * @param cause the cause.
   */
  public CreationException(String message,
          Throwable cause) {
    super(message, cause);
  }
}