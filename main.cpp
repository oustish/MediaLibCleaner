/**
 * @file
 * @author Szymon Oracki <szymon.oracki@oustish.pl>
 * @version 0.4
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
int error_level = 0,
/**
 * Global variable containing user defined max. amount of threads to be used in process() and scan() functions; 0 - unlimited
 */
	max_threads = 0;

/**
 * Global variable representing MediaLibCleaner::FilesAggregator object
 */
std::unique_ptr<MediaLibCleaner::FilesAggregator> filesAggregator;

/**
* Global variable containing all currently processed MediaLibCleaner::File object by different threads
*/
MediaLibCleaner::File** current_file_thd;

/**
* Global variable containing MediaLibCleaner::LogAlert object
*/
std::unique_ptr<MediaLibCleaner::LogAlert> alertlog;

/**
* Global variable containing MediaLibCleaner::LogProgram object
*/
std::unique_ptr<MediaLibCleaner::LogProgram> programlog;

/**
* Global variable containing files total count (used for alias)
*/
int total_files = 0;

/**
* Global variable containing timestamp of program startup
*/
time_t datetime_raw = 0;

/**
 * Function calling lua_IsAudioFile() function. This function is registered withing lua processor!
 *
 * @param[in] L	lua_State object to config file
 *
 * @return Number of output arguments on stack for lua processor
 */
static int lua_caller_isaudiofile(lua_State *L) {
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_IsAudioFile(L, cfile, &programlog, &alertlog);
}

/**
* Function calling lua_SetTags() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_settags(lua_State *L) {
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_SetTags(L, cfile, &programlog, &alertlog);
}

/**
* Function calling lua_RemoveTags() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_removetags(lua_State *L) {
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_RemoveTags(L, cfile, &programlog, &alertlog);
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
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_SetRequiredTags(L, cfile, &programlog, &alertlog);
}

/**
* Function calling lua_CheckTagsValues() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_checktagvalues(lua_State *L)
{
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_CheckTagValues(L, cfile, &programlog, &alertlog);
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
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_Rename(L, cfile, &programlog, &alertlog);
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
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_Move(L, cfile, &programlog, &alertlog);
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
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_Delete(L, cfile, &programlog, &alertlog);
}

/**
* Function calling lua_Log() function. This function is registered within lua processor!
*
* @param[in] L lua_State object to config file
*
* @return Number of output arguments on stack for lua processor
*/
static int lua_caller_log(lua_State *L)
{
	lua_getglobal(L, "__thread");
	int thd = static_cast<int>(lua_tonumber(L, -1));
	auto cfile = current_file_thd[thd];

	return lua_Log(L, cfile, &programlog, &alertlog);
}







/**
 * Main function
 * 
 * @param[in] argc	Amount of input arguments (always >= 1)
 * @param[in] argv	Input parameters
 *
 * @return Exit code for entire program
 */
