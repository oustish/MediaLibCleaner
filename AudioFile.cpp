#include "AudioFile.hpp"
#include <memory>


/**
 * Constructor for MediaLibCleaner::AudioFile class.
 * 
 * AudioFile class constructor creaties instance of TagLib::FileRef object (or simillar) and reads all common tags from the file.
 * @param[in] _path  Path to audio file this instance will represent
 */
MediaLibCleaner::AudioFile::AudioFile(std::string _path)
{
	this->pathname = _path;

	// check if file exists
	struct stat buffer;
	if (stat(this->pathname.c_str(), &buffer) != 0) {
		return;
	}

	std::unique_ptr <TagLib::FileRef> f(new TagLib::FileRef(TagLib::FileName(this->pathname.c_str())));

	this->artist = f->tag()->artist();
	this->title = f->tag()->title();
	this->album = f->tag()->album();
	this->genre = f->tag()->genre();
	this->comment = f->tag()->comment();
	this->track = f->tag()->track();
	this->year = f->tag()->year();

	this->bitrate = f->audioProperties()->bitrate();
	this->channels = f->audioProperties()->channels();
	this->sampleRate = f->audioProperties()->sampleRate();
	this->length = f->audioProperties()->length();

	this->isInitiated = true;
}

MediaLibCleaner::AudioFile::~AudioFile() {

}

/**
 * Method returns full path to audio file given object represents
 *
 * @return Path to audio file
 */
std::string MediaLibCleaner::AudioFile::GetPathname() {
	return this->pathname;
}

/**
 * Method allowing to read artist tag from an audio file
 *
 * @return Artist tag
 */
std::string MediaLibCleaner::AudioFile::GetArtist() {
	return this->artist.toCString();
}

/**
* Method allowing to read title tag from an audio file
*
* @return Title tag
*/
std::string MediaLibCleaner::AudioFile::GetTitle() {
	return this->title.toCString();
}

/**
* Method allowing to read album tag from an audio file
*
* @return Album tag
*/
std::string MediaLibCleaner::AudioFile::GetAlbum() {
	return this->album.toCString();
}

/**
* Method allowing to read genre tag from an audio file
*
* @return Genre tag
*/
std::string MediaLibCleaner::AudioFile::GetGenre() {
	return this->genre.toCString();
}

/**
* Method allowing to read comment tag from an audio file
*
* @return Comment tag
*/
std::string MediaLibCleaner::AudioFile::GetComment() {
	return this->comment.toCString();
}

/**
* Method allowing to read track tag from an audio file
*
* @return Track tag
*/
int MediaLibCleaner::AudioFile::GetTrack() {
	return this->track;
}

/**
* Method allowing to read year tag from an audio file
*
* @return Year tag
*/
int MediaLibCleaner::AudioFile::GetYear() {
	return this->year;
}

/**
* Method allowing to read bitrate of an audio file
*
* @return Bitrate (in kbps)
*/
int MediaLibCleaner::AudioFile::GetBitrate() {
	return this->bitrate;
}

/**
* Method allowing to read amount of channels in audio file
*
* @return Amount of channels
*/
int MediaLibCleaner::AudioFile::GetChannels() {
	return this->channels;
}

/**
* Method allowing to read audio file sample rate
*
* @return Sample rate (in Hz)
*/
int MediaLibCleaner::AudioFile::GetSampleRate() {
	return this->sampleRate;
}

/**
* Method allowing to read length of an audio file
*
* @return Bitrate in kbps
*/
int MediaLibCleaner::AudioFile::GetLength() {
	return this->length;
}

/**
* Method allowing to read status of given object
*
* @return Object status
*/
bool MediaLibCleaner::AudioFile::IsInitiated() {
	return this->isInitiated;
}

/**
* Method allowing to read length of an audio file as string
*
* @return Length of audio file in [[HH:]MM:]SS format
*/
std::string MediaLibCleaner::AudioFile::GetLengthAsString() {
	std::string _out = "";
	int hours = 0, minutes = 0, seconds = 0;

	if (this->length >= 3600) { // if longer than or equal to 1 hour
		hours = this->length / 3600; // no rest, only full hours

		if (hours < 10) {
			_out += "0";
		}
		_out += std::to_string(hours) + ":";
	}
	
	if (this->length >= 60) { // if longer than or equal to 1 minute
		minutes = (this->length - hours * 3600) / 60; //  no rest, only full remaining minutes

		if (minutes < 10) {
			_out += "0";
		}
		_out += std::to_string(minutes) + ":";
	}

	seconds = this->length - hours * 3600 - minutes * 60;
	if (seconds < 10) {
		_out += "0";
	}
	_out += std::to_string(seconds);

	return _out;
}