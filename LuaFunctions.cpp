/**
 @file
 @author Szymon Oracki <szymon.oracki@oustish.pl>
 @version 0.1

 This file contains definitions of all functions used as lua functions (via lua_register() function)
 */
#include "LuaFunctions.hpp"

/**
 * Function processing given file and returning if it is an audio file
 *
 * @param[in] L				lua_State object to config file
 * @param[in] _audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
 *
 * @return Number of output arguments (for lua_register)
 */
int lua_IsAudioFile(lua_State *L, MediaLibCleaner::File* _audiofile) {
	// check if file is na audio file
	// real audio files will have IsInitialized() == true

	lua_pushboolean(L, _audiofile->IsInitiated());

	return 1; // numer of output arguments
}

/*int lua_IsAudioFile(lua_State *L, std::unique_ptr<MediaLibCleaner::File>& _audiofile) {
	// check if file is na audio file
	// real audio files will have IsInitialized() == true

	lua_pushboolean(L, _audiofile->IsInitiated());

	return 1; // numer of output arguments
}*/

/**
 * Function to set tag(s) in given audio file
 *
 * @param[in]		L			lua_State object to config file
 * @param[in,out]   _audiofile	std::nique_ptr to MediaLibCleaner::File object representing current file
 */
int lua_SetTags(lua_State *L, MediaLibCleaner::File* _audiofile) {
	int n = lua_gettop(L); // argc for function

	if (n % 2 == 1 && n > 0) { // requiers odd, positive amount of arguments
		lua_pushboolean(L, false);
		std::cerr << "[LUAPROC] _SetTags(): function requires odd and positive amount of arguments." << std::endl;
		return 1;
	}

	for (int i = 1; i <= n; i += 2) {
		// debug
		std::cout << "Setting tag: '" << lua_tostring(L, i) << "' to value: '" << lua_tostring(L, i + 1) << "'" << std::endl;
	}

	lua_pushboolean(L, true);
	return 1;
}

/**
 * Function redirecting lua errors to std::cerr stream
 *
 * @param[in] L			lua_State object to config file
 * @param[in] status	Integer containing lua processor status code
 */
void lua_ErrorReporting(lua_State *L, int status)
{
	if (status != 0) { // if error occured
		std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1); // remove error message
	}
}