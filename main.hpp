/**
@file
@author Szymon Oracki <szymon.oracki@oustish.pl>
@version 0.4

This file is a start point header file for entire application.
*/

/**
@mainpage Documentation introduction

@section intro_sec Introduction
MediaLibCleaner is meant to be powerful tool helping everyone in continuous struggle to maintain large audio libraries by checking (and editing) tags, file names and folders structure to prevent chaos in the library

Especially it will allow user to define multiple sets of rules for each (in)depended library folder, which includes (but is not limited to): checking, setting and deleting tags; moving, renaming and deleting any file (including non-audio files); checking files and folder integrity (files damage, unsupported content, tags content cohesion in directory etc.).

Project is carried out by students of Gdansk University of Technology, Faculty of Electronics, Telecommunications and Informatics as part of the classes lead by Mulimedia Systems Department.

Project documentation and other releated files are being kept at the server managed by the Department.

@section used_sec Used libaries / credits
This project is using these libraries:
<ul>
	<li><b>Boost</b> - licensed under the Boost Software License, Version 1.0,</li>
	<li><b>Lua</b> - licensed under MIT license - property of PUC-Rio,</li>
	<li><b>taglib</b> - licensed under LGPL and MPL.</li>
</ul>
*/
/*#ifdef _MLC_DEBUG
#include <vld/vld.h>
#endif*/

#include <iostream>
#include <memory> // for unique_ptr
#include <fstream> // for file opening
#include <streambuf> // for file opening
#include <ctime> // for time()
#include <map> // for std::multimap
#include <exception> // exceptions
#include <locale> // for setlocale()
#include <codecvt>

#include <lua/lua.hpp>

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/program_options.hpp>

#include "helpers.hpp"
#include "LuaFunctions.hpp"
#include "MediaLibCleaner.hpp"

#include <Windows.h>

#include <omp.h>



void lua_error_reporting(lua_State*, int);
void process(std::wstring, std::unique_ptr<MediaLibCleaner::FilesAggregator>*, std::unique_ptr<MediaLibCleaner::LogProgram>*);
void scan(std::list<MediaLibCleaner::DFC*>* dfcl, MediaLibCleaner::PathsAggregator* pathl, std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la, std::string pth, std::unique_ptr<MediaLibCleaner::FilesAggregator>* fA, int* tf);