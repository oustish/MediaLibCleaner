/**
 * @file
 * @author Szymon Oracki <szymon.oracki@oustish.pl>
 * @version 0.1
 *
 * This file contains definitions of all methods for MediaLibCleaner namespace
 *
 * @todo check in each MediaLibCleaner::File::? method if file was initialized and return values only if it was
 */

#include "MediaLibCleaner.hpp"



/**
 * Constructor for MediaLibCleaner::File class.
 * 
 * File class constructor creaties instance of TagLib::FileRef object (or simillar) and reads all common tags from the file.
 * @param[in] path  Path to audio file this instance will represent
 * @param[in] dfc	An instance of MediaLibCleaner::DFC
 */
MediaLibCleaner::File::File(std::wstring path, MediaLibCleaner::DFC* dfc)
{
	this->d_path = path;
	this->d_dfc = dfc;

	// check if file exists
	if (!boost::filesystem::exists(path)) {
		return;
	}

	std::unique_ptr <TagLib::FileRef> f(new TagLib::FileRef(TagLib::FileName(this->d_path.c_str())));

	this->fileref.swap(f);

	// BOOST INIT FOR PATH INFORMATIONS
	namespace fs = boost::filesystem;
	fs::path temp = this->d_path;
	fs::path fileParentDir = temp.parent_path();
	fs::path pDirParentDir = fileParentDir.parent_path();

	// PATH INFO
	this->d_directory = fileParentDir.filename().wstring();
	this->d_ext = temp.extension().wstring();					this->d_ext = this->d_ext.substr(1);
	this->d_filename = temp.filename().wstring();				this->d_filename = this->d_filename.substr(0, this->d_filename.length() - this->d_ext.length() - 1);
	this->d_folderpath = fileParentDir.wstring();
	this->d_parent_dir = pDirParentDir.filename().wstring();
	// _path is setted before - required
#ifdef WIN32
	this->d_volume = temp.root_name().wstring();
#endif

	// STAT INIT FOR DATE INFORMATION
	// WARNING: MAY ONLY WORK IN WINDOWS!!!

	struct stat attrib;
	stat(ws2s(this->d_path).c_str(), &attrib);

	// FILE PROPERTIES
	this->d_file_create_datetime_raw = attrib.st_ctime;
	this->d_file_mod_datetime_raw = attrib.st_mtime;
	this->d_file_size_bytes = fs::file_size(temp);

	// check if file is in fact audio file (as it sometimes cannot be!)
	// effect: this->isInitalized == false, but rest info (about files) is present
	if (this->fileref->isNull()) return;

	this->d_dfc->IncCount();
	
	// SONG INFO
	this->artist = this->fileref->tag()->artist();
	this->title = this->fileref->tag()->title();
	this->album = this->fileref->tag()->album();
	this->genre = this->fileref->tag()->genre();
	this->comment = this->fileref->tag()->comment();
	this->track = this->fileref->tag()->track();
	this->year = this->fileref->tag()->year();
	// rest of aliases defined below

	// TECHNICAL INFO
	this->d_bitrate = this->fileref->audioProperties()->bitrate();
	//this->d_codec = 
	//this->d_cover_mimetype = 
	//this->d_cover_size = 
	//this->d_cover_type = 
	//this->d_covers = 
	this->d_channels = this->fileref->audioProperties()->channels();
	this->d_sampleRate = this->fileref->audioProperties()->sampleRate();
	this->d_length = this->fileref->audioProperties()->length();

	TagLib::FileRef *fr = this->fileref.release();
	delete fr;

	// CAN WORK WITH THE REST OF ALIASES NOW,
	// SINCE _EXT IS AVALIABLE

	//check for file type
	
	if (this->d_ext == L"mp3") {
		std::unique_ptr<TagLib::MPEG::File> temp(new TagLib::MPEG::File(TagLib::FileName(this->d_path.c_str())));
		temp.swap(this->taglib_file_mp3);
	}
	else if (this->d_ext == L"ogg") {
		std::unique_ptr<TagLib::Ogg::Vorbis::File> temp(new TagLib::Ogg::Vorbis::File(TagLib::FileName(this->d_path.c_str())));
		temp.swap(this->taglib_file_ogg);
	}
	else if (this->d_ext == L"flac") {
		std::unique_ptr<TagLib::FLAC::File> temp(new TagLib::FLAC::File(TagLib::FileName(this->d_path.c_str())));
		temp.swap(this->taglib_file_flac);
	}
	else if (this->d_ext == L"m4a" || this->d_ext == L"mp4") {
		std::unique_ptr<TagLib::MP4::File> temp(new TagLib::MP4::File(TagLib::FileName(this->d_path.c_str())));
		temp.swap(this->taglib_file_m4a);
	}
	else { this->isInitiated = false; return; }


	if (this->d_ext == L"mp3") { // ID3v1, ID3v2 or APE tags present
		TagLib::ID3v2::Tag *id3v2tag = this->taglib_file_mp3->ID3v2Tag();
		TagLib::APE::Tag *apetag = this->taglib_file_mp3->APETag();

		if (this->taglib_file_mp3->hasID3v2Tag()) {
			TagLib::ID3v2::FrameList::ConstIterator it = id3v2tag->frameList().begin();
			for (; it != id3v2tag->frameList().end(); it++) {
				std::wstring name = TagLib::String((*it)->frameID()).toWString();
				std::wstring value = (*it)->toString().toWString();

				if (name == L"TPE2") {
					this->albumartist = value;
				}
				else if (name == L"TBPM") {
					this->bpm = value;
				}
				else if (name == L"TCOP") {
					this->copyright = value;
				}
				else if (name == L"TLAN") {
					this->language = value;
				}
				else if (name == L"TLEN") {
					this->length = value;
				}
				else if (name == L"TMOO") {
					this->mood = value;
				}
				else if (name == L"TXXX" && value.substr(0, 6) == L"[MOOD]") {
					this->mood = value.substr(12); // format: [MOOD] MOOD %mood%
				}
				else if (name == L"TOAL") {
					this->origalbum = value;
				}
				else if (name == L"TOPE") {
					this->origartist = value;
				}
				else if (name == L"TOFN") {
					this->origfilename = value;
				}
				else if (name == L"TDOR") {
					this->origyear = value;
				}
				else if (name == L"TPUB") {
					this->publisher = value;
				}
				else if (name == L"USLT") {
					this->unsyncedlyrics = value;
				}
				else if (name == L"WXXX" && value.substr(0, 2) == L"[]") {
					this->www = value.substr(3);// format: [] www
				}
			}
		}
		else if (this->taglib_file_mp3->hasAPETag()) {
			TagLib::APE::ItemListMap tags = apetag->itemListMap();

			this->albumartist = tags["ALBUMARTIST"].toString();
			this->bpm = tags["BPM"].toString();
			this->copyright = tags["COPYRIGHT"].toString();
			this->language = tags["LANGUAGE"].toString();
			this->length = tags["LENGTH"].toString();
			this->mood = tags["MOOD"].toString();
			this->origalbum = tags["ORIGALBUM"].toString();
			this->origartist = tags["ORIGARTIST"].toString();
			this->origfilename = tags["ORIGFILENAME"].toString();
			this->origyear = tags["ORIGYEAR"].toString();
			this->publisher = tags["PUBLISHER"].toString();
			this->unsyncedlyrics = tags["UNSYNCEDLYRICS"].toString();
			this->www = tags["WWW"].toString();
		}
		else {
			// ID3v1 dosen't have any of the extended tags
			// clear them out to be on the safe side
			this->albumartist = "";
			this->bpm = "";
			this->copyright = "";
			this->language = "";
			this->length = "";
			this->mood = "";
			this->origalbum = "";
			this->origartist = "";
			this->origfilename = "";
			this->origyear = "";
			this->publisher = "";
			this->unsyncedlyrics = "";
			this->www = "";
		}
	}
	else if (this->d_ext == L"ogg") {
		TagLib::PropertyMap tags = this->taglib_file_ogg->tag()->properties();

		for (auto it = tags.begin(); it != tags.end(); it++) {
			if (it->first.toWString() == L"ALBUMARTIST")
				this->albumartist = it->second.toString().toWString();
			else if (it->first.toWString() == L"BPM")
				this->bpm = it->second.toString().toWString();
			else if (it->first.toWString() == L"COPYRIGHT")
				this->copyright = it->second.toString().toWString();
			else if (it->first.toWString() == L"LANGUAGE")
				this->language = it->second.toString().toWString();
			else if (it->first.toWString() == L"LENGTH")
				this->length = it->second.toString().toWString();
			else if (it->first.toWString() == L"MOOD")
				this->mood = it->second.toString().toWString();
			else if (it->first.toWString() == L"ORIGALBUM")
				this->origalbum = it->second.toString().toWString();
			else if (it->first.toWString() == L"ORIGARTIST")
				this->origartist = it->second.toString().toWString();
			else if (it->first.toWString() == L"ORIGFILENAME")
				this->origfilename = it->second.toString().toWString();
			else if (it->first.toWString() == L"ORIGYEAR")
				this->origyear = it->second.toString().toWString();
			else if (it->first.toWString() == L"ORGANIZATION") // publisher
				this->publisher = it->second.toString().toWString();
			else if (it->first.toWString() == L"UNSYNCEDLYRICS")
				this->unsyncedlyrics = it->second.toString().toWString();
			else if (it->first.toWString() == L"URL" || it->first.toWString() == L"WWW")
				this->www = it->second.toString().toWString();
		}
	}
	else if (this->d_ext == L"flac") {
		TagLib::ID3v2::Tag *id3v2tag = this->taglib_file_flac->ID3v2Tag();
		TagLib::Ogg::XiphComment *xiphcomment = this->taglib_file_flac->xiphComment();

		if (this->taglib_file_flac->hasXiphComment()) {
			TagLib::Ogg::FieldListMap tags = xiphcomment->fieldListMap();

			this->albumartist = tags["ALBUMARTIST"].toString();
			this->bpm = tags["BPM"].toString();
			this->copyright = tags["COPYRIGHT"].toString();
			this->language = tags["LANGUAGE"].toString();
			this->length = tags["LENGTH"].toString();
			this->mood = tags["MOOD"].toString();
			this->origalbum = tags["ORIGALBUM"].toString();
			this->origartist = tags["ORIGARTIST"].toString();
			this->origfilename = tags["ORIGFILENAME"].toString();
			this->origyear = tags["ORIGYEAR"].toString();
			this->publisher = tags["ORGANIZATION"].toString();
			this->unsyncedlyrics = tags["UNSYNCEDLYRICS"].toString();
			this->www = tags["WWW"].toString();
		}
		else if (this->taglib_file_flac->hasID3v2Tag()) {
			TagLib::ID3v2::FrameList::ConstIterator it = id3v2tag->frameList().begin();
			for (; it != id3v2tag->frameList().end(); it++) {
				std::wstring name = TagLib::String((*it)->frameID()).toWString();
				std::wstring value = (*it)->toString().toWString();

				if (name == L"TPE2") {
					this->albumartist = value;
				}
				else if (name == L"TBPM") {
					this->bpm = value;
				}
				else if (name == L"TCOP") {
					this->copyright = value;
				}
				else if (name == L"TLAN") {
					this->language = value;
				}
				else if (name == L"TLEN") {
					this->length = value;
				}
				else if (name == L"TMOO") {
					this->mood = value;
				}
				else if (name == L"TXXX" && value.substr(0, 6) == L"[MOOD]") {
					this->mood = value.substr(12); // format: [MOOD] MOOD %mood%
				}
				else if (name == L"TOAL") {
					this->origalbum = value;
				}
				else if (name == L"TOPE") {
					this->origartist = value;
				}
				else if (name == L"TOFN") {
					this->origfilename = value;
				}
				else if (name == L"TDOR") {
					this->origyear = value;
				}
				else if (name == L"TPUB") {
					this->publisher = value;
				}
				else if (name == L"USLT") {
					this->unsyncedlyrics = value;
				}
				else if (name == L"WXXX" && value.substr(0, 2) == L"[]") {
					this->www = value.substr(3);// format: [] www
				}
			}
		}
		else {
			// ID3v1 dosen't have any of the extended tags
			// clear them out to be on the safe side
			this->albumartist = "";
			this->bpm = "";
			this->copyright = "";
			this->language = "";
			this->length = "";
			this->mood = "";
			this->origalbum = "";
			this->origartist = "";
			this->origfilename = "";
			this->origyear = "";
			this->publisher = "";
			this->unsyncedlyrics = "";
			this->www = "";
		}
	}
	else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
	{
		TagLib::MP4::ItemListMap taglist = this->taglib_file_m4a->tag()->itemListMap();
		for (auto it = taglist.begin(); it != taglist.end(); ++it)
		{
			if (it->first.toWString() == L"aART")
				this->albumartist = it->second.toStringList().toString(", ").toWString();
			if (it->first.toWString() == L"----:com.apple.iTunes:LENGTH")
				this->length = it->second.toStringList().toString(", ").toWString();
			if (it->first.toWString() == L"----:com.apple.iTunes:ORIGARTIST")
				this->origartist = it->second.toStringList().toString(", ").toWString();
			if (it->first.toWString() == L"----:com.apple.iTunes:ORIGALBUM")
				this->origalbum = it->second.toStringList().toString(", ").toWString();
			if (it->first.toWString() == L"----:com.apple.iTunes:ORIGFILENAME")
				this->origfilename = it->second.toStringList().toString(", ").toWString();
			if (it->first.toWString() == L"----:com.apple.iTunes:ORIGYEAR")
				this->origyear = it->second.toStringList().toString(", ").toWString();
			if (it->first.toWString() == L"----:com.apple.iTunes:PUBLISHER")
				this->publisher = it->second.toStringList().toString(", ").toWString();
			if (it->first.toWString() == L"----:com.apple.iTunes:WWW")
				this->www = it->second.toStringList().toString(", ").toWString();
		}

		TagLib::PropertyMap tags = this->taglib_file_m4a->tag()->properties();

		for (auto it = tags.begin(); it != tags.end(); it++) {
			if (it->first.toWString() == L"BPM")
				this->bpm = it->second.toString().toWString();
			else if (it->first.toWString() == L"COPYRIGHT")
				this->copyright = it->second.toString().toWString();
			else if (it->first.toWString() == L"LANGUAGE")
				this->language = it->second.toString().toWString();
			else if (it->first.toWString() == L"MOOD")
				this->mood = it->second.toString().toWString();
			else if (it->first.toWString() == L"LYRICS")
				this->unsyncedlyrics = it->second.toString().toWString();
		}
	}


	// OTHER
	this->isInitiated = true;
}

