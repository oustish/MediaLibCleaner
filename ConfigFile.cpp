#include "ConfigFile.hpp"

using namespace MediaLibCleaner;

ConfigFile::ConfigFileExpressionEvaluator::ConfigFileExpressionEvaluator(std::string d_task, int d_task_nr, std::string expr)
{
	this->expression = expr;
	this->task = d_task;
	this->task_nr = d_task_nr;

	if (expr == "true") // default value for expression
	{
		return;
	}

	// form:
	// <|oper1|>operation>|oper2|<
	// reflected in regex expression 'e' below
	// lets divide them into parts

	static const boost::regex e("<\\|(.*)\\|>(.*)>\\|(.*)\\|<");
	static const boost::regex o("([^;]*)");
	boost::smatch match1, match2;

	boost::regex_search(expr, match1, e);

	// match1[1] is oper1
	// match1[2] is operation
	// match1[3] is oper2

	this->oper1 = match1[1].str();

	// find out what kind of operation is going on
	std::string oper = match1[2].str();
	if (oper == "eq")
		this->operation = OPERATION_EQUAL;
	else if (oper == "!eq")
		this->operation = OPERATION_NOT_EQUAL;
	else if (oper == "gt")
		this->operation = OPERATION_GREATER;
	else if (oper == "ge")
		this->operation = OPERATION_GREATER_EQUAL;
	else if (oper == "lt")
		this->operation = OPERATION_LOWER;
	else if (oper == "le")
		this->operation = OPERATION_LOWER_EQUAL;
	else
	{
		this->operation = OPERATION_UNKNOWN;
		return;
	}

	// oper2 is string with ';' delimeted values
	// reflected in regex expresion 'o' above
	// lets divide them
	boost::regex_search(match1[3].str(), match2, o);

	for (int i = 0; i < match2.size(); ++i)
	{
		this->oper2.push_back(match2[i + 1]);
	}

	// that's all folks
	// rest of processing is done by evaluate()
}


ConfigFile::ConfigFileExpressionEvaluator::~ConfigFileExpressionEvaluator() {}


std::wstring ConfigFile::ConfigFileExpressionEvaluator::evaluate()
{
	if (this->expression == "true")
		return L"true";

	if (this->operation == OPERATION_UNKNOWN)
	{
		// debug message here
		return L"false";
	}

	if (this->oper1 == "" || !this->oper2.size())
	{
		// debug message here
		return L"false";
	}

	std::wstring retval, oper1 = s2ws(this->oper1);

	for (int i = 0; i < this->oper2.size(); ++i)
	{
		if (i % 2 == 0) // operands
		{
			retval += L"(";
			retval += oper1;
			switch (this->operation)
			{
			case OPERATION_EQUAL:
				retval += L" == ";
				break;
			case OPERATION_NOT_EQUAL:
				retval += L" ~= ";
				break;
			case OPERATION_GREATER:
				retval += L" > ";
				break;
			case OPERATION_GREATER_EQUAL:
				retval += L" >= ";
				break;
			case OPERATION_LOWER:
				retval += L" < ";
				break;
			case OPERATION_LOWER_EQUAL:
				retval += L" <= ";
				break;
			case OPERATION_UNKNOWN: break;
			default: break;
			}
			retval += s2ws(this->oper2[i]);
			retval += L")";
		}
		else // and/or values
		{
			if (this->oper2[i] == "and")
			{
				retval += L" and ";
			}
			else if (this->oper2[i] == "or")
			{
				retval += L" or ";
			}
		}
	}

	return retval;
}


