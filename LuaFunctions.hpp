/**
* @file
* @author Szymon Oracki <szymon.oracki@oustish.pl>
* @version 0.1
*
* This file contains declarations of all functions used as lua functions (via lua_register() function)
*/
#include <lua/lua.hpp>
#include <iostream>
#include <memory>

#include "MediaLibCleaner.hpp"

int lua_IsAudioFile(lua_State *, MediaLibCleaner::File*);
int lua_SetTags(lua_State *, MediaLibCleaner::File*);
void lua_ErrorReporting(lua_State *, int);