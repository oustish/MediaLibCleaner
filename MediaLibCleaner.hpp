/**
 * @file
 * @author Szymon Oracki <szymon.oracki@oustish.pl>
 * @version 0.1
 *
 * File contains declaration of every class, field and method in MediaLibCleaner namespace
 */
#include <iostream>
#include <stdlib.h>

#include <boost/locale.hpp>

#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/mp4file.h>
#include <taglib/flacfile.h>
#include <taglib/tbytevector.h>
#include <taglib/tpropertymap.h>
#include <taglib/tmap.h>

// ID3v1 headers
#include <taglib/id3v1genres.h>
#include <taglib/id3v1tag.h>

// ID3v2 headers
#include <taglib/id3v2header.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

// Xiph headers
#include <taglib/xiphcomment.h>

// FLAC/Xiph headers
#include <taglib/flacpicture.h>

// MP4 headers
#include <taglib/mp4tag.h>
#include <taglib/mp4item.h>
#include <taglib/mp4coverart.h>

// APE headers
#include <taglib/apetag.h>


#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include <string>
#include <sys/stat.h>
#include <time.h>
#include <memory>

#include "helpers.hpp"
#include <mutex>
#include <codecvt>

/**
* @namespace MediaLibCleaner
*
* Namespace containing all classes and functions required for the application to complete designed tasks.
*/

namespace MediaLibCleaner
{
	/**
	 * @class LogAlert MediaLibCleaner.hpp
	 *
	 * Class MediaLibCleaner::LogAlert enables app to print human readable messages regarding user-defined rules.
	 */
	class LogAlert
	{
	public:
		LogAlert(std::wstring filename);
		~LogAlert();

		void Log(std::wstring module, std::wstring message);
		void Close();
		void Flush();

		bool IsOpen();

	private:
		std::wofstream outputfile;
		std::mutex synch;
	};

	/**
	 * @class LogProgram MediaLibCleaner.hpp
	 *
	 * Class MediaLibCleaner::LogProgram enables app to print human readable messages regarding program execution and debuging.
	 */
	class LogProgram
	{
	public:
		LogProgram(std::wstring filename, int init_debug_level);
		~LogProgram();

		void Log(std::wstring module, std::wstring message, int debug_level);
		void Close();
		void Flush();

		bool IsOpen();

	private:
		std::wofstream outputfile;
		int init_debug_level;
		std::mutex synch;
	};

	/**
	 * @class DFC MediaLibCleaner.hpp
	 *
	 * Class MediaLibCleaner::DFC (Directory Files Counter) is a class shared amongst all MediaLibCleaner::File objects
	 * It allows all those objects to know how much audio files are there in each directory
	 */
	class DFC {

	protected:
		std::string path;
		int count;
		std::unique_ptr<LogAlert>* logalert;
		std::unique_ptr<LogProgram>* logprogram;

	public:
		DFC(std::string, std::unique_ptr<MediaLibCleaner::LogProgram>*, std::unique_ptr<MediaLibCleaner::LogAlert>*);
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
		 * An TagLib::String object containing \%artist% name written in audio file
		 */
		TagLib::String artist;
		/**
		* An TagLib::String object containing \%title% written in audio file
		*/
		TagLib::String title;
		/**
		* An TagLib::String object containing \%album% name written in audio file
		*/
		TagLib::String album;
		/**
		* An TagLib::String object containing \%genre% written in audio file
		*/
		TagLib::String genre;
		/**
		* An TagLib::String object containing \%comment% written in audio file
		*/
		TagLib::String comment;
		/**
		* An TagLib::uint object containing \%track% number written in audio file
		*/
		TagLib::uint track;
		/**
		* An TagLib::uint object containing release \%year% written in audio file
		*/
		TagLib::uint year;
		/**
		* An TagLib::String object containing \%albumartist% written in audio file
		*/
		TagLib::String albumartist;
		/**
		* An TagLib::String object containing \%bpm% written in audio file
		*/
		TagLib::String bpm;
		/**
		* An TagLib::String object containing \%copyright% written in audio file
		*/
		TagLib::String copyright;
		/**
		* An TagLib::String object containing \%language% written in audio file
		*/
		TagLib::String language;
		/**
		* An TagLib::String object containing \%length% written in audio file
		*/
		TagLib::String length;
		/**
		* An TagLib::String object containing \%mood% written in audio file
		*/
		TagLib::String mood;
		/**
		* An TagLib::String object containing \%origalbum% written in audio file
		*/
		TagLib::String origalbum;
		/**
		* An TagLib::String object containing \%origartist% written in audio file
		*/
		TagLib::String origartist;
		/**
		* An TagLib::String object containing \%origfilename% written in audio file
		*/
		TagLib::String origfilename;
		/**
		* An TagLib::String object containing \%origyear% written in audio file
		*/
		TagLib::String origyear;
		/**
		* An TagLib::String object containing \%publisher% written in audio file
		*/
		TagLib::String publisher;
		/**
		* An TagLib::String object containing \%unsyncedlyrics% written in audio file
		*/
		TagLib::String unsyncedlyrics;
		/**
		* An TagLib::String object containing \%www% written in audio file
		*/
		TagLib::String www;