ConfigFile::ConfigFileReader::ConfigFileReader(int argc, char *argv[])
{
	// init this->config.tasks
	this->config.tasks[0] = TASK_NONE;
	this->config.tasks[1] = TASK_NONE;
	this->config.tasks[2] = TASK_NONE;
	this->config.tasks[3] = TASK_NONE;
	this->config.tasks[4] = TASK_NONE;

	boost::program_options::variables_map vm = this->parse_command_line(argc, argv);

	if (vm.count("help"))
	{
		this->isHelp = true;
		return;
	}

	if (vm.count("config"))
	{
		this->isConfigFile = true;
		this->cfg_file = vm["config"].as<std::string>();

		boost::filesystem::path cfgfilepath = this->cfg_file;
		if (boost::filesystem::exists(cfgfilepath) && boost::filesystem::is_regular_file(cfgfilepath))
		{
			if (vm.count("force-lua-config"))
			{
				this->cfg_type = CFG_FILE_LUA;
				return;
			}
			else if (vm.count("force-ini-config"))
			{
				this->cfg_type = CFG_FILE_INI;
				return;
			}
			else if (cfgfilepath.has_extension())
			{
				boost::filesystem::path ext = cfgfilepath.extension();

				if (ext == ".lua")
				{
					this->cfg_type = CFG_FILE_LUA;
					return;
				}
				else if (ext == ".ini")
				{
					this->cfg_type = CFG_FILE_INI;
				}
				else
				{
					this->cfg_file = CFG_FILE_UNKNOWN;
					return;
				}

			}
			else
			{
				std::wcerr << L"Could not find out what type of config file was specified. Use '--force-ini-config' or '--force-lua-config' to help." << std::endl << std::endl;

				this->isHelp = true;
				return;
			}
		}
	}

	if (this->cfg_type == CFG_FILE_INI)
	{
		vm = this->parse_config_file(vm["path"].as<std::string>());
		config = configini;

		if (vm.count("System.tasks"))
		{
			std::vector<std::string> d_tasks = vm["System.tasks"].as<std::vector<std::string>>();

			if (d_tasks.size() > 5)
			{
				// too much tasks to perform, limit is 5
				std::wcerr << L"Too much tasks to perform, limit is 5." << std::endl << std::endl;
				return;
			}

			int i = 0;
			for (auto it = d_tasks.begin(); it != d_tasks.end(); ++it)
			{
				std::string t = *it;
				t = boost::locale::to_lower(t);

				if (t == "tagsnormalization")
				{
					this->config.tasks[i] = TASK_TAGS_NORMALIZATION;
				}
				else if (t == "namesnormalization")
				{
					this->config.tasks[i] = TASK_NAMES_NORMALIZATION;
				}
				else if (t == "requiredtagsscan")
				{
					this->config.tasks[i] = TASK_REQUIRED_TAGS_SCAN;
				}
				else if (t == "filesblacklist")
				{
					this->config.tasks[i] = TASK_FILES_BLACKLIST;
				}
				else if (t == "fileswhitelist")
				{
					this->config.tasks[i] = TASK_FILES_WHITELIST;
				}
				else
				{
					// decrementing counter, because program did not found any useful data in last iteration
					// hence the tasks[i] will be TASK_NONE unless done something else
					// i will be incremented in a moment
					--i;
				}

				++i;
			}
		}
		else
		{
			// if no tasks is specified - no actions will be taken
			// no point in running the program :)
			std::wcerr << L"No tasks specified." << std::endl << std::endl;
			return;
		}

		// check nn_rules, tn_rules, rt_rules, wl_rules and bl_wules
		// if they have required values
		if (this->config.nn_rules < 0)
			this->config.nn_rules = 0;
		else if (this->config.nn_rules > 20)
			this->config.nn_rules = 20;

		if (this->config.tn_rules < 0)
			this->config.tn_rules = 0;
		else if (this->config.tn_rules > 20)
			this->config.tn_rules = 20;

		if (this->config.rt_rules < 0)
			this->config.rt_rules = 0;
		else if (this->config.rt_rules > 20)
			this->config.rt_rules = 20;

		if (this->config.wl_rules < 0)
			this->config.wl_rules = 0;
		else if (this->config.wl_rules > 20)
			this->config.wl_rules = 20;

		if (this->config.bl_rules < 0)
			this->config.bl_rules = 0;
		else if (this->config.bl_rules > 20)
			this->config.bl_rules = 20;

		// now time to analyze expression rules for nn
		for (int i = 1; i <= this->config.nn_rules; ++i)
		{
			std::string act = vm["NamesNormalization.rule" + std::to_string(i) + ".action"].as<std::string>();
			std::string expr = vm["NamesNormalization.rule" + std::to_string(i) + ".expression"].as<std::string>();
			this->config.nn_rule[i - 1].expression = new ConfigFile::ConfigFileExpressionEvaluator("NamesNormalization", i, expr);

			act = boost::locale::to_lower(act);
			if (act == "move")
				this->config.nn_rule[i - 1].action = ACTION_MOVE;
			else if (act == "rename")
				this->config.nn_rule[i - 1].action = ACTION_RENAME;
			else
				this->config.nn_rule[i - 1].action = ACTION_UNKNOWN;
		}

		// ... and for tn's
		for (int i = 1; i <= this->config.tn_rules; ++i)
		{
			std::string act = vm["TagsNormalization.rule" + std::to_string(i) + ".action"].as<std::string>();
			std::string expr = vm["TagsNormalization.rule" + std::to_string(i) + ".expression"].as<std::string>();

			act = boost::locale::to_lower(act);
			if (act == "delete")
				this->config.tn_rule[i - 1].action = ACTION_DELETE;
			else if (act == "set")
				this->config.tn_rule[i - 1].action = ACTION_SET;
			else
				this->config.tn_rule[i - 1].action = ACTION_UNKNOWN;

			this->config.tn_rule[i - 1].expression = new ConfigFile::ConfigFileExpressionEvaluator("TagsNormalization", i, expr);
		}

		// ... and for whitelist's
		for (int i = 0; i < this->config.wl_rules; ++i)
		{
			std::string expr = vm["FilesWhitelist.rule" + std::to_string(i) + ".expression"].as<std::string>();

			this->config.whitelist_rule[i - 1].expression = new ConfigFileExpressionEvaluator("FilesWhitelist", i, expr);
			this->config.whitelist_rule[i - 1].isWhitelist = true;
		}

		// ... and finaly, blacklists
		for (int i = 0; i < this->config.bl_rules; ++i)
		{
			std::string expr = vm["FilesBlacklist.rule" + std::to_string(i) + ".expression"].as<std::string>();

			this->config.blacklist_rule[i - 1].expression = new ConfigFileExpressionEvaluator("FilesBlacklist", i, expr);
		}
	}
	else if (this->cfg_type == CFG_FILE_NONE)
	{
		// no config file was specified
		// analyzing cmd line params

		if (vm.count("tasks"))
		{
			std::vector<std::string> d_tasks = vm["tasks"].as<std::vector<std::string>>();

			if (d_tasks.size() > 5)
			{
				// too much tasks to perform, limit is 5
				std::wcerr << L"Too much tasks to perform, limit is 5." << std::endl << std::endl;

				this->isHelp = true;
				return;
			}

			int i = 0;
			for (auto it = d_tasks.begin(); it != d_tasks.end(); ++it)
			{
				std::string t = *it;
				t = boost::locale::to_lower(t);

				if (t == "tagsnormalization")
				{
					this->config.tasks[i] = TASK_TAGS_NORMALIZATION;
				}
				else if (t == "namesnormalization")
				{
					this->config.tasks[i] = TASK_NAMES_NORMALIZATION;
				}
				else if (t == "requiredtagsscan")
				{
					this->config.tasks[i] = TASK_REQUIRED_TAGS_SCAN;
				}
				else if (t == "filesblacklist")
				{
					this->config.tasks[i] = TASK_FILES_BLACKLIST;
				}
				else if (t == "fileswhitelist")
				{
					this->config.tasks[i] = TASK_FILES_WHITELIST;
				}
				else
				{
					// decrementing counter, because program did not found any useful data in last iteration
					// hence the tasks[i] will be TASK_NONE unless done something else
					// i will be incremented in a moment
					--i;
				}

				++i;
			}
		}
		else
		{
			// if no tasks is specified - no actions will be taken
			// no point in running the program :)
			std::wcerr << L"No tasks specified." << std::endl << std::endl;

			this->isHelp = true;
			return;
		}

		// check nn_rules, tn_rules, rt_rules, wl_rules and bl_wules
		// if they have required values
		if (this->config.nn_rules < 0)
			this->config.nn_rules = 0;
		else if (this->config.nn_rules > 20)
			this->config.nn_rules = 20;

		if (this->config.tn_rules < 0)
			this->config.tn_rules = 0;
		else if (this->config.tn_rules > 20)
			this->config.tn_rules = 20;

		if (this->config.rt_rules < 0)
			this->config.rt_rules = 0;
		else if (this->config.rt_rules > 20)
			this->config.rt_rules = 20;

		if (this->config.wl_rules < 0)
			this->config.wl_rules = 0;
		else if (this->config.wl_rules > 20)
			this->config.wl_rules = 20;

		if (this->config.bl_rules < 0)
			this->config.bl_rules = 0;
		else if (this->config.bl_rules > 20)
			this->config.bl_rules = 20;

		// now time to analyze expression rules for nn
		for (int i = 1; i <= this->config.nn_rules; ++i)
		{
			std::string act = vm["nn-rule" + std::to_string(i) + "-action"].as<std::string>();
			std::string expr = vm["nn-rule" + std::to_string(i) + "-expression"].as<std::string>();
			this->config.nn_rule[i - 1].expression = new ConfigFile::ConfigFileExpressionEvaluator("NamesNormalization", i, expr);

			act = boost::locale::to_lower(act);
			if (act == "move")
				this->config.nn_rule[i - 1].action = ACTION_MOVE;
			else if (act == "rename")
				this->config.nn_rule[i - 1].action = ACTION_RENAME;
			else
				this->config.nn_rule[i - 1].action = ACTION_UNKNOWN;
		}

		// ... and for tn's
		for (int i = 1; i <= this->config.tn_rules; ++i)
		{
			std::string act = vm["tn-rule" + std::to_string(i) + "-action"].as<std::string>();
			std::string expr = vm["tn-rule" + std::to_string(i) + "-expression"].as<std::string>();

			act = boost::locale::to_lower(act);
			if (act == "delete")
				this->config.tn_rule[i - 1].action = ACTION_DELETE;
			else if (act == "set")
				this->config.tn_rule[i - 1].action = ACTION_SET;
			else
				this->config.tn_rule[i - 1].action = ACTION_UNKNOWN;

			this->config.tn_rule[i - 1].expression = new ConfigFile::ConfigFileExpressionEvaluator("TagsNormalization", i, expr);
		}

		// ... and for whitelist's
		for (int i = 0; i < this->config.wl_rules; ++i)
		{
			std::string expr = vm["wh-rule" + std::to_string(i) + "-expression"].as<std::string>();

			this->config.whitelist_rule[i - 1].expression = new ConfigFileExpressionEvaluator("FilesWhitelist", i, expr);
			this->config.whitelist_rule[i - 1].isWhitelist = true;
		}

		// ... and finaly, blacklists
		for (int i = 0; i < this->config.bl_rules; ++i)
		{
			std::string expr = vm["bl-rule" + std::to_string(i) + "-expression"].as<std::string>();

			this->config.blacklist_rule[i - 1].expression = new ConfigFileExpressionEvaluator("FilesBlacklist", i, expr);
		}
	}
}


