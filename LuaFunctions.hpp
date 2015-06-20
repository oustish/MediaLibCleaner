#include <lua/lua.hpp>
#include <iostream>
#include <memory>

#include "AudioFile.hpp"

int lua_IsAudioFile(lua_State *, std::unique_ptr<MediaLibCleaner::AudioFile>&);
int lua_SetTags(lua_State *, std::unique_ptr<MediaLibCleaner::AudioFile>&);
void lua_ErrorReporting(lua_State *, int);