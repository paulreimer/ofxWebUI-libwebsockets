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

#include "Poco/Net/DNS.h"
#include "Poco/Net/SocketAddress.h"

template <class T>
class ofxWebUI
: public ofxWebSocketProtocol
{
public:
  ofxWebUI()
  : lastChangedTimestamp(0)
  , pb(NULL)
  {
    binary = true;
  }

  void setup(T& _pb)
  {  
    pb = &_pb;
    
    if (reactor != NULL)
    {
      std::string fqdn = "127.0.0.1";
      Poco::Net::HostEntry host = Poco::Net::DNS::thisHost();
      fqdn = host.name();

      url = "http://" + fqdn + ":" + ofToString(reactor->port) + "/";
    }
    
    ofRegisterURLNotification(this);
  }

  void launchBrowser()
  {
    ofLaunchBrowser(url);
  }

  void fetchQRcode(unsigned int size=320)
  {
    std::stringstream googleChartsQRurl;
    googleChartsQRurl
    << "http://chart.googleapis.com/chart?"
    << "chs=" << size << "x" << size << "&"
    << "cht=qr&"
    << "chld=L|1&"
    << "choe=UTF-8&"
    << "chl=" << url;

    //  std::cout << urlencode(url) << std::endl;
    ofLoadURLAsync(googleChartsQRurl.str(), "qrcode");
  }

  void urlResponse(ofHttpResponse& response)
  {
    if(response.status == 200 && response.request.name == "qrcode")
      QRcode.loadImage(response.data);
    else
      std::cout
      << response.status << " " << response.error
      << std::endl;

    ofUnregisterURLNotification(this);
  }

  T* pb;
  T lastChangedDiff;
  
  ofImage QRcode;
  unsigned long lastChangedTimestamp;
  std::string lastChangedIpAddress;

protected:
  void onopen(ofxWebSocketEvent& args)
  {
    if (pbSerialized.empty())
      pb->SerializeToString(&pbSerialized);

    args.conn.send(pbSerialized);
  }

  void onclose(ofxWebSocketEvent& args)
  {
    std::cout << "Connection closed" << std::endl;
  }
  
  void onmessage(ofxWebSocketEvent& args)
  {
    T pb_diff;
    
    if (pb_diff.ParseFromString(args.message))
    {
      lastChangedDiff = pb_diff;
      pb->MergeFrom(pb_diff);
      
      std::string _pbSerialized;
      pb->SerializeToString(&_pbSerialized);
      
      if (_pbSerialized != pbSerialized)
      {
        pbSerialized = _pbSerialized;
        broadcast(args.message);

        lastChangedTimestamp = ofGetSystemTime();
        
        int fd = libwebsocket_get_socket_fd(args.conn.ws);

        static char sockAddrBuffer[Poco::Net::SocketAddress::MAX_ADDRESS_LENGTH];
        struct sockaddr* _sockAddr = reinterpret_cast<struct sockaddr*>(sockAddrBuffer);
        poco_socklen_t sockAddrSize = sizeof(sockAddrBuffer);
        int rc = ::getpeername(fd, _sockAddr, &sockAddrSize);

        if (rc == 0)
        {
          Poco::Net::SocketAddress sockAddr(_sockAddr, sockAddrSize);
          Poco::Net::IPAddress ipAddr(sockAddr.host());
          lastChangedIpAddress = ipAddr.toString();
        }
      }
    }
  }

private:
  std::string url;
  std::string pbSerialized;
};

static const std::string urlencode(const std::string& url);
