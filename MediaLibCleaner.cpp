/**
 * @file
 * @author Szymon Oracki <szymon.oracki@oustish.pl>
 * @version 0.4
 *
 * This file contains definitions of all methods for MediaLibCleaner namespace
 */

#include "MediaLibCleaner.hpp"



/**
 * Constructor for MediaLibCleaner::File class.
 * 
 * File class constructor creaties instance of TagLib::FileRef object (or simillar) and reads all common tags from the file.
 * @param[in] path        Path to audio file this instance will represent
 * @param[in] dfc	      An instance of MediaLibCleaner::DFC
 * @param[in] logprogram  std::unique_ptr to MediaLibCleaner::LogProgram object for logging purposses
 * @param[in] logalert    std::unique_ptr to MediaLibCleaner::LogAlert object for logging purposses
 */
MediaLibCleaner::File::File(std::wstring path, MediaLibCleaner::DFC* dfc, std::unique_ptr<MediaLibCleaner::LogProgram>* logprogram, std::unique_ptr<MediaLibCleaner::LogAlert>* logalert)
{
	this->d_path = path;
	this->d_dfc = dfc;
	this->logalert = logalert;
	this->logprogram = logprogram;

	(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Beginning: " + path, 3);

	// check if file exists
	if (!boost::filesystem::exists(path)) {
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"File does not exists!", 1);
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
	(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading path informations", 3);
	try {
		this->d_directory = fileParentDir.filename().wstring();
		this->d_ext = temp.extension().wstring();					if (this->d_ext.length() > 1) { this->d_ext = this->d_ext.substr(1); }
		this->d_filename = temp.filename().wstring();				this->d_filename = this->d_filename.substr(0, this->d_filename.length() - this->d_ext.length() - 1);
		this->d_folderpath = fileParentDir.wstring();
		this->d_parent_dir = pDirParentDir.filename().wstring();
		// _path is setted before - required
#ifdef WIN32
		this->d_volume = temp.root_name().wstring();
#endif
	}
	catch (...)
	{
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"File path info reading failed", 2);
	}

	// STAT INIT FOR DATE INFORMATION
	// WARNING: MAY ONLY WORK IN WINDOWS!!!

	struct stat attrib;
	stat(ws2s(this->d_path).c_str(), &attrib);

	// FILE PROPERTIES
	(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading file properties", 3);
	try {
		this->d_file_create_datetime_raw = attrib.st_ctime;
		this->d_file_mod_datetime_raw = attrib.st_mtime;
		this->d_file_size_bytes = static_cast<size_t>(fs::file_size(temp));
	}
	catch (const boost::filesystem::filesystem_error& e)
	{
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"File properities reading failed: " + s2ws(e.code().message()), 2);
	}

	// check if file is in fact audio file (as it sometimes cannot be!)
	// effect: this->isInitalized == false, but rest info (about files) is present
	if (this->fileref->isNull()) return;

	this->d_dfc->IncCount();
	
	// SONG INFO
	(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading basic song tags", 3);
	this->artist = this->fileref->tag()->artist();
	this->title = this->fileref->tag()->title();
	this->album = this->fileref->tag()->album();
	this->genre = this->fileref->tag()->genre();
	this->comment = this->fileref->tag()->comment();
	this->track = this->fileref->tag()->track();
	this->year = this->fileref->tag()->year();
	// rest of aliases defined below

	// TECHNICAL INFO
	(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading technical file info", 3);
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
	(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Checking file type and creating appropirate objects", 3);
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
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Is MP3 file", 3);
		TagLib::ID3v2::Tag *id3v2tag = this->taglib_file_mp3->ID3v2Tag();
		TagLib::APE::Tag *apetag = this->taglib_file_mp3->APETag();

		if (this->taglib_file_mp3->hasID3v2Tag()) {
			(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading ID3v2 tags", 3);
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
			(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading APE tags", 3);
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
			(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Has ID3v1 tags", 3);
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
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Is OGG file", 3);
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
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Is FLAC file", 3);
		TagLib::ID3v2::Tag *id3v2tag = this->taglib_file_flac->ID3v2Tag();
		TagLib::Ogg::XiphComment *xiphcomment = this->taglib_file_flac->xiphComment();

		if (this->taglib_file_flac->hasXiphComment()) {
			(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading Xips comments", 3);
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
			(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Reading ID3v2 tags", 3);
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
			(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Has ID3v1 tags", 3);
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
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Is MP$/M4A file", 3);
		TagLib::MP4::ItemListMap taglist = this->taglib_file_m4a->tag()->itemListMap();
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"First part of tags is being read", 3);
		for (auto it = taglist.begin(); it != taglist.end(); ++it)
		{
			// debug
			std::wstring temp = it->second.toStringList().toString(", ").toWString();

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
		(*this->logprogram)->Log(L"MediaLibCleaner::File(" + path + L")", L"Second part of tags is being read", 3);
		for (auto it = tags.begin(); it != tags.end(); it++) {
			//debug
			std::wstring temp = it->second.toString().toWString();

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

/**
 * Deconstructor for MediaLibCleaner::File class.
 */
MediaLibCleaner::File::~File() {
	(*this->logprogram)->Log(L"MediaLibCleaner::File", L"Calling destructor: " + this->d_path, 3);
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




/**
* Method allowing for easy tags setting
*
* @param[in] id3tag   ID3v2 frame name of given tag
* @param[in] xiphtag  XiphComment tag name
* @param[in] apetag   APEv2 tag name
* @param[in] mp4tag   M4A/MP4 tag name
* @param[in] value    New value of given tag
*
* @return Status of changing tag value operation
*/
bool MediaLibCleaner::File::setTagUniversal(std::string id3tag, std::string xiphtag, std::string apetag, std::string mp4tag, TagLib::String value)
{
	if (this->isInitiated)
	{
		// change value in file
		if (this->d_ext == L"mp3")
		{
			if (this->taglib_file_mp3->hasID3v2Tag())
			{
				TagLib::ID3v2::Tag *tag = this->taglib_file_mp3->ID3v2Tag(true);
				TagLib::ByteVector handle = id3tag.c_str();

				if (value == TagLib::String::null)
				{
					tag->removeFrames(handle);
				}
				else
				{
					if (value.length() > 1 && value.substr(0, 1) == "T")
					{
						if (!tag->frameList(handle).isEmpty())
						{
							tag->frameList(handle).front()->setText(value);
						}
						else
						{
							TagLib::ID3v2::TextIdentificationFrame *frame =
								new TagLib::ID3v2::TextIdentificationFrame(handle, TagLib::String::UTF8);
							tag->addFrame(frame);
							frame->setText(value);
						}
					}
					else if (value.length() > 1 && value.substr(0, 1) == "W")
					{
						if (value == "WXXX[WWW]")
						{
							if (!tag->frameList(handle).isEmpty())
							{
								tag->frameList(handle).front()->setText(value);
							}
							else
							{
								TagLib::ID3v2::UserUrlLinkFrame *frame = new TagLib::ID3v2::UserUrlLinkFrame(TagLib::String::UTF8);
								frame->setDescription("WWW");
								frame->setText(value);
								tag->addFrame(frame);
							}
						}
						else
							return false;
					}
					else if (value.length() >= 4 && value.substr(0, 4) == "USLT")
					{
						if (!tag->frameList(handle).isEmpty())
						{
							tag->frameList(handle).front()->setText(value);
						}
						else
						{
							TagLib::ID3v2::UnsynchronizedLyricsFrame *frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame(TagLib::String::UTF8);
							frame->setText(value);
							frame->setDescription("en");
							frame->setLanguage("eng");
							tag->addFrame(frame);
						}
					}
					else
						return false;
				}

				this->taglib_file_mp3->save();
			}

			if (this->taglib_file_mp3->hasAPETag())
			{
				TagLib::APE::Tag *tag = this->taglib_file_mp3->APETag(true);
				std::string handle = apetag;

				if (value == TagLib::String::null)
				{
					tag->removeItem(handle);
				}
				else
				{
					TagLib::APE::Item *item = new TagLib::APE::Item(handle, value);
					tag->setItem(handle, *item);
				}

				this->taglib_file_mp3->save();
			}
		}
		else if (this->d_ext == L"ogg")
		{
			TagLib::Ogg::XiphComment *tag = this->taglib_file_ogg->tag();
			std::string handle = xiphtag;
			if (value == TagLib::String::null)
			{
				tag->removeField(handle);
			}
			else
			{
				tag->addField(handle, value, true);
			}
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			if (this->taglib_file_flac->hasID3v2Tag())
			{
				TagLib::ID3v2::Tag *tag = this->taglib_file_flac->ID3v2Tag(true);
				TagLib::ByteVector handle = id3tag.c_str();

				if (value == TagLib::String::null)
				{
					tag->removeFrames(handle);
				}
				else
				{
					if (value.length() > 1 && value.substr(0, 1) == "T")
					{
						if (!tag->frameList(handle).isEmpty())
						{
							tag->frameList(handle).front()->setText(value);
						}
						else
						{
							TagLib::ID3v2::TextIdentificationFrame *frame =
								new TagLib::ID3v2::TextIdentificationFrame(handle, TagLib::String::UTF8);
							tag->addFrame(frame);
							frame->setText(value);
						}
					}
					else if (value.length() > 1 && value.substr(0, 1) == "W")
					{
						if (value == "WXXX[WWW]")
						{
							if (!tag->frameList(handle).isEmpty())
							{
								tag->frameList(handle).front()->setText(value);
							}
							else
							{
								TagLib::ID3v2::UserUrlLinkFrame *frame = new TagLib::ID3v2::UserUrlLinkFrame(TagLib::String::UTF8);
								frame->setDescription("WWW");
								frame->setText(value);
								tag->addFrame(frame);
							}
						}
						else
							return false;
					}
					else if (value.length() >= 4 && value.substr(0, 4) == "USLT")
					{
						if (!tag->frameList(handle).isEmpty())
						{
							tag->frameList(handle).front()->setText(value);
						}
						else
						{
							TagLib::ID3v2::UnsynchronizedLyricsFrame *frame = new TagLib::ID3v2::UnsynchronizedLyricsFrame(TagLib::String::UTF8);
							frame->setText(value);
							frame->setDescription("en");
							frame->setLanguage("eng");
							tag->addFrame(frame);
						}
					}
					else
						return false;
				}
			}

			if (this->taglib_file_flac->hasXiphComment())
			{
				TagLib::Ogg::XiphComment *tag = this->taglib_file_flac->xiphComment(true);
				std::string handle = xiphtag;
				if (value == TagLib::String::null)
				{
					tag->removeField(handle);
				}
				else
				{
					tag->addField(handle, value, true);
				}
			}

			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			std::string handle = mp4tag;
			TagLib::MP4::Tag *tag = this->taglib_file_m4a->tag();
			TagLib::MP4::ItemListMap items = tag->itemListMap();

			TagLib::StringList *values = new TagLib::StringList(value);
			TagLib::MP4::Item *newitem = new TagLib::MP4::Item(values);

			TagLib::MP4::Item *emptyitem = new TagLib::MP4::Item();

			if (value == TagLib::String::null)
			{
				if (items.contains(handle))
				{
					items[handle] = emptyitem;
				}
			}
			else
			{
				if (items.contains(handle))
				{
					items[handle] = newitem;
				}
				else
				{
					items.insert(handle, newitem);
				}
			}

			tag->save();
		}
		return true;
	}
	return false;
}






/**
* Method allowing to set \%artist% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetArtist(TagLib::String value)
{
	if (this->isInitiated)
	{
		// change inner value
		this->artist = value;

		// change value in file
		if (this->d_ext == L"mp3")
		{
			this->taglib_file_mp3->tag()->setArtist(value);
			this->taglib_file_mp3->save();
		}
		else if (this->d_ext == L"ogg")
		{
			this->taglib_file_ogg->tag()->setArtist(value);
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			this->taglib_file_flac->tag()->setArtist(value);
			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			this->taglib_file_m4a->tag()->setArtist(value);
			this->taglib_file_m4a->save();
		}
		return true;
	}
	return false;
}

/**
* Method allowing to set \%title% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetTitle(TagLib::String value) {
	if (this->isInitiated)
	{
		// change inner value
		this->title = value;

		// change value in file
		if (this->d_ext == L"mp3")
		{
			this->taglib_file_mp3->tag()->setTitle(value);
			this->taglib_file_mp3->save();
		}
		else if (this->d_ext == L"ogg")
		{
			this->taglib_file_ogg->tag()->setTitle(value);
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			this->taglib_file_flac->tag()->setTitle(value);
			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			this->taglib_file_m4a->tag()->setTitle(value);
			this->taglib_file_m4a->save();
		}
		return true;
	}
	return false;
}

/**
* Method allowing to set \%album% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetAlbum(TagLib::String value) {
	if (this->isInitiated)
	{
		// change inner value
		this->album = value;

		// change value in file
		if (this->d_ext == L"mp3")
		{
			this->taglib_file_mp3->tag()->setAlbum(value);
			this->taglib_file_mp3->save();
		}
		else if (this->d_ext == L"ogg")
		{
			this->taglib_file_ogg->tag()->setAlbum(value);
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			this->taglib_file_flac->tag()->setAlbum(value);
			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			this->taglib_file_m4a->tag()->setAlbum(value);
			this->taglib_file_m4a->save();
		}
		return true;
	}
	return false;
}

/**
* Method allowing to set \%genre% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetGenre(TagLib::String value) {
	if (this->isInitiated)
	{
		// change inner value
		this->genre = value;

		// change value in file
		if (this->d_ext == L"mp3")
		{
			this->taglib_file_mp3->tag()->setGenre(value);
			this->taglib_file_mp3->save();
		}
		else if (this->d_ext == L"ogg")
		{
			this->taglib_file_ogg->tag()->setGenre(value);
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			this->taglib_file_flac->tag()->setGenre(value);
			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			this->taglib_file_m4a->tag()->setGenre(value);
			this->taglib_file_m4a->save();
		}
		return true;
	}
	return false;
}

/**
* Method allowing to set \%comment% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetComment(TagLib::String value) {
	if (this->isInitiated)
	{
		// change inner value
		this->comment = value;

		// change value in file
		if (this->d_ext == L"mp3")
		{
			this->taglib_file_mp3->tag()->setComment(value);
			this->taglib_file_mp3->save();
		}
		else if (this->d_ext == L"ogg")
		{
			this->taglib_file_ogg->tag()->setComment(value);
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			this->taglib_file_flac->tag()->setComment(value);
			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			this->taglib_file_m4a->tag()->setComment(value);
			this->taglib_file_m4a->save();
		}
		return true;
	}
	return false;
}

/**
* Method allowing to set \%track% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetTrack(TagLib::uint value) {
	if (this->isInitiated)
	{
		// change inner value
		this->track = value;

		// change value in file
		if (this->d_ext == L"mp3")
		{
			this->taglib_file_mp3->tag()->setTrack(value);
			this->taglib_file_mp3->save();
		}
		else if (this->d_ext == L"ogg")
		{
			this->taglib_file_ogg->tag()->setTrack(value);
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			this->taglib_file_flac->tag()->setTrack(value);
			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			this->taglib_file_m4a->tag()->setTrack(value);
			this->taglib_file_m4a->save();
		}
		return true;
	}
	return false;
}

/**
* Method allowing to set \%year% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetYear(TagLib::uint value) {
	if (this->isInitiated)
	{
		// change inner value
		this->year = value;

		// change value in file
		if (this->d_ext == L"mp3")
		{
			this->taglib_file_mp3->tag()->setYear(value);
			this->taglib_file_mp3->save();
		}
		else if (this->d_ext == L"ogg")
		{
			this->taglib_file_ogg->tag()->setYear(value);
			this->taglib_file_ogg->save();
		}
		else if (this->d_ext == L"flac")
		{
			this->taglib_file_flac->tag()->setYear(value);
			this->taglib_file_flac->save();
		}
		else if (this->d_ext == L"m4a" || this->d_ext == L"mp4")
		{
			this->taglib_file_m4a->tag()->setYear(value);
			this->taglib_file_m4a->save();
		}
		return true;
	}
	return false;
}

/**
* Method allowing to set \%albumartist% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetAlbumArtist(TagLib::String value)
{
	this->albumartist = value;
	return this->setTagUniversal("TPE2", "ALBUMARTIST", "ALBUMARTIST", "aART", value);
}

/**
* Method allowing to set \%bpm% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetBPM(TagLib::String value) {
	this->bpm = value;
	return this->setTagUniversal("TBPM", "BPM", "BPM", "tmpo", value);
}

/**
* Method allowing to set \%copyright% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetCopyright(TagLib::String value) {
	this->copyright = value;
	return this->setTagUniversal("TCOP", "COPYRIGHT", "COPYRIGHT", "cprt", value);
}

/**
* Method allowing to set \%language% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetLanguage(TagLib::String value) {
	this->language = value;
	return this->setTagUniversal("TLAN", "LANGUAGE", "LANGUAGE", "----:com.apple.iTunes:LANGUAGE", value);
}

/**
* Method allowing to set \%length% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetTagLength(TagLib::String value) {
	this->length = value;
	return this->setTagUniversal("TLEN", "LENGTH", "LENGTH", "----:com.apple.iTunes:LENGTH ", value);
}

/**
* Method allowing to set \%mood% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetMood(TagLib::String value) {
	this->mood = value;
	return this->setTagUniversal("TMOO", "MOOD", "MOOD", "----:com.apple.iTunes:MOOD", value);
}

/**
* Method allowing to set \%origalbum% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetOrigAlbum(TagLib::String value) {
	this->origalbum = value;
	return this->setTagUniversal("TOAL", "ORIGALBUM", "ORIGALBUM", "----:com.apple.iTunes:ORIGALBUM", value);
}

/**
* Method allowing to set \%origartist% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetOrigArtist(TagLib::String value) {
	this->origartist = value;
	return this->setTagUniversal("TOPE", "ORIGARTIST", "ORIGARTIST", "----:com.apple.iTunes:ORIGARTIST", value);
}

/**
* Method allowing to set \%origfilename% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetOrigFilename(TagLib::String value) {
	this->origfilename = value;
	return this->setTagUniversal("TOFN", "ORIGFILENAME", "ORIGFILENAME", "----:com.apple.iTunes:ORIGFILENAME", value);
}

/**
* Method allowing to set \%origyear% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetOrigYear(TagLib::String value) {
	this->origyear = value;
	return this->setTagUniversal("TDOR", "ORIGYEAR", "ORIGYEAR", "----:com.apple.iTunes:ORIGYEAR", value);
}

/**
* Method allowing to set \%publisher% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetPublisher(TagLib::String value) {
	this->publisher = value;
	return this->setTagUniversal("TPUB", "ORGANIZATION", "PUBLISHER", "----:com.apple.iTunes:PUBLISHER ", value);
}

/**
* Method allowing to set \%unsyncedlyrics% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetLyricsUnsynced(TagLib::String value) {
	this->unsyncedlyrics = value;
	return this->setTagUniversal("USLT", "UNSYNCEDLYRICS", "UNSYNCEDLYRICS", "©lyr", value);
}

/**
* Method allowing to set \%www% tag from an audio file
*
* @param[in] value  New value of the tag
*
* @return Information if setting tag has completed successfully
*/
bool MediaLibCleaner::File::SetWWW(TagLib::String value) {
	this->www = value;
	return this->setTagUniversal("WXXX[WWW]", "WWW", "WWW", "----:com.apple.iTunes:WWW", value);
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
	float temp = static_cast<float>(this->d_file_size_bytes) / 1048576; // MB

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
bool MediaLibCleaner::File::HasTag(std::wstring tag, TagLib::String val)
{
	// debug
	(*this->logalert)->Log(L"File::HasTag(" + this->d_path + L")", L"Cheking for tag: '" + tag + L"' with possible value: '" + val.toWString() + L"'");
	return true;
}

/**
* Method for renaming file
*
* @param[in] nname New file name
*
* @return Status of renaming operation
*/
bool MediaLibCleaner::File::Rename(std::wstring nname)
{
	// debug
	(*this->logalert)->Log(L"File::Rename(" + this->d_path + L")", L"Reanming file to: '" + nname + L"'");
	return true;
}

/**
* Method for moving file to new destination in the user filesystem
*
* @param[in] nloc New file location
*
* @return Status of move operation
*/
bool MediaLibCleaner::File::Move(std::wstring nloc)
{
	// debug
	(*this->logalert)->Log(L"File::Move(" + this->d_path + L")", L"Moving file to: '" + nloc + L"'");
	return true;
}

/**
* Method for deleting given file
*
* @return Status of delete operation
*/
bool MediaLibCleaner::File::Delete()
{
	// debug
	(*this->logalert)->Log(L"File::Delete(" + this->d_path + L")", L"Deleting file");
	return true;
}

/**
 * Method for easy setting tag in the audio file using already existing methods.
 * This method is for TagLib::String type of value variable
 *
 * @param[in] key  Tag name (wthout \% signs)
 * @param[in] val  New tag value
 *
 * @return True if tag setting operation succeded, false otherwise
 */
bool MediaLibCleaner::File::SetTag(std::wstring key, TagLib::String val)
{
	if (key == L"artist")
	{
		return this->SetArtist(val);
	}
	else if (key == L"title")
	{
		return this->SetTitle(val);
	}
	else if (key == L"album")
	{
		return this->SetAlbum(val);
	}
	else if (key == L"genre")
	{
		return this->SetGenre(val);
	}
	else if (key == L"comment")
	{
		return this->SetComment(val);
	}
	else if (key == L"albumartist")
	{
		return this->SetAlbumArtist(val);
	}
	else if (key == L"bpm")
	{
		return this->SetBPM(val);
	}
	else if (key == L"copyright")
	{
		return this->SetCopyright(val);
	}
	else if (key == L"language")
	{
		return this->SetLanguage(val);
	}
	else if (key == L"length")
	{
		return this->SetTagLength(val);
	}
	else if (key == L"mood")
	{
		return this->SetMood(val);
	}
	else if (key == L"origalbum")
	{
		return this->SetOrigAlbum(val);
	}
	else if (key == L"origartist")
	{
		return this->SetOrigArtist(val);
	}
	else if (key == L"origfilename")
	{
		return this->SetOrigFilename(val);
	}
	else if (key == L"origyear")
	{
		return this->SetOrigYear(val);
	}
	else if (key == L"publisher")
	{
		return this->SetPublisher(val);
	}
	else if (key == L"unsyncedlyrics")
	{
		return this->SetLyricsUnsynced(val);
	}
	else if (key == L"www")
	{
		return this->SetWWW(val);
	}
	return false;
}

/**
* Method for easy setting tag in the audio file using already existing methods.
* This version is for TagLib::uint type of value variable
*
* @param[in] key  Tag name (wthout \% signs)
* @param[in] val  New tag value
*
* @return True if tag setting operation succeded, false otherwise
*/
bool MediaLibCleaner::File::SetTag(std::wstring key, TagLib::uint val)
{

	if (key == L"track")
	{
		return this->SetTrack(val);
	}
	else if (key == L"year")
	{
		return this->SetYear(val);
	}
	return false;
}

/**
* Method allowing to read status of given object
*
* @return Object status
*/
bool MediaLibCleaner::File::IsInitiated() {
	return this->isInitiated;
}

/**
 * Method returning MediaLibCleaner::DFC object of the current file
 *
 * @return DFC for given file.
 */
MediaLibCleaner::DFC* MediaLibCleaner::File::GetDFC() {
	return this->d_dfc;
}








/**
 * MediaLibCleaner::FilesAggreagator constructor.
 *
 * @param[in] logprogram  std::unique_ptr to MediaLibCleaner::LogProgram object for logging purposes
 * @param[in] logalert    std::unique_ptr to MediaLibCleaner::LogAlert object for logging purposes
 */
MediaLibCleaner::FilesAggregator::FilesAggregator(std::unique_ptr<MediaLibCleaner::LogProgram>* logprogram, std::unique_ptr<MediaLibCleaner::LogAlert>* logalert)
{
	this->logprogram = logprogram;
	this->logalert = logalert;

	(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator", L"Creating object", 3);
}

/**
 * MediaLibCleaner::FilesAggregator class destructor
 */
MediaLibCleaner::FilesAggregator::~FilesAggregator() {
	(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator", L"Calling destructor", 3);

	auto nd = this->end();
	for (auto it = this->begin(); it != nd; it++)
		delete *it;
}

/**
 * Method for adding MediaLibCleaner::File objects to FilesAggregator object. Uses mutex to prevent racing conditions.
 *
 * @param[in] file  New MediaLibCleaner::File object to be added
 */
void MediaLibCleaner::FilesAggregator::AddFile(MediaLibCleaner::File *file) {
	this->add_synch.lock();

	(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator::AddFile", L"Adding file", 3);
	this->d_files.push_back(file);

	this->add_synch.unlock();
}

/**
 * Method for getting MediaLibCleaner::File object provided the object with given filepath exists in the FilesAggregator object. Uses mutex to prevent racing conditions.
 *
 * @param[in] filepath  Path of the MediaLibCleaner::File object to be returned
 *
 * @return Pointer to MediaLibCleaner::File object which path is identical to given filepath parameter
 */
MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::GetFile(std::wstring filepath) {
	this->get_synch.lock();

	(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator::GetFile", L"Searching for File object...", 3);
	auto nd = this->end();
	for (auto it = this->begin(); it != nd; it++) {
		if ((*it)->GetPath() == filepath) {
			(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator::GetFile", L"...successful", 3);
			this->get_synch.unlock();
			return *it;
		}
	}
	(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator::GetFile", L"...unsuccessful", 3);
	this->get_synch.unlock();
	return nullptr;
}

/**
 * Method to get current MediaLibCleaner::File object selected in the FilesAggregator object.
 * Usable only in single-thread environments (racing conditions!)
 *
 * @return Currently selected MediaLibCleaner::File object stored inside FilesAggregator object
 */
MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::CurrentFile() {
	std::list<MediaLibCleaner::File*>::iterator it = this->begin();
	std::advance(it, this->cfile);

	return *it;
}

/**
 * Method to retrieve iterator pointing to the beginning of the MediaLibCleaner::File list
 *
 * @return Iterator pointing to the beginning of std::list containing MediaLibCleaner::File class objects
 */
std::list<MediaLibCleaner::File*>::iterator MediaLibCleaner::FilesAggregator::begin() {
	return this->d_files.begin();
}

/**
* Method to retrieve iterator pointing to the ending of the MediaLibCleaner::File list
*
* @return Iterator pointing to the ending of std::list containing MediaLibCleaner::File class objects
*/
std::list<MediaLibCleaner::File*>::iterator MediaLibCleaner::FilesAggregator::end() {
	return this->d_files.end();
}

/**
* Method to advance and retrieve next MediaLibCleaner::File object in the list
*
* @return Pointer to next MediaLibCleaner::File object
*/
MediaLibCleaner::File* MediaLibCleaner::FilesAggregator::next() {
	if (this->cfile == this->d_files.size() - 1) return nullptr;

	this->next_synch.lock();

	this->cfile++;

	auto it = this->begin();
	std::advance(it, this->cfile);

	(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator::next", L"Selected next element, returning it", 3);

	this->next_synch.unlock();

	return *it;
}

/**
* Method to return to the beginning of the std::list containing MediaLibCleaner::File class objects
*/
void MediaLibCleaner::FilesAggregator::rewind()
{
	this->get_synch.lock();
	this->next_synch.lock();
	this->rewind_synch.lock();

	this->cfile = -1;

	(*this->logprogram)->Log(L"MediaLibCleaner::FilesAggregator::rewind", L"Rewind completed", 3);

	this->rewind_synch.unlock();
	this->next_synch.unlock();
	this->get_synch.unlock();
}





/**
* MediaLibCleaner::PathsAggreagator constructor.
*
* @param[in] logprogram  std::unique_ptr to MediaLibCleaner::LogProgram object for logging purposes
* @param[in] logalert    std::unique_ptr to MediaLibCleaner::LogAlert object for logging purposes
*/
MediaLibCleaner::PathsAggregator::PathsAggregator(std::unique_ptr<MediaLibCleaner::LogProgram>* logprogram, std::unique_ptr<MediaLibCleaner::LogAlert>* logalert)
{
	this->logprogram = logprogram;
	this->logalert = logalert;

	(*this->logprogram)->Log(L"MediaLibCleaner::PathsAggregator", L"Creating object", 3);
}

/**
 * MediaLibCleaner::PathsAggregator destructor
 */
MediaLibCleaner::PathsAggregator::~PathsAggregator() {
	(*this->logprogram)->Log(L"MediaLibCleaner::PathsAggregator", L"Calling destructor", 3);
}

/**
 * Method for adding new path to the list
 *
 * @param[in] path  New path to be added to the list
 */
void MediaLibCleaner::PathsAggregator::AddPath(boost::filesystem::path path) {
	this->add_synch.lock();

	(*this->logprogram)->Log(L"MediaLibCleaner::PathsAggregator::AddPath", L"Adding file", 3);
	this->d_files.push_back(path);

	this->add_synch.unlock();
}

/**
 * Method to retrieve currently selected path. Usable in single-thread environments (racing conditions!)
 *
 * @return boost::filesystem::path object containing currently selected path
 */
boost::filesystem::path MediaLibCleaner::PathsAggregator::CurrentPath() {
	std::list<boost::filesystem::path>::iterator it = this->begin();
	std::advance(it, this->cfile);

	return *it;
}

/**
* Method to retrieve iterator pointing to the beginning of the boost::filesystem::path list
*
* @return Iterator pointing to the beginning of std::list containing boost::filesystem::path objects
*/
std::list<boost::filesystem::path>::iterator MediaLibCleaner::PathsAggregator::begin() {
	return this->d_files.begin();
}

/**
* Method to retrieve iterator pointing to the ending of the boost::filesystem::path list
*
* @return Iterator pointing to the ending of std::list containing boost::filesystem::path objects
*/
std::list<boost::filesystem::path>::iterator MediaLibCleaner::PathsAggregator::end() {
	return this->d_files.end();
}

/**
* Method to advance and retrieve next boost::filesystem::path object in the list
*
* @return boost::filesystem::path object containing next path
*/
boost::filesystem::path MediaLibCleaner::PathsAggregator::next() {
	if (this->cfile == this->d_files.size() - 1)
	{
		(*this->logprogram)->Log(L"MediaLibCleaner::PathsAggregator::next", L"Last element reached, returning empty string", 3);
		return "";
	}

	this->next_synch.lock();

	this->cfile++;

	auto it = this->begin();
	std::advance(it, this->cfile);

	(*this->logprogram)->Log(L"MediaLibCleaner::PathsAggregator::next", L"Selected next element, returning it", 3);

	this->next_synch.unlock();

	return *it;
}

/**
* Method to return to the beginning of the std::list containing boost::filesystem::path objects
*/
void MediaLibCleaner::PathsAggregator::rewind()
{
	this->get_synch.lock();
	this->next_synch.lock();
	this->rewind_synch.lock();

	this->cfile = -1;

	(*this->logprogram)->Log(L"MediaLibCleaner::PathsAggregator::rewind", L"Rewind completed", 3);

	this->rewind_synch.unlock();
	this->next_synch.unlock();
	this->get_synch.unlock();
}






/**
 * MediaLibCleaner::DFC constructor
 *
 * @param[in] path        Path for new DFC object
 * @param[in] logprogram  std::unique_ptr to MediaLibCleaner::LogProgram object for logging purposes
 * @param[in] logalert    std::unique_ptr to MediaLibCleaner::LogAlert object for logging purposes
 */
MediaLibCleaner::DFC::DFC(std::wstring path, std::unique_ptr<MediaLibCleaner::LogProgram>* logprogram, std::unique_ptr<MediaLibCleaner::LogAlert>* logalert) {
	this->path = path;
	this->count = 0;
	this->logalert = logalert;
	this->logprogram = logprogram;

	(*this->logprogram)->Log(L"MediaLibCleaner::DFC", L"Created object for: " + this->path, 3);
}

/**
 * MediaLibCleaner::DFC destructor.
 */
MediaLibCleaner::DFC::~DFC()
{
	(*this->logprogram)->Log(L"MediaLibCleaner::DFC", L"Destructor called: " + this->path, 3);
}

/**
 * Method for retrieveing value of the counter inside DFC object.
 * Please note that this value may not be final during scan() operation!
 *
 * @return Amount of files inside given folder the DFC object represents
 */
int MediaLibCleaner::DFC::GetCounter() {
	return this->count;
}

/**
 * Method for retrieving path of the DFC object
 *
 * @return Path of the folder given DFC object represents.
 */
std::wstring MediaLibCleaner::DFC::GetPath() {
	return this->path;
}

/**
 * Method to increase counter inside given DFC object.
 * Using method instead of operator overloading because operator overloading created unwanted results.
 */
void MediaLibCleaner::DFC::IncCount() {
	(*this->logprogram)->Log(L"MediaLibCleaner::DFC::IncCount", L"Incrementing DFC count for " + this->path, 3);
	this->count++;
}




/**
* MediaLibCleaner::LogAlert constructor
*
* @param[in] filename  Path to log file
*/
MediaLibCleaner::LogAlert::LogAlert(std::wstring filename)
{
	if (filename != L"-") {
		this->outputfile.open(filename);
		std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		this->outputfile.imbue(loc);
		this->initCompleted = true;
	}
	else
	{
		this->initCompleted = false;
	}
}

/**
* MediaLibCleaner::LogAlert destructor
*/
MediaLibCleaner::LogAlert::~LogAlert()
{
	this->synch.lock();
	this->synch.unlock();

	this->Close();
	this->initCompleted = false;
}

/**
* Method to close output stream
*/
void MediaLibCleaner::LogAlert::Close()
{
	if (this->IsOpen())
	{
		this->outputfile.flush();
		this->outputfile.close();
	}
}

/**
* Method to flush buffer and save its content to output file
*/
void MediaLibCleaner::LogAlert::Flush()
{
	if (this->IsOpen())
		this->outputfile.flush();
}

/**
* Method to check if output stream is still open
*/
bool MediaLibCleaner::LogAlert::IsOpen()
{
	return (this->initCompleted && this->outputfile.is_open());
}

/**
* Method to write message to the log output stream
*
* @param[in] module   Module name which is logging given message
* @param[in] message  Message to be written into log file
*/
void MediaLibCleaner::LogAlert::Log(std::wstring module, std::wstring message)
{
	if (this->IsOpen())
	{
		this->synch.lock();
		this->outputfile << L"[" << module << L"] " << message << std::endl;
		this->synch.unlock();
	}
	else
	{
		std::wcout << L"[" << module << L"] " << message << std::endl;
	}
}






/**
* MediaLibCleaner::LogProgram constructor
*
* @param[in] filename          Path to log file
* @param[in] init_debug_level  Inital debug level
*/
MediaLibCleaner::LogProgram::LogProgram(std::wstring filename, int init_debug_level)
{
	if (filename != L"-") {
		this->outputfile.open(filename);
		std::locale loc(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
		this->outputfile.imbue(loc);

		this->init_debug_level = init_debug_level;
		this->initCompleted = true;
	}
	else
	{
		this->initCompleted = false;
	}
}

/**
* MediaLibCleaner::LogProgram destructor
*/
MediaLibCleaner::LogProgram::~LogProgram()
{
	this->synch.lock();
	this->synch.unlock();

	this->Close();
	this->initCompleted = false;
}

/**
* Method to close output stream
*/
void MediaLibCleaner::LogProgram::Close()
{
	if (this->IsOpen())
	{
		this->outputfile.flush();
		this->outputfile.close();
	}
}

/**
* Method to flush buffer and save its content to output file
*/
void MediaLibCleaner::LogProgram::Flush()
{
	if (this->IsOpen())
		this->outputfile.flush();
}

/**
* Method to check if output stream is still open
*/
bool MediaLibCleaner::LogProgram::IsOpen()
{
	return (this->initCompleted && this->outputfile.is_open());
}

/**
* Method to write message to the log output stream
*
* @param[in] module       Module name which is logging given message
* @param[in] message      Message to be written into log file
* @param[in] debug_level  Parameter containing info about current message priority level
*/
void MediaLibCleaner::LogProgram::Log(std::wstring module, std::wstring message, int debug_level)
{
	if (this->init_debug_level >= debug_level) {
		this->synch.lock();

		std::wstring code;

		switch (debug_level)
		{
		case 1:
			code = L"ERROR:     ";
			break;
		case 2:
			code = L"WARNING:   ";
			break;
		case 3:
			code = L"DEBUG:     ";
		}

		if (this->IsOpen())
		{
			this->outputfile << code << L"[" << module << L"] " << message << std::endl; 
		}
		else
		{
			std::wcout << code << L"[" << module << L"] " << message << std::endl;
		}
		
		this->synch.unlock();
	}
}

/**
* Method to add or create DFC object (depending on its presence in dfc_list).
* If given path has already assigned DFC object it is returned; if not, new DFC object is created and returned.
* Uses mutex to prevent racing conditions.
*
* @param[in,out] dfc_list  List of DFC objects
* @param[in]     pth       Path to directory
* @param[in]     synch     std::mutex used to prevent racing conditions
* @param[in]     lp        std::unique_ptr to MediaLibCleaner::LogProgram object for logging purposes
* @param[in]     la        std::unique_ptr to MediaLibCleaner::LogAlert object for logging purposes
*/
MediaLibCleaner::DFC* MediaLibCleaner::AddDFC(std::list<MediaLibCleaner::DFC*>* dfc_list, boost::filesystem::path pth, std::mutex* synch,
	std::unique_ptr<MediaLibCleaner::LogProgram>* lp, std::unique_ptr<MediaLibCleaner::LogAlert>* la)
{
	synch->lock();

	(*lp)->Log(L"AddDFC(" + pth.generic_wstring() + L")", L"Creating and/or returning DFC", 3);

	auto end = dfc_list->end();
	for (auto it = dfc_list->begin(); it != end; ++it)
	{
		if ((*it)->GetPath() == pth) {
			synch->unlock();
			(*lp)->Log(L"AddDFC(" + pth.generic_wstring() + L")", L"Returning existing DFC", 3);
			return (*it);
		}
	}
	
	(*lp)->Log(L"AddDFC(" + pth.generic_wstring() + L")", L"Creating new DFC", 3);

	DFC* newdfc = new DFC(pth.generic_wstring(), lp, la);
	dfc_list->push_back(newdfc);

	synch->unlock();
	return newdfc;
}