MediaLibCleaner::File::~File() {

}




// SONG INFO

/**
 * Method allowing to read \%artist% tag from an audio file
 *
 * @return Artist tag or empty string if file is not audio file
 */
std::wstring MediaLibCleaner::File::GetArtist() {
	if (this->isInitiated)
		return this->artist.toWString();
	return L"";
}

/**
* Method allowing to read \%title% tag from an audio file
*
* @return Title tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetTitle() {
	if (this->isInitiated)
		return this->title.toWString();
	return L"";
}

/**
* Method allowing to read \%album% tag from an audio file
*
* @return Album tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetAlbum() {
	if (this->isInitiated)
		return this->album.toWString();
	return L"";
}

/**
* Method allowing to read \%genre% tag from an audio file
*
* @return Genre tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetGenre() {
	if (this->isInitiated)
		return this->genre.toWString();
	return L"";
}

/**
* Method allowing to read \%comment% tag from an audio file
*
* @return Comment tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetComment() {
	if (this->isInitiated)
		return this->comment.toWString();
	return L"";
}

/**
* Method allowing to read \%track% tag from an audio file
*
* @return Track tag or -1 if file is not audio file
*/
int MediaLibCleaner::File::GetTrack() {
	if (this->isInitiated)
		return this->track;
	return -1;
}

