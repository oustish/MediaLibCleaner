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
 * @param[in] audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
 *
 * @return Number of output arguments (for lua_register)
 */
int lua_IsAudioFile(lua_State *L, MediaLibCleaner::File* audiofile) {
	int n = lua_gettop(L);
	if (n > 0)
	{
		lua_pushboolean(L, false);
		std::wcerr << "[LUAPROC] _IsAudioFile(): function expects 0 arguments (" << std::to_wstring(n) << " given)." << std::endl;
		return 1;
	}

	lua_pushboolean(L, audiofile->IsInitiated());

	return 1; // numer of output arguments
}

/**
 * Function to set tag(s) in given audio file
 *
 * @param[in]		L			lua_State object to config file
 * @param[in,out]   audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
 */
int lua_SetTags(lua_State *L, MediaLibCleaner::File* audiofile) {
	int n = lua_gettop(L); // argc for function

	if (n % 2 == 1 && n > 0) { // requiers even, positive amount of arguments
		lua_pushboolean(L, false);
		std::wcerr << "[LUAPROC] _SetTags(): function expects even and positive amount of argument pairs [tag, value] (" << std::to_wstring(n) << " given)." << std::endl;
		return 1;
	}

	for (int i = 1; i <= n; i += 2) {
		// debug
		//std::wcout << L"Setting tag: '" << s2ws(lua_tostring(L, i)) << L"' to value: '" << s2ws(lua_tostring(L, i + 1)) << L"'" << std::endl;
	}

	// return - indicates function completed it's run
	lua_pushboolean(L, true);
	return 1;
}

/**
* Function to set required tag(s) in given audio file
*
* @param[in]		L			lua_State object to config file
* @param[in,out]    audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
*/
int lua_SetRequiredTags(lua_State *L, MediaLibCleaner::File* audiofile) {
	int n = lua_gettop(L); // argc for function
	// input parameters are simple tag names, like this: "artist"
	// note lack of % signs at the beginning and end

	for (int i = 1; i <= n; i++)
	{
		std::string tag = lua_tostring(L, i);
		if (tag == "") continue;

		audiofile->HasTag(s2ws(lua_tostring(L, i)));

		// debug
		//std::wcout << L"Tag is required: '" << s2ws(lua_tostring(L, i)) << "'" << std::endl;
	}

	// indicates function completed it's run
	lua_pushboolean(L, true);
	return 1;
}

/**
* Function to set required tag(s) in given audio file, as well checks if it has desired values
*
* @param[in]		L			lua_State object to config file
* @param[in,out]    audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
*/
int lua_CheckTagsValues(lua_State *L, MediaLibCleaner::File* audiofile) {
	int n = lua_gettop(L); // argc for function

	if (n % 2 == 1 && n > 0) { // requiers even, positive amount of arguments
		lua_pushboolean(L, false);
		std::wcerr << "[LUAPROC] _CheckTagsValues(): function expects even and positive amount of argument pairs [tag, value] (" << std::to_wstring(n) << " given)." << std::endl;
		return 1;
	}

	for (int i = 1; i <= n; i += 2) {
		audiofile->HasTag(s2ws(lua_tostring(L, i)), s2ws(lua_tostring(L, i + 1)));

		// debug
		//std::wcout << L"Tag is required: '" << s2ws(lua_tostring(L, i)) << L"' with value '" << s2ws(lua_tostring(L, i + 1)) << L"'" << std::endl;
	}

	// indicates function completed it's run
	lua_pushboolean(L, true);
	return 1;
}

/**
* Function renames source file to the value specified by the user
*
* @param[in]		L			lua_State object to config file
* @param[in,out]    audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
*/
int lua_Rename(lua_State *L, MediaLibCleaner::File* audiofile)
{
	int n = lua_gettop(L); // argc for function

	if (n != 1) // requires exactly 1 argument
	{
		lua_pushboolean(L, false);
		std::wcerr << "[LUAPROC] _Rename(): function expects exactly 1 argument (" << std::to_wstring(n) << " given)." << std::endl;
	}

	std::wstring nname = s2ws(lua_tostring(L, 1));

	lua_pushboolean(L, audiofile->Rename(nname));
	return 1;
}

/**
* Function moves source file to the value specified by the user
*
* @param[in]		L			lua_State object to config file
* @param[in,out]    audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
*/
int lua_Move(lua_State *L, MediaLibCleaner::File* audiofile)
{
	int n = lua_gettop(L); // argc for function

	if (n != 1) // requires exactly 1 argument
	{
		lua_pushboolean(L, false);
		std::wcerr << "[LUAPROC] _Move(): function expects exactly 1 argument (" << std::to_wstring(n) << " given)." << std::endl;
	}

	std::wstring nloc = s2ws(lua_tostring(L, 1));

	lua_pushboolean(L, audiofile->Move(nloc));
	return 1;
}

/**
* Function deletes specified file
*
* @param[in]		L			lua_State object to config file
* @param[in,out]    audiofile	std::unique_ptr to MediaLibCleaner::File object representing current file
*/
int lua_Delete(lua_State *L, MediaLibCleaner::File* audiofile)
{
	int n = lua_gettop(L); // argc for function

	if (n > 0) //does not expect arguments
	{
		lua_pushboolean(L, false);
		std::wcerr << "[LUAPROC] _Delete(): function expects exactly 0 arguments (" << std::to_wstring(n) << " given)." << std::endl;
	}

	lua_pushboolean(L, audiofile->Delete());
	return 1;
}



/**
 * Function redirecting lua errors to std::cerr stream
 *
 * @param[in] L			lua_State object to config file
 * @param[in] status	Integer containing lua processor status code
 */
void lua_ErrorReporting(lua_State *L, int status, std::unique_ptr<MediaLibCleaner::LogProgram>* programlog)
{
	if (status != 0) { // if error occured
		(*programlog)->Log(L"LUA", s2ws(lua_tostring(L, -1)), 1);
		lua_pop(L, 1); // remove error message
	}
}