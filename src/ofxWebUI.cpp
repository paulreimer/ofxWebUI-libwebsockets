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
#include "ofxWebSocketReactor.h"
#include "Poco/Net/DNS.h"

//--------------------------------------------------------------
ofxWebUI::ofxWebUI()
{
  binary = true;
}

//--------------------------------------------------------------
void
ofxWebUI::setup(protobuf::ui& _pb)
{  
  pb = &_pb;
  
  if (reactor != NULL)
  {
    std::string fqdn = Poco::Net::DNS::thisHost().name();
    url = "http://" + fqdn + ":" + ofToString(reactor->port) + "/";
  }

  ofRegisterURLNotification(this);
}

//--------------------------------------------------------------
void
ofxWebUI::onopen(ofxWebSocketEvent& args)
{
  if (pbSerialized.empty())
    pb->SerializeToString(&pbSerialized);

  args.conn.send(pbSerialized);
}

//--------------------------------------------------------------
void
ofxWebUI::onmessage(ofxWebSocketEvent& args)
{
  protobuf::ui pb_diff;

  if (pb_diff.ParseFromString(args.message))
  {
    pb->MergeFrom(pb_diff);

    std::string _pbSerialized;
    pb->SerializeToString(&_pbSerialized);

    if (_pbSerialized != pbSerialized)
    {
      pbSerialized = _pbSerialized;
      broadcast(args.message);
    }
  }
}

//--------------------------------------------------------------
void
ofxWebUI::onclose(ofxWebSocketEvent& args)
{
  std::cout << "Connection closed" << std::endl;
}

//--------------------------------------------------------------
void
ofxWebUI::fetchQRcode()
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
void
ofxWebUI::launchBrowser()
{
  ofLaunchBrowser(url);
}

//--------------------------------------------------------------
void
ofxWebUI::urlResponse(ofHttpResponse& response)
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
std::string
urlencode(const std::string& url)
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
