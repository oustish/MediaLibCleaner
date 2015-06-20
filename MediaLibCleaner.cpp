/**
 * @file
 * @author Szymon Oracki <szymon.oracki@oustish.pl>
 * @version 0.1
 *
 * This file contains definitions of all methods for MediaLibCleaner::File class
 */

#include "MediaLibCleaner.hpp"



/**
 * Constructor for MediaLibCleaner::File class.
 * 
 * File class constructor creaties instance of TagLib::FileRef object (or simillar) and reads all common tags from the file.
 * @param[in] path  Path to audio file this instance will represent
 * @param[in] dfc	An instance of MediaLibCleaner::DFC
 */
MediaLibCleaner::File::File(std::string path, MediaLibCleaner::DFC* dfc)
{
	this->_path = path;
	this->_dfc = dfc;

	// check if file exists
	if (!boost::filesystem::exists(path)) {
		return;
	}

	std::unique_ptr <TagLib::FileRef> f(new TagLib::FileRef(TagLib::FileName(this->_path.c_str())));

	// check if file is in fact audio file (as it sometimes cannot be!)
	// effect: this->isInitalized == false
	if (f->isNull()) return;
	
	// SONG INFO
	this->artist = f->tag()->artist();
	this->title = f->tag()->title();
	this->album = f->tag()->album();
	this->genre = f->tag()->genre();
	this->comment = f->tag()->comment();
	this->track = f->tag()->track();
	this->year = f->tag()->year();
	//this->albumartist = 
	//this->bpm = 
	//this->copyright = 
	//this->language = 
	//this->length = 
	//this->mood = 
	//this->origalbum = 
	//this->origartist = 
	//this->origfilename = 
	//this->origyear = 
	//this->publisher = 
	//this->unsyncedlyrics = 
	//this->www = 

	// TECHNICAL INFO
	this->_bitrate = f->audioProperties()->bitrate();
	//this->_codec = 
	//this->_cover_mimetype = 
	//this->_cover_size = 
	//this->_cover_type = 
	//this->_covers = 
	this->_channels = f->audioProperties()->channels();
	this->_sampleRate = f->audioProperties()->sampleRate();
	this->_length = f->audioProperties()->length();


	// BOOST INIT FOR PATH INFORMATIONS
	namespace fs = boost::filesystem;
	fs::path temp = this->_path;
	fs::path fileParentDir = temp.parent_path();
	fs::path pDirParentDir = fileParentDir.parent_path();

	// PATH INFO
	this->_directory = fileParentDir.filename().string();
	this->_ext = temp.extension().string();					this->_ext = this->_ext.substr(1);
	this->_filename = temp.filename().string();				this->_filename = this->_filename.substr(0, this->_filename.length() - this->_ext.length() - 1);
	this->_folderpath = fileParentDir.string();
	this->_parent_dir = pDirParentDir.filename().string();
	// _path is setted before - required
#ifdef WIN32
	this->_volume = temp.root_name().string();
#endif

	// STAT INIT FOR DATE INFORMATION
	// WARNING: MAY ONLY WORK IN WINDOWS!!!

	struct stat attrib;
	stat(this->_path.c_str(), &attrib);

	// FILE PROPERTIES
	this->_file_create_datetime_raw = attrib.st_ctime;
	this->_file_mod_datetime_raw = attrib.st_mtime;
	this->_file_size_bytes = fs::file_size(temp);


	// OTHER
	this->isInitiated = true;
}

MediaLibCleaner::File::~File() {

}




// SONG INFO

/**
 * Method allowing to read \%artist% tag from an audio file
 *
 * @return Artist tag
 */
std::string MediaLibCleaner::File::GetArtist() {
	return this->artist.toCString();
}

/**
* Method allowing to read \%title% tag from an audio file
*
* @return Title tag
*/
std::string MediaLibCleaner::File::GetTitle() {
	return this->title.toCString();
}

/**
* Method allowing to read \%album% tag from an audio file
*
* @return Album tag
*/
std::string MediaLibCleaner::File::GetAlbum() {
	return this->album.toCString();
}

/**
* Method allowing to read \%genre% tag from an audio file
*
* @return Genre tag
*/
std::string MediaLibCleaner::File::GetGenre() {
	return this->genre.toCString();
}

/**
* Method allowing to read \%comment% tag from an audio file
*
* @return Comment tag
*/
std::string MediaLibCleaner::File::GetComment() {
	return this->comment.toCString();
}

