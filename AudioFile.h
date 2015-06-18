#include <taglib.h>
#include <fileref.h>

#include <string>
#include <sys/stat.h>

namespace MediaLibCleaner
{
	class AudioFile {
	protected:
		std::string pathname = "";
		TagLib::String artist, title, album, genre, comment;
		TagLib::uint track, year;
		int bitrate, channels, sampleRate, length;
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

		bool IsInitiated();
	};
}