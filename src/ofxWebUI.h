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

//#include "ofUtils.h"

#include "ofxWebSocket.h"

#include "Poco/Timestamp.h"
#include "Poco/URI.h"
#include "Poco/Net/DNS.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/FIFOEvent.h"

#include <iostream>

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
  }

  T* pb;
  T lastChangedDiff;
  
  unsigned long lastChangedTimestamp;
  std::string lastChangedIpAddress;

  Poco::FIFOEvent<T> onchange;

protected:
  void onopen(ofxWebSocketEvent& args)
  {
    if (pbSerialized.empty())
      pb->SerializeToString(&pbSerialized);

    args.conn.send(pbSerialized);
  }

  void onclose(ofxWebSocketEvent& args)
  {
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

        Poco::Timestamp now;
        lastChangedTimestamp = now.epochMicroseconds() * 1000.0;
        
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

        onchange.notify(NULL, pb_diff);
      }
    }
  }

private:
  std::string pbSerialized;
};

