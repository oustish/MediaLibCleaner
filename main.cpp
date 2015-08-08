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




std::unique_ptr<MediaLibCleaner::LogAlert> alertlog;

std::unique_ptr<MediaLibCleaner::LogProgram> programlog;

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
	//debug
	static std::basic_stringbuf<std::ostream::char_type> buf;
	std::cerr.rdbuf(&buf);

	// capture time app was launched
	datetime_raw = time(nullptr);

	// easy way to fix found problems with diacritic letters in filenames :)
	// it wasn't so easy to find though...
	setlocale(LC_ALL, "");

	std::wcout << L"Beginning program..." << std::endl; //d

	// reading config file into wstring
	std::wifstream wfile(L"C:\\Users\\Szymon\\Documents\\test.lua");
	wfile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
	std::wstringstream wss;
	wss << wfile.rdbuf();
	std::wstring wconfig = wss.str();

	// converting from wstring to string
	// IMPORTANT: uses Windows.h functions
	// need to come up with solution for Linux-based OS later
	std::wstring wcfgc = wconfig;
	replaceAll(wcfgc, L"\\", L"\\\\");
	std::string config = ws2s(wcfgc);

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

	std::wcout << L"Executing script... (SYSTEM)" << std::endl; //d

	// execute script
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
		std::unique_ptr<MediaLibCleaner::LogProgram> temp(new MediaLibCleaner::LogProgram(s2ws(error_log), error_level));
		programlog.swap(temp);
	}

	if (alert_log != "-") {
		std::unique_ptr<MediaLibCleaner::LogAlert> temp(new MediaLibCleaner::LogAlert(s2ws(alert_log)));
		alertlog.swap(temp);
	}

	lua_close(L);

	// BELOW ARE PROCEDURES TO SCAN GIVEN DIRECTORY AND RETRIEVE ALL INFO WE REQUIRE
	// create MediaLibCleaner::FilesAggregator object nad swap it with global variable one
	programlog->Log(L"Main", L"Creating MediaLibCleaner::FilesAggregator object", 3);
	std::unique_ptr<MediaLibCleaner::FilesAggregator> filesAgg(new MediaLibCleaner::FilesAggregator(&programlog, &alertlog));
	filesAgg.swap(filesAggregator);

	boost::filesystem::path workingdir(path);

	programlog->Log(L"Main", L"Checking for working directory existence.", 3);
	if (!boost::filesystem::exists(workingdir) || !boost::filesystem::is_directory(workingdir)) {
		programlog->Log(L"Main", L"Working dir does not exist. Check your _path variable in LUA script.", 1);
		return 3; // ret. val; debug
	}

	// object that will hold all DFC objects
	// for deleting purposes later
	std::list<MediaLibCleaner::DFC*> dfc_list;

	// object that will hold all paths
	std::list<boost::filesystem::path> path_list;

	std::wcout << L"Scanning for files..." << std::endl;

	// this bit of code will be executed on single core
	programlog->Log(L"Main", L"Beginning scan for files inside working dir", 3);

	for (boost::filesystem::recursive_directory_iterator dir(path), dir_end; dir != dir_end; ++dir)
	{
		boost::filesystem::path filepath = dir->path();

		programlog->Log(L"Main", L"Adding path to list: " + filepath.generic_wstring(), 3);

		path_list.push_back(filepath);
	}

	// parsing paths and files
	// full multi-core support (in theory)
	programlog->Log(L"Main", L"Beginning parsing paths and files", 3);
	std::wcout << L"Scanning files..." << std::endl;
	scan(&dfc_list, &path_list, &programlog, &alertlog, path, &filesAggregator, &total_files);


	// ITERATE OVER COLLECTION AND PROCESS FILES
	// multi-core
	programlog->Log(L"Main", L"Starting iteration through collection.", 3);
	std::wcout << L"Processing files..." << std::endl;
	filesAggregator->rewind();
	process(wconfig, &filesAggregator, &programlog);

	for (auto it = dfc_list.begin(); it != dfc_list.end(); ++it)
		delete (*it);

	time_t dt_end = time(nullptr);
	time_t diff = dt_end - datetime_raw;

	programlog->Log(L"Main", L"Program execution time: " + std::to_wstring(diff) + L" sec", 3);
	programlog->Log(L"Main", L"Total files: " + std::to_wstring(total_files), 3);

	programlog->Log(L"Main", L"Program finished", 3);

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
	lua_ErrorReporting(L, status, &programlog);
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

	replaceAll(newc, L"\\", L"\\\\");

	return newc;
}

