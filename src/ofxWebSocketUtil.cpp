/*
 * Copyright Paul Reimer, 2012
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-nc/3.0/
 * or send a letter to
 * Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"

#include <sstream>
#include <string.h>

//--------------------------------------------------------------
unsigned int b64_decode_string(const char* message,
                               char* decoded,
                               const unsigned int len)
{
  std::istringstream preencoded_stream(std::string(message, len));
  Poco::Base64Decoder decoder(preencoded_stream);
  std::string _decoded;
  decoder >> _decoded;

  memcpy(decoded, _decoded.c_str(), _decoded.size());

  return _decoded.size();
}

//--------------------------------------------------------------
unsigned int b64_encode_string(const char* message,
                               const unsigned int len,
                               char* encoded,
                               const unsigned int max_encoded_len)
{
  std::ostringstream encoded_stream;
  Poco::Base64Encoder encoder(encoded_stream);
  encoder << std::string(message, len);
  encoder.close();

  const std::string& _encoded = encoded_stream.str();
  if (_encoded.size() <= max_encoded_len)
    memcpy(encoded, _encoded.c_str(), _encoded.size());

  return _encoded.size();
}
