/*
 * Copyright Paul Reimer, 2012
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc/3.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#pragma once 

#include <string>

class ofxWebSocketConnection;

class ofxWebSocketEvent {
public:
  ofxWebSocketEvent(ofxWebSocketConnection& _conn,
                    std::string& _message);

  ofxWebSocketConnection& conn;
  std::string& message;
};