int main(int argc, char *argv[]) {
	//>> - T: All engines look good, begining roll programe.
	//>>      Prepare for stage 1 separation... Stage 1!
	//>>      There is Mach 1.
	//>>      Everybody good? Plenty of slaves for my robot colony?
	//>> - D: They gave him a humor settings to fit better with his unit... He thinks it relaxes us.
	//>> - c: A giant sarcastic robot... What a great idea.
	//>> - T: I have cue light I can use <cue light flashes> when I joke if you like.
	//>> - C: That would propably help.
	//>> - T: Yeah, you can use it to find your way back to the ship after I blow you out the airlock... <cue light flashes>
	//>> - C: What's your humor setting, TARS?
	//>> - T: That's 100%.
	//>> - C: Yeah, bring it down to 75%, please.
	//>> - T: Stage 2 separation.


	// for performance wizard
	Sleep(5 * 1000); // 5 secs initial halt


	// capture time app was launched
	datetime_raw = time(nullptr);

	// easy way to fix found problems with diacritic letters in filenames :)
	// it wasn't so easy to find though...
	setlocale(LC_ALL, "");

	
	// get cmd line parameters figured out
	namespace po = boost::program_options;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("config", po::value<std::string>(), "path to LUA config file")
		;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	// display help message if needed
	if (vm.count("help"))
	{
		std::cerr << desc << std::endl;
		return 7;
	}

	// if not, try to figure out which config file to use
	std::wstring wconfig;
	if (vm.count("config")) // LUA
	{
		std::string rpath = vm["config"].as<std::string>();
		// check if path exists, then continue
		if (!boost::filesystem::exists(rpath) || !boost::filesystem::is_regular_file(rpath)) {
			std::wcout << L"ERROR: Config LUA file does not exists in the given path. Please make sure file exists and path given is correct." << std::endl;
			std::cout << desc << std::endl;
			return 6;
		}

		// reading config file into wstring
		std::wifstream wfile(s2ws(rpath));
		wfile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		std::wstringstream wss;
		wss << wfile.rdbuf();
		wconfig = wss.str();
	}
	else // OOPS, NO MATCHES
	{
		std::cerr << desc << std::endl;
		return 7;
	}

	std::wcout << L"Beginning program..." << std::endl;

#ifdef _MLC_DEBUG
	//debug
	static std::basic_stringbuf<std::ostream::char_type> buf;
	std::cerr.rdbuf(&buf);
#endif

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
	lua_register(L, "_RemoveTags", lua_caller_removetags);
	lua_register(L, "_SetRequiredTags", lua_caller_setrequiredtags);
	lua_register(L, "_CheckTagValues", lua_caller_checktagvalues);
	lua_register(L, "_Rename", lua_caller_rename);
	lua_register(L, "_Move", lua_caller_move);
	lua_register(L, "_Delete", lua_caller_delete);
	lua_register(L, "_Log", lua_caller_log);

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
	lua_getglobal(L, "_max_threads"); // -5
	lua_getglobal(L, "_error_level"); // -4
	lua_getglobal(L, "_error_log"); // -3
	lua_getglobal(L, "_alert_log"); // -2
	lua_getglobal(L, "_path"); // -1

	// check if returning values have proper type
	if (!lua_isstring(L, -1) || !lua_isstring(L, -2) || !lua_isstring(L, -3) || !lua_isnumber(L, -4) || !lua_isnumber(L, -5)) {
		std::wcerr << L"One or more of startup LUA parameters is incorrect. Exiting..." << std::endl;
		return 2; // return error code here, so entire program will end; it's desired; debug
	}

	// read all parameters
	path = lua_tostring(L, -1);
	alert_log = lua_tostring(L, -2);
	error_log = lua_tostring(L, -3);
	error_level = static_cast<int>(lua_tonumber(L, -4));
	max_threads = static_cast<int>(lua_tonumber(L, -5));


	//>> - C: It's hard to leave everything... My kids, your father...
	//>> - B: We're gonna be spending a lot of time together.
	//>> - C: We should learn to talk.
	//>> - B: And when not to.
	//>>      Just being honest.
	//>> - C: I don't think you need be that honest.
	//>>      Hey, TARS? What's your honesty parameter?
	//>> - T: 90%.
	//>> - C: 90%?
	//>> - T: Absolute honesty isn't always the most dyplomatic nor the safest form of comunication with emotional beings.
	//>> - C: OK. 90% it is, doctor Brand.


	std::unique_ptr<MediaLibCleaner::LogProgram> temp(new MediaLibCleaner::LogProgram(s2ws(error_log), error_level));
	programlog.swap(temp);

	std::unique_ptr<MediaLibCleaner::LogAlert> temp2(new MediaLibCleaner::LogAlert(s2ws(alert_log)));
	alertlog.swap(temp2);

	lua_close(L);

	// log all input LUA parameters
	programlog->Log(L"Main", L"_path value: " + s2ws(path), 3);
	programlog->Log(L"Main", L"_alert_log value: " + s2ws(alert_log), 3);
	programlog->Log(L"Main", L"_error_log value: " + s2ws(error_log), 3);
	programlog->Log(L"Main", L"_error_level value: " + std::to_wstring(error_level), 3);
	programlog->Log(L"Main", L"_max_threads value: " + std::to_wstring(max_threads) , 3);

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
	MediaLibCleaner::PathsAggregator* path_list = new MediaLibCleaner::PathsAggregator(&programlog, &alertlog);

	std::wcout << L"Scanning for files..." << std::endl;

	// this bit of code will be executed on single core
	programlog->Log(L"Main", L"Beginning scan for files inside working dir", 3);

	for (boost::filesystem::recursive_directory_iterator dir(path), dir_end; dir != dir_end; ++dir)
	{
		boost::filesystem::path filepath = dir->path();

		programlog->Log(L"Main", L"Adding path to list: " + filepath.generic_wstring(), 3);

		path_list->AddPath(filepath);
	}


	//>> - D: We're coming up on the Endurance. 12 minutes out.
	//>> - C: OK, taking control.
	//>>      Approaching module port, 500 meters.
	//>>      It's all you, Doyle.
	//>>      Nice and easy, Doyle. Nice and easy.
	//>> - D: I'm feeling good.
	//>> - C: Take us home.
	//>> - D: Locked.
	//>> - C: Target locked.
	//>> - D: Ok, helmets on.
	//>> - B: Good job.


	// parsing paths and files
	// full multi-core support (in theory)
	programlog->Log(L"Main", L"Beginning parsing paths and files", 3);
	std::wcout << L"Scanning files..." << std::endl;
	scan(&dfc_list, path_list, &programlog, &alertlog, path, &filesAggregator, &total_files);


	// ITERATE OVER COLLECTION AND PROCESS FILES
	// multi-core
	int thdmax = omp_get_max_threads();
	current_file_thd = new MediaLibCleaner::File*[thdmax];


	//>> - C: Rommilly, are you reading these forces?
	//>> - R: Unbelievable.
	//>> - D: A litteral heart of darkness.
	//>> - R: If we could just see the collapsed star insinde - the singularity - yeah, we'd solve gravity.
	//>> - C: We can't get anything from it?
	//>> - R: Nothing escapes that horizon, not even light. All the answers there, just no way to see it.
	//>> - B: There's Millers planet.
	//>> - R: Goodbye, Ranger.


	programlog->Log(L"Main", L"Starting iteration through collection.", 3);
	std::wcout << L"Processing files..." << std::endl;
	process(wconfig, &filesAggregator, &programlog);


	// moment of relax :)
	// follow it to the end

	//>> - Did it worked?
	//>> - I think it may have.
	//>> - How do you know?


	// deleting DFC objects
	for (auto it = dfc_list.begin(); it != dfc_list.end(); ++it)
		delete (*it);

	// deleting other things
	delete[] current_file_thd;
	delete path_list;


	//>> - Because the bulk being are closing the tesseract.
	//>> - Don't you get it yet, TARS? They're not beings. They're us! (...)
	//>> - Cooper, people couldn't build it.


	// saving time diff for log output
	time_t dt_end = time(nullptr);
	time_t diff = dt_end - datetime_raw;

	programlog->Log(L"Main", L"Program execution time: " + std::to_wstring(diff) + L" sec", 3);
	programlog->Log(L"Main", L"Total files: " + std::to_wstring(total_files), 3);


	//>> - No. No, not yet. But one day. Not you and me, but a people. The civilization that evolved past the dimmensions that we know.

	// needs to be done this way to avoid massive amount of std::wcout calls upon destruction
	MediaLibCleaner::FilesAggregator *d = filesAggregator.release();
	delete d;

	programlog->Log(L"Main", L"Program finished", 3);

	return 0;


	//>>   What happens now?
}








