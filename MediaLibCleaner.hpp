/**
 * @file
 * @author Szymon Oracki <szymon.oracki@oustish.pl>
 * @version 0.1
 *
 * File contains declaration of every class, field and method in MediaLibCleaner namespace
 */

#include <taglib/taglib.h>
#include <taglib/fileref.h>

#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <string>
#include <sys/stat.h>
#include <time.h>

#include "helpers.hpp"

/**
* @namespace MediaLibCleaner
*
* Namespace containing all classes and functions required for the application to complete designed tasks.
*/

namespace MediaLibCleaner
{
	class DFC {

	protected:
		std::string path;
		int count;

	public:
		DFC(std::string);
		~DFC();

		int GetCounter();
		std::string GetPath();

		void IncCount();
	};

	/**
	* @class File MediaLibCleaner.hpp
	*
	* Class MediaLibCleaner::File contains all fileds and methods to effectively mainpulate (read and write) tags within a file.
	*/
	class File {

	protected:
		
		// SONG INFO
		/**
		 * An TagLib::String object containing %artist% name written in audio file
		 */
		TagLib::String artist;
		/**
		* An TagLib::String object containing %title% written in audio file
		*/
		TagLib::String title;
		/**
		* An TagLib::String object containing %album% name written in audio file
		*/
		TagLib::String album;
		/**
		* An TagLib::String object containing %genre% written in audio file
		*/
		TagLib::String genre;
		/**
		* An TagLib::String object containing %comment% written in audio file
		*/
		TagLib::String comment;
		/**
		* An TagLib::uint object containing %track% number written in audio file
		*/
		TagLib::uint track;
		/**
		* An TagLib::uint object containing release %year% written in audio file
		*/
		TagLib::uint year;
		/**
		* An TagLib::String object containing %albumartist% written in audio file
		*/
		TagLib::String albumartist;
		/**
		* An TagLib::String object containing %bpm% written in audio file
		*/
		TagLib::String bpm;
		/**
		* An TagLib::String object containing %copyright% written in audio file
		*/
		TagLib::String copyright;
		/**
		* An TagLib::String object containing %language% written in audio file
		*/
		TagLib::String language;
		/**
		* An TagLib::String object containing %length% written in audio file
		*/
		TagLib::String length;
		/**
		* An TagLib::String object containing %mood% written in audio file
		*/
		TagLib::String mood;
		/**
		* An TagLib::String object containing %origalbum% written in audio file
		*/
		TagLib::String origalbum;
		/**
		* An TagLib::String object containing %origartist% written in audio file
		*/
		TagLib::String origartist;
		/**
		* An TagLib::String object containing %origfilename% written in audio file
		*/
		TagLib::String origfilename;
		/**
		* An TagLib::String object containing %origyear% written in audio file
		*/
		TagLib::String origyear;
		/**
		* An TagLib::String object containing %publisher% written in audio file
		*/
		TagLib::String publisher;
		/**
		* An TagLib::String object containing %unsyncedlyrics% written in audio file
		*/
		TagLib::String unsyncedlyrics;
		/**
		* An TagLib::String object containing %www% written in audio file
		*/
		TagLib::String www;



		// TECHNICAL INFO
		/**
		* An int containing information about audio file bitrate
		*/
		int _bitrate;
		/**
		* An int containing information about audio file codec
		*/
		std::string _codec;
		/**
		* An int containing information about audio file first cover mimetype
		*/
		std::string _cover_mimetype;
		/**
		* An int containing information about audio file first cover size (in bytes)
		*/
		size_t _cover_size;
		/**
		* An int containing information about audio file first cover type
		*/
		std::string _cover_type;
		/**
		* An int containing information about audio file covers count
		*/
		int _covers;
		/**
		* An int containing information about amount of channels in audio file
		*/
		int _channels;
		/**
		* An int containing information about auido file sample rate
		*/
		int _sampleRate;
		/**
		* An int containing information about audio file length in seconds
		*/
		int _length;




		// PATH INFO
		/**
		* An std::string containing full name of directory file resides in
		*/
		std::string _directory = "";
		/**
		* An std::string containing file extension
		*/
		std::string _ext = "";
		/**
		* An std::string containing file name (without extension)
		*/
		std::string _filename = "";
		/**
		* An std::string containing full path of directory file resides in
		*/
		std::string _folderpath = "";
		/**
		* An std::string containing name of parent directory
		*/
		std::string _parent_dir = "";
		/**
		* An std::string containing full path to audio file
		*/
		std::string _path = "";

#ifdef WIN32
		/**
		* An std::string containing volume letter (Windows only)
		*/
		std::string _volume = "";
#endif




		// FILES PROPERITIES
		/**
		* An std::string containing file created date in unix timestamp format
		*/
		time_t _file_create_datetime_raw = 0;
		/**
		* An std::string containing file modified date in unix timestamp format
		*/
		time_t _file_mod_datetime_raw = 0;
		/**
		* A size_t containing file size (in bytes)
		*/
		size_t _file_size_bytes = 0;



		/**
		* Indicates if file has been properly initialized
		* a.k.a. if it is audio file
		*/
		bool isInitiated = false;

		DFC* _dfc = NULL;

	public:

		File(std::string, MediaLibCleaner::DFC*);
		~File();



		// SONG INFO
		std::string GetArtist();
		std::string GetTitle();
		std::string GetAlbum();
		std::string GetGenre();
		std::string GetComment();
		int GetTrack();
		int GetYear();
		std::string GetAlbumArtist();
		std::string GetBPM();
		std::string GetCopyright();
		std::string GetLanguage();
		std::string GetTagLength();
		std::string GetMood();
		std::string GetOrigAlbum();
		std::string GetOrigArtist();
		std::string GetOrigFilename();
		std::string GetOrigYear();
		std::string GetPublisher();
		std::string GetLyricsUnsynced();
		std::string GetWWW();



		// TECHNICAL INFO
		int GetBitrate();
		std::string GetCodec();
		std::string GetCoverMimetype();
		size_t GetCoverSize();
		std::string GetCoverType();
		int GetCovers();
		std::string GetLengthAsString();
		int GetLength();
		int GetChannels();
		int GetSampleRate();



		// PATH INFO
		std::string GetDirectory();
		std::string GetExt();
		std::string GetFilename();
		std::string GetFilenameExt();
		std::string GetFolderPath();
		std::string GetParentDir();
		std::string GetPath();
#ifdef WIN32
		std::string GetVolume();
#endif
		

		// FILE PROPERITES
		std::string GetFileCreateDate();
		std::string GetFileCreateDatetime();
		time_t GetFileCreateDatetimeRaw();
		std::string GetFileModDate();
		std::string GetFileModDatetime();
		time_t GetFileModDatetimeRaw();
		std::string GetFileSize();
		size_t GetFileSizeBytes();
		std::string GetFileSizeKB();
		std::string GetFileSizeMB();



		bool IsInitiated();
		DFC* GetDFC();
	};





	class FilesAggregator {

	protected:
		std::list<File*> _files;
		int cfile = 0;


	public:
		FilesAggregator();
		~FilesAggregator();

		void AddFile(File*);
		File* GetFile(std::string);
		File* CurrentFile();

		std::list<File*>::iterator begin();
		std::list<File*>::iterator end();

		File* next();
	};





	/*class DFCAggregator {
	
	protected:
		std::list<DFC*> _directories;

	public:
		DFCAggregator();
		~DFCAggregator();

		void AddDirectory(DFC*);
		DFC* GetDirectory(std::string);

		std::list<DFC*>::iterator begin();
		std::list<DFC*>::iterator end();
	};*/
}