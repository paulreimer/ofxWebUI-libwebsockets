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
#include "ofxWebSocketReactor.h"
#include "ofxWebSocketConnection.h"

//--------------------------------------------------------------
ofxWebSocketProtocol::ofxWebSocketProtocol()
: defaultAllowPolicy(true)
{
  ofAddListener(onconnectEvent,      this, &ofxWebSocketProtocol::_onconnect);
  ofAddListener(onopenEvent,         this, &ofxWebSocketProtocol::_onopen);
  ofAddListener(oncloseEvent,        this, &ofxWebSocketProtocol::_onclose);
  ofAddListener(onidleEvent,         this, &ofxWebSocketProtocol::_onidle);
  ofAddListener(onmessageEvent,      this, &ofxWebSocketProtocol::_onmessage);
  ofAddListener(onbroadcastEvent,    this, &ofxWebSocketProtocol::_onbroadcast);
}

//--------------------------------------------------------------
ofxWebSocketProtocol::~ofxWebSocketProtocol()
{
  ofRemoveListener(onconnectEvent,   this, &ofxWebSocketProtocol::_onconnect);
  ofRemoveListener(onopenEvent,      this, &ofxWebSocketProtocol::_onopen);
  ofRemoveListener(oncloseEvent,     this, &ofxWebSocketProtocol::_onclose);
  ofRemoveListener(onidleEvent,      this, &ofxWebSocketProtocol::_onidle);
  ofRemoveListener(onmessageEvent,   this, &ofxWebSocketProtocol::_onmessage);
  ofRemoveListener(onbroadcastEvent, this, &ofxWebSocketProtocol::_onbroadcast);
}

//--------------------------------------------------------------
bool
ofxWebSocketProtocol::_allowClient(const std::string name,
                                   const std::string ip) const
{
  std::map<std::string, bool>::const_iterator allow_iter;
  
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
                                  const std::string ip) const
{
  return defaultAllowPolicy;
}

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onconnect(ofxWebSocketEvent& args)
{ onconnect(args); }  

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onopen(ofxWebSocketEvent& args)
{ onopen(args); }

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onclose(ofxWebSocketEvent& args)
{ onclose(args); }

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onidle(ofxWebSocketEvent& args)
{ onidle(args); }

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onmessage(ofxWebSocketEvent& args)
{
  args.message = args.conn.recv(args.message);
  //std::cout << args.message << std::endl;
  onmessage(args);
}

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onbroadcast(ofxWebSocketEvent& args)
{ onbroadcast(args); }

//--------------------------------------------------------------
void
ofxWebSocketProtocol::broadcast(const std::string& message)
{
  std::string buf(LWS_SEND_BUFFER_PRE_PADDING+1024+LWS_SEND_BUFFER_POST_PADDING, 0);
  unsigned char *p = (unsigned char*)&buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (reactor != NULL)
  {
    memcpy(p, message.c_str(), message.size());
    int n = libwebsockets_broadcast(&reactor->lws_protocols[idx], p, message.size());
    if (n < 0)
      fprintf(stderr, "ERROR writing to socket");
  }
}
