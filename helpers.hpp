/**
@file
@author Szymon Oracki <szymon.oracki@oustish.pl>
@version 0.4

This file contains declarations of all helper functions
*/

#include <iostream>
#include <time.h>
#include <oaidl.h>
#include <stdlib.h>

#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/locale.hpp>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

void replaceAll(std::string&, const std::string&, const std::string&);
void replaceAll(std::wstring&, const std::wstring&, const std::wstring&);
std::string get_date_iso_8601(time_t);
std::string get_date_rfc_2822(time_t);
std::wstring get_date_iso_8601_wide(time_t);
std::wstring get_date_rfc_2822_wide(time_t);
std::wstring s2ws(const std::string&);
std::string ws2s(const std::wstring&);