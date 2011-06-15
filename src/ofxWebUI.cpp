/*
 * Copyright Paul Reimer, 2012
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc/3.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "ofxWebUI.h"

//--------------------------------------------------------------
ofxWebUI::ofxWebUI()
: http_server(_http_server)
, websocket_server(_websocket_server)
{}

//--------------------------------------------------------------
void ofxWebUI::setup(protobuf::ui& _pb)
{
  http_server.setup();  
  //  websocket_server.port = "";
  websocket_server.setup();

  pb = &_pb;
  ofAddListener(ofxWebSocketServer::onopen, this, &ofxWebUI::onopen);
  ofAddListener(ofxWebSocketServer::onclose, this, &ofxWebUI::onclose);
  ofAddListener(ofxWebSocketServer::onmessage, this, &ofxWebUI::onmessage);
  
  url = "http://beeswax.local:7681/index.html";
  ofRegisterURLNotification(this);
}

//--------------------------------------------------------------
void ofxWebUI::onopen(ofxWebSocketServer::WebSocketEventArgs& args)
{
  if (pbSerialized.empty())
    pb->SerializeToString(&pbSerialized);

  websocket_server.send(args.ws, pbSerialized);
}

//--------------------------------------------------------------
void ofxWebUI::onmessage(ofxWebSocketServer::WebSocketEventArgs& args)
{
  protobuf::ui pb_diff;
  pb_diff.ParseFromString(args.message);
  pb->MergeFrom(pb_diff);

  std::string _pbSerialized;
  pb->SerializeToString(&_pbSerialized);

  if (_pbSerialized != pbSerialized)
  {
    pbSerialized = _pbSerialized;
    websocket_server.broadcast(args.message);
  }
}

//--------------------------------------------------------------
void ofxWebUI::onclose(ofxWebSocketServer::WebSocketEventArgs& args)
{  
}

//--------------------------------------------------------------
void ofxWebUI::fetchQRcode()
{
  std::stringstream googleChartsQRurl;
  googleChartsQRurl
  << "http://chart.googleapis.com/chart?"
  << "chs=320x320&"
  << "cht=qr&"
  << "chld=L|1&"
  << "choe=UTF-8&"
  << "chl=" << url;
  
  //  std::cout << urlencode(url) << std::endl;
  ofLoadURLAsync(googleChartsQRurl.str(), "qrcode");
}

//--------------------------------------------------------------
void ofxWebUI::launchBrowser()
{
  ofLaunchBrowser(url);
}

//--------------------------------------------------------------
void ofxWebUI::urlResponse(ofHttpResponse& response)
{
  if(response.status == 200 && response.request.name == "qrcode")
    QRcode.loadImage(response.data);
  else
    std::cout
    << response.status << " " << response.error
    << std::endl;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
std::string urlencode(const std::string& url)
{
  std::stringstream ss;
  static std::string okchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-~";
  size_t from = 0;
  size_t to = 0;
  
  while (from < url.size() && to != std::string::npos)
  {
    to = url.find_first_not_of(okchars, from);
    if (to == std::string::npos)
    {
      ss << url.substr(from, url.size()-from);
    }
    else {
      ss << url.substr(from, to-from);
      ss << "%" << std::hex << (unsigned int)url.at(to);
      from = to+1;
    }
  }
  return ss.str();
}
