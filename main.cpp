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
std::string _path = "", 
/**
 * Global variable of std::string containg path to error log file
 */
	_error_log = "",
/**
 * Global variable of std::string containing path to log file
 */
	_alert_log = "";
/**
 * Global variable containing int coded level indicating what kind of messages will be written to error log
 */
int _error_level = 0;

/**
 * Global variable representing MediaLibCleaner::FilesAggregator object
 */
std::unique_ptr<MediaLibCleaner::FilesAggregator> _filesAggregator;

/**
 * Function calling lua_IsAuidoFile() function. This function is registered withing lua processor!
 *
 * @param[in] L	lua_State object to config file
 *
 * @return Number of output arguments on stack for lua processor
 */
static int lua_caller_isaudiofile(lua_State *L) {
	return lua_IsAudioFile(L, _filesAggregator->CurrentFile());
}

/**
 * Function calling lua_SetTags() function. This function is registered within lua processor!
 *
 * @param[in] L lua_State object to config file
 *
 * @return Number of output arguments on stack for lua processor
 */
static int lua_caller_settags(lua_State *L) {
	return lua_SetTags(L, _filesAggregator->CurrentFile());
}







// cerr and cout streams, if required by the user
/**
 * Global variable to store new std::cerr::rdbuf to allow user to define his own error stream (as a file)
 */
std::ofstream _cerr;

/**
 * Global variable to store new std::cout::rdbuf to allow user to define his own output stream (as a file)
 */
std::ofstream _cout;

