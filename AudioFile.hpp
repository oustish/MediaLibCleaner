#include <taglib/taglib.h>
#include <taglib/fileref.h>

#include <string>
#include <sys/stat.h>

/**
* @namespace MediaLibCleaner
*
* Namespace containing all classes and functions required for the application to complete designed tasks.
*/

namespace MediaLibCleaner
{
	/**
	* @class AudioFile AudioFile.hpp
	*
	* Class MediaLibCleaner::AudioFile contains all fileds and methods to effectively mainuplate (read and write) tags whitin a file.
	*/
	class AudioFile {

	protected:
		/**
		 * An std::string containing full path to audio file 
		 */
		std::string pathname = "";

		/**
		 * An TagLib::String object containing artist name written in audio file
		 */
		TagLib::String artist;
		/**
		* An TagLib::String object containing title written in audio file
		*/
		TagLib::String title;
		/**
		* An TagLib::String object containing album name written in audio file
		*/
		TagLib::String album;
		/**
		* An TagLib::String object containing genre written in audio file
		*/
		TagLib::String genre;
		/**
		* An TagLib::String object containing comment written in audio file
		*/
		TagLib::String comment;
		/**
		* An TagLib::uint object containing track number written in audio file
		*/
		TagLib::uint track;
		/**
		* An TagLib::uint object containing release year written in audio file
		*/
		TagLib::uint year;

		/**
		* An int containing information about audio file bitrate 
		*/
		int bitrate;
		/**
		* An int containing information about amount of channels in audio file
		*/
		int channels;
		/**
		* An int containing information about auido file sample rate
		*/
		int sampleRate;
		/**
		* An int containing information about audio file length in seconds
		*/
		int length;

		/**
		* Indicates if file has been properly initialized
		*/
		bool isInitiated = false;

	public:

		AudioFile(std::string);
		~AudioFile();

		std::string GetPathname();
		std::string GetArtist();
		std::string GetTitle();
		std::string GetAlbum();
		std::string GetGenre();
		std::string GetComment();

		int GetTrack();
		int GetYear();
		int GetBitrate();
		int GetChannels();
		int GetSampleRate();
		int GetLength();

		std::string GetLengthAsString();

		bool IsInitiated();
	};
}