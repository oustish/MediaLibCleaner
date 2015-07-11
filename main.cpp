/**
 * @file
 * @author Szymon Oracki <szymon.oracki@oustish.pl>
 * @version 0.1
 *
 *This file is a start point for entire application.
 */

#include "main.hpp"


// preparing variables for reading program settings from lua file
/**
 * Global variable of std::string containing path to working directory
 */
std::string path = "", 
/**
 * Global variable of std::string containg path to error log file
 */
	error_log = "",
/**
 * Global variable of std::string containing path to log file
 */
	alert_log = "";
/**
 * Global variable containing int coded level indicating what kind of messages will be written to error log
 */
int error_level = 0;

/**
 * Global variable representing MediaLibCleaner::FilesAggregator object
 */
std::unique_ptr<MediaLibCleaner::FilesAggregator> filesAggregator;

/**
 * Function calling lua_IsAuidoFile() function. This function is registered withing lua processor!
 *
 * @param[in] L	lua_State object to config file
 *
 * @return Number of output arguments on stack for lua processor
 */
static int lua_caller_isaudiofile(lua_State *L) {
	return lua_IsAudioFile(L, filesAggregator->CurrentFile());
}

/**
 * Function calling lua_SetTags() function. This function is registered within lua processor!
 *
 * @param[in] L lua_State object to config file
 *
 * @return Number of output arguments on stack for lua processor
 */
static int lua_caller_settags(lua_State *L) {
	return lua_SetTags(L, filesAggregator->CurrentFile());
}

/**
* Function calling lua_SetRequiredTags() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_setrequiredtags(lua_State *L)
{
	return lua_SetRequiredTags(L, filesAggregator->CurrentFile());
}

/**
* Function calling lua_CheckTagsValues() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_checktagsvalues(lua_State *L)
{
	return lua_CheckTagsValues(L, filesAggregator->CurrentFile());
}

/**
* Function calling lua_Rename() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_rename(lua_State *L)
{
	return lua_Rename(L, filesAggregator->CurrentFile());
}

/**
* Function calling lua_Move() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_move(lua_State *L)
{
	return lua_Move(L, filesAggregator->CurrentFile());
}

/**
* Function calling lua_Delete() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_delete(lua_State *L)
{
	return lua_Delete(L, filesAggregator->CurrentFile());
}







// cerr and cout streams, if required by the user
/**
 * Global variable to store new std::wcerr::rdbuf to allow user to define his own error stream (as a file)
 */
std::ofstream wcerr;

/**
 * Global variable to store new std::wcout::rdbuf to allow user to define his own output stream (as a file)
 */
std::ofstream wcout;

// some variables required for aliases
int total_files = 0;
time_t datetime_raw = 0;







/**
 * Main function
 * 
 * @param[in] argc	Amount of input arguments (always >= 1)
 * @param[in] argv	Input parameters
 *
 * @return Exit code for entire program
 */