/**
 * Function reporting errors of lua processor to std::cerr.
 *
 * This functions purpose is to check for errors after *.lua config file
 * has been processed and, if present, print them to std::cerr.
 *
 * @param[in] L       A lua_State object holding information about lua processor
 * @param[in] status  Integer with status code returned after calling lua_pcall() or similar function
 */
void lua_error_reporting(lua_State *L, int status) {
	lua_ErrorReporting(L, status, &programlog);
}










/**
* Function processing all files inside fA object according to rules in wconfig LUA file
*
* Function calls lua functions required to process given file according to rules specified by the user.
* It uses OpenMP directives to force the code to run in multi-thread environment.
* Function gets MediaLibCleaner::File object, replaces all alias occurences in wconfig, then registers all LUA functions and executes the LUA script.
* Each started threat exits as soon as fA->next() method will return nullptr; function exits as soon as all threads will exit (OpenMP sets auto barrier at the end of the block).
*
* @param[in] wconfig std::wstring containing LUA config file
* @param[in] fA MediaLibCleaner::FilesAggregator object containing all files that will be processed
* @param[in] lp MediaLibCleaner::LogProgram object for logging purposses
*/
void process(std::wstring wconfig, std::unique_ptr<MediaLibCleaner::FilesAggregator>* fA, std::unique_ptr<MediaLibCleaner::LogProgram>* lp)
{
	std::wstring new_config, wid;
	lua_State *L = nullptr;
	std::string nc;
	int s = 0, id = 0;
	MediaLibCleaner::File* cfile;

	(*fA)->rewind();

	if (max_threads > 0)
		omp_set_num_threads(max_threads);


	//>> - R: If we're talking about a couple of years, I can use time to research gravity. Observations from the wormhole - that's gold to professor Brand. 


	#pragma omp parallel shared(lp, fA, wconfig) private(new_config, L, nc, s, cfile, id, wid)
	{
		id = omp_get_thread_num();
		wid = std::to_wstring(id);

		(*lp)->Log(L"Process (" + wid + L")", L"Thread starting", 3);

		cfile = (*fA)->next();
		do {
			(*lp)->Log(L"Process (" + wid + L")", L"File: " + cfile->GetPath(), 3);
			(*lp)->Log(L"Process (" + wid + L")", L"Creating config file", 3);
			new_config = MediaLibCleaner::ReplaceAllAliasOccurences(wconfig, cfile, path, datetime_raw, total_files);

			(*lp)->Log(L"Process (" + wid + L")", L"Lua procesor init", 3);
			lua_State *L = luaL_newstate();
			luaL_openlibs(L);

			(*lp)->Log(L"Process (" + wid + L")", L"Registering functions", 3);
			// register C functions in lua processor
			lua_register(L, "_IsAudioFile", lua_caller_isaudiofile);
			lua_register(L, "_SetTags", lua_caller_settags);
			lua_register(L, "_RemoveTags", lua_caller_removetags);
			lua_register(L, "_SetRequiredTags", lua_caller_setrequiredtags);
			lua_register(L, "_CheckTagValues", lua_caller_checktagvalues);
			lua_register(L, "_Rename", lua_caller_rename);
			lua_register(L, "_Move", lua_caller_move);
			lua_register(L, "_Delete", lua_caller_delete);
			lua_register(L, "_Log", lua_caller_log);

			(*lp)->Log(L"Process (" + wid + L")", L"Converting wide string to string", 3);
			nc = ws2s(new_config);

			(*lp)->Log(L"Process (" + wid + L")", L"Lua procesor loads string", 3);
			s = luaL_loadstring(L, nc.c_str());

			lua_pushstring(L, "");
			lua_setglobal(L, "_action");

			lua_pushinteger(L, id);
			lua_setglobal(L, "__thread");

			current_file_thd[id] = cfile;

			(*lp)->Log(L"Process (" + wid + L")", L"Executing script", 3);
			// exetute script
			if (s == 0) {
				s = lua_pcall(L, 0, LUA_MULTRET, 0);
			}
			if (s != 0) { // because error code may change after execution
				// report any errors, if found
				(*lp)->Log(L"Process (" + wid + L")", L"Error occured", 3);
				lua_error_reporting(L, s);
			}

			lua_close(L);

			cfile->save();
			cfile = (*fA)->next();
		} while (cfile != nullptr);

		(*lp)->Log(L"Process (" + wid + L")", L"Thread exiting", 3);
	}
}

