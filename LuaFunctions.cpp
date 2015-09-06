/**
 @file
 @author Szymon Oracki <szymon.oracki@oustish.pl>
 @version 0.4

 This file contains definitions of all functions used as lua functions (via lua_register() function)
 */
#include "LuaFunctions.hpp"

/**
 * Function processing given file and returning if it is an audio file
 *
 * @param[in] L	         lua_State object to config file
 * @param[in] audiofile  std::unique_ptr to MediaLibCleaner::File object representing current file
 * @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
 * @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
 *
 * @return Number of output arguments (for lua_register)
 */
int lua_IsAudioFile(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la) {
	int n = lua_gettop(L);
	if (n > 1)
	{
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_IsAudioFile(" + audiofile->GetPath() + L")", L"Function expects 0 arguments (" + std::to_wstring(n) + L" given)", 2);
		return 1;
	}

	lua_pushboolean(L, audiofile->IsInitiated());

	return 1; // numer of output arguments
}

/**
 * Function to set tag(s) in given audio file
 *
 * @param[in] L			 lua_State object to config file
 * @param[in] audiofile	 std::unique_ptr to MediaLibCleaner::File object representing current file
 * @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
 * @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
 *
 * @return Number of output arguments (for lua_register)
 */
int lua_SetTags(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la) {
	int n = lua_gettop(L) - 1; // argc for function

	if (n % 2 == 1 && n > 0) { // requiers even, positive amount of arguments
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_SetTags(" + audiofile->GetPath() + L")", L"Function expects even and positive amount of argument pairs [tag, value] (" + std::to_wstring(n) + L" given)", 2);
		return 1;
	}

	for (int i = 1; i <= n; i += 2) {
		std::wstring tag = s2ws(lua_tostring(L, i));
		TagLib::String val;

		val = s2ws(lua_tostring(L, i + 1));
		audiofile->SetTag(tag, val);
	}

	// return - indicates function completed it's run
	lua_pushboolean(L, true);
	return 1;
}

/**
* Function to remove tag(s) in given audio file
*
* @param[in] L			 lua_State object to config file
* @param[in] audiofile	 std::unique_ptr to MediaLibCleaner::File object representing current file
* @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
* @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
*
* @return Number of output arguments (for lua_register)
*/
int lua_RemoveTags(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la) {
	int n = lua_gettop(L) - 1; // argc for function

	if (n == 0) { // requires positive amount of arguments
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_RemoveTags(" + audiofile->GetPath() + L")", L"Function expects positive amount of arguments (" + std::to_wstring(n) + L" given)", 2);
		return 1;
	}

	for (int i = 1; i <= n; ++i) {
		std::wstring tag = s2ws(lua_tostring(L, i));
		audiofile->SetTag(tag, TagLib::String::null);
	}

	// return - indicates function completed it's run
	lua_pushboolean(L, true);
	return 1;
}

/**
* Function to set required tag(s) in given audio file
*
* @param[in] L          lua_State object to config file
* @param[in] audiofile  std::unique_ptr to MediaLibCleaner::File object representing current file
* @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
* @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
*
* @return Number of output arguments (for lua_register)
*/
int lua_SetRequiredTags(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la) {
	int n = lua_gettop(L) - 1; // argc for function
	// input parameters are simple tag names, like this: "artist"
	// note lack of % signs at the beginning and end

	bool retval = true;
	for (int i = 1; i <= n; i++)
	{
		std::string tag = lua_tostring(L, i);
		if (tag == "") continue;

		if ( !audiofile->HasTag( s2ws( lua_tostring(L, i) ) ) )
		{
			retval = false;
		}
	}

	// indicates function completed it's run
	lua_pushboolean(L, retval);
	return 1;
}

/**
* Function to set required tag(s) in given audio file, as well checks if it has desired values
*
* @param[in] L          lua_State object to config file
* @param[in] audiofile  std::unique_ptr to MediaLibCleaner::File object representing current file
* @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
* @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
*
* @return Number of output arguments (for lua_register)
*/
int lua_CheckTagValues(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la) {
	int n = lua_gettop(L) - 1; // argc for function

	if (n < 2) { // requiers even, positive amount of arguments
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_CheckTagsValues(" + audiofile->GetPath() + L")", L"Function expects at least 2 arguments in given format: [tag, value1, value2, ...] (" + std::to_wstring(n) + L" given)", 2);
		return 1;
	}

	std::wstring tag = s2ws(lua_tostring(L, 1));
	std::vector< std::wstring > val;
	for (int i = 2; i <= n; i++)
	{
		val.push_back(s2ws(lua_tostring(L, i)));
	}

	// indicates function completed it's run
	lua_pushboolean(L, audiofile->HasTag(tag, val));
	return 1;
}

