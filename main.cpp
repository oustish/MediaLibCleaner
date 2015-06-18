#include <iostream>
#include <memory> // for unique_ptr
#include "AudioFile.h"

int main(int argc, char *argv[]) {
	std::unique_ptr <MediaLibCleaner::AudioFile> file(new MediaLibCleaner::AudioFile("D:\\Muzyka\\!Synced\\NEW!\\NEW! 2015.06\\Shinedown - Call Me.mp3"));

	if (!file->IsInitiated()) return 1;

	std::cout << "FILE:       " << file->GetPathname() << std::endl;
	std::cout << "ARTIST:     " << file->GetArtist() << std::endl;
	std::cout << "TITLE:      " << file->GetTitle() << std::endl;
	std::cout << "ALBUM:      " << file->GetAlbum() << std::endl;
	std::cout << "TRACK:      " << file->GetTrack() << std::endl;
	std::cout << std::endl;
	std::cout << "BITRATE:    " << file->GetBitrate() << " [kbps]" << std::endl;
	std::cout << "SAMPLERATE: " << file->GetSampleRate() << " [Hz]" << std::endl;
	std::cout << "CHANNELS:   " << file->GetChannels() << std::endl;
	std::cout << "LENGTH:     " << file->GetLength() << " [sec]" << std::endl;

	system("pause > NUL");

	return 0;
}