int main(int argc, char *argv[]) {
	// capture time app was launched
	datetime_raw = time(nullptr);

	// easy way to fix found problems with diacritic letters in filenames :)
	// it wasn't so easy to find though...
	setlocale(LC_ALL, "");

	// reading config file into wstring
	std::wifstream wfile(L"C:\\Users\\Szymon\\Documents\\test.lua");
	wfile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstringstream wss;
	wss << wfile.rdbuf();
	std::wstring wconfig = wss.str();

	// converting from wstring to string
	// IMPORTANT: uses Windows.h functions
	// need to come up with solution for Linux-based OS later
	std::string config = ws2s(wconfig);

	// init lua processor
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// register C functions in lua processor
	lua_register(L, "_IsAudioFile", lua_caller_isaudiofile);
	lua_register(L, "_SetTags", lua_caller_settags);
	lua_register(L, "_SetRequiredTags", lua_caller_setrequiredtags);
	lua_register(L, "_CheckTagsValues", lua_caller_checktagsvalues);
	lua_register(L, "_Rename", lua_caller_rename);
	lua_register(L, "_Move", lua_caller_move);
	lua_register(L, "_Delete", lua_caller_delete);

	// _action == System
	// as we need these informations once at the beginning
	int s = luaL_loadstring(L, config.c_str());
	lua_pushstring(L, "System");
	lua_setglobal(L, "_action");

	// exetute script
	if (s == 0) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}
	if (s != 0) { // because error code may change after execution
		// report any errors, if found
		lua_error_reporting(L, s);
	}
	// read values to stack
	// as this is stack - read it in reverse
	// so returned values will be in order
	lua_getglobal(L, "_error_level"); // -4
	lua_getglobal(L, "_error_log"); // -3
	lua_getglobal(L, "_alert_log"); // -2
	lua_getglobal(L, "_path"); // -1

	// check if returning values have proper type
	if (!lua_isstring(L, -1) || !lua_isstring(L, -2) || !lua_isstring(L, -3) || !lua_isnumber(L, -4)) {
		return 2; // return error code here, so entire program will end; it's desired; debug
	}

	// read all parameters
	path = lua_tostring(L, -1);
	alert_log = lua_tostring(L, -2);
	error_log = lua_tostring(L, -3);
	error_level = static_cast<int>(lua_tonumber(L, -4));

	if (error_log != "-") {
		// set cerr to proper value
		wcerr.open(error_log);
		std::cerr.rdbuf(wcerr.rdbuf());
	}

	if (alert_log != "-") {
		// set cout to proper value
		wcout.open(alert_log);
		std::cout.rdbuf(wcout.rdbuf());
	}

	lua_close(L);

	// BELOW ARE PROCEDURES TO SCAN GIVEN DIRECTORY AND RETRIEVE ALL INFO WE REQUIRE
	// create MediaLibCleaner::FilesAggregator object nad swap it with global variable one
	std::unique_ptr<MediaLibCleaner::FilesAggregator> filesAgg(new MediaLibCleaner::FilesAggregator());
	filesAgg.swap(filesAggregator);

	// shorten namespace
	namespace fs = boost::filesystem;
	fs::path workingdir(path);

	if (!fs::exists(workingdir) || !fs::is_directory(workingdir)) {
		return 3; // ret. val; debug
	}

	fs::path dirpath = workingdir;
	MediaLibCleaner::DFC *currdfc = new MediaLibCleaner::DFC(path);
	for (fs::recursive_directory_iterator dir(path), dir_end; dir != dir_end; ++dir)
	{
		fs::path filepath = dir->path();

		if (filepath == L"." || filepath == L"..") continue;

		if (!(filepath.has_filename() && filepath.has_extension())) {
			// not a file, but a directory!
			dirpath = filepath;

			currdfc = new MediaLibCleaner::DFC(dirpath.string());

			continue;
		}

		// create File object for file
		MediaLibCleaner::File *filez = new MediaLibCleaner::File(filepath.wstring(), currdfc);
		filesAggregator->AddFile(filez);

		// increment total_files counter if audio file
		if (filez->IsInitiated()) total_files++;

		// increment folder counter
		if (dirpath.string() != "")
			currdfc->IncCount();
	}



	// ITERATE OVER COLLECTION AND PROCESS FILES
	MediaLibCleaner::File* cfile = filesAggregator->re_set();
	do {
		std::wcout << std::endl << std::endl << std::endl << std::endl;
		std::wcout << L"File: " << cfile->GetPath() << std::endl << std::endl;

		std::wstring new_config = ReplaceAllAliasOccurences(wconfig, cfile);

		//std::wcout << new_config << std::endl << std::endl;

		L = luaL_newstate();
		luaL_openlibs(L);

		// register C functions in lua processor
		lua_register(L, "_IsAudioFile", lua_caller_isaudiofile);
		lua_register(L, "_SetTags", lua_caller_settags);
		lua_register(L, "_SetRequiredTags", lua_caller_setrequiredtags);
		lua_register(L, "_CheckTagsValues", lua_caller_checktagsvalues);
		lua_register(L, "_Rename", lua_caller_rename);
		lua_register(L, "_Move", lua_caller_move);
		lua_register(L, "_Delete", lua_caller_delete);

		std::string nc = ws2s(new_config);

		s = luaL_loadstring(L, nc.c_str());
		lua_pushstring(L, "");
		lua_setglobal(L, "_action");

		// exetute script
		if (s == 0) {
			s = lua_pcall(L, 0, LUA_MULTRET, 0);
		}
		if (s != 0) { // because error code may change after execution
			// report any errors, if found
			lua_error_reporting(L, s);
		}

		lua_close(L);

		cfile = filesAggregator->next();
	} while (cfile != nullptr);
	


	// wait for user
	system("pause >NUL");

	// flush and close cout and cerr, if opened previously
	if (wcerr.is_open()) {
		wcerr.flush();
		wcerr.close();
	}

	if (wcout.is_open()) {
		wcout.flush();
		wcout.close();
	}

	return 0;
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
 * Function replacing every occurence of all aliases with proper tag values read from audio file.
 *
 * Function that will replace all aliases (for example \%artist%) with proper values from each audio file
 * given as second parameter.
 *
 * @see http://flute.eti.pg.gda.pl/trac/student-projects/wiki/MediaLibCleaner/Aliases for aliases definitions
 *
 * @param[in]	_config	    String with config file content. Any format is accepted.
 * @param[in]	_audiofile  std::unique_ptr to MediaLibCleaner::AudioFile object representing current file
 *
 * @return String containing config file with aliases replaced
 */
