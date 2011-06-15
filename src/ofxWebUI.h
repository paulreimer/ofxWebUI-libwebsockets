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

#include "ofImage.h"
#include "ofUtils.h"
#include "ofURLFileLoader.h"

#include "ofxWebSocketServer.h"
#include "ofxHttpServer.h"

#include "ui.pb.h"

class ofxWebUI
{
public:
  ofxWebUI();
  void setup(protobuf::ui& _pb);

  void launchBrowser();
  void fetchQRcode();

  void urlResponse(ofHttpResponse& response);

  ofxWebSocketServer& websocket_server;
  ofxHttpServer& http_server;

  protobuf::ui* pb;
  
  ofImage QRcode;

private:
  std::string url;
  std::string pbSerialized;

  void onopen(ofxWebSocketServer::WebSocketEventArgs& args);
  void onclose(ofxWebSocketServer::WebSocketEventArgs& args);
  void onmessage(ofxWebSocketServer::WebSocketEventArgs& args);

};

static std::string urlencode(const std::string& url);
