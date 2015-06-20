#include <iostream>
#include <memory> // for unique_ptr
/**
@file
@author Szymon Oracki <szymon.oracki@oustish.pl>
@version 0.1

@section DESCRIPTION

This file is a start point for entire application.
*/

#include <fstream> // for file opening
#include <streambuf> // for file opening
#include <ctime> // for time()

#include <lua/lua.hpp>

#include "LuaFunctions.hpp"




void lua_error_reporting(lua_State*, int);
void replaceAll(std::string&, const std::string&, const std::string&);
static std::string _ReplaceAllAliasOccurences(std::string&, std::unique_ptr<MediaLibCleaner::AudioFile>&);


std::string path = "D:\\Muzyka\\!Synced\\NEW!\\NEW! 2015.06\\Shinedown - Call Me.mp3";
std::unique_ptr<MediaLibCleaner::AudioFile> audiofile;


static int lua_caller_isaudiofile(lua_State *L) {
	return lua_IsAudioFile(L, audiofile);
}

static int lua_caller_settags(lua_State *L) {
	return lua_SetTags(L, audiofile);
}




// cerr and cout streams, if required by the user
std::ofstream _cerr, _cout;

// main function
int main(int argc, char *argv[]) {

	/*
	 *
	 * taglib test
	 *
	 **/
	if (argc == 2) {
		path = argv[1];
	}

	// init - debug
	std::unique_ptr<MediaLibCleaner::AudioFile> audiofilet(new MediaLibCleaner::AudioFile(path));
	audiofile.swap(audiofilet);
	if (!audiofile->IsInitiated()) return 1;


	/**
	 *
	 * READING LUA SETTINGS
	 *
	 **/
	std::string _path = "", _error_log = "", _alert_log = "";
	int _error_level = 0;

	// read config file to memory
	std::ifstream file("C:\\Users\\Szymon\\Documents\\test.lua");
	std::string config((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());

	// replace all occurences in string and return it as new string
	std::string _newconfig = _ReplaceAllAliasOccurences(config, audiofile);

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	lua_register(L, "_IsAudioFile", lua_caller_isaudiofile);
	lua_register(L, "_SetTags", lua_caller_settags);

	// _action == System
	// as we need these informations once at the beginning
	int s = luaL_loadstring(L, _newconfig.c_str());
	lua_pushstring(L, "System");
	lua_setglobal(L, "_action");

	// exetute script
	if (s == 0) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	if (s == 0) { // because error code may change after execution
		// read values to stack
		// as this is stack - read it in reverse
		// so returned values will be in order
		lua_getglobal(L, "_error_level"); // -4
		lua_getglobal(L, "_error_log"); // -3
		lua_getglobal(L, "_alert_log"); // -2
		lua_getglobal(L, "_path"); // -1

		if (!lua_isstring(L, -1) || !lua_isstring(L, -2) || !lua_isstring(L, -3) || !lua_isnumber(L, -4)) {
			return 2; // return error code here, so entire program will end; it's desired
		}

		// read all parameters
		_path = lua_tostring(L, -1);
		_alert_log = lua_tostring(L, -2);
		_error_log = lua_tostring(L, -3);
		_error_level = (int)lua_tonumber(L, -4);

		if (_error_log != "-") {
			// set cerr to proper value
			_cerr.open(_error_log);
			std::cerr.rdbuf(_cerr.rdbuf());
		}

		if (_alert_log != "-") {
			// set cout to proper value
			_cout.open(_alert_log);
			std::cout.rdbuf(_cout.rdbuf());
		}
	}

	std::cerr << "Testing" << std::endl;

	lua_error_reporting(L, s);



	
}

/**
 * Function reporting errors of lua processor to std::cerr.
 *
 * This functions purpose is to check for errors after *.lua config file
 * has been processed and, if present, print them to std::cerr.
 * If error appeared after System initialization then
 * std::cerr may be normal file defined by the user.
 *
 * @param[in] L       A lua_State object holding information about lua processor
 * @param[in] status  Integer with status code returned after calling lua_pcall() or similar function
 */
void lua_error_reporting(lua_State *L, int status) {
	lua_ErrorReporting(L, status);
}

/**
 * Function replacing every occurence of one string with another in given string.
 *
 * This function allows to replace all occurences of one string with another string,
 * all within another string.
 *
 * @param[in/out] str   String in which changes will be made. Please note argument is passed as referrence!
 * @param[in]     from  String which will be replaced with another string
 * @param[in]     to    String which will replace previous string
 */
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

static std::string _ReplaceAllAliasOccurences(std::string& _config, std::unique_ptr<MediaLibCleaner::AudioFile>& _audiofile) {
	std::string _newc = _config;

	// copy original path to not confuse rest of the program
	std::string _path = _audiofile->GetPathname();
	replaceAll(_path, "\\", "\\\\");

	/*
	remaining to implement:

	%alumartist%, %bpm%, %copyright%, %language%, %length%, %mood%, %origalbum% ..., %unsyncedlyrics%, %www%
	%_codec%, %_cover_mimetype%, %_cover_size%, %_cover_type%, %_covers%
	%_ext%, %_filename%, %_filename_ext%, %_folderpath%, %_parent_dir%, %_volume%, %_workingdir%, %_workingpath%
	%_file_{create,mod}_date%, %_file_{create,mod}_datetime%, %_file_{create,mod}_datetime_raw%,
	%_file_size%, %file_size_bytes%, %_file_size_kb%, %_file_size_mb%
	%_counter_dir%, %_counter_total%, %_date%, %_datetime%, %_total_files%, %_total_files_dir%
	*/

	// do the magic!
	replaceAll(_newc, "%_path%", _path);
	replaceAll(_newc, "%artist%", _audiofile->GetArtist());
	replaceAll(_newc, "%title%", _audiofile->GetTitle());
	replaceAll(_newc, "%album%", _audiofile->GetAlbum());
	replaceAll(_newc, "%genre%", _audiofile->GetGenre());
	replaceAll(_newc, "%comment%", _audiofile->GetComment());
	replaceAll(_newc, "%track%", std::to_string(_audiofile->GetTrack()));
	replaceAll(_newc, "%year%", std::to_string(_audiofile->GetYear()));

	replaceAll(_newc, "%_bitrate%", std::to_string(_audiofile->GetBitrate()));
	replaceAll(_newc, "%_channels%", std::to_string(_audiofile->GetChannels()));
	replaceAll(_newc, "%_samplerate%", std::to_string(_audiofile->GetSampleRate()));
	replaceAll(_newc, "%_length_seconds%", std::to_string(_audiofile->GetLength()));
	replaceAll(_newc, "%_length%", _audiofile->GetLengthAsString());

	replaceAll(_newc, "%_datetime_raw%", std::to_string(time(NULL)));

	return _newc;
}

/*// evaluate rest of the file
	s = luaL_loadstring(L, _newconfig.c_str());
	lua_pushstring(L, ""); // make sure nothing is remaining in _action
	lua_setglobal(L, "_action");

	// execute script
	if (s == 0) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	if (s == 0) {

	}

	lua_error_reporting(L, s);
	lua_close(L);

	// wait for user
	system("pause >NUL");

	// flush and close cout and cerr, if opened previously
	if (_cerr.is_open()) {
		_cerr.flush();
		_cerr.close();
	}

	if (_cout.is_open()) {
		_cout.flush();
		_cout.close();
	}

	return 0;*/