/**
* Method allowing to read \%track% tag from an audio file
*
* @return Track tag
*/
int MediaLibCleaner::File::GetTrack() {
	return this->track;
}

/**
* Method allowing to read \%year% tag from an audio file
*
* @return Year tag
*/
int MediaLibCleaner::File::GetYear() {
	return this->year;
}

/**
* Method allowing to read \%albumartist% tag from an audio file
*
* @return Album artist tag
*/
std::string MediaLibCleaner::File::GetAlbumArtist() {
	return this->albumartist.toCString();
}

/**
* Method allowing to read \%bpm% tag from an audio file
*
* @return BPM tag
*/
std::string MediaLibCleaner::File::GetBPM() {
	return this->bpm.toCString();
}

/**
* Method allowing to read \%copyright% tag from an audio file
*
* @return Copyright tag
*/
std::string MediaLibCleaner::File::GetCopyright() {
	return this->copyright.toCString();
}

/**
* Method allowing to read \%language% tag from an audio file
*
* @return Language tag
*/
std::string MediaLibCleaner::File::GetLanguage() {
	return this->language.toCString();
}

/**
* Method allowing to read \%length% tag from an audio file
*
* @return Length tag
*/
std::string MediaLibCleaner::File::GetTagLength() {
	return this->length.toCString();
}

/**
* Method allowing to read \%mood% tag from an audio file
*
* @return Mood tag
*/
std::string MediaLibCleaner::File::GetMood() {
	return this->mood.toCString();
}

/**
* Method allowing to read \%origalbum% tag from an audio file
*
* @return Original album tag
*/
std::string MediaLibCleaner::File::GetOrigAlbum() {
	return this->origalbum.toCString();
}

/**
* Method allowing to read \%origartist% tag from an audio file
*
* @return Original artist tag
*/
std::string MediaLibCleaner::File::GetOrigArtist() {
	return this->origartist.toCString();
}

/**
* Method allowing to read \%origfilename% tag from an audio file
*
* @return Original filename tag
*/
std::string MediaLibCleaner::File::GetOrigFilename() {
	return this->origfilename.toCString();
}

/**
* Method allowing to read \%origyear% tag from an audio file
*
* @return Original year tag
*/
std::string MediaLibCleaner::File::GetOrigYear() {
	return this->origyear.toCString();
}

/**
* Method allowing to read \%publisher% tag from an audio file
*
* @return Publisher tag
*/
std::string MediaLibCleaner::File::GetPublisher() {
	return this->publisher.toCString();
}

/**
* Method allowing to read \%unsyncedlyrics% tag from an audio file
*
* @return Unsynced lyrics tag
*/
std::string MediaLibCleaner::File::GetLyricsUnsynced() {
	return this->unsyncedlyrics.toCString();
}

/**
* Method allowing to read \%www% tag from an audio file
*
* @return WWW tag
*/
std::string MediaLibCleaner::File::GetWWW() {
	return this->www.toCString();
}



// TECHNICAL INFO


/**
* Method allowing to read bitrate of an audio file
*
* @return Bitrate (in kbps)
*/
int MediaLibCleaner::File::GetBitrate() {
	return this->_bitrate;
}

/**
* Method allowing to read codec of an audio file
*
* @return Codec id
*/
std::string MediaLibCleaner::File::GetCodec() {
	return this->_codec;
}

/**
* Method allowing to read mimetype of first cover in an audio file
*
* @return Mimetype of first cover
*/
std::string MediaLibCleaner::File::GetCoverMimetype() {
	return this->_cover_mimetype;
}

/**
* Method allowing to read size of first cover in an audio file
*
* @return Size of first cover
*/
size_t MediaLibCleaner::File::GetCoverSize() {
	return this->_cover_size;
}

/**
* Method allowing to read type of first cover in an audio file
*
* @return Type of first cover
*/
std::string MediaLibCleaner::File::GetCoverType() {
	return this->_cover_type;
}

/**
* Method allowing to read amount of covers in an audio file
*
* @return Amount of covers in file
*/
int MediaLibCleaner::File::GetCovers() {
	return this->_covers;
}

/**
* Method allowing to read amount of channels in audio file
*
* @return Amount of channels
*/
int MediaLibCleaner::File::GetChannels() {
	return this->_channels;
}

/**
* Method allowing to read audio file sample rate
*
* @return Sample rate (in Hz)
*/
int MediaLibCleaner::File::GetSampleRate() {
	return this->_sampleRate;
}