/**
* Method allowing to read \%year% tag from an audio file
*
* @return Year tag or -1 if file is not audio file
*/
int MediaLibCleaner::File::GetYear() {
	if (this->isInitiated)
		return this->year;
	return -1;
}

/**
* Method allowing to read \%albumartist% tag from an audio file
*
* @return Album artist tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetAlbumArtist() {
	if (this->isInitiated)
		return this->albumartist.toWString();
	return L"";
}

/**
* Method allowing to read \%bpm% tag from an audio file
*
* @return BPM tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetBPM() {
	if (this->isInitiated)
		return this->bpm.toWString();
	return L"";
}

/**
* Method allowing to read \%copyright% tag from an audio file
*
* @return Copyright tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetCopyright() {
	if (this->isInitiated)
		return this->copyright.toWString();
	return L"";
}

/**
* Method allowing to read \%language% tag from an audio file
*
* @return Language tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetLanguage() {
	if (this->isInitiated)
		return this->language.toWString();
	return L"";
}

/**
* Method allowing to read \%length% tag from an audio file
*
* @return Length tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetTagLength() {
	if (this->isInitiated)
		return this->length.toWString();
	return L"";
}

/**
* Method allowing to read \%mood% tag from an audio file
*
* @return Mood tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetMood() {
	if (this->isInitiated)
		return this->mood.toWString();
	return L"";
}

/**
* Method allowing to read \%origalbum% tag from an audio file
*
* @return Original album tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetOrigAlbum() {
	if (this->isInitiated)
		return this->origalbum.toWString();
	return L"";
}

/**
* Method allowing to read \%origartist% tag from an audio file
*
* @return Original artist tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetOrigArtist() {
	if (this->isInitiated)
		return this->origartist.toWString();
	return L"";
}

/**
* Method allowing to read \%origfilename% tag from an audio file
*
* @return Original filename tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetOrigFilename() {
	if (this->isInitiated)
		return this->origfilename.toWString();
	return L"";
}

/**
* Method allowing to read \%origyear% tag from an audio file
*
* @return Original year tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetOrigYear() {
	if (this->isInitiated)
		return this->origyear.toWString();
	return L"";
}

/**
* Method allowing to read \%publisher% tag from an audio file
*
* @return Publisher tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetPublisher() {
	if (this->isInitiated)
		return this->publisher.toWString();
	return L"";
}

/**
* Method allowing to read \%unsyncedlyrics% tag from an audio file
*
* @return Unsynced lyrics tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetLyricsUnsynced() {
	if (this->isInitiated)
		return this->unsyncedlyrics.toWString();
	return L"";
}

/**
* Method allowing to read \%www% tag from an audio file
*
* @return WWW tag or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetWWW() {
	if (this->isInitiated)
		return this->www.toWString();
	return L"";
}



// TECHNICAL INFO


/**
* Method allowing to read bitrate of an audio file
*
* @return Bitrate (in kbps) or -1 if file is not audio file
*/
int MediaLibCleaner::File::GetBitrate() {
	if (this->isInitiated)
		return this->d_bitrate;
	return -1;
}

