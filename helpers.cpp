/**
@file
@author Szymon Oracki <szymon.oracki@oustish.pl>
@version 1.0.0

This file contains definitions of all helper functions
*/
#include "helpers.hpp"

/**
* Function replacing every occurence of one string with another in given string.
*
* This function allows to replace all occurences of one string with another string,
* all within another string.
*
* @param[in,out]	str	  String in which changes will be made. Please note argument is passed as referrence and will contain output!
* @param[in]	from  String which will be replaced with another string
* @param[in]	to	  String which will replace previous string
*/
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

/**
* Function replacing every occurence of one std::wstring with another in given std::wstring.
*
* This function allows to replace all occurences of one std::wstring with another std::wstring,
* all within another std::wstring.
*
* @param[in,out]	str	  std::wstring in which changes will be made. Please note argument is passed as referrence and will contain output!
* @param[in]	from  std::wstring which will be replaced with another string
* @param[in]	to	  std::wstring which will replace previous string
*/
void replaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

/**
* Function processing given timestamp and returning date in ISO-8601 compliant std::string 
*
* @param[in] t  Timestamp to be changed into ISO-8601 compliant date format as std::string
*
* @return ISO-8601 compliant std::string of given timestamp
*/
std::string get_date_iso_8601(time_t t) {
	using namespace boost::posix_time;

	std::string ss;

	char bufor[20];
	tm czas;
	localtime_s(&czas, &t);

	strftime(bufor, 20, "%Y-%m-%dT%H:%M", &czas);
	ss = bufor;


	// +02:00 or other
	typedef boost::date_time::c_local_adjustor<ptime> local_adj;

	const ptime utc_now = second_clock::universal_time();
	const ptime now = local_adj::utc_to_local(utc_now);

	int h = (now - utc_now).hours();
	int m = (now - utc_now).minutes();

	if (h < 0) {
		ss += "-";
	}
	else {
		ss += "+";
	}

	if (h < 10)
		ss += "0";

	ss += std::to_string(std::abs(h));

	ss += ":";

	if (m < 10)
		ss += "0";

	ss += std::to_string(m);

	return ss;
}

/**
* Function processing given timestamp and returning date in RFC2822 compliant std::string
*
* @param[in] t  Timestamp to be changed into RFC2822 compliant date format as std::string
*
* @return RFC2822 compliant std::string of given timestamp
*/
std::string get_date_rfc_2822(time_t t) {
	using namespace boost::posix_time;

	std::string ss;

	char bufor[31];
	struct tm czas;
	localtime_s(&czas, &t);

	strftime(bufor, 31, "%a, %d %b %Y %X", &czas);

	ss = bufor;

	// +0200 ot other
	typedef boost::date_time::c_local_adjustor<ptime> local_adj;

	const ptime utc_now = second_clock::universal_time();
	const ptime now = local_adj::utc_to_local(utc_now);

	int h = (now - utc_now).hours();
	int m = (now - utc_now).minutes();

	if (h < 0) {
		ss += "-";
	}
	else {
		ss += "+";
	}

	if (h < 10)
		ss += "0";

	ss += std::to_string(std::abs(h));

	if (m < 10)
		ss += "0";

	ss += std::to_string(m);

	return ss;
}

/**
* Function processing given timestamp and returning date in ISO-8601 compliant std::string
*
* @param[in] t  Timestamp to be changed into ISO-8601 compliant date format and returned as std::wstring
*
* @return ISO-8601 compliant std::wstring of given timestamp
*/
std::wstring get_date_iso_8601_wide(time_t t) {
	return s2ws(get_date_iso_8601(t));
}

/**
* Function processing given timestamp and returning date in IRFC2822 compliant std::string
*
* @param[in] t  Timestamp to be changed into RFC2822 compliant date format and returned as std::wstring
*
* @return RFC2822 compliant std::wstring of given timestamp
*/
std::wstring get_date_rfc_2822_wide(time_t t) {
	return s2ws(get_date_rfc_2822(t));
}

/**
* Function to change encoding of given wide char string and convert it to normal std::string
*
* @param[in] win  Input wide string to be converted
*
* @return std::string representation of wide string given as parameter
*/
std::string ws2s(const std::wstring& win)
{
	int len;
	int slength = static_cast<int>(win.length()) + 1;
	len = WideCharToMultiByte(CP_ACP, 0, win.c_str(), slength, 0, 0, 0, 0);
	char* buf = new char[len];
	WideCharToMultiByte(CP_ACP, 0, win.c_str(), slength, buf, len, 0, 0);
	std::string r(buf);
	delete[] buf;
	return r;
}

/**
* Function to change encoding of given multi byte string and convert it to wide string
*
* @param[in] in  Input string to be converted
*
* @return std::wstring representation of normal string given as parameter
*/
std::wstring s2ws(const std::string& in)
{
	int len;
	int slength = static_cast<int>(in.length()) + 1;
	len = MultiByteToWideChar(CP_ACP, 0, in.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, in.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;

	
}