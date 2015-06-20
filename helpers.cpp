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


std::string get_date_iso_8601(time_t t) {
	using namespace boost::posix_time;

	std::string ss = "";

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

std::string get_date_rfc_2822(time_t t) {
	using namespace boost::posix_time;

	std::string ss = "";

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