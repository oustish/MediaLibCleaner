/**
@file
@author Szymon Oracki <szymon.oracki@oustish.pl>
@version 0.4

This file contains definitions of all helper functions
*/
#include "helpers.hpp"

/**
* Function replacing every occurence of one string with another in given string.
*
* This function allows to replace all occurences of one string with another string,
* all within another string.
*
* @param[in,out]	str	  String in which changes will be made. Please note argument is passed as referrence and will contain output!
* @param[in]	from  String which will be replaced with another string
* @param[in]	to	  String which will replace previous string
*/
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

/**
* Function replacing every occurence of one std::wstring with another in given std::wstring.
*
* This function allows to replace all occurences of one std::wstring with another std::wstring,
* all within another std::wstring.
*
* @param[in,out]	str	  std::wstring in which changes will be made. Please note argument is passed as referrence and will contain output!
* @param[in]	from  std::wstring which will be replaced with another string
* @param[in]	to	  std::wstring which will replace previous string
*/
void replaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

/**
* Function processing given timestamp and returning date in ISO-8601 complaint std::string 
*
* @param[in] t  Timestamp to be changed into ISO-8601 complaint date format as std::string
*
* @return ISO-8601 complaint std::string of given timestamp
*/
std::string get_date_iso_8601(time_t t) {
	using namespace boost::posix_time;

	std::string ss;

	char bufor[20];
	tm czas;
	localtime_s(&czas, &t);

	strftime(bufor, 20, "%Y-%m-%dT%H:%M", &czas);
	ss = bufor;


	// +02:00 or other
	typedef boost::date_time::c_local_adjustor<ptime> local_adj;

	const ptime utc_now = second_clock::universal_time();
	const ptime now = local_adj::utc_to_local(utc_now);

	int h = (now - utc_now).hours();
	int m = (now - utc_now).minutes();

	if (h < 0) {
		ss += "-";
	}
	else {
		ss += "+";
	}

	if (h < 10)
		ss += "0";

	ss += std::to_string(std::abs(h));

	ss += ":";

	if (m < 10)
		ss += "0";

	ss += std::to_string(m);

	return ss;
}

/**
* Function processing given timestamp and returning date in RFC2822 complaint std::string
*
* @param[in] t  Timestamp to be changed into RFC2822 complaint date format as std::string
*
* @return RFC2822 complaint std::string of given timestamp
*/
std::string get_date_rfc_2822(time_t t) {
	using namespace boost::posix_time;

	std::string ss;

	char bufor[31];
	struct tm czas;
	localtime_s(&czas, &t);

	strftime(bufor, 31, "%a, %d %b %Y %X", &czas);

	ss = bufor;

	// +0200 ot other
	typedef boost::date_time::c_local_adjustor<ptime> local_adj;

	const ptime utc_now = second_clock::universal_time();
	const ptime now = local_adj::utc_to_local(utc_now);

	int h = (now - utc_now).hours();
	int m = (now - utc_now).minutes();

	if (h < 0) {
		ss += "-";
	}
	else {
		ss += "+";
	}

	if (h < 10)
		ss += "0";

	ss += std::to_string(std::abs(h));

	if (m < 10)
		ss += "0";

	ss += std::to_string(m);

	return ss;
}

/**
* Function processing given timestamp and returning date in ISO-8601 complaint std::string
*
* @param[in] t  Timestamp to be changed into ISO-8601 complaint date format and returned as std::wstring
*
* @return ISO-8601 complaint std::wstring of given timestamp
*/
std::wstring get_date_iso_8601_wide(time_t t) {
	return s2ws(get_date_iso_8601(t));
}

/**
* Function processing given timestamp and returning date in IRFC2822 complaint std::string
*
* @param[in] t  Timestamp to be changed into RFC2822 complaint date format and returned as std::wstring
*
* @return RFC2822 complaint std::wstring of given timestamp
*/
std::wstring get_date_rfc_2822_wide(time_t t) {
	return s2ws(get_date_rfc_2822(t));
}

#ifdef WIN32

