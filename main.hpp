#include <iostream>
#include <memory> // for unique_ptr
#include <fstream> // for file opening
#include <streambuf> // for file opening
#include <ctime> // for time()
#include <map> // for std::multimap
#include <exception> // exceptions

#include <lua/lua.hpp>

#include <boost/filesystem.hpp>

#include "LuaFunctions.hpp"
#include "helpers.hpp"



void lua_error_reporting(lua_State*, int);
static std::string _ReplaceAllAliasOccurences(std::string&, std::unique_ptr<MediaLibCleaner::File>&);