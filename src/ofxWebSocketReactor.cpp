/*
 * Copyright Paul Reimer, 2012
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc/3.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "ofxWebSocketReactor.h"

#include "ofUtils.h"

#include <sys/time.h>
#include "decode.h"

//--------------------------------------------------------------
ofxWebSocketReactor::ofxWebSocketReactor()
: context(NULL)
, buf(LWS_SEND_BUFFER_PRE_PADDING+1024+LWS_SEND_BUFFER_POST_PADDING)
, waitMillis(50)
{}

//--------------------------------------------------------------
ofxWebSocketReactor::~ofxWebSocketReactor()
{
  exit();
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::registerProtocol(const std::string name,
                                      ofxWebSocketProtocol* protocol)
{
  if (protocol != NULL)
  {
    char* _name = (char*)malloc(name.size()+1);
    memcpy(_name, name.c_str(), name.size()+1);
    struct libwebsocket_protocols _protocol = {
      _name,
      lws_callback,
      sizeof(protocol->session)
    };
    _protocols.push_back(_protocol);

    protocol->idx = protocols.size();
    protocols.push_back(protocol);
  }
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::setup(const short _port,
                           const std::string _sslCertFilename,
                           const std::string _sslKeyFilename)
{
  port = _port;  
  bool useSSL = (!_sslCertFilename.empty() && !_sslKeyFilename.empty());

  std::string sslCertPath, sslKeyPath;
  const char *_sslCertPath = NULL;
  const char *_sslKeyPath = NULL;
  if (useSSL)
  {
    if (_sslCertFilename.at(0) == '/')
      sslCertPath = _sslCertFilename;
    else
      sslCertPath = ofToDataPath(_sslCertFilename, true);
    _sslCertPath = sslCertPath.c_str();

    if (_sslKeyFilename.at(0) == '/')
      sslKeyPath = _sslKeyFilename;
    else
      sslKeyPath = ofToDataPath(_sslKeyFilename, true);
    _sslKeyPath = sslKeyPath.c_str();
  }
  int opts = 0;
  context = libwebsocket_create_context(port, interface.c_str(), &_protocols[0],
                                        libwebsocket_internal_extensions,
                                        _sslCertPath, _sslKeyPath,
                                        -1, -1, opts);

	if (context == NULL)
    std::cerr << "libwebsocket init failed" << std::endl;
  else
    startThread(true, false); // blocking, non-verbose
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::exit()
{
  if (context != NULL)
  {
    libwebsocket_context_destroy(context);
    context = NULL;
  }
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::threadedFunction()
{
  while (isThreadRunning())
  {
    for (int i=0; i<protocols.size(); ++i)
      protocols[i]->execute();

    libwebsocket_service(context, waitMillis);
  }
}

//--------------------------------------------------------------
unsigned int
ofxWebSocketReactor::_notify(unsigned int idx,
                             enum libwebsocket_callback_reasons reason,
                             libwebsocket* const ws,
                             void* const session,
                             const char* const message,
                             const unsigned int len) const
{
  if (idx >= protocols.size())
    return 1;

  ofxWebSocketEventArgs args;
  args.ws = ws;
  args.session = session;
  args.message = std::string(message? message:"", len);

  std::string client_ip(128, 0);
  std::string client_name(128, 0);

  ofxWebSocketProtocol* protocol = protocols[idx];

	switch (reason)
  {
    case LWS_CALLBACK_ESTABLISHED:
      ofNotifyEvent(protocol->onopenEvent, args);
      break;

    case LWS_CALLBACK_CLOSED:
      ofNotifyEvent(protocol->oncloseEvent, args);
      break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
      ofNotifyEvent(protocol->onidleEvent, args);
      break;

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
      libwebsockets_get_peer_addresses((int)(long)session,
                                       (char*)&client_name[0], client_name.size(),
                                       (char*)&client_ip[0], client_ip.size());
      
      return protocol->_allowClient(client_name, client_ip)? 0 : 1;
      break;

    case LWS_CALLBACK_HTTP:
      ofNotifyEvent(protocol->httprequestEvent, args);      
      break;

    default:
      break;
	}
  return 0;
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::close(libwebsocket* const ws) const
{
  libwebsocket_close_and_free_session(context, ws, LWS_CLOSE_STATUS_NORMAL);
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::broadcast(const std::string& message,
                               unsigned int protocol_idx)
{
  broadcast(message.c_str(), message.size(), protocol_idx);
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::broadcast(const char* const message,
                               unsigned int len,
                               unsigned int protocol_idx)
{
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
  
  if (protocol_idx < _protocols.size())
  {
    memcpy(p, message, len);
    int n = libwebsockets_broadcast(&_protocols[protocol_idx], p, len);
    if (n < 0) {
      fprintf(stderr, "ERROR writing to socket");
      return;
    }
  }
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::send(libwebsocket* const ws,
                          const std::string& message,
                          bool binary)
{
  send(ws, message.c_str(), message.size(), binary);
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::send(libwebsocket* const ws,
                          const char* const message,
                          unsigned int len,
                          bool binary)
{
  int n = 0;
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  if (binary)
  {
    //TODO: when libwebsockets has an API supporting something this, we should use it
    if (1) // && ws->supportBinary, for example
    {
      memcpy(p, message, len);
      n = libwebsocket_write(ws, p, len, LWS_WRITE_BINARY);
    }
    else {
      int encoded_len;
      encoded_len = lws_b64_encode_string((char*)message, len, (char*)p, buf.size());
      if (encoded_len > 0)
        n = libwebsocket_write(ws, p, encoded_len, LWS_WRITE_TEXT);
    }
  }
  else {
    memcpy(p, message, len);
    n = libwebsocket_write(ws, p, len, LWS_WRITE_TEXT);
  }

  if (n < 0)
    std::cout << "ERROR writing to socket" << std::endl;
}

//--------------------------------------------------------------
unsigned int b64_decode_string(const char* message,
                               char* decoded,
                               const unsigned int len)
{
  std::stringstream encoded_stream(std::string(message, len));
  std::stringstream decoded_stream;
  base64::decoder decoder;

  decoder.decode(encoded_stream, decoded_stream);

  const std::string& _decoded = decoded_stream.str();
  memcpy(decoded, _decoded.c_str(), _decoded.size());

  return _decoded.size();
}

extern "C"
int
lws_callback(struct libwebsocket_context* context,
             struct libwebsocket *ws,
             enum libwebsocket_callback_reasons reason,
             void *session,
             void *_message, size_t len)
{
  const struct libwebsocket_protocols* protocol = libwebsockets_get_protocol(ws);
  int idx = protocol? protocol->protocol_index : 0;
  char* message = (char*)_message;
  static char* decoded;
  bool binary = true; // TODO:session? ((ofxWebSocketProtocol*)session)->binary : NULL;
  //TODO: when libwebsockets has an API supporting something this, we should use it
  if (reason == LWS_CALLBACK_RECEIVE && binary) // && !ws->supportBinary, for example
  {
    realloc(decoded, len);
    //TODO: libwebsockets base64 decode is broken @2011-06-19
    //len = lws_b64_decode_string(message, decoded, len);
    len = b64_decode_string(message, decoded, len);
    message = decoded;
  }

  if (reason == LWS_CALLBACK_BROADCAST)
  {
    _websocket_reactor.send(ws, message, len, binary);
    return 0;
  }

  return _websocket_reactor._notify(idx, reason, ws, session, message, len);
}
