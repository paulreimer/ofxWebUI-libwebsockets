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

extern "C" {
#include <libwebsockets.h>
}

class ofxWebSocketConnection;
class ofxWebSocketProtocol;

class ofxWebSocketReactor;

class ofxWebSocketReactor
: public ofThread
{
  friend class ofxWebSocketProtocol;
public:
  ofxWebSocketReactor();
  ~ofxWebSocketReactor();

  static ofxWebSocketReactor& instance();
  static ofxWebSocketReactor* _instance;  

  void setup(const short _port=7681,
             const std::string sslCertFilename="libwebsockets-test-server.pem",
             const std::string sslKeyFilename="libwebsockets-test-server.key.pem");

  void exit();

  void registerProtocol(const std::string& name,
                        ofxWebSocketProtocol& protocol);

  void close(ofxWebSocketConnection* const conn);

  ofxWebSocketProtocol* const protocol(const unsigned int idx);

	short port;

//private:
  unsigned int _allow(ofxWebSocketProtocol* const protocol, const long fd);

  unsigned int _notify(ofxWebSocketConnection* const conn,
                       enum libwebsocket_callback_reasons const reason,
                       const char* const _message,
                       const unsigned int len);

  unsigned int _http(struct libwebsocket *ws,
                     const char* const url);

  std::string document_root;

  std::vector<std::pair<std::string, ofxWebSocketProtocol*> > protocols;

protected:
  unsigned int waitMillis;
  std::string interface;

private:
  void threadedFunction();  
  
  std::vector<struct libwebsocket_protocols> lws_protocols;
	struct libwebsocket_context *context;  
};

extern "C"
int
lws_callback(struct libwebsocket_context *context,
             struct libwebsocket *ws,
             enum libwebsocket_callback_reasons reason,
             void *user,
             void *_message, size_t len);
