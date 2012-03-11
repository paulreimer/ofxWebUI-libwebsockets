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
//#include "Poco/URI.h"

//--------------------------------------------------------------
//--------------------------------------------------------------
static const std::string
urlencode(const std::string& url)
{
  static std::string okchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-~";
//  std::string encodedUrl;
//  Poco::URI::encode(url, okchars, encodedUrl);
//  return encodedUrl;

  std::stringstream ss;
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