std::wstring ReplaceAllAliasOccurences(std::wstring& wcfg, MediaLibCleaner::File* audiofile) {
	std::wstring newc = wcfg;

	// copy original path to not confuse rest of the program
	std::wstring lpath = audiofile->GetPath();
	replaceAll(lpath, L"\\", L"\\\\");

	// do the magic!
	// SONG DATA
	replaceAll(newc, L"%artist%", audiofile->GetArtist());
	replaceAll(newc, L"%title%", audiofile->GetTitle());
	replaceAll(newc, L"%album%", audiofile->GetAlbum());
	replaceAll(newc, L"%genre%", audiofile->GetGenre());
	replaceAll(newc, L"%comment%", audiofile->GetComment());
	replaceAll(newc, L"%track%", std::to_wstring(audiofile->GetTrack()));
	replaceAll(newc, L"%year%", std::to_wstring(audiofile->GetYear()));
	replaceAll(newc, L"%albumartist%", audiofile->GetAlbumArtist());
	replaceAll(newc, L"%bpm%", audiofile->GetBPM());
	replaceAll(newc, L"%copyright%", audiofile->GetCopyright());
	replaceAll(newc, L"%language%", audiofile->GetLanguage());
	replaceAll(newc, L"%length%", audiofile->GetTagLength());
	replaceAll(newc, L"%mood%", audiofile->GetMood());
	replaceAll(newc, L"%origalbum%", audiofile->GetOrigAlbum());
	replaceAll(newc, L"%origartist%", audiofile->GetOrigArtist());
	replaceAll(newc, L"%origfilename%", audiofile->GetOrigFilename());
	replaceAll(newc, L"%origyear%", audiofile->GetOrigYear());
	replaceAll(newc, L"%publisher%", audiofile->GetPublisher());
	replaceAll(newc, L"%unsyncedlyrics%", audiofile->GetLyricsUnsynced());
	replaceAll(newc, L"%www%", audiofile->GetWWW());

	// TECHNICAL INFO
	replaceAll(newc, L"%_bitrate%", std::to_wstring(audiofile->GetBitrate()));
	replaceAll(newc, L"%_codec%", audiofile->GetCodec());
	replaceAll(newc, L"%_cover_mimetype%", audiofile->GetCoverMimetype());
	replaceAll(newc, L"%_cover_size%", std::to_wstring(audiofile->GetCoverSize()));
	replaceAll(newc, L"%_coder_type%", audiofile->GetCoverType());
	replaceAll(newc, L"%_covers%", std::to_wstring(audiofile->GetCovers()));
	replaceAll(newc, L"%_length%", audiofile->GetLengthAsString());
	replaceAll(newc, L"%_length_seconds%", std::to_wstring(audiofile->GetLength()));
	replaceAll(newc, L"%_channels%", std::to_wstring(audiofile->GetChannels()));
	replaceAll(newc, L"%_samplerate%", std::to_wstring(audiofile->GetSampleRate()));


	// PATH INFO
	replaceAll(newc, L"%_directory%", audiofile->GetDirectory());
	replaceAll(newc, L"%_ext%", audiofile->GetExt());
	replaceAll(newc, L"%_filename%", audiofile->GetFilename());
	replaceAll(newc, L"%_filename_ext%", audiofile->GetFilenameExt());
	replaceAll(newc, L"%_folderpath%", audiofile->GetFolderPath());
	replaceAll(newc, L"%_parent_dir%", audiofile->GetParentDir());
	replaceAll(newc, L"%_path%", audiofile->GetPath());
#ifdef WIN32
	replaceAll(newc, L"%_volume%", audiofile->GetVolume());
#endif
	//replaceAll(_newc, "%_workingdir%", );
	//replaceAll(_newc, "%_workingpath%", );


	// FILES PROPERTIES
	replaceAll(newc, L"%_file_create_date%", audiofile->GetFileCreateDate());
	replaceAll(newc, L"%_file_create_datetime%", audiofile->GetFileCreateDatetime());
	replaceAll(newc, L"%_file_create_datetime_raw%", std::to_wstring(audiofile->GetFileCreateDatetimeRaw()));
	replaceAll(newc, L"%_file_mod_date%", audiofile->GetFileModDate());
	replaceAll(newc, L"%_file_mod_datetime%", audiofile->GetFileModDatetime());
	replaceAll(newc, L"%_file_mod_datetime_raw%", std::to_wstring(audiofile->GetFileModDatetimeRaw()));
	replaceAll(newc, L"%_file_size%", audiofile->GetFileSize());
	replaceAll(newc, L"%_file_size_bytes%", std::to_wstring(audiofile->GetFileSizeBytes()));
	replaceAll(newc, L"%_file_size_kb%", audiofile->GetFileSizeKB());
	replaceAll(newc, L"%_file_size_mb%", audiofile->GetFileSizeMB());


	// SYSTEM DATA
	//replaceAll(_newc, L"%_counter_dir%", audiofile->GetComment());
	replaceAll(newc, L"%_counter_total%", std::to_wstring(total_files));
	replaceAll(newc, L"%_date%", get_date_iso_8601_wide(datetime_raw));
	replaceAll(newc, L"%_datetime%", get_date_rfc_2822_wide(datetime_raw));
	replaceAll(newc, L"%_datetime_raw%", std::to_wstring(time(nullptr)));
	replaceAll(newc, L"%_total_files%", std::to_wstring(total_files));
	replaceAll(newc, L"%_total_files_dir%", std::to_wstring(audiofile->GetDFC()->GetCounter()));

	return newc;
}