#pragma once

#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

#include "MediaLibCleaner.hpp"

namespace MediaLibCleaner
{
	namespace ConfigFile
	{
		enum ActionType
		{
			ACTION_DELETE, ACTION_SET, ACTION_LEAVE_ONLY, ACTION_UNKNOWN,
			ACTION_RENAME, ACTION_MOVE
		};

		enum Task
		{
			TASK_FILES_BLACKLIST, TASK_FILES_WHITELIST, TASK_TAGS_NORMALIZATION, TASK_NAMES_NORMALIZATION, TASK_REQUIRED_TAGS_SCAN, TASK_NONE
		};

		enum Operation
		{
			OPERATION_EQUAL, OPERATION_NOT_EQUAL, OPERATION_GREATER, OPERATION_GREATER_EQUAL, OPERATION_LOWER, OPERATION_LOWER_EQUAL, OPERATION_UNKNOWN
		};

		enum ConfigFileType
		{
			CFG_FILE_LUA, CFG_FILE_INI, CFG_FILE_NONE, CFG_FILE_UNKNOWN
		};



		class ConfigFileExpressionEvaluator
		{
		protected:
			std::string task;
			int task_nr = -1;

			std::string expression;
			std::string oper1;
			std::vector<std::string> oper2;
			Operation operation = OPERATION_UNKNOWN;

		public:
			ConfigFileExpressionEvaluator(std::string, int, std::string);
			~ConfigFileExpressionEvaluator();

			std::wstring evaluate();
		};

		struct TagsNormalizationRule
		{
			std::vector<std::string> tags;
			ActionType action = ACTION_UNKNOWN;
			ConfigFileExpressionEvaluator *expression;
			std::string new_value;
		};

		struct NamesNormalizationRule
		{
			std::string new_name;
			ConfigFileExpressionEvaluator *expression;
			ActionType action = ACTION_UNKNOWN;
		};

		struct RequiredTagsRule
		{
			std::vector<std::string> tags;
			std::vector<std::string> values;
		};

		struct FilesWBRule
		{
			std::vector<std::string> files;
			ConfigFileExpressionEvaluator *expression;
			bool isWhitelist;
		};

		struct ConfigFileData
		{
			bool forceLUA = false;
			bool forceINI = false;

			std::string path;
			std::string alert_log;
			std::string error_log;
			int error_level = 0;
			int max_threads = 0;
			Task tasks[5];

			int bl_rules = 0,
				tn_rules = 0,
				nn_rules = 0,
				rt_rules = 0,
				wl_rules = 0;

			TagsNormalizationRule tn_rule[20];
			NamesNormalizationRule nn_rule[20];
			RequiredTagsRule rt_rule[20];
			FilesWBRule whitelist_rule[20];
			FilesWBRule blacklist_rule[20];
		};

		class ConfigFileReader
		{
		protected:
			std::string usage_str;

			std::string cfg_file;
			ConfigFileData config, configini;

			bool isHelp = false;
			bool isConfigFile = false;
			ConfigFileType cfg_type = CFG_FILE_NONE;

			boost::program_options::variables_map parse_command_line(int argc, char *argv[]);
			boost::program_options::variables_map parse_config_file(std::string path);

		public:
			ConfigFileReader(int argc, char *argv[]);
			~ConfigFileReader();

			ConfigFileData& read();
			std::string usage();
			bool help();
			bool configFile();
			int cfgType();
			std::string configFilePath();

			std::wstring generate();
		};
	}
}