/**
* Method allowing to read codec of an audio file
*
* @return Codec id or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetCodec() {
	if (this->isInitiated)
		return this->d_codec;
	return L"";
}

/**
* Method allowing to read mimetype of first cover in an audio file
*
* @return Mimetype of first cover or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetCoverMimetype() {
	if (this->isInitiated)
		return this->d_cover_mimetype;
	return L"";
}

/**
* Method allowing to read size of first cover in an audio file
*
* @return Size of first cover or -1 if file is not audio file
*/
size_t MediaLibCleaner::File::GetCoverSize() {
	if (this->isInitiated)
		return this->d_cover_size;
	return -1;
}

/**
* Method allowing to read type of first cover in an audio file
*
* @return Type of first cover or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetCoverType() {
	if (this->isInitiated)
		return this->d_cover_type;
	return L"";
}

/**
* Method allowing to read amount of covers in an audio file
*
* @return Amount of covers in file or -1 if file is not audio file
*/
int MediaLibCleaner::File::GetCovers() {
	if (this->isInitiated)
		return this->d_covers;
	return -1;
}

/**
* Method allowing to read amount of channels in audio file
*
* @return Amount of channels or -1 if file is not audio file
*/
int MediaLibCleaner::File::GetChannels() {
	if (this->isInitiated)
		return this->d_channels;
	return -1;
}

