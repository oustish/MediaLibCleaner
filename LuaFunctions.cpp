#include "LuaFunctions.hpp"

int lua_IsAudioFile(lua_State *L, std::unique_ptr<MediaLibCleaner::AudioFile>& _audiofile) {
	// check if file is na audio file
	std::string _ext = "mp3";
	std::string _path = _audiofile->GetPathname();

	lua_pushboolean(L, (0 == _path.compare(_path.length() - _ext.length(), _ext.length(), _ext)));

	return 1; // numer of output arguments
}

int lua_SetTags(lua_State *L, std::unique_ptr<MediaLibCleaner::AudioFile>& _audiofile) {
	int n = lua_gettop(L); // argc for function

	if (n % 2 == 1 && n > 0) { // requiers odd, positive amount of arguments
		lua_pushboolean(L, false);
		std::cerr << "[LUAPROC] _SetTags(): function requires odd and positive amout of arguments." << std::endl;
		return 1;
	}

	for (int i = 1; i <= n; i += 2) {
		// debug
		std::cout << "Setting tag: '" << lua_tostring(L, i) << "' to value: '" << lua_tostring(L, i + 1) << "'" << std::endl;
	}

	lua_pushboolean(L, true);
	return 1;
}

void lua_ErrorReporting(lua_State *L, int status)
{
	if (status != 0) {
		std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1); // remove error message
	}
}