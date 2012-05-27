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

#include "Poco/URI.h"
#include "Poco/String.h"
#include "Poco/StringTokenizer.h"

#include <map>

//--------------------------------------------------------------
unsigned int b64_decode_string(const char* message,
                               char* decoded,
                               const unsigned int len);

//--------------------------------------------------------------
unsigned int b64_encode_string(const char* message,
                               const unsigned int len,
                               char* encoded,
                               const unsigned int max_encoded_len);


typedef std::map<std::string, std::string> QueryStringArgs;
typedef std::multimap<std::string, std::string> QueryStringArgsMulti;

//--------------------------------------------------------------
template<class MapT=QueryStringArgs>
MapT parseQueryString(const std::string& qs)
{
  MapT args;
  
  const std::string argDelimeter("&");
  const std::string keyDelimiter("=");

  Poco::StringTokenizer qsTokenizer(qs, argDelimeter);
  Poco::StringTokenizer::Iterator arg_iter;
  for (arg_iter=qsTokenizer.begin(); arg_iter!=qsTokenizer.end(); ++arg_iter)
  {
    const std::string& arg(*arg_iter);

    std::string kRaw;
    std::string vRaw;

    size_t delimiterIdx = arg.find(keyDelimiter);
    if (delimiterIdx == std::string::npos)
    {
      kRaw = arg;
      vRaw.clear();
    }
    else {
      kRaw = arg.substr(0, delimiterIdx);
      vRaw = arg.substr(delimiterIdx+1);
    }
/*
    Poco::StringTokenizer kvTokenizer(arg, keyDelimiter);
    const std::string& kRaw(kvTokenizer[0]);
    std::string vRaw;
    for (size_t i=1; i<kvTokenizer.count(); ++i)
      vRaw += kvTokenizer[i];
*/
    std::string k,v;
    Poco::URI::decode(kRaw, k);
    Poco::replaceInPlace(k, "+", " ");

    Poco::URI::decode(vRaw, v);
    Poco::replaceInPlace(v, "+", " ");

    args.insert(make_pair(k, v));
  }
  
  return args;
}
