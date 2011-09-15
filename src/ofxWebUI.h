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

#include "ofxWebSocket.h"
#include "ui.pb.h"

class ofxWebUI
: public ofxWebSocketProtocol
{
public:
  ofxWebUI();
  void setup(protobuf::ui& _pb);

  void launchBrowser();
  void fetchQRcode();

  void urlResponse(ofHttpResponse& response);

  protobuf::ui* pb;
  
  ofImage QRcode;

protected:
  void onopen(ofxWebSocketEvent& args);
  void onclose(ofxWebSocketEvent& args);
  void onmessage(ofxWebSocketEvent& args);

private:
  std::string url;
  std::string pbSerialized;
};

static const std::string urlencode(const std::string& url);