/**
* Function to change encoding of given wide char string and convert it to normal std::string
*
* @param[in] win  Input wide string to be converted
*
* @return std::string representation of wide string given as parameter
*/
std::string ws2s(const std::wstring& win)
{
	int len;
	int slength = static_cast<int>(win.length()) + 1;
	len = WideCharToMultiByte(CP_ACP, 0, win.c_str(), slength, 0, 0, 0, 0);
	char* buf = new char[len];
	WideCharToMultiByte(CP_ACP, 0, win.c_str(), slength, buf, len, 0, 0);
	std::string r(buf);
	delete[] buf;
	return r;
}

/**
* Function to change encoding of given multi byte string and convert it to wide string
*
* @param[in] in  Input string to be converted
*
* @return std::wstring representation of normal string given as parameter
*/
std::wstring s2ws(const std::string& in)
{
	int len;
	int slength = static_cast<int>(in.length()) + 1;
	len = MultiByteToWideChar(CP_ACP, 0, in.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, in.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;

	
}

#else

/**
* Function to change encoding of given wide char string and convert it to normal std::string
*
* @param[in] win  Input wide string to be converted
*
* @return std::string representation of wide string given as parameter
*/
std::string ws2s(const std::wstring& win)
{
	int lens = win.length();
	char* buffer = new char[lens * 3];

	int ret = wcstombs(buffer, win.c_str(), sizeof(buffer));

	std::string out = buffer;
	delete[] buffer;

	if (ret) return out;
	return "";
}

/**
* Function to change encoding of given multi byte string and convert it to wide string
*
* @param[in] in  Input string to be converted
*
* @return std::wstring representation of normal string given as parameter
*/
std::wstring s2ws(const std::string& in)
{
	int lens = in.length();
	wchar_t* buffer = new wchar_t[lens * 3];

	int ret = mbstowcs(buffer, in.c_str(), sizeof(buffer));

	std::wstring out = buffer;
	delete[] buffer;

	if (ret) return out;
	return L"";
}

#endif



/**
* Function replacing every occurence of all aliases with proper tag values read from audio file.
*
* Function that will replace all aliases (for example \%artist%) with proper values from each audio file
* given as second parameter.
*
* @see http://flute.eti.pg.gda.pl/trac/student-projects/wiki/MediaLibCleaner/Aliases for aliases definitions
*
* @param[in]	wcfg	   String with config file content. Any format is accepted.
* @param[in]	audiofile  std::unique_ptr to MediaLibCleaner::AudioFile object representing current file
*
* @return String containing config file with aliases replaced
*/
std::wstring ReplaceAllAliasOccurences(std::wstring& wcfg, MediaLibCleaner::File* audiofile) {
	std::wstring newc = wcfg;

	// copy original path to not confuse rest of the program
	std::wstring lpath = audiofile->GetPath();

	// do the magic!
	// SONG DATA
	replaceAll(newc, L"%artist%", audiofile->GetArtist());
	replaceAll(newc, L"%title%", audiofile->GetTitle());
	replaceAll(newc, L"%album%", audiofile->GetAlbum());
	replaceAll(newc, L"%genre%", audiofile->GetGenre());
	replaceAll(newc, L"%comment%", audiofile->GetComment());
	replaceAll(newc, L"%track%", audiofile->GetTrack());
	replaceAll(newc, L"%year%", audiofile->GetYear());
	replaceAll(newc, L"%albumartist%", audiofile->GetAlbumArtist());
	replaceAll(newc, L"%bpm%", audiofile->GetBPM());
	replaceAll(newc, L"%copyright%", audiofile->GetCopyright());
	replaceAll(newc, L"%language%", audiofile->GetLanguage());
	replaceAll(newc, L"%length%", audiofile->GetTagLength());
	replaceAll(newc, L"%mood%", audiofile->GetMood());
	replaceAll(newc, L"%origalbum%", audiofile->GetOrigAlbum());
	replaceAll(newc, L"%origartist%", audiofile->GetOrigArtist());
	replaceAll(newc, L"%origfilename%", audiofile->GetOrigFilename());
	replaceAll(newc, L"%origyear%", audiofile->GetOrigYear());
	replaceAll(newc, L"%publisher%", audiofile->GetPublisher());
	replaceAll(newc, L"%unsyncedlyrics%", audiofile->GetLyricsUnsynced());
	replaceAll(newc, L"%www%", audiofile->GetWWW());

	// TECHNICAL INFO
	replaceAll(newc, L"%_bitrate%", std::to_wstring(audiofile->GetBitrate()));
	replaceAll(newc, L"%_codec%", audiofile->GetCodec());
	replaceAll(newc, L"%_cover_mimetype%", audiofile->GetCoverMimetype());
	replaceAll(newc, L"%_cover_size%", std::to_wstring(audiofile->GetCoverSize()));
	replaceAll(newc, L"%_cover_type%", audiofile->GetCoverType());
	replaceAll(newc, L"%_covers%", std::to_wstring(audiofile->GetCovers()));
	replaceAll(newc, L"%_length%", audiofile->GetLengthAsString());
	replaceAll(newc, L"%_length_seconds%", std::to_wstring(audiofile->GetLength()));
	replaceAll(newc, L"%_channels%", std::to_wstring(audiofile->GetChannels()));
	replaceAll(newc, L"%_samplerate%", std::to_wstring(audiofile->GetSampleRate()));


	// PATH INFO
	replaceAll(newc, L"%_directory%", audiofile->GetDirectory());
	replaceAll(newc, L"%_ext%", audiofile->GetExt());
	replaceAll(newc, L"%_filename%", audiofile->GetFilename());
	replaceAll(newc, L"%_filename_ext%", audiofile->GetFilenameExt());
	replaceAll(newc, L"%_folderpath%", audiofile->GetFolderPath());
	replaceAll(newc, L"%_parent_dir%", audiofile->GetParentDir());
	replaceAll(newc, L"%_path%", audiofile->GetPath());
#ifdef WIN32
	replaceAll(newc, L"%_volume%", audiofile->GetVolume());
#endif
	boost::filesystem::path loc_path = path;
	replaceAll(newc, L"%_workingdir%", loc_path.filename().generic_wstring());
	replaceAll(newc, L"%_workingpath%", loc_path.generic_wstring());


	// FILES PROPERTIES
	replaceAll(newc, L"%_file_create_date%", audiofile->GetFileCreateDate());
	replaceAll(newc, L"%_file_create_datetime%", audiofile->GetFileCreateDatetime());
	replaceAll(newc, L"%_file_create_datetime_raw%", std::to_wstring(audiofile->GetFileCreateDatetimeRaw()));
	replaceAll(newc, L"%_file_mod_date%", audiofile->GetFileModDate());
	replaceAll(newc, L"%_file_mod_datetime%", audiofile->GetFileModDatetime());
	replaceAll(newc, L"%_file_mod_datetime_raw%", std::to_wstring(audiofile->GetFileModDatetimeRaw()));
	replaceAll(newc, L"%_file_size%", audiofile->GetFileSize());
	replaceAll(newc, L"%_file_size_bytes%", std::to_wstring(audiofile->GetFileSizeBytes()));
	replaceAll(newc, L"%_file_size_kb%", audiofile->GetFileSizeKB());
	replaceAll(newc, L"%_file_size_mb%", audiofile->GetFileSizeMB());


	// SYSTEM DATA
	replaceAll(newc, L"%_counter_dir%", std::to_wstring(audiofile->GetCounterDir()));
	replaceAll(newc, L"%_counter_total%", std::to_wstring(total_files));
	replaceAll(newc, L"%_date%", get_date_iso_8601_wide(datetime_raw));
	replaceAll(newc, L"%_datetime%", get_date_rfc_2822_wide(datetime_raw));
	replaceAll(newc, L"%_datetime_raw%", std::to_wstring(datetime_raw));
	replaceAll(newc, L"%_total_files%", std::to_wstring(total_files));
	replaceAll(newc, L"%_total_files_dir%", std::to_wstring(audiofile->GetCounterTotal()));

	replaceAll(newc, L"\\", L"\\\\");

	return newc;
}