/**
* Method allowing to read length of an audio file as string
*
* @return Length of audio file in [[HH:]MM:]SS format
*/
std::string MediaLibCleaner::File::GetLengthAsString() {
	std::string _out = "";
	int hours = 0, minutes = 0, seconds = 0;

	if (this->_length >= 3600) { // if longer than or equal to 1 hour
		hours = this->_length / 3600; // no rest, only full hours

		if (hours < 10) {
			_out += "0";
		}
		_out += std::to_string(hours) + ":";
	}

	if (this->_length >= 60) { // if longer than or equal to 1 minute
		minutes = (this->_length - hours * 3600) / 60; //  no rest, only full remaining minutes

		if (minutes < 10) {
			_out += "0";
		}
		_out += std::to_string(minutes) + ":";
	}

	seconds = this->_length - hours * 3600 - minutes * 60;
	if (seconds < 10) {
		_out += "0";
	}
	_out += std::to_string(seconds);

	return _out;
}

/**
* Method allowing to read length of an audio file
*
* @return Bitrate in kbps
*/
int MediaLibCleaner::File::GetLength() {
	return this->_length;
}





// PATH INFO
/**
* Method returns full name of direcotry the file resides in
*
* @return Directory name containing file
*/
std::string MediaLibCleaner::File::GetDirectory() {
	return this->_directory;
}
/**
* Method returns extension of the file
*
* @return File extension
*/
std::string MediaLibCleaner::File::GetExt() {
	return this->_ext;
}
/**
* Method returns name of the file without extension
*
* @return Filename without extension
*/
std::string MediaLibCleaner::File::GetFilename() {
	return this->_filename;
}
/**
* Method returns filename with the extension
*
* @return Filename with extensions
*/
std::string MediaLibCleaner::File::GetFilenameExt() {
	return (this->_filename + "." + this->_ext);
}
/**
* Method returns path to directory that contains the file
*
* @return Path to directory containing file
*/
std::string MediaLibCleaner::File::GetFolderPath() {
	return this->_folderpath;
}
/**
* Method returns name of parent directory for %_directory% dir
*
* @return Name of parent dir for %_directory% dir
*/
std::string MediaLibCleaner::File::GetParentDir() {
	return this->_parent_dir;
}
/**
* Method returns full path to audio file given object represents
*
* @return Full path to audio file
*/
std::string MediaLibCleaner::File::GetPath() {
	return this->_path;
}

#ifdef WIN32
	/**
	* Method returns volume letter (Windows only)
	*
	* @return Letter followed by colon of volume the file resides on
	*/
	std::string MediaLibCleaner::File::GetVolume() {
		return this->_volume;
	}
#endif



// FILE PROPERITES
/**
* Method returns file created date in ISO 8601 format
*
* @return File created date in ISO 8601 format
*/
std::string MediaLibCleaner::File::GetFileCreateDate() {
	return get_date_iso_8601(this->_file_create_datetime_raw);
}
/**
* Method returns file created date in RFC 2822 format
*
* @return File created date in RFC 2822 format
*/
std::string MediaLibCleaner::File::GetFileCreateDatetime() {
	return get_date_rfc_2822(this->_file_create_datetime_raw);
}
/**
* Method returns file created date in unix timestamp format
*
* @return File created date in unix timestamp format
*/
time_t MediaLibCleaner::File::GetFileCreateDatetimeRaw() {
	return this->_file_create_datetime_raw;
}
/**
* Method returns file modified date in ISO 8601 format
*
* @return File modified date in ISO 8601 format
*/
std::string MediaLibCleaner::File::GetFileModDate() {
	return get_date_iso_8601(this->_file_mod_datetime_raw);
}
/**
* Method returns file modified date in RFC 2822 format
*
* @return File modified date in RFC 2822 format
*/
std::string MediaLibCleaner::File::GetFileModDatetime() {
	return get_date_rfc_2822(this->_file_mod_datetime_raw);
}
/**
* Method returns file modified date in unix timestamp format
*
* @return File modified date in unix timestamp format
*/
time_t MediaLibCleaner::File::GetFileModDatetimeRaw() {
	return this->_file_mod_datetime_raw;
}
/**
* Method returns file size in human readable format
*
* @return File size in human readable format
*/
std::string MediaLibCleaner::File::GetFileSize() {
	float temp = this->_file_size_bytes / 1048576; // MB

	if (this->_file_size_bytes <= 1023) { // B
		return std::to_string(this->_file_size_bytes) + "B";
	}
	else if (this->_file_size_bytes > 1023 && this->_file_size_bytes <= 1048575) { // KB
		return this->GetFileSizeKB();
	}
	else if (this->_file_size_bytes > 1048575 && temp < 1024) { // MB
		return this->GetFileSizeMB();
	}
	else { // GB
		return std::to_string(temp / 1024);
	}
}
/**
* Method returns file size in bytes
*
* @return File size in bytes
*/
size_t MediaLibCleaner::File::GetFileSizeBytes() {
	return this->_file_size_bytes;
}
/**
* Method returns file size in kilo bytes
*
* @return File size in kilo bytes
*/
std::string MediaLibCleaner::File::GetFileSizeKB() {
	return std::to_string(this->_file_size_bytes / 1024) + "KB";
}
/**
* Method returns file size in mega bytes
*
* @return File size in mega bytes
*/
std::string MediaLibCleaner::File::GetFileSizeMB() {
	std::string t = std::to_string(this->_file_size_bytes / 1048576) + "MB";
	
	return t;
}