ConfigFile::ConfigFileReader::~ConfigFileReader()
{
	for (int i = 0; i < 20; i++)
	{
		if (this->config.blacklist_rule[i].expression != nullptr)
			delete this->config.blacklist_rule[i].expression;
	}

	for (int i = 0; i < 20; i++)
	{
		if (this->config.whitelist_rule[i].expression != nullptr)
			delete this->config.whitelist_rule[i].expression;
	}

	for (int i = 0; i < 20; i++)
	{
		if (this->config.tn_rule[i].expression != nullptr)
			delete this->config.tn_rule[i].expression;
	}

	for (int i = 0; i < 20; i++)
	{
		if (this->config.nn_rule[i].expression != nullptr)
			delete this->config.nn_rule[i].expression;
	}
}


ConfigFile::ConfigFileData& ConfigFile::ConfigFileReader::read()
{
	return this->config;
}

std::string ConfigFile::ConfigFileReader::usage()
{
	return this->usage_str;
}

bool ConfigFile::ConfigFileReader::help()
{
	return this->isHelp;
}

bool ConfigFile::ConfigFileReader::configFile()
{
	return this->isConfigFile;
}

int ConfigFile::ConfigFileReader::cfgType()
{
	return this->cfg_type;
}

std::string ConfigFile::ConfigFileReader::configFilePath()
{
	return this->cfg_file;
}

std::wstring ConfigFile::ConfigFileReader::generate()
{
	// code below IS very crude, but effective

	std::wstringstream retval;
	retval << L"if(_action == \"System\") then" << std::endl;

	// [System] section

	// _path
	retval << L"	_path = \"" << s2ws(this->config.path) << L"\"" << std::endl;

	// _alert_log
	retval << L"	_alert_log = \"" << s2ws(this->config.alert_log) << L"\"" << std::endl;

	//_error_log
	retval << L"	_error_log = \"" << s2ws(this->config.error_log) << L"\"" << std::endl;

	// _error_level
	retval << L"	_error_level = " << std::to_wstring(this->config.error_level) << std::endl;

	// _max_threads
	retval << L"	_max_threads = " << std::to_wstring(this->config.max_threads) << std::endl;

	retval << L"	return" << std::endl;
	retval << L"elseif(_action == \"\") then" << std::endl;


	// other section
	// here the iteration through tasks have place
	// code below is a maze, enter only if necessary :)
	// NOTE: maze != bad
	for (int i = 0; i < 5; ++i)
	{
		if (this->config.tasks[i] == TASK_NONE)
			continue;

		if (this->config.tasks[i] == TASK_REQUIRED_TAGS_SCAN)
		{
			for (int k = 0; k < this->config.rt_rules; ++k)
			{
				if (!this->config.rt_rule[k].values.size() /* if empty */ && this->config.rt_rule[k].tags.size() /* if not empty */) // _SetRequiredTags()
				{
					retval << L"	_SetRequiredTags(";
					for (int l = 0; l < this->config.rt_rule[k].tags.size(); ++l)
					{
						retval << L"\"" << s2ws(this->config.rt_rule[k].tags[l]) << L"\"";

						if (l != (this->config.rt_rule[k].tags.size() - 1))
							retval << L", ";
					}
					retval << L");" << std::endl;
				}
				else if (this->config.rt_rule[k].tags.size() >= 1) // _CheckTagValues
				{
					retval << L"	_CheckTagValues(\"" << s2ws(this->config.rt_rule[k].tags[0]) << L"\", ";

					for (int l = 0; l < this->config.rt_rule[k].values.size(); ++l)
					{
						retval << s2ws(this->config.rt_rule[k].values[l]);

						if (l != (this->config.rt_rule[k].values.size() - 1))
							retval << L", ";
					}
					retval << L");" << std::endl;
				}
			}
		}
		else if (this->config.tasks[i] == TASK_TAGS_NORMALIZATION)
		{
			for (int k = 0; k < this->config.tn_rules; ++k)
			{
				if (this->config.tn_rule[k].action == ACTION_DELETE && this->config.tn_rule[k].tags.size() /* if not empty */) // _RemoveTags
				{
					retval << L"	if(" << this->config.tn_rule[k].expression->evaluate() << L") then" << std::endl;
					retval << L"		_RemoveTags(";

					for (int l = 0; l < this->config.tn_rule[k].tags.size(); ++l)
					{
						retval << L"\"" << s2ws(this->config.tn_rule[k].tags[l]) << L"\"";

						if (l != (this->config.tn_rule[k].tags.size() - 1))
							retval << L", ";
					}

					retval << L");" << std::endl;
					retval << L"	end" << std::endl;
				}
				else if (this->config.tn_rule[k].action == ACTION_SET && this->config.tn_rule[k].tags.size() >= 1 && this->config.tn_rule[k].new_value != "") // _SetTags
				{
					retval << L"	if(" << this->config.tn_rule[k].expression->evaluate() << L") then" << std::endl;
					retval << L"		_SetTags(\"" << s2ws(this->config.tn_rule[k].tags[0]) << "\", \"" << s2ws(this->config.tn_rule[k].new_value) << L"\");" << std::endl;
					retval << L"	end" << std::endl;
				}
			}
		}
		else if (this->config.tasks[i] == TASK_NAMES_NORMALIZATION)
		{
			for (int k = 0; k < this->config.nn_rules; ++k)
			{
				if (this->config.nn_rule[k].action == ACTION_RENAME) // _Rename
				{
					retval << L"	if(" << this->config.nn_rule[k].expression->evaluate() << L") then" << std::endl;
					retval << L"		_Rename(\"" << s2ws(this->config.nn_rule[k].new_name) << L"\");" << std::endl;
					retval << L"	end" << std::endl;
				}
				else if (this->config.nn_rule[k].action == ACTION_MOVE) // _Move
				{
					retval << L"	if(" << this->config.nn_rule[k].expression->evaluate() << L") then" << std::endl;
					retval << L"		_Move(\"" << s2ws(this->config.nn_rule[k].new_name) << L"\");" << std::endl;
					retval << L"	end" << std::endl;
				}
			}
		}
		else if (this->config.tasks[i] == TASK_FILES_BLACKLIST)
		{
			for (int k = 0; k < this->config.bl_rules; ++k)
			{
				if (this->config.blacklist_rule[k].files.size() == 1 && this->config.blacklist_rule[k].files[0] == "*")
				{
					// if(expression) _Delete()
					retval << L"	if(" << this->config.blacklist_rule[k].expression->evaluate() << L") then" << std::endl;
					retval << L"		_Delete();" << std::endl;
					retval << L"	end" << std::endl;
				}
				else if (this->config.blacklist_rule[k].files.size()) // if not empty
				{
					// if(("%_filename_ext%" == files[1]) or ("%_filename_ext%" == files[2]) or ... ) _Delete(); -- no expression here
					retval << L"	if(";

					for (int l = 0; l < this->config.blacklist_rule[k].files.size(); ++l)
					{
						retval << L"(\"%_filename_ext%\" == " << s2ws(this->config.blacklist_rule[k].files[l]) << L")";

						if (l != (this->config.blacklist_rule[k].files.size() - 1))
							retval << L" or ";
					}

					retval << L") then" << std::endl;
					retval << L"		_Delete();" << std::endl;
					retval << L"	end" << std::endl;
				}
			}
			
		}
		else if (this->config.tasks[i] == TASK_FILES_WHITELIST)
		{
			for (int k = 0; k < this->config.wl_rules; ++k)
			{
				if (this->config.whitelist_rule[k].files.size() == 1 && this->config.whitelist_rule[k].files[0] == "*")
				{
					// if(not (expression)) _Delete()
					retval << L"	if(not (" << this->config.whitelist_rule[k].expression->evaluate() << L")) then" << std::endl;
					retval << L"		_Delete();" << std::endl;
					retval << L"	end" << std::endl;
				}
				else if (this->config.whitelist_rule[k].files.size()) // if not empty
				{
					// if("%_filename_ext%" ~= files[1] and "%_filename_ext%" ~= files[2] and ... ) _Delete();
					retval << L"	if(";

					for (int l = 0; l < this->config.whitelist_rule[k].files.size(); ++l)
					{
						retval << L"\"%_filename_ext%\" ~= " << s2ws(this->config.whitelist_rule[k].files[l]) << L")";

						if (l != (this->config.whitelist_rule[k].files.size() - 1))
							retval << L" and ";
					}

					retval << L") then" << std::endl;
					retval << L"		_Delete();" << std::endl;
					retval << L"	end" << std::endl;
				}
			}
		}
		retval << std::endl << std::endl;
	}




	return retval.str();
}