		// TECHNICAL INFO
		/**
		* An int containing information about audio file bitrate
		*/
		int d_bitrate;
		/**
		* An int containing information about audio file codec
		*/
		std::wstring d_codec;
		/**
		* An int containing information about audio file first cover mimetype
		*/
		std::wstring d_cover_mimetype;
		/**
		* An int containing information about audio file first cover size (in bytes)
		*/
		size_t d_cover_size;
		/**
		* An int containing information about audio file first cover type
		*/
		std::wstring d_cover_type;
		/**
		* An int containing information about audio file covers count
		*/
		int d_covers;
		/**
		* An int containing information about amount of channels in audio file
		*/
		int d_channels;
		/**
		* An int containing information about auido file sample rate
		*/
		int d_sampleRate;
		/**
		* An int containing information about audio file length in seconds
		*/
		int d_length;




		// PATH INFO
		/**
		* An std::string containing full name of directory file resides in
		*/
		std::wstring d_directory = L"";
		/**
		* An std::string containing file extension
		*/
		std::wstring d_ext = L"";
		/**
		* An std::string containing file name (without extension)
		*/
		std::wstring d_filename = L"";
		/**
		* An std::string containing full path of directory file resides in
		*/
		std::wstring d_folderpath = L"";
		/**
		* An std::string containing name of parent directory
		*/
		std::wstring d_parent_dir = L"";
		/**
		* An std::string containing full path to audio file
		*/
		std::wstring d_path = L"";

#ifdef WIN32
		/**
		* An std::string containing volume letter (Windows only)
		*/
		std::wstring d_volume = L"";
#endif




		// FILES PROPERITIES
		/**
		* An std::string containing file created date in unix timestamp format
		*/
		time_t d_file_create_datetime_raw = 0;
		/**
		* An std::string containing file modified date in unix timestamp format
		*/
		time_t d_file_mod_datetime_raw = 0;
		/**
		* A size_t containing file size (in bytes)
		*/
		size_t d_file_size_bytes = 0;



		/**
		* Indicates if file has been properly initialized
		* a.k.a. if it is audio file
		*/
		bool isInitiated = false;

		DFC* d_dfc = nullptr;

		std::unique_ptr<TagLib::FileRef> fileref;

		std::unique_ptr<TagLib::MPEG::File> taglib_file_mp3;
		std::unique_ptr<TagLib::Ogg::Vorbis::File> taglib_file_ogg;
		std::unique_ptr<TagLib::FLAC::File> taglib_file_flac;
		std::unique_ptr<TagLib::MP4::File> taglib_file_m4a;

		std::unique_ptr<LogAlert>* logalert;
		std::unique_ptr<LogProgram>* logprogram;

	public:

		File(std::wstring, MediaLibCleaner::DFC*, std::unique_ptr<MediaLibCleaner::LogProgram>*, std::unique_ptr<MediaLibCleaner::LogAlert>*);
		~File();



		// SONG INFO
		std::wstring GetArtist();
		std::wstring GetTitle();
		std::wstring GetAlbum();
		std::wstring GetGenre();
		std::wstring GetComment();
		int GetTrack();
		int GetYear();
		std::wstring GetAlbumArtist();
		std::wstring GetBPM();
		std::wstring GetCopyright();
		std::wstring GetLanguage();
		std::wstring GetTagLength();
		std::wstring GetMood();
		std::wstring GetOrigAlbum();
		std::wstring GetOrigArtist();
		std::wstring GetOrigFilename();
		std::wstring GetOrigYear();
		std::wstring GetPublisher();
		std::wstring GetLyricsUnsynced();
		std::wstring GetWWW();


		// TECHNICAL INFO
		int GetBitrate();
		std::wstring GetCodec();
		std::wstring GetCoverMimetype();
		size_t GetCoverSize();
		std::wstring GetCoverType();
		int GetCovers();
		std::wstring GetLengthAsString();
		int GetLength();
		int GetChannels();
		int GetSampleRate();



		// PATH INFO
		std::wstring GetDirectory();
		std::wstring GetExt();
		std::wstring GetFilename();
		std::wstring GetFilenameExt();
		std::wstring GetFolderPath();
		std::wstring GetParentDir();
		std::wstring GetPath();
#ifdef WIN32
		std::wstring GetVolume();
#endif
		

		// FILE PROPERITES
		std::wstring GetFileCreateDate();
		std::wstring GetFileCreateDatetime();
		time_t GetFileCreateDatetimeRaw();
		std::wstring GetFileModDate();
		std::wstring GetFileModDatetime();
		time_t GetFileModDatetimeRaw();
		std::wstring GetFileSize();
		size_t GetFileSizeBytes();
		std::wstring GetFileSizeKB();
		std::wstring GetFileSizeMB();

		// methods for lua processor manipulations
		bool HasTag(std::wstring tag, std::wstring val = L"::null::");
		bool Rename(std::wstring);
		bool Move(std::wstring);
		bool Delete();

		bool IsInitiated();
		DFC* GetDFC();
	};

	/**
	 * @class FilesAggregator MediaLibCleaner.hpp
	 *
	 * Class MediaLibCleaner::FilesAggregator aggregates all files that are subject to be processed acording to user-defined rules
	 */
	class FilesAggregator {

	protected:
		std::list<File*> d_files;
		int cfile = 0;
		std::unique_ptr<LogAlert>* logalert;
		std::unique_ptr<LogProgram>* logprogram;


	public:
		FilesAggregator(std::unique_ptr<MediaLibCleaner::LogProgram>*, std::unique_ptr<MediaLibCleaner::LogAlert>*);
		~FilesAggregator();

		void AddFile(File*);
		File* GetFile(std::wstring);
		File* CurrentFile(); // pamiêtaj aby dodaæ info ¿e po pobraniu elementu "oznacza" nastêpny z listy

		std::list<File*>::iterator begin();
		std::list<File*>::iterator end();

		File* next();
		File* rewind();
	};
}