/**
* Function scanning given directory to find all files
*
* Function calls all required functions and creates MediaLibCleaner::File object for each file found in previous steps.
* Each started threat exits as soon as pathl->next() method will return nullptr; function exits as soon as all threads will exit (OpenMP sets auto barrier at the end of the block).
*
* @param[in] dfcl std::list object containing MediaLibCleaner::DFC objects
* @param[in] pathl MediaLibCleaner::PathsAggregator object containing all files paths
* @param[in] lp MediaLibCleaner::LogProgram object for logging purposes
* @param[in] la MediaLibCleaner::LogAlert object for logging purposes
* @param[in] pth Scanning insertion path
* @param[out] fA std::unique_ptr to MediaLibCleaner::FilesAggregator object into which all new MediaLibCleaner::File objects will be saved
* @param[out] tf Total files amount (global)
*/
void scan(std::list<MediaLibCleaner::DFC*>* dfcl, MediaLibCleaner::PathsAggregator* pathl, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la,
	std::string pth, std::unique_ptr<MediaLibCleaner::FilesAggregator>* fA, int* tf)
{
	std::mutex dfcl_mutex;
	MediaLibCleaner::DFC *currdfc = MediaLibCleaner::AddDFC(dfcl, pth, &dfcl_mutex, lp, la);
	boost::filesystem::path dirpath, currpath;
	int id = 0;
	std::wstring wid;

	pathl->rewind();

	if (max_threads > 0)
		omp_set_num_threads(max_threads);

	#pragma omp parallel shared(dfcl_mutex, pathl, lp, la, pth, fA, tf, dfcl) private(currdfc, currpath, dirpath, id, wid)
	{
		id = omp_get_thread_num();
		wid = std::to_wstring(id);

		currdfc = MediaLibCleaner::AddDFC(dfcl, pth, &dfcl_mutex, lp, la);

		(*lp)->Log(L"Scan (" + wid + L")", L"Thread starting", 3);

		currpath = pathl->next();

		do
		{
			(*lp)->Log(L"Scan (" + wid + L")", L"Current file: " + currpath.generic_wstring(), 3);

			if (boost::filesystem::is_directory(currpath)) {
				(*lp)->Log(L"Scan (" + wid + L")", L"Current file is a directory.", 3);

				// not a file, but a directory!
				dirpath = currpath;

				currdfc = MediaLibCleaner::AddDFC(dfcl, dirpath, &dfcl_mutex, lp, la);

				currpath = pathl->next();
				continue;
			}

			// create File object for file
			(*lp)->Log(L"Scan (" + wid + L")", L"Creating MediaLibCleaner::File object for file.", 3);
			MediaLibCleaner::File *filez = new MediaLibCleaner::File(currpath.generic_wstring(), currdfc, lp, la);
			(*fA)->AddFile(filez);

			// increment total_files counter if audio file
			if (filez->IsInitiated()) {
				#pragma omp critical
				{
					(*tf)++;
				}
			}

			currpath = pathl->next();
		} while (currpath != "");

		(*lp)->Log(L"Scan (" + wid + L")", L"Thread exiting", 3);
	}
}