boost::program_options::variables_map ConfigFile::ConfigFileReader::parse_command_line(int argc, char *argv[])
{
	namespace po = boost::program_options;

	po::options_description generic("Command-line parameters");
	generic.add_options()
		("help", "produce this help message")
		("config", po::value<std::string>(&this->cfg_file), "path to config file")
		("force-lua-config", "forces program to evaluate config file as LUA file, ignoring file extension")
		("force-ini-config", "forces program to evaluate config file as INI file, ignoring file extension")
		;

	po::options_description system("System options");
	system.add_options()
		("path", po::value<std::string>(&this->config.path), "path to directory")
		("alert-log", po::value<std::string>(&this->config.alert_log)->default_value("-"), "path to alert log file or '-' if messages should be redirected to stdout")
		("error-log", po::value<std::string>(&this->config.error_log)->default_value("-"), "path to error log file or '-' if messages should be redirected to stderr")
		("error-level", po::value<int>(&this->config.error_level)->default_value(2), "minimum message level to be logged; 3 - debug, 2 - warnings, 1 - critical errors, 0 - nothing")
		("max-threads", po::value<int>(&this->config.max_threads)->default_value(0), "maximum number of threads to be used (0 - unlimited)")
		("tasks", po::value< std::vector< std::string > >(), "list of tasks to be performed")
		;

	po::options_description nnrules("Names normalization rules");
	nnrules.add_options()
		("nn-rules", po::value<int>(&this->config.nn_rules)->default_value(0), "number of names normalization rules; max. is 20")
		("nn-rule1-expression", po::value<std::string>()->default_value("true"), "first rule if() expression in MLC special conditional format; tag aliases supported; number can be changed to up to '--nn-rules' value")
		("nn-rule1-new-name", po::value<std::string>(&this->config.nn_rule[0].new_name), "new name for file if first if() statement results in true; tag aliases supported; number can be changed to up to '--nn-rules' value")
		("nn-rule1-action", po::value<std::string>(), "action taken on file if if() expression evaluates to true (either 'move' or 'rename'); number can be changed up to '--nn-rules' value")
		;

	po::options_description nnrules_hidden("Hidden names normalization rules");
	nnrules_hidden.add_options()
		("nn-rule2-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule2-new-name", po::value<std::string>(&this->config.nn_rule[1].new_name), "")
		("nn-rule2-action", po::value<std::string>(), "")

		("nn-rule3-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule3-new-name", po::value<std::string>(&this->config.nn_rule[2].new_name), "")
		("nn-rule3-action", po::value<std::string>(), "")

		("nn-rule4-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule4-new-name", po::value<std::string>(&this->config.nn_rule[3].new_name), "")
		("nn-rule4-action", po::value<std::string>(), "")

		("nn-rule5-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule5-new-name", po::value<std::string>(&this->config.nn_rule[4].new_name), "")
		("nn-rule5-action", po::value<std::string>(), "")

		("nn-rule6-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule6-new-name", po::value<std::string>(&this->config.nn_rule[5].new_name), "")
		("nn-rule6-action", po::value<std::string>(), "")

		("nn-rule7-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule7-new-name", po::value<std::string>(&this->config.nn_rule[6].new_name), "")
		("nn-rule7-action", po::value<std::string>(), "")

		("nn-rule8-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule8-new-name", po::value<std::string>(&this->config.nn_rule[7].new_name), "")
		("nn-rule8-action", po::value<std::string>(), "")

		("nn-rule9-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule9-new-name", po::value<std::string>(&this->config.nn_rule[8].new_name), "")
		("nn-rule9-action", po::value<std::string>(), "")

		("nn-rule10-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule10-new-name", po::value<std::string>(&this->config.nn_rule[9].new_name), "")
		("nn-rule10-action", po::value<std::string>(), "")

		("nn-rule11-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule11-new-name", po::value<std::string>(&this->config.nn_rule[10].new_name), "")
		("nn-rule11-action", po::value<std::string>(), "")

		("nn-rule12-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule12-new-name", po::value<std::string>(&this->config.nn_rule[11].new_name), "")
		("nn-rule12-action", po::value<std::string>(), "")

		("nn-rule13-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule13-new-name", po::value<std::string>(&this->config.nn_rule[12].new_name), "")
		("nn-rule13-action", po::value<std::string>(), "")

		("nn-rule14-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule14-new-name", po::value<std::string>(&this->config.nn_rule[13].new_name), "")
		("nn-rule14-action", po::value<std::string>(), "")

		("nn-rule15-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule15-new-name", po::value<std::string>(&this->config.nn_rule[14].new_name), "")
		("nn-rule15-action", po::value<std::string>(), "")

		("nn-rule16-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule16-new-name", po::value<std::string>(&this->config.nn_rule[15].new_name), "")
		("nn-rule16-action", po::value<std::string>(), "")

		("nn-rule17-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule17-new-name", po::value<std::string>(&this->config.nn_rule[16].new_name), "")
		("nn-rule17-action", po::value<std::string>(), "")

		("nn-rule18-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule18-new-name", po::value<std::string>(&this->config.nn_rule[17].new_name), "")
		("nn-rule18-action", po::value<std::string>(), "")

		("nn-rule19-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule19-new-name", po::value<std::string>(&this->config.nn_rule[18].new_name), "")
		("nn-rule19-action", po::value<std::string>(), "")

		("nn-rule20-expression", po::value<std::string>()->default_value("true"), "")
		("nn-rule20-new-name", po::value<std::string>(&this->config.nn_rule[19].new_name), "")
		("nn-rule20-action", po::value<std::string>(), "")
		;

	po::options_description rtrules("Required tags rules"), rtrules_hidden("Required tags rules (hidden)");
	rtrules.add_options()
		("rt-rules", po::value<int>(&this->config.rt_rules)->default_value(0), "number of required tags rules; max. is 20")
		("rt-rule1-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[0].tags), "list of tags required to be set; tags cannot be used with % signs around the tag name; number can be changed up to '--rt-rules' value")
		("rt-rule1-values", po::value<std::vector<std::string>>(&this->config.rt_rule[0].values), "value that needs to be set for tags defined for this rule; tag aliasses supported; number can be changed up to '--rt-rules' value")
		;

	rtrules_hidden.add_options()
		("rt-rule2-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[1].tags), "")
		("rt-rule2-values", po::value<std::vector<std::string>>(&this->config.rt_rule[1].values), "")

		("rt-rule3-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[2].tags), "")
		("rt-rule3-values", po::value<std::vector<std::string>>(&this->config.rt_rule[2].values), "")

		("rt-rule4-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[3].tags), "")
		("rt-rule4-values", po::value<std::vector<std::string>>(&this->config.rt_rule[3].values), "")

		("rt-rule5-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[4].tags), "")
		("rt-rule5-values", po::value<std::vector<std::string>>(&this->config.rt_rule[4].values), "")

		("rt-rule6-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[5].tags), "")
		("rt-rule6-values", po::value<std::vector<std::string>>(&this->config.rt_rule[5].values), "")

		("rt-rule7-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[6].tags), "")
		("rt-rule7-values", po::value<std::vector<std::string>>(&this->config.rt_rule[6].values), "")

		("rt-rule8-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[7].tags), "")
		("rt-rule8-values", po::value<std::vector<std::string>>(&this->config.rt_rule[7].values), "")

		("rt-rule9-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[8].tags), "")
		("rt-rule9-values", po::value<std::vector<std::string>>(&this->config.rt_rule[8].values), "")

		("rt-rule10-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[9].tags), "")
		("rt-rule10-values", po::value<std::vector<std::string>>(&this->config.rt_rule[9].values), "")

		("rt-rule11-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[10].tags), "")
		("rt-rule11-values", po::value<std::vector<std::string>>(&this->config.rt_rule[10].values), "")

		("rt-rule12-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[11].tags), "")
		("rt-rule12-values", po::value<std::vector<std::string>>(&this->config.rt_rule[11].values), "")

		("rt-rule13-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[12].tags), "")
		("rt-rule13-values", po::value<std::vector<std::string>>(&this->config.rt_rule[12].values), "")

		("rt-rule14-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[13].tags), "")
		("rt-rule14-values", po::value<std::vector<std::string>>(&this->config.rt_rule[13].values), "")

		("rt-rule15-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[14].tags), "")
		("rt-rule15-values", po::value<std::vector<std::string>>(&this->config.rt_rule[14].values), "")

		("rt-rule16-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[15].tags), "")
		("rt-rule16-values", po::value<std::vector<std::string>>(&this->config.rt_rule[15].values), "")

		("rt-rule17-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[16].tags), "")
		("rt-rule17-values", po::value<std::vector<std::string>>(&this->config.rt_rule[16].values), "")

		("rt-rule18-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[17].tags), "")
		("rt-rule18-values", po::value<std::vector<std::string>>(&this->config.rt_rule[17].values), "")

		("rt-rule19-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[18].tags), "")
		("rt-rule19-values", po::value<std::vector<std::string>>(&this->config.rt_rule[18].values), "")

		("rt-rule20-tags", po::value< std::vector< std::string > >(&this->config.rt_rule[19].tags), "")
		("rt-rule20-values", po::value<std::vector<std::string>>(&this->config.rt_rule[19].values), "")
		;

	po::options_description tnrules("Tags normalization rules"), tnrules_hidden("Tags normalization rules (hidden)");
	tnrules.add_options()
		("tn-rules", po::value<int>(&this->config.tn_rules)->default_value(0), "number of tags normalization rules; max. is 20")
		("tn-rule1-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[0].tags), "list of tags for first rule expression and action; tags cannot be used with % signs around the tag name; number can be changed up to '--tn-rules' value")
		("tn-rule1-action", po::value<std::string>()->default_value("nothing"), "action taken on tags defined for rule if if() expression evaluates to true (either 'set' or 'delete'); number can be changed up to '--tn-rules' value")
		("tn-rule1-new-value", po::value<std::string>(&this->config.tn_rule[0].new_value), "new value for tags defined for this rule if if() expression evaluate to true and action is 'set' or 'replace'; tag aliasses supported; number can be changed up to '--tn-rules' value")
		("tn-rule1-expression", po::value<std::string>()->default_value("true"), "first rule if() expression; tag aliasses supported; number can be changed up to '--tn-rules' value")
		;

	tnrules_hidden.add_options()
		("tn-rule2-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[1].tags), "")
		("tn-rule2-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule2-new-value", po::value<std::string>(&this->config.tn_rule[1].new_value), "")
		("tn-rule2-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule3-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[2].tags), "")
		("tn-rule3-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule3-new-value", po::value<std::string>(&this->config.tn_rule[2].new_value), "")
		("tn-rule3-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule4-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[3].tags), "")
		("tn-rule4-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule4-new-value", po::value<std::string>(&this->config.tn_rule[3].new_value), "")
		("tn-rule4-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule5-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[4].tags), "")
		("tn-rule5-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule5-new-value", po::value<std::string>(&this->config.tn_rule[4].new_value), "")
		("tn-rule5-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule6-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[5].tags), "")
		("tn-rule6-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule6-new-value", po::value<std::string>(&this->config.tn_rule[5].new_value), "")
		("tn-rule6-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule7-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[6].tags), "")
		("tn-rule7-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule7-new-value", po::value<std::string>(&this->config.tn_rule[6].new_value), "")
		("tn-rule7-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule8-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[7].tags), "")
		("tn-rule8-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule8-new-value", po::value<std::string>(&this->config.tn_rule[7].new_value), "")
		("tn-rule8-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule9-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[8].tags), "")
		("tn-rule9-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule9-new-value", po::value<std::string>(&this->config.tn_rule[8].new_value), "")
		("tn-rule9-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule10-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[9].tags), "")
		("tn-rule10-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule10-new-value", po::value<std::string>(&this->config.tn_rule[9].new_value), "")
		("tn-rule10-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule11-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[10].tags), "")
		("tn-rule11-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule11-new-value", po::value<std::string>(&this->config.tn_rule[10].new_value), "")
		("tn-rule11-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule12-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[11].tags), "")
		("tn-rule12-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule12-new-value", po::value<std::string>(&this->config.tn_rule[11].new_value), "")
		("tn-rule12-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule13-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[12].tags), "")
		("tn-rule13-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule13-new-value", po::value<std::string>(&this->config.tn_rule[12].new_value), "")
		("tn-rule13-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule14-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[13].tags), "")
		("tn-rule14-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule14-new-value", po::value<std::string>(&this->config.tn_rule[13].new_value), "")
		("tn-rule14-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule15-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[14].tags), "")
		("tn-rule15-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule15-new-value", po::value<std::string>(&this->config.tn_rule[14].new_value), "")
		("tn-rule15-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule16-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[15].tags), "")
		("tn-rule16-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule16-new-value", po::value<std::string>(&this->config.tn_rule[15].new_value), "")
		("tn-rule16-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule17-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[16].tags), "")
		("tn-rule17-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule17-new-value", po::value<std::string>(&this->config.tn_rule[16].new_value), "")
		("tn-rule17-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule18-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[17].tags), "")
		("tn-rule18-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule18-new-value", po::value<std::string>(&this->config.tn_rule[17].new_value), "")
		("tn-rule18-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule19-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[18].tags), "")
		("tn-rule19-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule19-new-value", po::value<std::string>(&this->config.tn_rule[18].new_value), "")
		("tn-rule19-expression", po::value<std::string>()->default_value("true"), "")

		("tn-rule20-tags", po::value< std::vector< std::string > >(&this->config.tn_rule[19].tags), "")
		("tn-rule20-action", po::value<std::string>()->default_value("nothing"), "")
		("tn-rule20-new-value", po::value<std::string>(&this->config.tn_rule[19].new_value), "")
		("tn-rule20-expression", po::value<std::string>()->default_value("true"), "")
		;

	po::options_description whitelist("Files whitelist"), whitelist_hidden("Files whitelist (hidden)");
	whitelist.add_options()
		("wh-rules", po::value<int>(&this->config.wl_rules)->default_value(0), "number of whitelist rules; max. is 20")
		("wh-rule1-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[0].files), "list of files to be whitelisted if if() expression evaluates to true; tag aliasses supported; number can be changed up to '--wh-rules' value")
		("wh-rule1-expression", po::value<std::string>()->default_value("true"), "first whitelistr rule if() expression; tag aliasses supported; number can be changed up to '--wh-rules' value")
		;

	whitelist_hidden.add_options()
		("wh-rule2-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[1].files), "")
		("wh-rule2-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule3-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[2].files), "")
		("wh-rule3-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule4-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[3].files), "")
		("wh-rule4-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule5-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[4].files), "")
		("wh-rule5-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule6-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[5].files), "")
		("wh-rule6-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule7-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[6].files), "")
		("wh-rule7-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule8-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[7].files), "")
		("wh-rule8-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule9-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[8].files), "")
		("wh-rule9-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule10-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[9].files), "")
		("wh-rule10-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule11-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[10].files), "")
		("wh-rule11-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule12-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[11].files), "")
		("wh-rule12-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule13-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[12].files), "")
		("wh-rule13-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule14-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[13].files), "")
		("wh-rule14-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule15-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[14].files), "")
		("wh-rule15-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule16-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[15].files), "")
		("wh-rule16-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule17-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[16].files), "")
		("wh-rule17-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule18-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[17].files), "")
		("wh-rule18-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule19-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[18].files), "")
		("wh-rule19-expression", po::value<std::string>()->default_value("true"), "")

		("wh-rule20-files", po::value< std::vector< std::string > >(&this->config.whitelist_rule[19].files), "")
		("wh-rule20-expression", po::value<std::string>()->default_value("true"), "")
		;

	po::options_description blacklist("Files blacklist"), blacklist_hidden("Files blacklist (hidden)");
	blacklist.add_options()
		("bl-rules", po::value<int>(&this->config.bl_rules), "number of blacklist rules; max. is 20")
		("bl-rule1-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[0].files), "list of files to be blacklisted if if() expression evaluates to true; tag aliasses supported; number can be changed up to '--bl-rules' value")
		("bl-rule1-expression", po::value<std::string>()->default_value("true"), "first blacklist rule if() expression; tag aliasses supported; number can be changed up to '--bl-rules' value")
		;

	blacklist_hidden.add_options()
		("bl-rule2-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[1].files), "")
		("bl-rule2-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule3-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[2].files), "")
		("bl-rule3-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule4-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[3].files), "")
		("bl-rule4-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule5-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[4].files), "")
		("bl-rule5-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule6-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[5].files), "")
		("bl-rule6-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule7-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[6].files), "")
		("bl-rule7-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule8-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[7].files), "")
		("bl-rule8-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule9-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[8].files), "")
		("bl-rule9-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule10-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[9].files), "")
		("bl-rule10-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule11-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[10].files), "")
		("bl-rule11-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule12-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[11].files), "")
		("bl-rule12-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule13-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[12].files), "")
		("bl-rule13-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule14-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[13].files), "")
		("bl-rule14-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule15-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[14].files), "")
		("bl-rule15-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule16-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[15].files), "")
		("bl-rule16-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule17-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[16].files), "")
		("bl-rule17-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule18-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[17].files), "")
		("bl-rule18-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule19-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[18].files), "")
		("bl-rule19-expression", po::value<std::string>()->default_value("true"), "")

		("bl-rule20-files", po::value< std::vector< std::string > >(&this->config.blacklist_rule[19].files), "")
		("bl-rule20-expression", po::value<std::string>()->default_value("true"), "")
		;

	po::options_description cmdline_opts_desc, cmdline_opts;
	cmdline_opts_desc.add(generic).add(system).add(nnrules).add(rtrules).add(tnrules).add(whitelist).add(blacklist);
	cmdline_opts.add(generic).add(system).add(nnrules).add(nnrules_hidden).add(rtrules).add(rtrules_hidden).add(tnrules).add(tnrules_hidden).add(whitelist).add(whitelist_hidden).add(blacklist).add(blacklist_hidden);

	// getting usage() message
	std::stringstream ss;
	ss << cmdline_opts_desc;
	this->usage_str = ss.str();

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, cmdline_opts), vm);
	po::notify(vm);

	return vm;
}

boost::program_options::variables_map ConfigFile::ConfigFileReader::parse_config_file(std::string path)
{
	namespace po = boost::program_options;

	po::options_description system("System options");
	system.add_options()
		("System.path", po::value<std::string>(&this->configini.path), "path to directory")
		("System.alert_log", po::value<std::string>(&this->configini.alert_log)->default_value("-"), "path to alert log file or '-' if messages should be redirected to stdout")
		("System.error_log", po::value<std::string>(&this->configini.error_log)->default_value("-"), "path to error log file or '-' if messages should be redirected to stderr")
		("System.error_level", po::value<int>(&this->configini.error_level)->default_value(2), "minimum message level to be logged; 3 - debug, 2 - warnings, 1 - critical errors, 0 - nothing")
		("System.max_threads", po::value<int>(&this->configini.max_threads)->default_value(0), "maximum number of threads to be used (0 - unlimited)")
		("system.tasks", po::value< std::vector< std::string > >(), "list of tasks to be performed")
		;

	po::options_description nnrules("Names normalization rules");
	nnrules.add_options()
		("System.nn_rules", po::value<int>(&this->configini.nn_rules)->default_value(0), "number of names normalization rules; max. is 20")
		("NamesNormalization.rule1.expression", po::value<std::string>()->default_value("true"), "first rule if() expression in MLC special conditional format; tag aliases supported; number can be changed to up to '--NamesNormalization.rules' value")
		("NamesNormalization.rule1.new_name", po::value<std::string>(&this->configini.nn_rule[0].new_name), "new name for file if first if() statement results in true; tag aliases supported; number can be changed to up to '--NamesNormalization.rules' value")
		("NamesNormalization.rule1.action", po::value<std::string>(), "action taken on file if if() expression evaluates to true (either 'move' or 'rename'); number can be changed up to '--NamesNormalization.rules' value")
		;

	po::options_description nnrules_hidden("Hidden names normalization rules");
	nnrules_hidden.add_options()
		("NamesNormalization.rule2.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule2.new_name", po::value<std::string>(&this->configini.nn_rule[1].new_name), "")
		("NamesNormalization.rule2.action", po::value<std::string>(), "")

		("NamesNormalization.rule3.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule3.new_name", po::value<std::string>(&this->configini.nn_rule[2].new_name), "")
		("NamesNormalization.rule3.action", po::value<std::string>(), "")

		("NamesNormalization.rule4.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule4.new_name", po::value<std::string>(&this->configini.nn_rule[3].new_name), "")
		("NamesNormalization.rule4.action", po::value<std::string>(), "")

		("NamesNormalization.rule5.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule5.new_name", po::value<std::string>(&this->configini.nn_rule[4].new_name), "")
		("NamesNormalization.rule5.action", po::value<std::string>(), "")

		("NamesNormalization.rule6.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule6.new_name", po::value<std::string>(&this->configini.nn_rule[5].new_name), "")
		("NamesNormalization.rule6.action", po::value<std::string>(), "")

		("NamesNormalization.rule7.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule7.new_name", po::value<std::string>(&this->configini.nn_rule[6].new_name), "")
		("NamesNormalization.rule7.action", po::value<std::string>(), "")

		("NamesNormalization.rule8.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule8.new_name", po::value<std::string>(&this->configini.nn_rule[7].new_name), "")
		("NamesNormalization.rule8.action", po::value<std::string>(), "")

		("NamesNormalization.rule9.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule9.new_name", po::value<std::string>(&this->configini.nn_rule[8].new_name), "")
		("NamesNormalization.rule9.action", po::value<std::string>(), "")

		("NamesNormalization.rule10.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule10.new_name", po::value<std::string>(&this->configini.nn_rule[9].new_name), "")
		("NamesNormalization.rule10.action", po::value<std::string>(), "")

		("NamesNormalization.rule11.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule11.new_name", po::value<std::string>(&this->configini.nn_rule[10].new_name), "")
		("NamesNormalization.rule11.action", po::value<std::string>(), "")

		("NamesNormalization.rule12.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule12.new_name", po::value<std::string>(&this->configini.nn_rule[11].new_name), "")
		("NamesNormalization.rule12.action", po::value<std::string>(), "")

		("NamesNormalization.rule13.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule13.new_name", po::value<std::string>(&this->configini.nn_rule[12].new_name), "")
		("NamesNormalization.rule13.action", po::value<std::string>(), "")

		("NamesNormalization.rule14.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule14.new_name", po::value<std::string>(&this->configini.nn_rule[13].new_name), "")
		("NamesNormalization.rule14.action", po::value<std::string>(), "")

		("NamesNormalization.rule15.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule15.new_name", po::value<std::string>(&this->configini.nn_rule[14].new_name), "")
		("NamesNormalization.rule15.action", po::value<std::string>(), "")

		("NamesNormalization.rule16.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule16.new_name", po::value<std::string>(&this->configini.nn_rule[15].new_name), "")
		("NamesNormalization.rule16.action", po::value<std::string>(), "")

		("NamesNormalization.rule17.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule17.new_name", po::value<std::string>(&this->configini.nn_rule[16].new_name), "")
		("NamesNormalization.rule17.action", po::value<std::string>(), "")

		("NamesNormalization.rule18.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule18.new_name", po::value<std::string>(&this->configini.nn_rule[17].new_name), "")
		("NamesNormalization.rule18.action", po::value<std::string>(), "")

		("NamesNormalization.rule19.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule19.new_name", po::value<std::string>(&this->configini.nn_rule[18].new_name), "")
		("NamesNormalization.rule19.action", po::value<std::string>(), "")

		("NamesNormalization.rule20.expression", po::value<std::string>()->default_value("true"), "")
		("NamesNormalization.rule20.new_name", po::value<std::string>(&this->configini.nn_rule[19].new_name), "")
		("NamesNormalization.rule20.action", po::value<std::string>(), "")
		;

	po::options_description rtrules("Required tags rules"), rtrules_hidden("Required tags rules (hidden)");
	rtrules.add_options()
		("System.rt_rules", po::value<int>(&this->configini.rt_rules)->default_value(0), "number of required tags rules; max. is 20")
		("RequiredTagsScan.rule1.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[0].tags), "list of tags required to be set; tags cannot be used with % signs around the tag name; number can be changed up to '--RequiredTagsScan.rules' value")
		("RequiredTagsScan.rule1.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[0].values), "value that needs to be set for tags defined for this rule; tag aliasses supported; number can be changed up to '--RequiredTagsScan.rules' value")
		;

	rtrules_hidden.add_options()
		("RequiredTagsScan.rule2.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[1].tags), "")
		("RequiredTagsScan.rule2.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[1].values), "")

		("RequiredTagsScan.rule3.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[2].tags), "")
		("RequiredTagsScan.rule3.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[2].values), "")

		("RequiredTagsScan.rule4.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[3].tags), "")
		("RequiredTagsScan.rule4.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[3].values), "")

		("RequiredTagsScan.rule5.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[4].tags), "")
		("RequiredTagsScan.rule5.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[4].values), "")

		("RequiredTagsScan.rule6.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[5].tags), "")
		("RequiredTagsScan.rule6.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[5].values), "")

		("RequiredTagsScan.rule7.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[6].tags), "")
		("RequiredTagsScan.rule7.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[6].values), "")

		("RequiredTagsScan.rule8.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[7].tags), "")
		("RequiredTagsScan.rule8.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[7].values), "")

		("RequiredTagsScan.rule9.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[8].tags), "")
		("RequiredTagsScan.rule9.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[8].values), "")

		("RequiredTagsScan.rule10.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[9].tags), "")
		("RequiredTagsScan.rule10.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[9].values), "")

		("RequiredTagsScan.rule11.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[10].tags), "")
		("RequiredTagsScan.rule11.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[10].values), "")

		("RequiredTagsScan.rule12.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[11].tags), "")
		("RequiredTagsScan.rule12.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[11].values), "")

		("RequiredTagsScan.rule13.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[12].tags), "")
		("RequiredTagsScan.rule13.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[12].values), "")

		("RequiredTagsScan.rule14.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[13].tags), "")
		("RequiredTagsScan.rule14.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[13].values), "")

		("RequiredTagsScan.rule15.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[14].tags), "")
		("RequiredTagsScan.rule15.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[14].values), "")

		("RequiredTagsScan.rule16.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[15].tags), "")
		("RequiredTagsScan.rule16.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[15].values), "")

		("RequiredTagsScan.rule17.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[16].tags), "")
		("RequiredTagsScan.rule17.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[16].values), "")

		("RequiredTagsScan.rule18.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[17].tags), "")
		("RequiredTagsScan.rule18.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[17].values), "")

		("RequiredTagsScan.rule19.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[18].tags), "")
		("RequiredTagsScan.rule19.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[18].values), "")

		("RequiredTagsScan.rule20.tags", po::value< std::vector< std::string > >(&this->configini.rt_rule[19].tags), "")
		("RequiredTagsScan.rule20.values", po::value<std::vector<std::string>>(&this->configini.rt_rule[19].values), "")
		;

	po::options_description tnrules("Tags normalization rules"), tnrules_hidden("Tags normalization rules (hidden)");
	tnrules.add_options()
		("System.tn_rules", po::value<int>(&this->configini.tn_rules)->default_value(0), "number of tags normalization rules; max. is 20")
		("TagsNormalization.rule1.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[0].tags), "list of tags for first rule expression and action; tags cannot be used with % signs around the tag name; number can be changed up to '-.tagsNormalization.rules' value")
		("TagsNormalization.rule1.action", po::value<std::string>()->default_value("nothing"), "action taken on tags defined for rule if if() expression evaluates to true (either 'set' or 'delete'); number can be changed up to '-.tagsNormalization.rules' value")
		("TagsNormalization.rule1.new_value", po::value<std::string>(&this->configini.tn_rule[0].new_value), "new value for tags defined for this rule if if() expression evaluate to true and action is 'set' or 'replace'; tag aliasses supported; number can be changed up to '-.tagsNormalization.rules' value")
		("TagsNormalization.rule1.expression", po::value<std::string>()->default_value("true"), "first rule if() expression; tag aliasses supported; number can be changed up to '-.tagsNormalization.rules' value")
		;

	tnrules_hidden.add_options()
		("TagsNormalization.rule2.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[1].tags), "")
		("TagsNormalization.rule2.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule2.new_value", po::value<std::string>(&this->configini.tn_rule[1].new_value), "")
		("TagsNormalization.rule2.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule3.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[2].tags), "")
		("TagsNormalization.rule3.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule3.new_value", po::value<std::string>(&this->configini.tn_rule[2].new_value), "")
		("TagsNormalization.rule3.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule4.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[3].tags), "")
		("TagsNormalization.rule4.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule4.new_value", po::value<std::string>(&this->configini.tn_rule[3].new_value), "")
		("TagsNormalization.rule4.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule5.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[4].tags), "")
		("TagsNormalization.rule5.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule5.new_value", po::value<std::string>(&this->configini.tn_rule[4].new_value), "")
		("TagsNormalization.rule5.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule6.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[5].tags), "")
		("TagsNormalization.rule6.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule6.new_value", po::value<std::string>(&this->configini.tn_rule[5].new_value), "")
		("TagsNormalization.rule6.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule7.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[6].tags), "")
		("TagsNormalization.rule7.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule7.new_value", po::value<std::string>(&this->configini.tn_rule[6].new_value), "")
		("TagsNormalization.rule7.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule8.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[7].tags), "")
		("TagsNormalization.rule8.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule8.new_value", po::value<std::string>(&this->configini.tn_rule[7].new_value), "")
		("TagsNormalization.rule8.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule9.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[8].tags), "")
		("TagsNormalization.rule9.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule9.new_value", po::value<std::string>(&this->configini.tn_rule[8].new_value), "")
		("TagsNormalization.rule9.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule10.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[9].tags), "")
		("TagsNormalization.rule10.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule10.new_value", po::value<std::string>(&this->configini.tn_rule[9].new_value), "")
		("TagsNormalization.rule10.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule11.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[10].tags), "")
		("TagsNormalization.rule11.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule11.new_value", po::value<std::string>(&this->configini.tn_rule[10].new_value), "")
		("TagsNormalization.rule11.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule12.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[11].tags), "")
		("TagsNormalization.rule12.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule12.new_value", po::value<std::string>(&this->configini.tn_rule[11].new_value), "")
		("TagsNormalization.rule12.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule13.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[12].tags), "")
		("TagsNormalization.rule13.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule13.new_value", po::value<std::string>(&this->configini.tn_rule[12].new_value), "")
		("TagsNormalization.rule13.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule14.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[13].tags), "")
		("TagsNormalization.rule14.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule14.new_value", po::value<std::string>(&this->configini.tn_rule[13].new_value), "")
		("TagsNormalization.rule14.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule15.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[14].tags), "")
		("TagsNormalization.rule15.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule15.new_value", po::value<std::string>(&this->configini.tn_rule[14].new_value), "")
		("TagsNormalization.rule15.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule16.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[15].tags), "")
		("TagsNormalization.rule16.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule16.new_value", po::value<std::string>(&this->configini.tn_rule[15].new_value), "")
		("TagsNormalization.rule16.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule17.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[16].tags), "")
		("TagsNormalization.rule17.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule17.new_value", po::value<std::string>(&this->configini.tn_rule[16].new_value), "")
		("TagsNormalization.rule17.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule18.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[17].tags), "")
		("TagsNormalization.rule18.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule18.new_value", po::value<std::string>(&this->configini.tn_rule[17].new_value), "")
		("TagsNormalization.rule18.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule19.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[18].tags), "")
		("TagsNormalization.rule19.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule19.new_value", po::value<std::string>(&this->configini.tn_rule[18].new_value), "")
		("TagsNormalization.rule19.expression", po::value<std::string>()->default_value("true"), "")

		("TagsNormalization.rule20.tags", po::value< std::vector< std::string > >(&this->configini.tn_rule[19].tags), "")
		("TagsNormalization.rule20.action", po::value<std::string>()->default_value("nothing"), "")
		("TagsNormalization.rule20.new_value", po::value<std::string>(&this->configini.tn_rule[19].new_value), "")
		("TagsNormalization.rule20.expression", po::value<std::string>()->default_value("true"), "")
		;

	po::options_description whitelist("Files whitelist"), whitelist_hidden("Files whitelist (hidden)");
	whitelist.add_options()
		("System.wh_rules", po::value<int>(&this->configini.wl_rules)->default_value(0), "number of whitelist rules; max. is 20")
		("FilesWhitelist.rule1.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[0].files), "list of files to be whitelisted if if() expression evaluates to true; tag aliasses supported; number can be changed up to '-.filesWhitelist.rules' value")
		("FilesWhitelist.rule1.expression", po::value<std::string>()->default_value("true"), "first whitelistr rule if() expression; tag aliasses supported; number can be changed up to '-.filesWhitelist.rules' value")
		;

	whitelist_hidden.add_options()
		("FilesWhitelist.rule2.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[1].files), "")
		("FilesWhitelist.rule2.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule3.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[2].files), "")
		("FilesWhitelist.rule3.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule4.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[3].files), "")
		("FilesWhitelist.rule4.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule5.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[4].files), "")
		("FilesWhitelist.rule5.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule6.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[5].files), "")
		("FilesWhitelist.rule6.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule7.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[6].files), "")
		("FilesWhitelist.rule7.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule8.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[7].files), "")
		("FilesWhitelist.rule8.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule9.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[8].files), "")
		("FilesWhitelist.rule9.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule10.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[9].files), "")
		("FilesWhitelist.rule10.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule11.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[10].files), "")
		("FilesWhitelist.rule11.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule12.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[11].files), "")
		("FilesWhitelist.rule12.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule13.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[12].files), "")
		("FilesWhitelist.rule13.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule14.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[13].files), "")
		("FilesWhitelist.rule14.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule15.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[14].files), "")
		("FilesWhitelist.rule15.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule16.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[15].files), "")
		("FilesWhitelist.rule16.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule17.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[16].files), "")
		("FilesWhitelist.rule17.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule18.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[17].files), "")
		("FilesWhitelist.rule18.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule19.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[18].files), "")
		("FilesWhitelist.rule19.expression", po::value<std::string>()->default_value("true"), "")

		("FilesWhitelist.rule20.files", po::value< std::vector< std::string > >(&this->configini.whitelist_rule[19].files), "")
		("FilesWhitelist.rule20.expression", po::value<std::string>()->default_value("true"), "")
		;

	po::options_description blacklist("Files blacklist"), blacklist_hidden("Files blacklist (hidden)");
	blacklist.add_options()
		("System.bl_rules", po::value<int>(&this->configini.bl_rules), "number of blacklist rules; max. is 20")
		("FilesBlacklist.rule1.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[0].files), "list of files to be blacklisted if if() expression evaluates to true; tag aliasses supported; number can be changed up to '-.filesBlacklist.rules' value")
		("FilesBlacklist.rule1.expression", po::value<std::string>()->default_value("true"), "first blacklist rule if() expression; tag aliasses supported; number can be changed up to '-.filesBlacklist.rules' value")
		;

	blacklist_hidden.add_options()
		("FilesBlacklist.rule2.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[1].files), "")
		("FilesBlacklist.rule2.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule3.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[2].files), "")
		("FilesBlacklist.rule3.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule4.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[3].files), "")
		("FilesBlacklist.rule4.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule5.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[4].files), "")
		("FilesBlacklist.rule5.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule6.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[5].files), "")
		("FilesBlacklist.rule6.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule7.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[6].files), "")
		("FilesBlacklist.rule7.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule8.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[7].files), "")
		("FilesBlacklist.rule8.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule9.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[8].files), "")
		("FilesBlacklist.rule9.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule10.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[9].files), "")
		("FilesBlacklist.rule10.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule11.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[10].files), "")
		("FilesBlacklist.rule11.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule12.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[11].files), "")
		("FilesBlacklist.rule12.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule13.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[12].files), "")
		("FilesBlacklist.rule13.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule14.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[13].files), "")
		("FilesBlacklist.rule14.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule15.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[14].files), "")
		("FilesBlacklist.rule15.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule16.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[15].files), "")
		("FilesBlacklist.rule16.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule17.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[16].files), "")
		("FilesBlacklist.rule17.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule18.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[17].files), "")
		("FilesBlacklist.rule18.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule19.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[18].files), "")
		("FilesBlacklist.rule19.expression", po::value<std::string>()->default_value("true"), "")

		("FilesBlacklist.rule20.files", po::value< std::vector< std::string > >(&this->configini.blacklist_rule[19].files), "")
		("FilesBlacklist.rule20.expression", po::value<std::string>()->default_value("true"), "")
		;

	po::options_description inifile_opts;
	inifile_opts.add(system).add(nnrules).add(nnrules_hidden).add(rtrules).add(rtrules_hidden).add(tnrules).add(tnrules_hidden).add(whitelist).add(whitelist_hidden).add(blacklist).add(blacklist_hidden);

	po::variables_map vm;
	po::store(po::parse_config_file<char>(path.c_str(), inifile_opts, false), vm);
	po::notify(vm);

	return vm;
}