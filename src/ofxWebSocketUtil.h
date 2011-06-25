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

//--------------------------------------------------------------
unsigned int b64_decode_string(const char* message,
                               char* decoded,
                               const unsigned int len);

//--------------------------------------------------------------
unsigned int b64_encode_string(const char* message,
                               const unsigned int len,
                               char* encoded,
                               const unsigned int max_encoded_len);
