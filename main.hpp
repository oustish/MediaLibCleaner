#include <iostream>
#include <memory> // for unique_ptr
#include <fstream> // for file opening
#include <streambuf> // for file opening
#include <ctime> // for time()
#include <map> // for std::multimap
#include <exception> // exceptions
#include <locale> // for setlocale()
#include <codecvt>

#include <lua/lua.hpp>

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include "LuaFunctions.hpp"
#include "helpers.hpp"

#include <Windows.h>


void lua_error_reporting(lua_State*, int);
std::wstring ReplaceAllAliasOccurences(std::wstring&, MediaLibCleaner::File*);
void process(std::wstring, std::unique_ptr<MediaLibCleaner::FilesAggregator>*, std::unique_ptr<MediaLibCleaner::LogProgram>*);
void scan(std::list<MediaLibCleaner::DFC*>* dfcl, std::list<boost::filesystem::path>* pathl, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la, std::string pth, std::unique_ptr<MediaLibCleaner::FilesAggregator>* fA, int* tf);