/**
* Method allowing to read status of given object
*
* @return Object status
*/
bool MediaLibCleaner::File::IsInitiated() {
	return this->isInitiated;
}

MediaLibCleaner::DFC* MediaLibCleaner::File::GetDFC() {
	return this->_dfc;
}









MediaLibCleaner::FilesAggregator::FilesAggregator() {}
MediaLibCleaner::FilesAggregator::~FilesAggregator() {
	std::list<MediaLibCleaner::File*>::iterator nd = this->end();
	for (std::list<MediaLibCleaner::File*>::iterator it = this->begin(); it != nd; it++) {
		delete *it;
	}
}

void MediaLibCleaner::FilesAggregator::AddFile(MediaLibCleaner::File *_file) {
	this->_files.push_back(_file);
}

MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::GetFile(std::string _filepath) {
	std::list<MediaLibCleaner::File*>::iterator nd = this->end();
	for (std::list<MediaLibCleaner::File*>::iterator it = this->begin(); it != nd; it++) {
		if ((*it)->GetPath() == _filepath) {
			return *it;
		}
	}
	return NULL;
}

MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::CurrentFile() {
	std::list<MediaLibCleaner::File*>::iterator it = this->begin();
	std::advance(it, this->cfile);

	return *it;
}

std::list<MediaLibCleaner::File*>::iterator MediaLibCleaner::FilesAggregator::begin() {
	return this->_files.begin();
}

std::list<MediaLibCleaner::File*>::iterator MediaLibCleaner::FilesAggregator::end() {
	return this->_files.end();
}

MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::next() {
	if (this->cfile == (this->CurrentFile()->GetDFC()->GetCounter() - 1)) return NULL;

	this->cfile++;

	std::list<MediaLibCleaner::File*>::iterator it = this->begin();
	std::advance(it, this->cfile);

	return *it;
}





MediaLibCleaner::DFC::DFC(std::string _path) {
	this->path = _path;
	this->count = 0;
}

MediaLibCleaner::DFC::~DFC() {}

int MediaLibCleaner::DFC::GetCounter() {
	return this->count;
}

std::string MediaLibCleaner::DFC::GetPath() {
	return this->path;
}

void MediaLibCleaner::DFC::IncCount() {
	this->count++;
}







/*MediaLibCleaner::DFCAggregator::DFCAggregator() {}
MediaLibCleaner::DFCAggregator::~DFCAggregator() {
	std::list<MediaLibCleaner::DFC*>::iterator nd = this->end();
	for (std::list<MediaLibCleaner::DFC*>::iterator it = this->begin(); it != nd; it++) {
		delete *it;
	}
}

void MediaLibCleaner::DFCAggregator::AddDirectory(MediaLibCleaner::DFC *_dfc) {
	this->_directories.push_back(_dfc);
}

MediaLibCleaner::DFC* MediaLibCleaner::DFCAggregator::GetDirectory(std::string _dirpath) {
	std::list<MediaLibCleaner::DFC*>::iterator nd = this->end();
	for (std::list<MediaLibCleaner::DFC*>::iterator it = this->begin(); it != nd; it++) {
		if ((*it)->GetPath() == _dirpath) {
			return *it;
		}
	}
	return NULL;
}

std::list<MediaLibCleaner::DFC*>::iterator MediaLibCleaner::DFCAggregator::begin() {
	return this->_directories.begin();
}

std::list<MediaLibCleaner::DFC*>::iterator MediaLibCleaner::DFCAggregator::end() {
	return this->_directories.end();
}*/