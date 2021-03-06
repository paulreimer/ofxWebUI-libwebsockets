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

#include "Poco/FIFOEvent.h"
#include "ofxWebSocketEvents.h"

class ofxWebSocketReactor;

class ofxWebSocketProtocol
{
  friend class ofxWebSocketReactor;
public:
  ofxWebSocketProtocol();
  ~ofxWebSocketProtocol();
  
  virtual bool allowClient(const std::string name,
                           const std::string ip) const;

  void broadcast(const std::string& message);

  unsigned int idx;
  bool binary;

protected:  
  virtual void execute() {}

  virtual void onconnect  (ofxWebSocketEvent& args);
  virtual void onopen     (ofxWebSocketEvent& args);
  virtual void onclose    (ofxWebSocketEvent& args);
  virtual void onidle     (ofxWebSocketEvent& args);
  virtual void onmessage  (ofxWebSocketEvent& args);
  virtual void onbroadcast(ofxWebSocketEvent& args);

  virtual unsigned short onhttp     (const std::string& url);  

  Poco::FIFOEvent<ofxWebSocketEvent> onconnectEvent;
  Poco::FIFOEvent<ofxWebSocketEvent> onopenEvent;
  Poco::FIFOEvent<ofxWebSocketEvent> oncloseEvent;
  Poco::FIFOEvent<ofxWebSocketEvent> onidleEvent;
  Poco::FIFOEvent<ofxWebSocketEvent> onmessageEvent;
  Poco::FIFOEvent<ofxWebSocketEvent> onbroadcastEvent;
  Poco::FIFOEvent<ofxWebSocketEvent> onhttpEvent;

  bool defaultAllowPolicy;
  std::map<std::string, bool> allowRules;

  ofxWebSocketReactor* reactor;

private:
  void _onconnect   (ofxWebSocketEvent& args);
  void _onopen      (ofxWebSocketEvent& args);
  void _onclose     (ofxWebSocketEvent& args);
  void _onidle      (ofxWebSocketEvent& args);
  void _onmessage   (ofxWebSocketEvent& args);
  void _onbroadcast (ofxWebSocketEvent& args);

  bool _allowClient(const std::string name,
                    const std::string ip) const;
};
