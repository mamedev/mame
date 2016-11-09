// license:Boost
// copyright-holders:Christopher M. Kohlhoff

#include "mime_types.hpp"

namespace http {
namespace server {
namespace mime_types {

struct mapping
{
  const char* extension;
  const char* mime_type;
} mappings[] =
{
	{ "aac",     "audio/aac" },
	{ "aat",     "application/font-sfnt" },
	{ "aif",     "audio/x-aif" },
	{ "arj",     "application/x-arj-compressed" },
	{ "asf",     "video/x-ms-asf" },
	{ "avi",     "video/x-msvideo" },
	{ "bmp",     "image/bmp" },
	{ "cff",     "application/font-sfnt" },
	{ "css",     "text/css" },
	{ "csv",     "text/csv" },
	{ "doc",     "application/msword" },
	{ "eps",     "application/postscript" },
	{ "exe",     "application/octet-stream" },
	{ "gif",     "image/gif" },
	{ "gz",      "application/x-gunzip" },
	{ "htm",     "text/html" },
	{ "html",    "text/html" },
	{ "ico",     "image/x-icon" },
	{ "ief",     "image/ief" },
	{ "jpeg",    "image/jpeg" },
	{ "jpg",     "image/jpeg" },
	{ "jpm",     "image/jpm" },
	{ "jpx",     "image/jpx" },
	{ "js",      "application/javascript" },
	{ "json",    "application/json" },
	{ "m3u",     "audio/x-mpegurl" },
	{ "m4v",     "video/x-m4v" },
	{ "mid",     "audio/x-midi" },
	{ "mov",     "video/quicktime" },
	{ "mp3",     "audio/mpeg" },
	{ "mp4",     "video/mp4" },
	{ "mpeg",    "video/mpeg" },
	{ "mpg",     "video/mpeg" },
	{ "oga",     "audio/ogg" },
	{ "ogg",     "audio/ogg" },
	{ "ogv",     "video/ogg" },
	{ "otf",     "application/font-sfnt" },
	{ "pct",     "image/x-pct" },
	{ "pdf",     "application/pdf" },
	{ "pfr",     "application/font-tdpfr" },
	{ "pict",    "image/pict" },
	{ "png",     "image/png" },
	{ "ppt",     "application/x-mspowerpoint" },
	{ "ps",      "application/postscript" },
	{ "qt",      "video/quicktime" },
	{ "ra",      "audio/x-pn-realaudio" },
	{ "ram",     "audio/x-pn-realaudio" },
	{ "rar",     "application/x-arj-compressed" },
	{ "rgb",     "image/x-rgb" },
	{ "rtf",     "application/rtf" },
	{ "sgm",     "text/sgml" },
	{ "shtm",    "text/html" },
	{ "shtml",   "text/html" },
	{ "sil",     "application/font-sfnt" },
	{ "svg",     "image/svg+xml" },
	{ "swf",     "application/x-shockwave-flash" },
	{ "tar",     "application/x-tar" },
	{ "tgz",     "application/x-tar-gz" },
	{ "tif",     "image/tiff" },
	{ "tiff",    "image/tiff" },
	{ "torrent", "application/x-bittorrent" },
	{ "ttf",     "application/font-sfnt" },
	{ "txt",     "text/plain" },
	{ "wav",     "audio/x-wav" },
	{ "webm",    "video/webm" },
	{ "woff",    "application/font-woff" },
	{ "wrl",     "model/vrml" },
	{ "xhtml",   "application/xhtml+xml" },
	{ "xls",     "application/x-msexcel" },
	{ "xml",     "text/xml" },
	{ "xsl",     "application/xml" },
	{ "xslt",    "application/xml" },
	{ "zip",     "application/x-zip-compressed" }
};

std::string extension_to_type(const std::string& extension)
{
  for (mapping m: mappings)
  {
    if (m.extension == extension)
    {
      return m.mime_type;
    }
  }

  return "text/plain";
}

} // namespace mime_types
} // namespace server
} // namespace http