/**
* Method allowing to read audio file sample rate
*
* @return Sample rate (in Hz) or -1 if file is not audio file
*/
int MediaLibCleaner::File::GetSampleRate() {
	if (this->isInitiated)
		return this->d_sampleRate;
	return -1;
}

/**
* Method allowing to read length of an audio file as string
*
* @return Length of audio file in [[HH:]MM:]SS format or empty string if file is not audio file
*/
std::wstring MediaLibCleaner::File::GetLengthAsString() {
	if (!this->isInitiated) return L"";

	std::wstring out = L"";
	int hours = 0, minutes = 0, seconds;

	if (this->d_length >= 3600) { // if longer than or equal to 1 hour
		hours = this->d_length / 3600; // no rest, only full hours

		if (hours < 10) {
			out += L"0";
		}
		out += std::to_wstring(hours) + L":";
	}

	if (this->d_length >= 60) { // if longer than or equal to 1 minute
		minutes = (this->d_length - hours * 3600) / 60; //  no rest, only full remaining minutes

		if (minutes < 10) {
			out += L"0";
		}
		out += std::to_wstring(minutes) + L":";
	}

	seconds = this->d_length - hours * 3600 - minutes * 60;
	if (seconds < 10) {
		out += L"0";
	}
	out += std::to_wstring(seconds);

	return out;
}

