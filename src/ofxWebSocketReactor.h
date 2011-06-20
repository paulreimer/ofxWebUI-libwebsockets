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

#include "ofThread.h"
#include "ofEvents.h"

#include "ofxWebSocketProtocol.h"

extern "C" {
#include <libwebsockets.h>
}

class ofxWebSocketReactor
: public ofThread
{
  friend class ofxWebSocketProtocol;
public:
  ofxWebSocketReactor();
  ~ofxWebSocketReactor();

  void setup(const short _port=7681,
             const std::string sslCertFilename="libwebsockets-test-server.pem",
             const std::string sslKeyFilename="libwebsockets-test-server.key.pem");

  void exit();

  void registerProtocol(const std::string name, ofxWebSocketProtocol* protocol);

  unsigned int _notify(unsigned int protocol_idx,
                       enum libwebsocket_callback_reasons reason,
                       libwebsocket* const ws,
                       void* const session,
                       const char* const message,
                       const unsigned int len) const;

  void send(libwebsocket* const ws,
            const std::string& message,
            bool binary=false);
  void send(libwebsocket* const ws,
            const char* const message,
            unsigned int len,
            bool binary=false);

	short port;

  void broadcast(const std::string& message,
                 unsigned int protocol_idx);
  void broadcast(const char* const message,
                 unsigned int len,
                 unsigned int protocol_idx);
  
  void close(libwebsocket* const ws) const;
  
protected:
  unsigned int waitMillis;
  std::string interface;
  std::vector<ofxWebSocketProtocol*> protocols;

private:
  void threadedFunction();  
  
  std::vector<unsigned char> buf;
  std::vector<struct libwebsocket_protocols> _protocols;
	struct libwebsocket_context *context;  
};

static ofxWebSocketReactor _websocket_reactor;

extern "C"
int
lws_callback(struct libwebsocket_context *context,
             struct libwebsocket *ws,
             enum libwebsocket_callback_reasons reason,
             void *session,
             void *_message, size_t len);
