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

extern "C" {
#include <libwebsockets.h>
}

#include <vector>
#include <string>

class ofxWebSocketReactor;
class ofxWebSocketProtocol;

class ofxWebSocketSession {
public:
};

class ofxWebSocketConnection {
  friend class ofxWebSocketReactor;
public:
  ofxWebSocketConnection(ofxWebSocketReactor* const _reactor=NULL,
                         ofxWebSocketProtocol* const _protocol=NULL,
                         const bool supportsBinary=false);

  void close();
  void send(const std::string& message);
  const std::string recv(const std::string& message);  

  libwebsocket* ws;

protected:
  ofxWebSocketReactor*  reactor;
  ofxWebSocketProtocol* protocol;
  ofxWebSocketSession*  session;

  bool binary;
  bool supportsBinary;

  std::vector<unsigned char> buf;
};

class ofxWebSocketEvent {
public:
  ofxWebSocketEvent(ofxWebSocketConnection& _conn,
                    std::string& _message)
  : conn(_conn)
  , message(_message)
  {}

  ofxWebSocketConnection& conn;
  std::string& message;
};
