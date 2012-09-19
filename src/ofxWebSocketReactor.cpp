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

#include "Poco/File.h"
#include "Poco/FIFOEvent.h"
#include "Poco/URI.h"
#include "Poco/Path.h"

#include <iostream>
#include <sstream>

ofxWebSocketReactor* ofxWebSocketReactor::_instance = NULL;

//--------------------------------------------------------------
ofxWebSocketReactor::ofxWebSocketReactor()
: Runnable()
, context(NULL)
, waitMillis(50)
{}

//--------------------------------------------------------------
ofxWebSocketReactor::~ofxWebSocketReactor()
{
  exit();
}

//--------------------------------------------------------------
ofxWebSocketReactor& ofxWebSocketReactor::instance()
{
  if (_instance == NULL)
    _instance = new ofxWebSocketReactor();
  
  return *_instance;
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::registerProtocol(const std::string& name,
                                      ofxWebSocketProtocol& protocol)
{
  protocol.idx = protocols.size()+1; // "http" is protocol 0
  protocol.reactor = this;
  protocols.push_back(make_pair(name, &protocol));
}

//--------------------------------------------------------------
ofxWebSocketProtocol* const
ofxWebSocketReactor::protocol(const unsigned int idx)
{
  return (idx < protocols.size())? protocols[idx].second : NULL;
}

//--------------------------------------------------------------
void
ofxWebSocketReactor::close(ofxWebSocketConnection* const conn)
{
  if (conn != NULL && conn->ws != NULL)
    libwebsocket_close_and_free_session(context, conn->ws, LWS_CLOSE_STATUS_NORMAL);
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
      sslCertPath = Poco::Path(_sslCertFilename, Poco::Path::PATH_UNIX).absolute().toString();
    _sslCertPath = sslCertPath.c_str();

    if (_sslKeyFilename.at(0) == '/')
      sslKeyPath = _sslKeyFilename;
    else
      sslKeyPath = Poco::Path(_sslKeyFilename, Poco::Path::PATH_UNIX).absolute().toString();
    _sslKeyPath = sslKeyPath.c_str();
  }
  
  if (document_root.empty())
    //document_root = "web"; // finicky, used for Poco only so we can't use ofToDataPath here
    document_root = "../Resources/web";

  if (document_root.at(0) != '/')
    document_root = Poco::Path(document_root, Poco::Path::PATH_UNIX).absolute().toString();

  struct libwebsocket_protocols http_protocol = { "http", lws_callback, 0 };
  struct libwebsocket_protocols null_protocol = { NULL, NULL, 0 };

  lws_protocols.clear();
  lws_protocols.push_back(http_protocol);
  for (int i=0; i<protocols.size(); ++i)
  {
    struct libwebsocket_protocols lws_protocol = {
      protocols[i].first.c_str(),
      lws_callback,
      sizeof(ofxWebSocketConnection)
    };
    lws_protocols.push_back(lws_protocol);
  }
  lws_protocols.push_back(null_protocol);

  int opts = 0;
  context = libwebsocket_create_context(port, interface.c_str(),
                                        lws_protocols.data(),
                                        libwebsocket_internal_extensions,
                                        _sslCertPath, _sslKeyPath,
                                        -1, -1, opts);

	if (context == NULL)
    std::cerr << "libwebsocket init failed" << std::endl;
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
ofxWebSocketReactor::run()
{
  for (;;)
  {
    for (int i=0; i<protocols.size(); ++i)
      if (protocols[i].second != NULL)
        protocols[i].second->execute();

    libwebsocket_service(context, waitMillis);
  }
}

//--------------------------------------------------------------
unsigned int
ofxWebSocketReactor::_allow(ofxWebSocketProtocol* const protocol, const long fd)
{
  std::string client_ip(128, 0);
  std::string client_name(128, 0);

  libwebsockets_get_peer_addresses((int)fd,
                                   &client_name[0], client_name.size(),
                                   &client_ip[0], client_ip.size());
  return protocol->_allowClient(client_name, client_ip);
}

//--------------------------------------------------------------
unsigned int
ofxWebSocketReactor::_notify(ofxWebSocketConnection* const conn,
                             enum libwebsocket_callback_reasons const reason,
                             const char* const _message,
                             const unsigned int len)
{
  if (conn == NULL || conn->protocol == NULL)
    return 1;

  std::string message;
  if (_message != NULL && len > 0)
    message = std::string(_message, len);

  ofxWebSocketEvent args(*conn, message);

  if      (reason==LWS_CALLBACK_ESTABLISHED)
    conn->protocol->onopenEvent.notify(NULL, args);
  else if (reason==LWS_CALLBACK_CLOSED)
    conn->protocol->oncloseEvent.notify(NULL, args);
  else if (reason==LWS_CALLBACK_SERVER_WRITEABLE)
    conn->protocol->onidleEvent.notify(NULL, args);
  else if (reason==LWS_CALLBACK_BROADCAST)
    conn->protocol->onbroadcastEvent.notify(NULL, args);
  else if (reason==LWS_CALLBACK_RECEIVE)
    conn->protocol->onmessageEvent.notify(NULL, args);
  else if (reason==LWS_CALLBACK_HTTP)
    conn->protocol->onhttpEvent.notify(NULL, args);

  return 0;
}

//--------------------------------------------------------------
unsigned int
ofxWebSocketReactor::_http(struct libwebsocket *ws,
                           const char* const _url)
{
  std::string encodedUrl(_url? _url : "");
  std::string url;
  Poco::URI::decode(encodedUrl, url);
  std::stringstream responseStream;

  unsigned short responseCode = 400;
  if (!protocols.empty())
  {
    ofxWebSocketProtocol* defaultProtocol = protocol(0);
    if (defaultProtocol)
      responseCode = defaultProtocol->onhttp(url);
  }
  
  if (url == "/")
    url = "/index.html";

  url.erase(0, 1);
/*
  if (url == "/index.html")
  {
    std::string cdnIndexUrl = "http://cdn.p-rimes.net/ofxWebUI/index.html";
    responseStream
    << "HTTP/1.1 307 Temporary Redirect"  << "\x0d\x0a"
    << "Location: " << cdnIndexUrl        << "\x0d\x0a"
    << "Server: libwebsockets"            << "\x0d\x0a"
    << "Content-Length: 0"                << "\x0d\x0a"
    << "\x0d\x0a";
  }
  else
*/
  {
    Poco::Path root(document_root);
    std::string ext = url.substr(url.find_last_of(".")+1);
    std::string file = Poco::Path(root, url).toString();
    std::string mimetype = "text/html; charset=utf-8";

    if (ext == "ico")
      mimetype = "image/x-icon";
    if (ext == "manifest")
      mimetype = "text/cache-manifest";
    if (ext == "swf")
      mimetype = "application/x-shockwave-flash";
    if (ext == "js")
      mimetype = "application/javascript";
    if (ext == "png")
      mimetype = "image/png";

    if (responseCode >= 400 && Poco::File(Poco::Path(file)).exists())
    {
      if (!libwebsockets_serve_http_file(ws, file.c_str(), mimetype.c_str()))
        return 0;
      else {
        std::cerr
        << "Failed to send HTTP file " << file << " for " << url
        << std::endl;
      }
    }
    else {
      std::stringstream responseStream;
      responseStream
      << "HTTP/1.0 " << responseCode  << "\x0d\x0a"
      << "Server: libwebsockets"      << "\x0d\x0a"
      << "Content-Length: 0"          << "\x0d\x0a"
      << "\x0d\x0a";
    }
  }

  return libwebsocket_write(ws,
                            (unsigned char *)responseStream.str().c_str(),
                            responseStream.str().size(),
                            LWS_WRITE_HTTP);
}

extern "C"
int
lws_callback(struct libwebsocket_context* context,
             struct libwebsocket *ws,
             enum libwebsocket_callback_reasons reason,
             void *user,
             void *data, size_t len)
{
  const struct libwebsocket_protocols* lws_protocol = libwebsockets_get_protocol(ws);
  int idx = lws_protocol? lws_protocol->protocol_index : 0;

  ofxWebSocketReactor* const reactor = ofxWebSocketReactor::_instance;
  ofxWebSocketProtocol* const protocol = reactor->protocol(idx-1);
  ofxWebSocketConnection** const conn_ptr = (ofxWebSocketConnection**)user;
  ofxWebSocketConnection* conn;

  if (reactor != NULL)
  {
    if (reason == LWS_CALLBACK_ESTABLISHED)
    {
      *conn_ptr = new ofxWebSocketConnection(reactor, protocol, false); //supportsBinary
    }
    else if (reason == LWS_CALLBACK_CLOSED)
      if (*conn_ptr != NULL)
        delete *conn_ptr;

    switch (reason)
    {
      case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        //TODO: what are the use cases for this callback?
        //1:
        return 0;

      case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        if (protocol != NULL)
          return reactor->_allow(protocol, (int)(long)user)? 0 : 1;
        else
          return 0;

      case LWS_CALLBACK_HTTP:
        return reactor->_http(ws, (char*)data);

      case LWS_CALLBACK_ESTABLISHED:
      case LWS_CALLBACK_CLOSED:
      case LWS_CALLBACK_SERVER_WRITEABLE:
      case LWS_CALLBACK_RECEIVE:
      case LWS_CALLBACK_BROADCAST:
        conn = *(ofxWebSocketConnection**)user;
        if (conn && conn->ws != ws)
          conn->ws = ws;

        return reactor->_notify(conn, reason, (char*)data, len);
        
      default:
        return 0;
    }
  }

  return 1; // FAIL (e.g. unhandled case/break in switch)
}
