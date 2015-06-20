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
	_datetime_raw = time(NULL);

	// read config file to memory
	std::ifstream file("C:\\Users\\Szymon\\Documents\\test.lua");
	std::string config((std::istreambuf_iterator<char>(file)),
						std::istreambuf_iterator<char>());

	// replace all occurences in string and return it as new string
	//std::string _newconfig = _ReplaceAllAliasOccurences(config);
	// aliases are not supported in System section

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


	// BELOW ARE PROCEDURES TO SCAN GIVEN DIRECTORY AND RETRIEVE ALL INFO WE REQUIRE
	// create MediaLibCleaner::FilesAggregator object nad swap it with global variable one
	std::unique_ptr<MediaLibCleaner::FilesAggregator> filesAgg(new MediaLibCleaner::FilesAggregator());
	filesAgg.swap(_filesAggregator);

	// create MediaLibCleaner::DFCAggregator object nad swap it with global variable one
	//std::unique_ptr<MediaLibCleaner::DFCAggregator> dfcAgg(new MediaLibCleaner::DFCAggregator());
	//dfcAgg.swap(_dfcAggregator);

	// shorten namespace
	namespace fs = boost::filesystem;
	fs::path _workingdir(_path);

	if (!fs::exists(_workingdir) || !fs::is_directory(_workingdir)) {
		return 3; // ret. val; debug
	}

	fs::path dirpath = "";
	MediaLibCleaner::DFC *_currdfc = new MediaLibCleaner::DFC(_path);
	for (fs::recursive_directory_iterator dir(_path), dir_end; dir != dir_end; ++dir)
	{
		//std::cout << *dir << std::endl;
		fs::path filepath = dir->path();

		if (!(filepath.has_filename() && filepath.has_extension())) {
			// not a file, but a directory!
			dirpath = filepath;

			_currdfc = new MediaLibCleaner::DFC(dirpath.string());
			//_dfcAggregator->AddDirectory(_dfc);

			continue;
		}

		// create File object for file
		MediaLibCleaner::File *file = new MediaLibCleaner::File(filepath.string(), _currdfc);
		_filesAggregator->AddFile(file);

		// increment total_files counter if audio file
		if (file->IsInitiated()) _total_files++;

		// increment folder counter
		if (dirpath.string() != "")
			_currdfc->IncCount();
	}



	// DEBUG - ITERATE OVER COLLECTION
	std::list<MediaLibCleaner::File*>::iterator end = _filesAggregator->end();
	for (std::list<MediaLibCleaner::File*>::iterator it = _filesAggregator->begin(); it != end; ++it) {
		if (!(*it)->IsInitiated()) continue;

		std::cout << "[" << (*it)->GetAlbum() << "] " << (*it)->GetArtist() << " - " << (*it)->GetTitle() << std::endl;
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
static std::string _ReplaceAllAliasOccurences(std::string& _config, std::unique_ptr<MediaLibCleaner::File>& _audiofile) {
	std::string _newc = _config;

	std::cout << _newc << std::endl << std::endl;

	// copy original path to not confuse rest of the program
	std::string _path = _audiofile->GetPath();
	replaceAll(_path, "\\", "\\\\");

	// do the magic!
	// SONG DATA
	replaceAll(_newc, "%artist%", _audiofile->GetArtist());
	replaceAll(_newc, "%title%", _audiofile->GetTitle());
	replaceAll(_newc, "%album%", _audiofile->GetAlbum());
	replaceAll(_newc, "%genre%", _audiofile->GetGenre());
	replaceAll(_newc, "%comment%", _audiofile->GetComment());
	replaceAll(_newc, "%track%", std::to_string(_audiofile->GetTrack()));
	replaceAll(_newc, "%year%", std::to_string(_audiofile->GetYear()));
	//replaceAll(_newc, "%albumartist%", _audiofile->GetAlbumArtist());
	//replaceAll(_newc, "%bpm%", _audiofile->GetBPM());
	//replaceAll(_newc, "%copyright%", _audiofile->GetCopyright());
	//replaceAll(_newc, "%language%", _audiofile->GetLanguage());
	//replaceAll(_newc, "%length%", _audiofile->GetTagLength());
	//replaceAll(_newc, "%mood%", _audiofile->GetMood());
	//replaceAll(_newc, "%origalbum%", _audiofile->GetOrigAlubm());
	//replaceAll(_newc, "%origartist%", _audiofile->GetOrigArtist());
	//replaceAll(_newc, "%origfilename%", _audiofile->GetOrigFilename());
	//replaceAll(_newc, "%origyear%", _audiofile->GetOrigYear());
	//replaceAll(_newc, "%publisher%", _audiofile->GetPublisher());
	//replaceAll(_newc, "%unsyncedlyrics%", _audiofile->GetLyricsUnsynced());
	//replaceAll(_newc, "%www%", _audiofile->GetWWW());

	// TECHNICAL INFO
	replaceAll(_newc, "%_bitrate%", std::to_string(_audiofile->GetBitrate()));
	//replaceAll(_newc, "%_codec%", _audiofile->GetCodec());
	//replaceAll(_newc, "%_cover_mimetype%", _audiofile->GetCoverMimetype());
	//replaceAll(_newc, "%_cover_size%", std::to_string(_audiofile->GetCoverSize()));
	//replaceAll(_newc, "%_coder_type%", _audiofile->GetCoverType());
	//replaceAll(_newc, "%_covers%", std::to_string(_audiofile->GetCovers()));
	replaceAll(_newc, "%_length%", _audiofile->GetLengthAsString());
	replaceAll(_newc, "%_length_seconds%", std::to_string(_audiofile->GetLength()));
	replaceAll(_newc, "%_channels%", std::to_string(_audiofile->GetChannels()));
	replaceAll(_newc, "%_samplerate%", std::to_string(_audiofile->GetSampleRate()));


	// PATH INFO
	replaceAll(_newc, "%_directory%", _audiofile->GetDirectory());
	replaceAll(_newc, "%_ext%", _audiofile->GetExt());
	replaceAll(_newc, "%_filename%", _audiofile->GetFilename());
	replaceAll(_newc, "%_filename_ext%", _audiofile->GetFilenameExt());
	replaceAll(_newc, "%_folderpath%", _audiofile->GetFolderPath());
	replaceAll(_newc, "%_parent_dir%", _audiofile->GetParentDir());
	replaceAll(_newc, "%_path%", _audiofile->GetPath());
#ifdef WIN32
	replaceAll(_newc, "%_volume%", _audiofile->GetVolume());
#endif
	//replaceAll(_newc, "%_workingdir%", );
	//replaceAll(_newc, "%_workingpath%", );


	// FILES PROPERTIES
	replaceAll(_newc, "%_file_create_date%", _audiofile->GetFileCreateDate());
	replaceAll(_newc, "%_file_create_datetime%", _audiofile->GetFileCreateDatetime());
	replaceAll(_newc, "%_file_create_datetime_raw%", std::to_string(_audiofile->GetFileCreateDatetimeRaw()));
	replaceAll(_newc, "%_file_mod_date%", _audiofile->GetFileModDate());
	replaceAll(_newc, "%_file_mod_datetime%", _audiofile->GetFileModDatetime());
	replaceAll(_newc, "%_file_mod_datetime_raw%", std::to_string(_audiofile->GetFileModDatetimeRaw()));
	replaceAll(_newc, "%_file_size%", _audiofile->GetFileSize());
	replaceAll(_newc, "%_file_size_bytes%", std::to_string(_audiofile->GetFileSizeBytes()));
	replaceAll(_newc, "%_file_size_kb%", _audiofile->GetFileSizeKB());
	replaceAll(_newc, "%_file_size_mb%", _audiofile->GetFileSizeMB());


	// SYSTEM DATA
	//replaceAll(_newc, "%_counter_dir%", _audiofile->GetComment());
	//replaceAll(_newc, "%_counter_total%", _audiofile->GetComment());
	replaceAll(_newc, "%_date%", get_date_iso_8601(_datetime_raw));
	replaceAll(_newc, "%_datetime%", get_date_rfc_2822(_datetime_raw));
	replaceAll(_newc, "%_datetime_raw%", std::to_string(time(NULL)));
	replaceAll(_newc, "%_total_files%", std::to_string(_total_files));
	//replaceAll(_newc, "%_total_files_dir%", _audiofile->GetComment());

	std::cout << _newc << std::endl;

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
	lua_close(L);*/