void process(std::wstring wconfig, std::unique_ptr<MediaLibCleaner::FilesAggregator>* fA, std::unique_ptr<MediaLibCleaner::LogProgram>* lp)
{
	auto cfile = (*fA)->next();
	
	if (cfile == nullptr) return;

	do {
		(*lp)->Log(L"Main", L"File: " + cfile->GetPath(), 3);
		(*lp)->Log(L"Main", L"Creating config file", 3);
		std::wstring new_config = ReplaceAllAliasOccurences(wconfig, cfile);

		(*lp)->Log(L"Main", L"Lua procesor init", 3);
		lua_State *L = luaL_newstate();
		luaL_openlibs(L);

		(*lp)->Log(L"Main", L"Registering functions", 3);
		// register C functions in lua processor
		lua_register(L, "_IsAudioFile", lua_caller_isaudiofile);
		lua_register(L, "_SetTags", lua_caller_settags);
		lua_register(L, "_SetRequiredTags", lua_caller_setrequiredtags);
		lua_register(L, "_CheckTagsValues", lua_caller_checktagsvalues);
		lua_register(L, "_Rename", lua_caller_rename);
		lua_register(L, "_Move", lua_caller_move);
		lua_register(L, "_Delete", lua_caller_delete);

		(*lp)->Log(L"Main", L"Converting wide string to string", 3);
		std::string nc = ws2s(new_config);

		(*lp)->Log(L"Main", L"Lua procesor loads string", 3);
		int s = luaL_loadstring(L, nc.c_str());
		lua_pushstring(L, "");
		lua_setglobal(L, "_action");

		(*lp)->Log(L"Main", L"Executing script", 3);
		// exetute script
		if (s == 0) {
			s = lua_pcall(L, 0, LUA_MULTRET, 0);
		}
		if (s != 0) { // because error code may change after execution
			// report any errors, if found
			lua_error_reporting(L, s);
			std::wcout << new_config << std::endl << std::endl;
		}

		lua_close(L);

		cfile = (*fA)->next();
	} while (cfile != nullptr);
}

void scan(std::list<MediaLibCleaner::DFC*>* dfcl, std::list<boost::filesystem::path>* pathl, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la, std::string pth, std::unique_ptr<MediaLibCleaner::FilesAggregator>* fA, int* tf)
{
	std::mutex dfcl_mutex;
	auto *currdfc = MediaLibCleaner::AddDFC(dfcl, pth, &dfcl_mutex, lp, la);
	boost::filesystem::path dirpath;

	for (auto it = pathl->begin(); it != pathl->end(); ++it)
	{
		(*lp)->Log(L"Main", L"Current file: " + (*it).generic_wstring(), 3);

		if (boost::filesystem::is_directory(*it)) {
			(*lp)->Log(L"Main", L"Current file is a directory.", 3);

			// not a file, but a directory!
			dirpath = *it;

			currdfc = MediaLibCleaner::AddDFC(dfcl, dirpath, &dfcl_mutex, lp, la);

			continue;
		}

		// create File object for file
		(*lp)->Log(L"Main", L"Creating MediaLibCleaner::File object for file.", 3);
		MediaLibCleaner::File *filez = new MediaLibCleaner::File((*it).wstring(), currdfc, lp, la);
		(*fA)->AddFile(filez);

		// increment total_files counter if audio file
		if (filez->IsInitiated()) {
			#pragma omp atomic
			(*tf)++;
		}

		// DFC counter is incremented inside MediaLibCleaner::File object
		// where additional checks are performed
	}
}