/**
* Method allowing to read length of an audio file
*
* @return Bitrate in kbps or -1 if file is not audio file
*/
int MediaLibCleaner::File::GetLength() {
	if (this->isInitiated)
		return this->d_length;
	return -1;
}





// PATH INFO
/**
* Method returns full name of directory the file resides in
*
* @return Directory name containing file
*/
std::wstring MediaLibCleaner::File::GetDirectory() {
	return this->d_directory;
}
/**
* Method returns extension of the file
*
* @return File extension
*/
std::wstring MediaLibCleaner::File::GetExt() {
	return this->d_ext;
}
/**
* Method returns name of the file without extension
*
* @return Filename without extension
*/
std::wstring MediaLibCleaner::File::GetFilename() {
	return this->d_filename;
}
/**
* Method returns filename with the extension
*
* @return Filename with extensions
*/
std::wstring MediaLibCleaner::File::GetFilenameExt() {
	return (this->d_filename + L"." + this->d_ext);
}
/**
* Method returns path to directory that contains the file
*
* @return Path to directory containing file
*/
std::wstring MediaLibCleaner::File::GetFolderPath() {
	return this->d_folderpath;
}
/**
* Method returns name of parent directory for %_directory% dir
*
* @return Name of parent dir for %_directory% dir
*/
std::wstring MediaLibCleaner::File::GetParentDir() {
	return this->d_parent_dir;
}
/**
* Method returns full path to audio file given object represents
*
* @return Full path to audio file
*/
std::wstring MediaLibCleaner::File::GetPath() {
	return this->d_path;
}

#ifdef WIN32
	/**
	* Method returns volume letter (Windows only)
	*
	* @return Letter followed by colon of volume the file resides on
	*/
	std::wstring MediaLibCleaner::File::GetVolume() {
		return this->d_volume;
	}
#endif



// FILE PROPERITES
/**
* Method returns file created date in ISO 8601 format
*
* @return File created date in ISO 8601 format
*/
std::wstring MediaLibCleaner::File::GetFileCreateDate() {
	return get_date_iso_8601_wide(this->d_file_create_datetime_raw);
}
/**
* Method returns file created date in RFC 2822 format
*
* @return File created date in RFC 2822 format
*/
std::wstring MediaLibCleaner::File::GetFileCreateDatetime() {
	return get_date_rfc_2822_wide(this->d_file_create_datetime_raw);
}
/**
* Method returns file created date in unix timestamp format
*
* @return File created date in unix timestamp format
*/
time_t MediaLibCleaner::File::GetFileCreateDatetimeRaw() {
	return this->d_file_create_datetime_raw;
}
/**
* Method returns file modified date in ISO 8601 format
*
* @return File modified date in ISO 8601 format
*/
std::wstring MediaLibCleaner::File::GetFileModDate() {
	return get_date_iso_8601_wide(this->d_file_mod_datetime_raw);
}
/**
* Method returns file modified date in RFC 2822 format
*
* @return File modified date in RFC 2822 format
*/
std::wstring MediaLibCleaner::File::GetFileModDatetime() {
	return get_date_rfc_2822_wide(this->d_file_mod_datetime_raw);
}
/**
* Method returns file modified date in unix timestamp format
*
* @return File modified date in unix timestamp format
*/
time_t MediaLibCleaner::File::GetFileModDatetimeRaw() {
	return this->d_file_mod_datetime_raw;
}
/**
* Method returns file size in human readable format
*
* @return File size in human readable format
*/
std::wstring MediaLibCleaner::File::GetFileSize() {
	float temp = this->d_file_size_bytes / 1048576; // MB

	if (this->d_file_size_bytes <= 1023) { // B
		return std::to_wstring(this->d_file_size_bytes) + L"B";
	}
	else if (this->d_file_size_bytes > 1023 && this->d_file_size_bytes <= 1048575) { // KB
		return this->GetFileSizeKB();
	}
	else if (this->d_file_size_bytes > 1048575 && temp < 1024) { // MB
		return this->GetFileSizeMB();
	}
	else { // GB
		return std::to_wstring(temp / 1024);
	}
}
/**
* Method returns file size in bytes
*
* @return File size in bytes
*/
size_t MediaLibCleaner::File::GetFileSizeBytes() {
	return this->d_file_size_bytes;
}
/**
* Method returns file size in kilo bytes
*
* @return File size in kilo bytes
*/
std::wstring MediaLibCleaner::File::GetFileSizeKB() {
	return std::to_wstring(this->d_file_size_bytes / 1024) + L"KB";
}
/**
* Method returns file size in mega bytes
*
* @return File size in mega bytes
*/
std::wstring MediaLibCleaner::File::GetFileSizeMB() {
	return std::to_wstring(this->d_file_size_bytes / 1048576) + L"MB";
}


