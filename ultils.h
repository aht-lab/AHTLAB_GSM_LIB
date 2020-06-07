/**
 * cafe5hsang@gmail.com - mita9497dev@gmail.com
 * ULTILS
 */

#ifndef _ULTILS_H__
#define _ULTILS_H__

#include <Arduino.h>

bool        Read_VARS       (const char* pattern, const char * data, ...);
void        convertUCS2     (const char* ucs2, char* text);
void        utf8tohex       (const char* utf8, char* hex);

#endif
