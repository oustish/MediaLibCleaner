#include <iostream>
#include <time.h>

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void replaceAll(std::string&, const std::string&, const std::string&);
std::string get_date_iso_8601(time_t);
std::string get_date_rfc_2822(time_t);