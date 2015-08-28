#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>
#include <taglib/tbytevector.h>

class Base64
{
public:
	static std::string encode(const std::vector<char>& buf);
	static std::string encode(const char* buf, int bufLen);
	static std::vector<char> decode(std::string encoded_string);
};

#endif