/**
* Function renames source file to the value specified by the user
*
* @param[in] L          lua_State object to config file
* @param[in] audiofile  std::unique_ptr to MediaLibCleaner::File object representing current file
* @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
* @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
*
* @return Number of output arguments (for lua_register)
*/
int lua_Rename(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la)
{
	int n = lua_gettop(L) - 1; // argc for function

	if (n != 1) // requires exactly 1 argument
	{
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_Rename(" + audiofile->GetPath() + L")", L"Function expects exactly 1 argument (" + std::to_wstring(n) + L" given)", 2);
	}

	std::wstring nname = s2ws(lua_tostring(L, 1));

	lua_pushboolean(L, audiofile->Rename(nname));
	return 1;
}

/**
* Function moves source file to the value specified by the user
*
* @param[in] L          lua_State object to config file
* @param[in] audiofile  std::unique_ptr to MediaLibCleaner::File object representing current file
* @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
* @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
*
* @return Number of output arguments (for lua_register)
*/
int lua_Move(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la)
{
	int n = lua_gettop(L) - 1; // argc for function

	if (n != 1) // requires exactly 1 argument
	{
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_Move(" + audiofile->GetPath() + L")", L"Function expects exactly 1 argument (" + std::to_wstring(n) + L" given)", 2);
	}

	std::wstring nloc = s2ws(lua_tostring(L, 1));

	lua_pushboolean(L, audiofile->Move(nloc));
	return 1;
}

/**
* Function deletes specified file
*
* @param[in] L          lua_State object to config file
* @param[in] audiofile  std::unique_ptr to MediaLibCleaner::File object representing current file
* @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
* @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
*
* @return Number of output arguments (for lua_register)
*/
int lua_Delete(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la)
{
	int n = lua_gettop(L) - 1; // argc for function

	if (n > 0) //does not expect arguments
	{
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_Delete(" + audiofile->GetPath() + L")", L"Function expects exactly 0 arguments (" + std::to_wstring(n) + L" given)", 2);
	}

	lua_pushboolean(L, audiofile->Delete());
	return 1;
}

/**
* Function to write message to AlertLog
*
* @param[in] L		    lua_State object to config file
* @param[in] audiofile  std::unique_ptr to MediaLibCleaner::File object representing current file
* @param[in] lp         std::unique_ptr to MediaLibCleaner::LogProgram object used for logging purposes
* @param[in] la         std::unique_ptr to MediaLibCleaner::LogAlert object used for logging purposes
*
* @return Number of output arguments (for lua_register)
*/
int lua_Log(lua_State *L, MediaLibCleaner::File* audiofile, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la) {
	int n = lua_gettop(L) - 1; // argc for function

	if (n == 0) { // requires positive amount of arguments
		lua_pushboolean(L, false);
		(*lp)->Log(L"lua_Log(" + audiofile->GetPath() + L")", L"Function expects exactly 1 argument (" + std::to_wstring(n) + L" given)", 2);
		return 1;
	}

	(*la)->Log(audiofile->GetPath(), L"[USER] " + s2ws(lua_tostring(L, 1)));

	// return - indicates function completed it's run
	lua_pushboolean(L, true);
	return 1;
}


/**
 * Function redirecting lua errors to MediaLibCleaner::LogProgram as an error message
 *
 * @param[in] L			lua_State object to config file
 * @param[in] status	Integer containing lua processor status code
 * @param[in] programlog unique_ptr to LogProgram object for logging purposses
 */
void lua_ErrorReporting(lua_State *L, int status, std::unique_ptr<MediaLibCleaner::LogProgram>* programlog)
{
	if (status != 0) { // if error occured
		(*programlog)->Log(L"LUA", s2ws(lua_tostring(L, -1)), 1);
		lua_pop(L, 1); // remove error message
	}
}