// some variables required for aliases
int _total_files = 0;
time_t _datetime_raw = 0;







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
	_datetime_raw = time(nullptr);

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
	_path = lua_tostring(L, -1);
	_alert_log = lua_tostring(L, -2);
	_error_log = lua_tostring(L, -3);
	_error_level = static_cast<int>(lua_tonumber(L, -4));

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


	// BELOW ARE PROCEDURES TO SCAN GIVEN DIRECTORY AND RETRIEVE ALL INFO WE REQUIRE
	// create MediaLibCleaner::FilesAggregator object nad swap it with global variable one
	std::unique_ptr<MediaLibCleaner::FilesAggregator> filesAgg(new MediaLibCleaner::FilesAggregator());
	filesAgg.swap(_filesAggregator);

	// shorten namespace
	namespace fs = boost::filesystem;
	fs::path _workingdir(_path);

	if (!fs::exists(_workingdir) || !fs::is_directory(_workingdir)) {
		return 3; // ret. val; debug
	}

	fs::path dirpath = _workingdir;
	MediaLibCleaner::DFC *_currdfc = new MediaLibCleaner::DFC(_path);
	for (fs::recursive_directory_iterator dir(_path), dir_end; dir != dir_end; ++dir)
	{
		fs::path filepath = dir->path();

		if (!(filepath.has_filename() && filepath.has_extension())) {
			// not a file, but a directory!
			dirpath = filepath;

			_currdfc = new MediaLibCleaner::DFC(dirpath.string());

			continue;
		}

		// create File object for file
		MediaLibCleaner::File *filez = new MediaLibCleaner::File(filepath.wstring(), _currdfc);
		_filesAggregator->AddFile(filez);

		// increment total_files counter if audio file
		if (filez->IsInitiated()) _total_files++;

		// increment folder counter
		if (dirpath.string() != "")
			_currdfc->IncCount();
	}



	// DEBUG - ITERATE OVER COLLECTION
	auto end = _filesAggregator->end();
	for (auto it = _filesAggregator->begin(); it != end; ++it) {
		if (!(*it)->IsInitiated()) continue;

		std::wcout << "[" << (*it)->GetAlbum() << "] " << (*it)->GetArtist() << " - " << (*it)->GetTitle() << std::endl;
	}

	std::cout << std::endl << "TOTAL AUDIO FILES: " << _total_files << std::endl;


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
static std::wstring _ReplaceAllAliasOccurences(std::wstring& _wconfig, std::unique_ptr<MediaLibCleaner::File>& _audiofile) {
	std::wstring _newc = _wconfig;

	// copy original path to not confuse rest of the program
	std::wstring _path = _audiofile->GetPath();
	replaceAll(_path, L"\\", L"\\\\");

	// do the magic!
	// SONG DATA
	replaceAll(_newc, L"%artist%", _audiofile->GetArtist());
	replaceAll(_newc, L"%title%", _audiofile->GetTitle());
	replaceAll(_newc, L"%album%", _audiofile->GetAlbum());
	replaceAll(_newc, L"%genre%", _audiofile->GetGenre());
	replaceAll(_newc, L"%comment%", _audiofile->GetComment());
	replaceAll(_newc, L"%track%", std::to_wstring(_audiofile->GetTrack()));
	replaceAll(_newc, L"%year%", std::to_wstring(_audiofile->GetYear()));
	replaceAll(_newc, L"%albumartist%", _audiofile->GetAlbumArtist());
	replaceAll(_newc, L"%bpm%", _audiofile->GetBPM());
	replaceAll(_newc, L"%copyright%", _audiofile->GetCopyright());
	replaceAll(_newc, L"%language%", _audiofile->GetLanguage());
	replaceAll(_newc, L"%length%", _audiofile->GetTagLength());
	replaceAll(_newc, L"%mood%", _audiofile->GetMood());
	replaceAll(_newc, L"%origalbum%", _audiofile->GetOrigAlbum());
	replaceAll(_newc, L"%origartist%", _audiofile->GetOrigArtist());
	replaceAll(_newc, L"%origfilename%", _audiofile->GetOrigFilename());
	replaceAll(_newc, L"%origyear%", _audiofile->GetOrigYear());
	replaceAll(_newc, L"%publisher%", _audiofile->GetPublisher());
	replaceAll(_newc, L"%unsyncedlyrics%", _audiofile->GetLyricsUnsynced());
	replaceAll(_newc, L"%www%", _audiofile->GetWWW());

	// TECHNICAL INFO
	replaceAll(_newc, L"%_bitrate%", std::to_wstring(_audiofile->GetBitrate()));
	replaceAll(_newc, L"%_codec%", _audiofile->GetCodec());
	replaceAll(_newc, L"%_cover_mimetype%", _audiofile->GetCoverMimetype());
	replaceAll(_newc, L"%_cover_size%", std::to_wstring(_audiofile->GetCoverSize()));
	replaceAll(_newc, L"%_coder_type%", _audiofile->GetCoverType());
	replaceAll(_newc, L"%_covers%", std::to_wstring(_audiofile->GetCovers()));
	replaceAll(_newc, L"%_length%", _audiofile->GetLengthAsString());
	replaceAll(_newc, L"%_length_seconds%", std::to_wstring(_audiofile->GetLength()));
	replaceAll(_newc, L"%_channels%", std::to_wstring(_audiofile->GetChannels()));
	replaceAll(_newc, L"%_samplerate%", std::to_wstring(_audiofile->GetSampleRate()));


	// PATH INFO
	replaceAll(_newc, L"%_directory%", _audiofile->GetDirectory());
	replaceAll(_newc, L"%_ext%", _audiofile->GetExt());
	replaceAll(_newc, L"%_filename%", _audiofile->GetFilename());
	replaceAll(_newc, L"%_filename_ext%", _audiofile->GetFilenameExt());
	replaceAll(_newc, L"%_folderpath%", _audiofile->GetFolderPath());
	replaceAll(_newc, L"%_parent_dir%", _audiofile->GetParentDir());
	replaceAll(_newc, L"%_path%", _audiofile->GetPath());
#ifdef WIN32
	replaceAll(_newc, L"%_volume%", _audiofile->GetVolume());
#endif
	//replaceAll(_newc, "%_workingdir%", );
	//replaceAll(_newc, "%_workingpath%", );


	// FILES PROPERTIES
	replaceAll(_newc, L"%_file_create_date%", _audiofile->GetFileCreateDate());
	replaceAll(_newc, L"%_file_create_datetime%", _audiofile->GetFileCreateDatetime());
	replaceAll(_newc, L"%_file_create_datetime_raw%", std::to_wstring(_audiofile->GetFileCreateDatetimeRaw()));
	replaceAll(_newc, L"%_file_mod_date%", _audiofile->GetFileModDate());
	replaceAll(_newc, L"%_file_mod_datetime%", _audiofile->GetFileModDatetime());
	replaceAll(_newc, L"%_file_mod_datetime_raw%", std::to_wstring(_audiofile->GetFileModDatetimeRaw()));
	replaceAll(_newc, L"%_file_size%", _audiofile->GetFileSize());
	replaceAll(_newc, L"%_file_size_bytes%", std::to_wstring(_audiofile->GetFileSizeBytes()));
	replaceAll(_newc, L"%_file_size_kb%", _audiofile->GetFileSizeKB());
	replaceAll(_newc, L"%_file_size_mb%", _audiofile->GetFileSizeMB());


	// SYSTEM DATA
	//replaceAll(_newc, L"%_counter_dir%", _audiofile->GetComment());
	replaceAll(_newc, L"%_counter_total%", std::to_wstring(_total_files));
	replaceAll(_newc, L"%_date%", get_date_iso_8601_wide(_datetime_raw));
	replaceAll(_newc, L"%_datetime%", get_date_rfc_2822_wide(_datetime_raw));
	replaceAll(_newc, L"%_datetime_raw%", std::to_wstring(time(nullptr)));
	replaceAll(_newc, L"%_total_files%", std::to_wstring(_total_files));
	replaceAll(_newc, L"%_total_files_dir%", std::to_wstring(_audiofile->GetDFC()->GetCounter()));

	return _newc;
}