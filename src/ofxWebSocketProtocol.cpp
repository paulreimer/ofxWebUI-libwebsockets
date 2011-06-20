/*
 * Copyright Paul Reimer, 2012
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc/3.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "ofxWebSocketProtocol.h"

//--------------------------------------------------------------
ofxWebSocketProtocol::ofxWebSocketProtocol()
: binary(false)
, defaultAllowPolicy(true)
, reactor(NULL)
{
  ofAddListener(onconnectEvent,      this, &ofxWebSocketProtocol::_onconnect);
  ofAddListener(onopenEvent,         this, &ofxWebSocketProtocol::_onopen);
  ofAddListener(oncloseEvent,        this, &ofxWebSocketProtocol::_onclose);
  ofAddListener(onidleEvent,         this, &ofxWebSocketProtocol::_onidle);
  ofAddListener(onmessageEvent,      this, &ofxWebSocketProtocol::_onmessage);
  ofAddListener(httprequestEvent,    this, &ofxWebSocketProtocol::_httprequest);
}

//--------------------------------------------------------------
ofxWebSocketProtocol::~ofxWebSocketProtocol()
{
  ofRemoveListener(onconnectEvent,   this, &ofxWebSocketProtocol::_onconnect);
  ofRemoveListener(onopenEvent,      this, &ofxWebSocketProtocol::_onopen);
  ofRemoveListener(oncloseEvent,     this, &ofxWebSocketProtocol::_onclose);
  ofRemoveListener(onidleEvent,      this, &ofxWebSocketProtocol::_onidle);
  ofRemoveListener(onmessageEvent,   this, &ofxWebSocketProtocol::_onmessage);
  ofRemoveListener(httprequestEvent, this, &ofxWebSocketProtocol::_httprequest);
}

//--------------------------------------------------------------
bool
ofxWebSocketProtocol::_allowClient(const std::string name,
                                   const std::string ip)
{
  std::cout
  << "Received network connect from "
  << name << " (" << ip << ")"
  << std::endl;
  
  std::map<std::string, bool>::iterator allow_iter;
  
  allow_iter = allowRules.find(name);
  if (allow_iter != allowRules.end())
    return allow_iter->second;
  
  allow_iter = allowRules.find(ip);
  if (allow_iter != allowRules.end())
    return allow_iter->second;
  
  return allowClient(name, ip);
}

//--------------------------------------------------------------
bool
ofxWebSocketProtocol::allowClient(const std::string name,
                                  const std::string ip)
{
  return defaultAllowPolicy;
}