/**
 * Method checks if file has given tag
 *
 * @param[in] tag Tag name, without % signs!
 * @param[in] val (Optional) value of the tag
 *
 * @return True if tag is present (and has given value), false otherwise
 */
bool MediaLibCleaner::File::HasTag(std::wstring tag, std::wstring val)
{
	// debug
	std::wcout << L"Checking for tag: '" + tag + L"' with possible value: '" + val + L"'" << std::endl;
	return true;
}

/**
* Method checks if file has given tag
*
* @param[in] nname New file name
*
* @return Status of renaming operation
*/
bool MediaLibCleaner::File::Rename(std::wstring nname)
{
	// debug
	std::wcout << L"Renaming file to: '" << nname << L"'" << std::endl;
	return true;
}

/**
* Method checks if file has given tag
*
* @param[in] nloc New file location
*
* @return Status of move operation
*/
bool MediaLibCleaner::File::Move(std::wstring nloc)
{
	// debug
	std::wcout << L"Moving file to: '" << nloc << L"'" << std::endl;
	return true;
}

/**
* Method checks if file has given tag
*
* @return Status of delete operation
*/
bool MediaLibCleaner::File::Delete()
{
	// debug
	std::wcout << L"Deleting file" << std::endl;
	return true;
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
	return this->d_dfc;
}









MediaLibCleaner::FilesAggregator::FilesAggregator() {}
MediaLibCleaner::FilesAggregator::~FilesAggregator() {
	std::list<MediaLibCleaner::File*>::iterator nd = this->end();
	for (std::list<MediaLibCleaner::File*>::iterator it = this->begin(); it != nd; it++) {
		delete *it;
	}
}

void MediaLibCleaner::FilesAggregator::AddFile(MediaLibCleaner::File *_file) {
	this->d_files.push_back(_file);
}

MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::GetFile(std::wstring filepath) {
	std::list<MediaLibCleaner::File*>::iterator nd = this->end();
	for (std::list<MediaLibCleaner::File*>::iterator it = this->begin(); it != nd; it++) {
		if ((*it)->GetPath() == filepath) {
			return *it;
		}
	}
	return nullptr;
}

MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::CurrentFile() {
	std::list<MediaLibCleaner::File*>::iterator it = this->begin();
	std::advance(it, this->cfile);

	return *it;
}

std::list<MediaLibCleaner::File*>::iterator MediaLibCleaner::FilesAggregator::begin() {
	return this->d_files.begin();
}

std::list<MediaLibCleaner::File*>::iterator MediaLibCleaner::FilesAggregator::end() {
	return this->d_files.end();
}

MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::next() {
	if (this->cfile == this->d_files.size() - 1) return nullptr;

	this->cfile++;

	std::list<MediaLibCleaner::File*>::iterator it = this->begin();
	std::advance(it, this->cfile);

	return *it;
}

MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::rewind()
{
	this->cfile = 0;

	std::list<MediaLibCleaner::File*>::iterator it = this->begin();
	std::advance(it, this->cfile);

	return *it;
}





MediaLibCleaner::DFC::DFC(std::string path) {
	this->path = path;
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