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
#include "ofxWebSocketUtil.h"

//--------------------------------------------------------------
ofxWebSocketConnection::ofxWebSocketConnection(ofxWebSocketReactor* const _reactor,
                                               ofxWebSocketProtocol* const _protocol,
                                               const bool _supportsBinary)
: reactor(_reactor)
, protocol(_protocol)
, ws(NULL)
, session(NULL)
, supportsBinary(_supportsBinary)
, buf(LWS_SEND_BUFFER_PRE_PADDING+1024+LWS_SEND_BUFFER_POST_PADDING)
{
  if (_protocol != NULL)
    binary = _protocol->binary;
}

//--------------------------------------------------------------
void
ofxWebSocketConnection::close()
{
  if (reactor != NULL)
    reactor->close(this);
}

//--------------------------------------------------------------
void
ofxWebSocketConnection::send(const std::string& message)
{
  int n = 0;
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (binary)
  {
    //TODO: when libwebsockets has an API supporting something this, we should use it
    if (supportsBinary)
    {
      memcpy(p, message.c_str(), message.size());
      n = libwebsocket_write(ws, p, message.size(), LWS_WRITE_BINARY);
    }
    else {
      int encoded_len;
      encoded_len = b64_encode_string(message.c_str(), message.size(), (char*)p, buf.size());
      if (encoded_len > 0)
        n = libwebsocket_write(ws, p, encoded_len, LWS_WRITE_TEXT);
    }
  }
  else {
    memcpy(p, message.c_str(), message.size());
    n = libwebsocket_write(ws, p, message.size(), LWS_WRITE_TEXT);
  }
  
  if (n < 0)
    std::cout << "ERROR writing to socket" << std::endl;
}

//--------------------------------------------------------------
const std::string
ofxWebSocketConnection::recv(const std::string& message)
{
  std::string decoded = message;

  //TODO: when libwebsockets has an API
  // to detect binary support, we should use it
  if (binary && !supportsBinary)
  {
    //TODO: libwebsockets base64 decode is broken @2011-06-19
    //len = lws_b64_decode_string(message, decoded, len);
    int decoded_len = b64_decode_string(message.c_str(), &decoded[0], message.size());
    decoded.resize(decoded_len);
  }

  return decoded;
}

