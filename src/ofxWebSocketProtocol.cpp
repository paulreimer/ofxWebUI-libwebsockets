/*
 * Copyright Paul Reimer, 2012
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc/3.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "ofxWebSocket.h"

#include "Poco/Delegate.h"

//--------------------------------------------------------------
ofxWebSocketProtocol::ofxWebSocketProtocol()
: defaultAllowPolicy(true)
{
  onconnectEvent   += Poco::delegate(this, &ofxWebSocketProtocol::_onconnect);
  onopenEvent      += Poco::delegate(this, &ofxWebSocketProtocol::_onopen);
  oncloseEvent     += Poco::delegate(this, &ofxWebSocketProtocol::_onclose);
  onidleEvent      += Poco::delegate(this, &ofxWebSocketProtocol::_onidle);
  onmessageEvent   += Poco::delegate(this, &ofxWebSocketProtocol::_onmessage);
  onbroadcastEvent += Poco::delegate(this, &ofxWebSocketProtocol::_onbroadcast);
}

//--------------------------------------------------------------
ofxWebSocketProtocol::~ofxWebSocketProtocol()
{
  onconnectEvent   -= Poco::delegate(this, &ofxWebSocketProtocol::_onconnect);
  onopenEvent      -= Poco::delegate(this, &ofxWebSocketProtocol::_onopen);
  oncloseEvent     -= Poco::delegate(this, &ofxWebSocketProtocol::_onclose);
  onidleEvent      -= Poco::delegate(this, &ofxWebSocketProtocol::_onidle);
  onmessageEvent   -= Poco::delegate(this, &ofxWebSocketProtocol::_onmessage);
  onbroadcastEvent -= Poco::delegate(this, &ofxWebSocketProtocol::_onbroadcast);
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

void
ofxWebSocketProtocol::onconnect(ofxWebSocketEvent& args)
{}

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onopen(ofxWebSocketEvent& args)
{ onopen(args); }

void
ofxWebSocketProtocol::onopen(ofxWebSocketEvent&args)
{}

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onclose(ofxWebSocketEvent& args)
{ onclose(args); }

void
ofxWebSocketProtocol::onclose(ofxWebSocketEvent&args)
{}

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onidle(ofxWebSocketEvent& args)
{ onidle(args); }

void
ofxWebSocketProtocol::onidle(ofxWebSocketEvent&args)
{}

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onmessage(ofxWebSocketEvent& args)
{
  args.message = args.conn.recv(args.message);
  onmessage(args);
}

void
ofxWebSocketProtocol::onmessage(ofxWebSocketEvent&args)
{}

//--------------------------------------------------------------
void
ofxWebSocketProtocol::_onbroadcast(ofxWebSocketEvent& args)
{ onbroadcast(args); }

void
ofxWebSocketProtocol::onbroadcast(ofxWebSocketEvent&args)
{ args.conn.send(args.message); }

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

unsigned short
ofxWebSocketProtocol::onhttp(const std::string& url)
{
  return 400;
}
