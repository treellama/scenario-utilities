/* macroman.cpp

   Copyright (C) 2008 by Gregory Smith
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   This license is contained in the file "COPYING", which is included
   with this source code; it is available online at
   http://www.gnu.org/licenses/gpl.html
   
*/

/*
  MacRoman character conversion utilities
*/

#include "macroman.h"

#include <cstdint>
#include <map>

class MacRomanUnicodeConverter
{
public:
	MacRomanUnicodeConverter();
	inline uint16_t ToUnicode(char c) {
		return mac_roman_to_unicode_table[(unsigned char) c];
	}

	inline char ToMacRoman(uint16_t c) {
		if (c <= 0x7f) return c;
		std::map<uint16_t, char>::iterator it = unicode_to_mac_roman.find(c);
		if (it != unicode_to_mac_roman.end())
			return it->second;
		else
			return '?';
	}

private:

	static uint16_t mac_roman_to_unicode_table[256];
	static std::map<uint16_t, char> unicode_to_mac_roman;
	static bool initialized;
};

// from ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/APPLE/ROMAN.TXT
uint16_t MacRomanUnicodeConverter::mac_roman_to_unicode_table[256] = {
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 
	0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F, 
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 
	0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
	0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F, 
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
	0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, 
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
	0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
	0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F, 
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
	0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
	0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F, 
	0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1, 
	0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8, 
	0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3, 
	0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC, 
	0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF, 
	0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8, 
	0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211, 
	0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8, 
	0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB, 
	0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153, 
	0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA, 
	0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02, 
	0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1, 
	0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4, 
	0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC, 
	0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7 
};

std::map<uint16_t, char> MacRomanUnicodeConverter::unicode_to_mac_roman;
bool MacRomanUnicodeConverter::initialized = false;

MacRomanUnicodeConverter::MacRomanUnicodeConverter()
{
	if (!initialized)
	{
		for (unsigned char c = 0x80; c < 0xff; c++)
		{
			unicode_to_mac_roman[mac_roman_to_unicode_table[c]] = (char) c;
		}
		initialized = true;
	}
}

static MacRomanUnicodeConverter macRomanUnicodeConverter;

void mac_roman_to_unicode(const char *input, uint16_t *output)
{
	const char *p = input;

	while (*p)
	{
		*output++ = macRomanUnicodeConverter.ToUnicode(*p++);
	}
	*output = 0x0;
}

void mac_roman_to_unicode(const char *input, uint16_t *output, int max_len)
{
	const char *p = input;

	while (*p && --max_len > 0)
	{
		*output++ = macRomanUnicodeConverter.ToUnicode(*p++);
	}
	*output = 0x0;
}

uint16_t mac_roman_to_unicode(char c)
{
	return macRomanUnicodeConverter.ToUnicode(c);
}

char unicode_to_mac_roman(uint16_t c)
{
	return macRomanUnicodeConverter.ToMacRoman(c);
}

static void unicode_to_utf8(uint16_t c, std::string& output)
{
	if (c < 0x80)
	{
		output += c;
	}
	else if (c < 0x800) 
	{
		output += (0xc0 | (c >> 6));
		output += (0x80 | (c & 0x3f));
	}
	else 
	{
		output += (0xe0 | (c >> 12));
		output += (0x80 | ((c >> 6) & 0x3f));
		output += (0x80 | (c & 0x3f));
	}
}

static uint16_t utf8_to_unicode(const char *s, int &chars_used)
{
	const unsigned char *input = (unsigned char *) s;
	if ((unsigned char) input[0] < 0x80)
	{
		chars_used = 1;
		return input[0];
	}
	else if ((unsigned char) input[0] < 0xe0)
	{
		chars_used = 2;
		return ((input[0] & 0x1f) << 6) | (input[1] & 0x3f);
	}
	else
	{
		chars_used = 3;
		return ((input[0] & 0xf) << 12) | ((input[1] & 0x3f) << 6) | (input[2] & 0x3f);
	}
}

std::string mac_roman_to_utf8(const std::string& input)
{
	const char *p = input.data();
	std::string output;
	while (*p)
	{
		uint16_t uc = mac_roman_to_unicode(*p++);
		unicode_to_utf8(uc, output);
	}

	return output;
}

std::string utf8_to_mac_roman(const std::string& input)
{
	const char *p = input.data();
	std::string output;
	while (*p)
	{
		int advance = 0;
		uint16_t uc = utf8_to_unicode(p, advance);
		output += unicode_to_mac_roman(uc);
		p += advance;
	}

	return output;
}
