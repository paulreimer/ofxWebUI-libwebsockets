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
#include <map>

#include "ofEvents.h"

#include "ofxWebSocketEvents.h"
class ofxWebSocketReactor;
class ofxWebSocketProtocol;

class ofxWebSocketProtocol
{
  friend class ofxWebSocketReactor;
public:
  ofxWebSocketProtocol();
  ~ofxWebSocketProtocol();
  
  virtual bool allowClient(const std::string name,
                           const std::string ip);
  
  bool binary;

  ofxWebSocketReactor* reactor;
  ofxWebSocketProtocol* session;

  unsigned int idx;  

protected:  
  virtual void execute() {}

  virtual void onconnect  (ofxWebSocketEventArgs& args) {}
  virtual void onopen     (ofxWebSocketEventArgs& args) {}
  virtual void onclose    (ofxWebSocketEventArgs& args) {}
  virtual void onidle     (ofxWebSocketEventArgs& args) {}
  virtual void onmessage  (ofxWebSocketEventArgs& args) {}
  virtual void httprequest(ofxWebSocketEventArgs& args) {}

  ofEvent<ofxWebSocketEventArgs> onconnectEvent;
  ofEvent<ofxWebSocketEventArgs> onopenEvent;
  ofEvent<ofxWebSocketEventArgs> oncloseEvent;
  ofEvent<ofxWebSocketEventArgs> onidleEvent;
  ofEvent<ofxWebSocketEventArgs> onmessageEvent;
  ofEvent<ofxWebSocketEventArgs> httprequestEvent;
  
  bool defaultAllowPolicy;
  std::map<std::string, bool> allowRules;

private:
  void _onconnect(ofxWebSocketEventArgs& args)
  { onconnect(args); }  

  void _onopen(ofxWebSocketEventArgs& args)
  { onopen(args); }

  void _onclose(ofxWebSocketEventArgs& args)
  { onclose(args); }

  void _onidle(ofxWebSocketEventArgs& args)
  { onidle(args); }

  void _onmessage(ofxWebSocketEventArgs& args)
  { onmessage(args); }

  void _httprequest(ofxWebSocketEventArgs& args)
  { httprequest(args); }
  
  bool _allowClient(const std::string name,
                    const std::string ip);  
};
