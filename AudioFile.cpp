#include "AudioFile.h"

MediaLibCleaner::AudioFile::AudioFile(std::string _path)
{
	this->pathname = _path;

	// check if file exists
	struct stat buffer;
	if (stat(this->pathname.c_str(), &buffer) != 0) {
		return;
	}

	TagLib::FileRef f(TagLib::FileName(this->pathname.c_str()));

	this->artist = f.tag()->artist();
	this->title = f.tag()->title();
	this->album = f.tag()->album();
	this->genre = f.tag()->genre();
	this->comment = f.tag()->comment();
	this->track = f.tag()->track();
	this->year = f.tag()->year();

	this->bitrate = f.audioProperties()->bitrate();
	this->channels = f.audioProperties()->channels();
	this->sampleRate = f.audioProperties()->sampleRate();
	this->length = f.audioProperties()->length();

	this->isInitiated = true;
}

MediaLibCleaner::AudioFile::~AudioFile() {

}

std::string MediaLibCleaner::AudioFile::GetPathname() {
	return this->pathname;
}

std::string MediaLibCleaner::AudioFile::GetArtist() {
	return this->artist.toCString();
}

std::string MediaLibCleaner::AudioFile::GetTitle() {
	return this->title.toCString();
}

std::string MediaLibCleaner::AudioFile::GetAlbum() {
	return this->album.toCString();
}

std::string MediaLibCleaner::AudioFile::GetGenre() {
	return this->genre.toCString();
}

std::string MediaLibCleaner::AudioFile::GetComment() {
	return this->comment.toCString();
}

int MediaLibCleaner::AudioFile::GetTrack() {
	return this->track;
}

int MediaLibCleaner::AudioFile::GetYear() {
	return this->year;
}

int MediaLibCleaner::AudioFile::GetBitrate() {
	return this->bitrate;
}

int MediaLibCleaner::AudioFile::GetChannels() {
	return this->channels;
}

int MediaLibCleaner::AudioFile::GetSampleRate() {
	return this->sampleRate;
}

int MediaLibCleaner::AudioFile::GetLength() {
	return this->length;
}

bool MediaLibCleaner::AudioFile::IsInitiated() {
	return this->isInitiated;
}