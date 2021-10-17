#include "mime.hpp"

const std::unordered_map<std::string, std::string> Mime::mime_types = {
  {".atom", "application/atom+xml"},
  {".epub", "application/epub+zip"},
  {".jar", "application/java-archive"},
  {".js", "application/javascript"},
  {".json", "application/json"},
  {".jsonml", "application/jsonml+json"},
  {".mp4s", "application/mp4"},
  {".doc", "application/msword"},
  {".dot", "application/msword"},
  {".bin", "application/octet-stream"},
  {".pkg", "application/octet-stream"},
  {".dump", "application/octet-stream"},
  {".deploy", "application/octet-stream"},
  {".ogx", "application/ogg"},
  {".pdf", "application/pdf"},
  {".apk", "application/vnd.android.package-archive"},
  {".dart", "application/vnd.dart"},
  {".xls", "application/vnd.ms-excel"},
  {".xlm", "application/vnd.ms-excel"},
  {".xla", "application/vnd.ms-excel"},
  {".xlc", "application/vnd.ms-excel"},
  {".xlt", "application/vnd.ms-excel"},
  {".xlw", "application/vnd.ms-excel"},
  {".eot", "application/vnd.ms-fontobject"},
  {".ppt", "application/vnd.ms-powerpoint"},
  {".pps", "application/vnd.ms-powerpoint"},
  {".pot", "application/vnd.ms-powerpoint"},
  {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
  {".odt", "application/vnd.oasis.opendocument.text"},
  {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
  {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
  {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
  {".7z", "application/x-7z-compressed"},
  {".dmg", "application/x-apple-diskimage"},
  {".torrent", "application/x-bittorrent"},
  {".bz", "application/x-bzip"},
  {".bz2", "application/x-bzip2"},
  {".boz", "application/x-bzip2"},
  {".cbr", "application/x-cbr"},
  {".cba", "application/x-cbr"},
  {".cbt", "application/x-cbr"},
  {".cbz", "application/x-cbr"},
  {".cb7", "application/x-cbr"},
  {".vcd", "application/x-cdlink"},
  {".cfs", "application/x-cfs-compressed"},
  {".deb", "application/x-debian-package"},
  {".udeb", "application/x-debian-package"},
  {".dgc", "application/x-dgc-compressed"},
  {".hdf", "application/x-hdf"},
  {".install", "application/x-install-instructions"},
  {".iso", "application/x-iso9660-image"},
  {".latex", "application/x-latex"},
  {".mobi", "application/x-mobipocket-ebook"},
  {".application", "application/x-ms-application"},
  {".exe", "application/x-msdownload"},
  {".dll", "application/x-msdownload"},
  {".com", "application/x-msdownload"},
  {".bat", "application/x-msdownload"},
  {".msi", "application/x-msdownload"},
  {".sql", "application/x-sql"},
  {".tar", "application/x-tar"},
  {".obj", "application/x-tgif"},
  {".src", "application/x-wais-source"},
  {".fig", "application/x-xfig"},
  {".xhtml", "application/xhtml+xml"},
  {".xml", "application/xml"},
  {".xsl", "application/xml"},
  {".zip", "application/zip"},
  {".midi", "audio/midi"},
  {".m4a", "audio/mp4"},
  {".mp4a", "audio/mp4"},
  {".mpga", "audio/mpeg"},
  {".mp2", "audio/mpeg"},
  {".mp2a", "audio/mpeg"},
  {".mp3", "audio/mpeg"},
  {".m2a", "audio/mpeg"},
  {".m3a", "audio/mpeg"},
  {".oga", "audio/ogg"},
  {".ogg", "audio/ogg"},
  {".spx", "audio/ogg"},
  {".opus", "audio/ogg"},
  {".s3m", "audio/s3m"},
  {".sil", "audio/silk"},
  {".weba", "audio/webm"},
  {".aac", "audio/x-aac"},
  {".aif", "audio/x-aiff"},
  {".aiff", "audio/x-aiff"},
  {".aifc", "audio/x-aiff"},
  {".caf", "audio/x-caf"},
  {".flac", "audio/x-flac"},
  {".mka", "audio/x-matroska"},
  {".m3u", "audio/x-mpegurl"},
  {".wax", "audio/x-ms-wax"},
  {".wma", "audio/x-ms-wma"},
  {".wav", "audio/x-wav"},
  {".xm", "audio/xm"},
  {".ttc", "font/collection"},
  {".otf", "font/otf"},
  {".ttf", "font/ttf"},
  {".woff", "font/woff"},
  {".woff2", "font/woff2"},
  {".bmp", "image/bmp"},
  {".cgm", "image/cgm"},
  {".g3", "image/g3fax"},
  {".gif", "image/gif"},
  {".ief", "image/ief"},
  {".jpeg", "image/jpeg"},
  {".jpg", "image/jpeg"},
  {".jpe", "image/jpeg"},
  {".ktx", "image/ktx"},
  {".png", "image/png"},
  {".btif", "image/prs.btif"},
  {".sgi", "image/sgi"},
  {".svg", "image/svg+xml"},
  {".svgz", "image/svg+xml"},
  {".tiff", "image/tiff"},
  {".tif", "image/tiff"},
  {".psd", "image/vnd.adobe.photoshop"},
  {".webp", "image/webp"},
  {".3ds", "image/x-3ds"},
  {".ico", "image/x-icon"},
  {".rgb", "image/x-rgb"},
  {".xbm", "image/x-xbitmap"},
  {".xpm", "image/x-xpixmap"},
  {".xwd", "image/x-xwindowdump"},
  {".appcache", "text/cache-manifest"},
  {".css", "text/css"},
  {".csv", "text/csv"},
  {".html", "text/html"},
  {".htm", "text/html"},
  {".n3", "text/n3"},
  {".txt", "text/plain"},
  {".text", "text/plain"},
  {".conf", "text/plain"},
  {".list", "text/plain"},
  {".log", "text/plain"},
  {".tsv", "text/tab-separated-values"},
  {".sub", "text/vnd.dvb.subtitle"},
  {".s", "text/x-asm"},
  {".asm", "text/x-asm"},
  {".c", "text/x-c"},
  {".cc", "text/x-c"},
  {".cxx", "text/x-c"},
  {".cpp", "text/x-c"},
  {".h", "text/x-c"},
  {".hh", "text/x-c"},
  {".dic", "text/x-c"},
  {".java", "text/x-java-source"},
  {".3gp", "video/3gpp"},
  {".3g2", "video/3gpp2"},
  {".h261", "video/h261"},
  {".h263", "video/h263"},
  {".h264", "video/h264"},
  {".jpgv", "video/jpeg"},
  {".jpm", "video/jpm"},
  {".jpgm", "video/jpm"},
  {".mj2", "video/mj2"},
  {".mjp2", "video/mj2"},
  {".mp4", "video/mp4"},
  {".mp4v", "video/mp4"},
  {".mpg4", "video/mp4"},
  {".mpeg", "video/mpeg"},
  {".mpg", "video/mpeg"},
  {".mpe", "video/mpeg"},
  {".m1v", "video/mpeg"},
  {".m2v", "video/mpeg"},
  {".ogv", "video/ogg"},
  {".ts", "video/mp2t"},
  {".qt", "video/quicktime"},
  {".mov", "video/quicktime"},
  {".uvh", "video/vnd.dece.hd"},
  {".uvvh", "video/vnd.dece.hd"},
  {".uvm", "video/vnd.dece.mobile"},
  {".uvvm", "video/vnd.dece.mobile"},
  {".uvp", "video/vnd.dece.pd"},
  {".uvvp", "video/vnd.dece.pd"},
  {".uvs", "video/vnd.dece.sd"},
  {".uvvs", "video/vnd.dece.sd"},
  {".uvv", "video/vnd.dece.video"},
  {".uvvv", "video/vnd.dece.video"},
  {".dvb", "video/vnd.dvb.file"},
  {".fvt", "video/vnd.fvt"},
  {".mxu", "video/vnd.mpegurl"},
  {".m4u", "video/vnd.mpegurl"},
  {".pyv", "video/vnd.ms-playready.media.pyv"},
  {".uvu", "video/vnd.uvvu.mp4"},
  {".uvvu", "video/vnd.uvvu.mp4"},
  {".viv", "video/vnd.vivo"},
  {".webm", "video/webm"},
  {".f4v", "video/x-f4v"},
  {".fli", "video/x-fli"},
  {".flv", "video/x-flv"},
  {".m4v", "video/x-m4v"},
  {".mkv", "video/x-matroska"},
  {".mk3d", "video/x-matroska"},
  {".mks", "video/x-matroska"},
  {".mng", "video/x-mng"},
  {".asf", "video/x-ms-asf"},
  {".asx", "video/x-ms-asf"},
  {".vob", "video/x-ms-vob"},
  {".wm", "video/x-ms-wm"},
  {".wmv", "video/x-ms-wmv"},
  {".wmx", "video/x-ms-wmx"},
  {".wvx", "video/x-ms-wvx"},
  {".avi", "video/x-msvideo"},
  {".movie", "video/x-sgi-movie"},
  {".smv", "video/x-smv"},
};

const std::string &Mime::ext_to_mime(const std::string &ext) {
  if (mime_types.find(ext) == mime_types.end()) {
    return "application/octet-stream";
  }
  return mime_types.at(ext);
}