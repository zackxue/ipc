
#include "generic.h"
#include "httpd_debug.h"
#include "http_common.h"

struct HTTP_MIME_MAP
{
	const char* extension;
	const char* mime_type;
};

static const struct HTTP_MIME_MAP _http_mime_map[] =
{
	{ "3dm", "x-world/x-3dmf", },
	{ "3dmf", "x-world/x-3dmf", },
	{ "a", "application/octet-stream", },
	{ "aab", "application/x-authorware-bin", },
	{ "aam", "application/x-authorware-map", },
	{ "aas", "application/x-authorware-seg", },
	{ "abc", "text/vnd.abc", },
	{ "acgi", "text/html", },
	{ "afl", "video/animaflex", },
	{ "ai", "application/postscript", },
	{ "aif", "audio/aiff", },
	{ "aif", "audio/x-aiff", },
	{ "aifc", "audio/aiff", },
	{ "aifc", "audio/x-aiff", },
	{ "aiff", "audio/aiff", },
	{ "aiff", "audio/x-aiff", },
	{ "aim", "application/x-aim", },
	{ "aip", "text/x-audiosoft-intra", },
	{ "ani", "application/x-navi-animation", },
	{ "aos", "application/x-nokia-9000-communicator-add-on-software", },
	{ "aps", "application/mime", },
	{ "arc", "application/octet-stream", },
	{ "arj", "application/arj", },
	{ "arj", "application/octet-stream", },
	{ "art", "image/x-jg", },
	{ "asf", "video/x-ms-asf", },
	{ "asm", "text/x-asm", },
	{ "asp", "text/asp", },
	{ "asx", "application/x-mplayer2", },
	{ "asx", "video/x-ms-asf", },
	{ "asx", "video/x-ms-asf-plugin", },
	{ "au", "audio/basic", },
	{ "au", "audio/x-au", },
	{ "avi", "application/x-troff-msvideo", },
	{ "avi", "video/avi", },
	{ "avi", "video/msvideo", },
	{ "avi", "video/x-msvideo", },
	{ "avs", "video/avs-video", },
	{ "bcpio", "application/x-bcpio", },
	{ "bin", "application/mac-binary", },
	{ "bin", "application/macbinary", },
	{ "bin", "application/octet-stream", },
	{ "bin", "application/x-binary", },
	{ "bin", "application/x-macbinary", },
	{ "bm", "image/bmp", },
	{ "bmp", "image/bmp", },
	{ "bmp", "image/x-windows-bmp", },
	{ "boo", "application/book", },
	{ "book", "application/book", },
	{ "boz", "application/x-bzip2", },
	{ "bsh", "application/x-bsh", },
	{ "bz", "application/x-bzip", },
	{ "bz2", "application/x-bzip2", },
	{ "c", "text/plain", },
	{ "c", "text/x-c", },
	{ "c++", "text/plain", },
	{ "cat", "application/vnd.ms-pki.seccat", },
	{ "cc", "text/plain", },
	{ "cc", "text/x-c", },
	{ "ccad", "application/clariscad", },
	{ "cco", "application/x-cocoa", },
	{ "cdf", "application/cdf", },
	{ "cdf", "application/x-cdf", },
	{ "cdf", "application/x-netcdf", },
	{ "cer", "application/pkix-cert", },
	{ "cer", "application/x-x509-ca-cert", },
	{ "cha", "application/x-chat", },
	{ "chat", "application/x-chat", },
	{ "class", "application/java", },
	{ "class", "application/java-byte-code", },
	{ "class", "application/x-java-class", },
	{ "com", "application/octet-stream", },
	{ "com", "text/plain", },
	{ "conf", "text/plain", },
	{ "cpio", "application/x-cpio", },
	{ "cpp", "text/x-c", },
	{ "cpt", "application/mac-compactpro", },
	{ "cpt", "application/x-compactpro", },
	{ "cpt", "application/x-cpt", },
	{ "crl", "application/pkcs-crl", },
	{ "crl", "application/pkix-crl", },
	{ "crt", "application/pkix-cert", },
	{ "crt", "application/x-x509-ca-cert", },
	{ "crt", "application/x-x509-user-cert", },
	{ "csh", "application/x-csh", },
	{ "csh", "text/x-script.csh", },
//	{ "css", "application/x-pointplus", },
	{ "css", "text/css", },
	{ "cxx", "text/plain", },
	{ "dcr", "application/x-director", },
	{ "deepv", "application/x-deepv", },
	{ "def", "text/plain", },
	{ "der", "application/x-x509-ca-cert", },
	{ "dif", "video/x-dv", },
	{ "dir", "application/x-director", },
	{ "dl", "video/dl", },
	{ "dl", "video/x-dl", },
	{ "doc", "application/msword", },
	{ "dot", "application/msword", },
	{ "dp", "application/commonground", },
	{ "drw", "application/drafting", },
	{ "dump", "application/octet-stream", },
	{ "dv", "video/x-dv", },
	{ "dvi", "application/x-dvi", },
	{ "dwf", "drawing/x-dwf (old)", },
	{ "dwf", "model/vnd.dwf", },
	{ "dwg", "application/acad", },
	{ "dwg", "image/vnd.dwg", },
	{ "dwg", "image/x-dwg", },
	{ "dxf", "application/dxf", },
	{ "dxf", "image/vnd.dwg", },
	{ "dxf", "image/x-dwg", },
	{ "dxr", "application/x-director", },
	{ "el", "text/x-script.elisp", },
	{ "elc", "application/x-bytecode.elisp (compiled elisp)", },
	{ "elc", "application/x-elc", },
	{ "env", "application/x-envoy", },
	{ "eps", "application/postscript", },
	{ "es", "application/x-esrehber", },
	{ "etx", "text/x-setext", },
	{ "evy", "application/envoy", },
	{ "evy", "application/x-envoy", },
	{ "exe", "application/octet-stream", },
	{ "f", "text/plain", },
	{ "f", "text/x-fortran", },
	{ "f77", "text/x-fortran", },
	{ "f90", "text/plain", },
	{ "f90", "text/x-fortran", },
	{ "fdf", "application/vnd.fdf", },
	{ "fif", "application/fractals", },
	{ "fif", "image/fif", },
	{ "fli", "video/fli", },
	{ "fli", "video/x-fli", },
	{ "flo", "image/florian", },
	{ "flx", "text/vnd.fmi.flexstor", },
	{ "fmf", "video/x-atomic3d-feature", },
	{ "for", "text/plain", },
	{ "for", "text/x-fortran", },
	{ "fpx", "image/vnd.fpx", },
	{ "fpx", "image/vnd.net-fpx", },
	{ "frl", "application/freeloader", },
	{ "funk", "audio/make", },
	{ "g", "text/plain", },
	{ "g3", "image/g3fax", },
	{ "gif", "image/gif", },
	{ "gl", "video/gl", },
	{ "gl", "video/x-gl", },
	{ "gsd", "audio/x-gsm", },
	{ "gsm", "audio/x-gsm", },
	{ "gsp", "application/x-gsp", },
	{ "gss", "application/x-gss", },
	{ "gtar", "application/x-gtar", },
	{ "gz", "application/x-compressed", },
	{ "gz", "application/x-gzip", },
	{ "gzip", "application/x-gzip", },
	{ "gzip", "multipart/x-gzip", },
	{ "h", "text/plain", },
	{ "h", "text/x-h", },
	{ "hdf", "application/x-hdf", },
	{ "help", "application/x-helpfile", },
	{ "hgl", "application/vnd.hp-hpgl", },
	{ "hh", "text/plain", },
	{ "hh", "text/x-h", },
	{ "hlb", "text/x-script", },
	{ "hlp", "application/hlp", },
	{ "hlp", "application/x-helpfile", },
	{ "hlp", "application/x-winhelp", },
	{ "hpg", "application/vnd.hp-hpgl", },
	{ "hpgl", "application/vnd.hp-hpgl", },
	{ "hqx", "application/binhex", },
	{ "hqx", "application/binhex4", },
	{ "hqx", "application/mac-binhex", },
	{ "hqx", "application/mac-binhex40", },
	{ "hqx", "application/x-binhex40", },
	{ "hqx", "application/x-mac-binhex40", },
	{ "hta", "application/hta", },
	{ "htc", "text/x-component", },
	{ "htm", "text/html", },
	{ "html", "text/html", },
	{ "htmls", "text/html", },
	{ "htt", "text/webviewhtml", },
	{ "htx", "text/html", },
	{ "ice", "x-conference/x-cooltalk", },
	{ "ico", "image/x-icon", },
	{ "idc", "text/plain", },
	{ "ief", "image/ief", },
	{ "iefs", "image/ief", },
	{ "iges", "application/iges", },
	{ "iges", "model/iges", },
	{ "igs", "application/iges", },
	{ "igs", "model/iges", },
	{ "ima", "application/x-ima", },
	{ "imap", "application/x-httpd-imap", },
	{ "inf", "application/inf", },
	{ "ins", "application/x-internett-signup", },
	{ "ip", "application/x-ip2", },
	{ "isu", "video/x-isvideo", },
	{ "it", "audio/it", },
	{ "iv", "application/x-inventor", },
	{ "ivr", "i-world/i-vrml", },
	{ "ivy", "application/x-livescreen", },
	{ "jam", "audio/x-jam", },
	{ "jav", "text/plain", },
	{ "jav", "text/x-java-source", },
	{ "java", "text/plain", },
	{ "java", "text/x-java-source", },
	{ "jcm", "application/x-java-commerce", },
	{ "jfif", "image/jpeg", },
	{ "jfif", "image/pjpeg", },
	{ "jfif-tbnl", "image/jpeg", },
	{ "jpe", "image/jpeg", },
	{ "jpe", "image/pjpeg", },
	{ "jpeg", "image/jpeg", },
	{ "jpeg", "image/pjpeg", },
	{ "jpg", "image/jpeg", },
	{ "jpg", "image/pjpeg", },
	{ "jps", "image/x-jps", },
	{ "js", "application/x-javascript", },
	{ "jut", "image/jutvision", },
	{ "kar", "audio/midi", },
	{ "kar", "music/x-karaoke", },
	{ "ksh", "application/x-ksh", },
	{ "ksh", "text/x-script.ksh", },
	{ "la", "audio/nspaudio", },
	{ "la", "audio/x-nspaudio", },
	{ "lam", "audio/x-liveaudio", },
	{ "latex	application/x-latex", },
	{ "lha", "application/lha", },
	{ "lha", "application/octet-stream", },
	{ "lha", "application/x-lha", },
	{ "lhx", "application/octet-stream", },
	{ "list", "text/plain", },
	{ "lma", "audio/nspaudio", },
	{ "lma", "audio/x-nspaudio", },
	{ "log", "text/plain", },
	{ "lsp", "application/x-lisp", },
	{ "lsp", "text/x-script.lisp", },
	{ "lst", "text/plain", },
	{ "lsx", "text/x-la-asf", },
	{ "ltx", "application/x-latex", },
	{ "lzh", "application/octet-stream", },
	{ "lzh", "application/x-lzh", },
	{ "lzx", "application/lzx", },
	{ "lzx", "application/octet-stream", },
	{ "lzx", "application/x-lzx", },
	{ "m", "text/plain", },
	{ "m", "text/x-m", },
	{ "m1v", "video/mpeg", },
	{ "m2a", "audio/mpeg", },
	{ "m2v", "video/mpeg", },
	{ "m3u", "audio/x-mpequrl", },
	{ "man", "application/x-troff-man", },
	{ "map", "application/x-navimap", },
	{ "mar", "text/plain", },
	{ "mbd", "application/mbedlet", },
	{ "mc$", "application/x-magic-cap-package-1.0", },
	{ "mcd", "application/mcad", },
	{ "mcd", "application/x-mathcad", },
	{ "mcf", "image/vasa", },
	{ "mcf", "text/mcf", },
	{ "mcp", "application/netmc", },
	{ "me", "application/x-troff-me", },
	{ "mht", "message/rfc822", },
	{ "mhtml", "message/rfc822", },
	{ "mid", "application/x-midi", },
	{ "mid", "audio/midi", },
	{ "mid", "audio/x-mid", },
	{ "mid", "audio/x-midi", },
	{ "mid", "music/crescendo", },
	{ "mid", "x-music/x-midi", },
	{ "midi", "application/x-midi", },
	{ "midi", "audio/midi", },
	{ "midi", "audio/x-mid", },
	{ "midi", "audio/x-midi", },
	{ "midi", "music/crescendo", },
	{ "midi", "x-music/x-midi", },
	{ "mif", "application/x-frame", },
	{ "mif", "application/x-mif", },
	{ "mime", "message/rfc822", },
	{ "mime", "www/mime", },
	{ "mjf", "audio/x-vnd.audioexplosion.mjuicemediafile", },
	{ "mjpg", "video/x-motion-jpeg", },
	{ "mm", "application/base64", },
	{ "mm", "application/x-meme", },
	{ "mme", "application/base64", },
	{ "mod", "audio/mod", },
	{ "mod", "audio/x-mod", },
	{ "moov", "video/quicktime", },
	{ "mov", "video/quicktime", },
	{ "movie", "video/x-sgi-movie", },
	{ "mp2", "audio/mpeg", },
	{ "mp2", "audio/x-mpeg", },
	{ "mp2", "video/mpeg", },
	{ "mp2", "video/x-mpeg", },
	{ "mp2", "video/x-mpeq2a", },
	{ "mp3", "audio/mpeg3", },
	{ "mp3", "audio/x-mpeg-3", },
	{ "mp3", "video/mpeg", },
	{ "mp3", "video/x-mpeg", },
	{ "mpa", "audio/mpeg", },
	{ "mpa", "video/mpeg", },
	{ "mpc", "application/x-project", },
	{ "mpe", "video/mpeg", },
	{ "mpeg", "video/mpeg", },
	{ "mpg", "audio/mpeg", },
	{ "mpg", "video/mpeg", },
	{ "mpga", "audio/mpeg", },
	{ "mpp", "application/vnd.ms-project", },
	{ "mpt", "application/x-project", },
	{ "mpv", "application/x-project", },
	{ "mpx", "application/x-project", },
	{ "mrc", "application/marc", },
	{ "ms", "application/x-troff-ms", },
	{ "mv", "video/x-sgi-movie", },
	{ "my", "audio/make", },
	{ "mzz", "application/x-vnd.audioexplosion.mzz", },
	{ "nap", "image/naplps", },
	{ "naplps", "image/naplps", },
	{ "nc", "application/x-netcdf", },
	{ "ncm", "application/vnd.nokia.configuration-message", },
	{ "nif", "image/x-niff", },
	{ "niff", "image/x-niff", },
	{ "nix", "application/x-mix-transfer", },
	{ "nsc", "application/x-conference", },
	{ "nvd", "application/x-navidoc", },
	{ "o", "application/octet-stream", },
	{ "oda", "application/oda", },
	{ "omc", "application/x-omc", },
	{ "omcd", "application/x-omcdatamaker", },
	{ "omcr", "application/x-omcregerator", },
	{ "p", "text/x-pascal", },
	{ "p10", "application/pkcs10", },
	{ "p10", "application/x-pkcs10", },
	{ "p12", "application/pkcs-12", },
	{ "p12", "application/x-pkcs12", },
	{ "p7a", "application/x-pkcs7-signature", },
	{ "p7c", "application/pkcs7-mime", },
	{ "p7c", "application/x-pkcs7-mime", },
	{ "p7m", "application/pkcs7-mime", },
	{ "p7m", "application/x-pkcs7-mime", },
	{ "p7r", "application/x-pkcs7-certreqresp", },
	{ "p7s", "application/pkcs7-signature", },
	{ "part", "application/pro_eng", },
	{ "pas", "text/pascal", },
	{ "pbm", "image/x-portable-bitmap", },
	{ "pcl", "application/vnd.hp-pcl", },
	{ "pcl", "application/x-pcl", },
	{ "pct", "image/x-pict", },
	{ "pcx", "image/x-pcx", },
	{ "pdb", "chemical/x-pdb", },
	{ "pdf", "application/pdf", },
	{ "pfunk", "audio/make", },
	{ "pfunk", "audio/make.my.funk", },
	{ "pgm", "image/x-portable-graymap", },
	{ "pgm", "image/x-portable-greymap", },
	{ "pic", "image/pict", },
	{ "pict", "image/pict", },
	{ "pkg", "application/x-newton-compatible-pkg", },
	{ "pko", "application/vnd.ms-pki.pko", },
	{ "pl", "text/plain", },
	{ "pl", "text/x-script.perl", },
	{ "plx", "application/x-pixclscript", },
	{ "pm", "image/x-xpixmap", },
	{ "pm", "text/x-script.perl-module", },
	{ "pm4", "application/x-pagemaker", },
	{ "pm5", "application/x-pagemaker", },
	{ "png", "image/png", },
	{ "pnm", "application/x-portable-anymap", },
	{ "pnm", "image/x-portable-anymap", },
	{ "pot", "application/mspowerpoint", },
	{ "pot", "application/vnd.ms-powerpoint", },
	{ "pov", "model/x-pov", },
	{ "ppa", "application/vnd.ms-powerpoint", },
	{ "ppm", "image/x-portable-pixmap", },
	{ "pps", "application/mspowerpoint", },
	{ "pps", "application/vnd.ms-powerpoint", },
	{ "ppt", "application/mspowerpoint", },
	{ "ppt", "application/powerpoint", },
	{ "ppt", "application/vnd.ms-powerpoint", },
	{ "ppt", "application/x-mspowerpoint", },
	{ "ppz", "application/mspowerpoint", },
	{ "pre", "application/x-freelance", },
	{ "prt", "application/pro_eng", },
	{ "ps", "application/postscript", },
	{ "psd", "application/octet-stream", },
	{ "pvu", "paleovu/x-pv", },
	{ "pwz", "application/vnd.ms-powerpoint", },
	{ "py", "text/x-script.phyton", },
	{ "pyc", "applicaiton/x-bytecode.python", },
	{ "qcp", "audio/vnd.qcelp", },
	{ "qd3", "x-world/x-3dmf", },
	{ "qd3d", "x-world/x-3dmf", },
	{ "qif", "image/x-quicktime", },
	{ "qt", "video/quicktime", },
	{ "qtc", "video/x-qtc", },
	{ "qti", "image/x-quicktime", },
	{ "qtif", "image/x-quicktime", },
	{ "ra", "audio/x-pn-realaudio", },
	{ "ra", "audio/x-pn-realaudio-plugin", },
	{ "ra", "audio/x-realaudio", },
	{ "ram", "audio/x-pn-realaudio", },
	{ "ras", "application/x-cmu-raster", },
	{ "ras", "image/cmu-raster", },
	{ "ras", "image/x-cmu-raster", },
	{ "rast", "image/cmu-raster", },
	{ "rexx", "text/x-script.rexx", },
	{ "rf", "image/vnd.rn-realflash", },
	{ "rgb", "image/x-rgb", },
	{ "rm", "application/vnd.rn-realmedia", },
	{ "rm", "audio/x-pn-realaudio", },
	{ "rmi", "audio/mid", },
	{ "rmm", "audio/x-pn-realaudio", },
	{ "rmp", "audio/x-pn-realaudio", },
	{ "rmp", "audio/x-pn-realaudio-plugin", },
	{ "rng", "application/ringing-tones", },
	{ "rng", "application/vnd.nokia.ringing-tone", },
	{ "rnx", "application/vnd.rn-realplayer", },
	{ "roff", "application/x-troff", },
	{ "rp", "image/vnd.rn-realpix", },
	{ "rpm", "audio/x-pn-realaudio-plugin", },
	{ "rt", "text/richtext", },
	{ "rt", "text/vnd.rn-realtext", },
	{ "rtf", "application/rtf", },
	{ "rtf", "application/x-rtf", },
	{ "rtf", "text/richtext", },
	{ "rtx", "application/rtf", },
	{ "rtx", "text/richtext", },
	{ "rv", "video/vnd.rn-realvideo", },
	{ "s", "text/x-asm", },
	{ "s3m", "audio/s3m", },
	{ "saveme", "application/octet-stream", },
	{ "sbk", "application/x-tbook", },
	{ "scm", "application/x-lotusscreencam", },
	{ "scm", "text/x-script.guile", },
	{ "scm", "text/x-script.scheme", },
	{ "scm", "video/x-scm", },
	{ "sdml", "text/plain", },
	{ "sdp", "application/sdp", },
	{ "sdp", "application/x-sdp", },
	{ "sdr", "application/sounder", },
	{ "sea", "application/sea", },
	{ "sea", "application/x-sea", },
	{ "set", "application/set", },
	{ "sgm", "text/sgml", },
	{ "sgm", "text/x-sgml", },
	{ "sgml", "text/sgml", },
	{ "sgml", "text/x-sgml", },
	{ "sh", "application/x-bsh", },
	{ "sh", "application/x-sh", },
	{ "sh", "application/x-shar", },
	{ "sh", "text/x-script.sh", },
	{ "shar", "application/x-bsh", },
	{ "shar", "application/x-shar", },
	{ "shtml	text/html", },
	{ "shtml", "text/x-server-parsed-html", },
	{ "sid", "audio/x-psid", },
	{ "sit", "application/x-sit", },
	{ "sit", "application/x-stuffit", },
	{ "skd", "application/x-koan", },
	{ "skm", "application/x-koan", },
	{ "skp", "application/x-koan", },
	{ "skt", "application/x-koan", },
	{ "sl", "application/x-seelogo", },
	{ "smi", "application/smil", },
	{ "smil", "application/smil", },
	{ "snd", "audio/basic", },
	{ "snd", "audio/x-adpcm", },
	{ "sol", "application/solids", },
	{ "spc", "application/x-pkcs7-certificates", },
	{ "spc", "text/x-speech", },
	{ "spl", "application/futuresplash", },
	{ "spr", "application/x-sprite", },
	{ "sprite", "application/x-sprite", },
	{ "src", "application/x-wais-source", },
	{ "ssi", "text/x-server-parsed-html", },
	{ "ssm", "application/streamingmedia", },
	{ "sst", "application/vnd.ms-pki.certstore", },
	{ "step", "application/step", },
	{ "stl", "application/sla", },
	{ "stl", "application/vnd.ms-pki.stl", },
	{ "stl", "application/x-navistyle", },
	{ "stp", "application/step", },
	{ "sv4cpio", "application/x-sv4cpio", },
	{ "sv4crc", "application/x-sv4crc", },
	{ "svf", "image/vnd.dwg", },
	{ "svf", "image/x-dwg", },
	{ "svr", "application/x-world", },
	{ "svr", "x-world/x-svr", },
	{ "swf", "application/x-shockwave-flash", },
	{ "t", "application/x-troff", },
	{ "talk", "text/x-speech", },
	{ "tar", "application/x-tar", },
	{ "tbk", "application/toolbook", },
	{ "tbk", "application/x-tbook", },
	{ "tcl", "application/x-tcl", },
	{ "tcl", "text/x-script.tcl", },
	{ "tcsh", "text/x-script.tcsh", },
	{ "tex", "application/x-tex", },
	{ "texi", "application/x-texinfo", },
	{ "texinfo", "application/x-texinfo", },
	{ "text", "application/plain", },
	{ "text", "text/plain", },
	{ "tgz", "application/gnutar", },
	{ "tgz", "application/x-compressed", },
	{ "tif", "image/tiff", },
	{ "tif", "image/x-tiff", },
	{ "tiff", "image/tiff", },
	{ "tiff", "image/x-tiff", },
	{ "tr", "application/x-troff", },
	{ "tsi", "audio/tsp-audio", },
	{ "tsp", "application/dsptype", },
	{ "tsp", "audio/tsplayer", },
	{ "tsv", "text/tab-separated-values", },
	{ "turbot", "image/florian", },
	{ "txt", "text/plain", },
	{ "uil", "text/x-uil", },
	{ "uni", "text/uri-list", },
	{ "unis", "text/uri-list", },
	{ "unv", "application/i-deas", },
	{ "uri", "text/uri-list", },
	{ "uris", "text/uri-list", },
	{ "ustar", "application/x-ustar", },
	{ "ustar", "multipart/x-ustar", },
	{ "uu", "application/octet-stream", },
	{ "uu", "text/x-uuencode", },
	{ "uue", "text/x-uuencode", },
	{ "vcd", "application/x-cdlink", },
	{ "vcs", "text/x-vcalendar", },
	{ "vda", "application/vda", },
	{ "vdo", "video/vdo", },
	{ "vew", "application/groupwise", },
	{ "viv", "video/vivo", },
	{ "viv", "video/vnd.vivo", },
	{ "vivo", "video/vivo", },
	{ "vivo", "video/vnd.vivo", },
	{ "vmd", "application/vocaltec-media-desc", },
	{ "vmf", "application/vocaltec-media-file", },
	{ "voc", "audio/voc", },
	{ "voc", "audio/x-voc", },
	{ "vos", "video/vosaic", },
	{ "vox", "audio/voxware", },
	{ "vqe", "audio/x-twinvq-plugin", },
	{ "vqf", "audio/x-twinvq", },
	{ "vql", "audio/x-twinvq-plugin", },
	{ "vrml", "application/x-vrml", },
	{ "vrml", "model/vrml", },
	{ "vrml", "x-world/x-vrml", },
	{ "vrt", "x-world/x-vrt", },
	{ "vsd", "application/x-visio", },
	{ "vst", "application/x-visio", },
	{ "vsw", "application/x-visio", },
	{ "w60", "application/wordperfect6.0", },
	{ "w61", "application/wordperfect6.1", },
	{ "w6w", "application/msword", },
	{ "wav", "audio/wav", },
	{ "wav", "audio/x-wav", },
	{ "wb1", "application/x-qpro", },
	{ "wbmp", "image/vnd.wap.wbmp", },
	{ "web", "application/vnd.xara", },
	{ "wiz", "application/msword", },
	{ "wk1", "application/x-123", },
	{ "wmf", "windows/metafile", },
	{ "wml", "text/vnd.wap.wml", },
	{ "wmlc", "application/vnd.wap.wmlc", },
	{ "wmls", "text/vnd.wap.wmlscript", },
	{ "wmlsc	application/vnd.wap.wmlscriptc", },
	{ "word", "application/msword", },
	{ "wp", "application/wordperfect", },
	{ "wp5", "application/wordperfect", },
	{ "wp5", "application/wordperfect6.0", },
	{ "wp6", "application/wordperfect", },
	{ "wpd", "application/wordperfect", },
	{ "wpd", "application/x-wpwin", },
	{ "wq1", "application/x-lotus", },
	{ "wri", "application/mswrite", },
	{ "wri", "application/x-wri", },
	{ "wrl", "application/x-world", },
	{ "wrl", "model/vrml", },
	{ "wrl", "x-world/x-vrml", },
	{ "wrz", "model/vrml", },
	{ "wrz", "x-world/x-vrml", },
	{ "wsc", "text/scriplet", },
	{ "wsrc", "application/x-wais-source", },
	{ "wtk", "application/x-wintalk", },
	{ "xbm", "image/x-xbitmap", },
	{ "xbm", "image/x-xbm", },
	{ "xbm", "image/xbm", },
	{ "xdr", "video/x-amt-demorun", },
	{ "xgz", "xgl/drawing", },
	{ "xif", "image/vnd.xiff", },
	{ "xl", "application/excel", },
	{ "xla", "application/excel", },
	{ "xla", "application/x-excel", },
	{ "xla", "application/x-msexcel", },
	{ "xlb", "application/excel", },
	{ "xlb", "application/vnd.ms-excel", },
	{ "xlb", "application/x-excel", },
	{ "xlc", "application/excel", },
	{ "xlc", "application/vnd.ms-excel", },
	{ "xlc", "application/x-excel", },
	{ "xld", "application/excel", },
	{ "xld", "application/x-excel", },
	{ "xlk", "application/excel", },
	{ "xlk", "application/x-excel", },
	{ "xll", "application/excel", },
	{ "xll", "application/vnd.ms-excel", },
	{ "xll", "application/x-excel", },
	{ "xlm", "application/excel", },
	{ "xlm", "application/vnd.ms-excel", },
	{ "xlm", "application/x-excel", },
	{ "xls", "application/excel", },
	{ "xls", "application/vnd.ms-excel", },
	{ "xls", "application/x-excel", },
	{ "xls", "application/x-msexcel", },
	{ "xlt", "application/excel", },
	{ "xlt", "application/x-excel", },
	{ "xlv", "application/excel", },
	{ "xlv", "application/x-excel", },
	{ "xlw", "application/excel", },
	{ "xlw", "application/vnd.ms-excel", },
	{ "xlw", "application/x-excel", },
	{ "xlw", "application/x-msexcel", },
	{ "xm", "audio/xm", },
	{ "xml", "application/xml", },
	{ "xml", "text/xml", },
	{ "xmz", "xgl/movie", },
	{ "xpix", "application/x-vnd.ls-xpix", },
	{ "xpm", "image/x-xpixmap", },
	{ "xpm", "image/xpm", },
	{ "x-png", "image/png", },
	{ "xsr", "video/x-amt-showrun", },
	{ "xwd", "image/x-xwd", },
	{ "xwd", "image/x-xwindowdump", },
	{ "xyz", "chemical/x-pdb", },
	{ "z", "application/x-compress", },
	{ "z", "application/x-compressed", },
	{ "zip", "application/x-compressed", },
	{ "zip", "application/x-zip-compressed", },
	{ "zip", "application/zip", },
	{ "zip", "multipart/x-zip", },
	{ "zoo", "application/octet-stream", },
	{ "zsh", "text/x-script.zsh", },
};

const char* http_get_file_mime(const char* extname)
{
	int i = 0;
	const char* ext = extname;
	if('.' == ext[0]){
		++ext;
	}
	for(i = 0; i < ARRAY_ITEM(_http_mime_map); ++i){
		if(STR_CASE_THE_SAME(ext, _http_mime_map[i].extension)){
			return _http_mime_map[i].mime_type;
		}
	}
	return "application/octet-stream";
}

const char* http_get_reason_phrase(int status_code)
{
	switch(status_code)
	{
	case 100: return "Continue";
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 201: return "Created";
	case 202: return "Accepted";
	case 203: return "Non-Authoritative Information";
	case 204: return "No Content";
	case 205: return "Reset Content";
	case 206: return "Partial Content";
	case 300: return "Multiple Choices";
	case 301: return "Moved Permanently";
	case 302: return "Found";
	case 303: return "See Other";
	case 304: return "Not Modified";
	case 305: return "Use Proxy";
	case 307: return "Temporary Redirect";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 402: return "Payment Required";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 406: return "Not Acceptable";
	case 407: return "Proxy Authentication Required";
	case 408: return "Request Time-out";
	case 409: return "Conflict";
	case 410: return "Gone";
	case 411: return "Length Required";
	case 412: return "Precondition Failed";
	case 413: return "Request Entity Too Large";
	case 414: return "Request-URI Too Large";
	case 415: return "Unsupported Media Type";
	case 416: return "Requested range not satisfiable";
	case 417: return "Expectation Failed";
	case 500: return "Internal Server Error";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	case 504: return "Gateway Time-out";
	case 505: return "HTTP Version not supported";
	default: ;
	}
	return NULL;
}

int http_read_header(const char* header, char* tag, AVal* ret_val)
{
	if(tag && strlen(tag) && ret_val){		
		char* key_with_colon = alloca(strlen(tag) + 16);
		char* value = NULL;
		strcpy(key_with_colon, tag);
		strcat(key_with_colon, ": ");

		// reset the value first
		ret_val->av_val = NULL;
		ret_val->av_len = 0;
		
		value = strcasestr(header, key_with_colon);
		if(NULL != value){
			// found!!
			ret_val->av_val = value + strlen(key_with_colon);
			ret_val->av_len = strstr(value, "\r\n") - ret_val->av_val;
//			HTTPD_TRACE("Success to parse \"%s : %s\"", tag, AVAL_STRDUPA(*ret_val));
			return 0;
		}
	}
	return -1;
}

void http_uri_slash_filter(char* uri_rw)
{
	int i = 0;
	char* slash = NULL;
	slash = strstr(uri_rw, "//");
	if(slash){
		do{
			char* offset = slash + 2;
			while('\0' != *offset){
				// overwrite the former letter
				*(offset - 1) = *offset;
				++offset;
			}
			*(offset - 1) = '\0';
		}while((slash = strstr(slash, "//")));
	}
}


int http_parse_request_line(char* request_msg, HTTP_REQUEST_LINE_t* ret_request_line)
{
	char* ch_ptr = NULL;
	AVal* const ret_method = &ret_request_line->method;
	AVal* const ret_uri = &ret_request_line->uri;
	AVal* const ret_uri_host = &ret_request_line->uri_host;
	AVal* const ret_uri_hostname = &ret_request_line->uri_hostname;
	AVal* const ret_uri_suffix = &ret_request_line->uri_suffix;
	AVal* const ret_uri_extname = &ret_request_line->uri_suffix_extname;
	AVal* const ret_uri_query_string = &ret_request_line->uri_query_string;
	AVal* const ret_version = &ret_request_line->version;

	
	
	// get the method of http header
	ret_method->av_val = request_msg;
	ch_ptr = strchr(ret_method->av_val, ' ');
	if(ch_ptr){
		ret_method->av_len = ch_ptr - ret_method->av_val;

		// get the path of http header
		ret_uri->av_val = &ch_ptr[1]; // exclude the space
		ch_ptr = strchr(ret_uri->av_val, ' ');
		if(ch_ptr){
			ret_uri->av_len = ch_ptr - ret_uri->av_val;

			// get the version of http header
			ret_version->av_val = &ch_ptr[1]; // exclude the space
			if(0 == strncasecmp(ret_version->av_val, "HTTP/", 5)){
				ret_version->av_val += 5;
			}else{
				// error version format
				HTTPD_TRACE("Parse HTTP version error!");//HTTPD_TRACE("version:%s, request:%s\n", ret_version->av_val, request_msg);
				return -1;
			}
			ret_version->av_len = strcspn(ret_version->av_val, "\r\n"); // end with \r\n

			// host / suffix analysis
			ret_uri_host->av_val = ret_uri->av_val;
			ret_uri_host->av_len = 0;
			ret_uri_suffix->av_val = ret_uri_host->av_val;
			ret_uri_suffix->av_len = 0;
			if('/' == ret_uri->av_val[0]){
				// simple suffix
				// no host and no suffix
				ret_uri_suffix->av_val = ret_uri_host->av_val;
				ret_uri_suffix->av_len = strcspn(ret_uri_suffix->av_val, " ?");
			}else if(0 == strncmp(ret_uri->av_val, "http://", strlen("http://"))){
				ret_uri_host->av_val = &ret_uri->av_val[strlen("http://")];
				ret_uri_host->av_len = strcspn(ret_uri_host->av_val, "/");
				ret_uri_suffix->av_val = &ret_uri_host->av_val[ret_uri_host->av_len]; // including '/'
				ret_uri_suffix->av_len = strcspn(ret_uri_suffix->av_val, " ?");
			}else{
				// i dont know what i could do
			}

			if(0 == ret_uri_host->av_len){
				if(0 != http_read_header(request_msg, "host", ret_uri_host)){
					// FIXME:
				}
			}
			*ret_uri_hostname = *ret_uri_host;
			if(ret_uri_hostname->av_len > 0){
				ch_ptr = strpbrk(ret_uri_hostname->av_val, ":/\r\n");
				if(NULL != ch_ptr && ':' == *ch_ptr){
					// with the port info
					ret_uri_hostname->av_len = ch_ptr - ret_uri_hostname->av_val;
				}
			}

			// suffix ext
			ret_uri_extname->av_val = ret_uri_suffix->av_val;
			ret_uri_extname->av_len = 0;
			if(ret_uri_suffix->av_len > 0){
				int off_ext = strcspn(ret_uri_suffix->av_val, ". ");
				if(off_ext > 0 && '.' == ret_uri_suffix->av_val[off_ext]){
					ret_uri_extname->av_val = &ret_uri_suffix->av_val[off_ext + 1];
					ret_uri_extname->av_len = strcspn(ret_uri_extname->av_val, " ?\r\n");
				}
			}
			
			// query string analysis
			ret_uri_query_string->av_val = &ret_uri_suffix->av_val[ret_uri_suffix->av_len];
			ret_uri_query_string->av_len = 0; // init
			if('?' == ret_uri_query_string->av_val[0]){
				// with a query string
				ret_uri_query_string->av_val += 1; // excluding '?'
				ret_uri_query_string->av_len = strcspn(ret_uri_query_string->av_val, " ");
			}
			return 0;
		}
	}
	return -1;
}

int http_parse_general_header(const char* request_msg, HTTP_GENERAL_HEADER_SET_t* ret_header)
{
	http_read_header(request_msg, "Cache-Control", &ret_header->cache_control);
	http_read_header(request_msg, "Connection", &ret_header->connection);
	http_read_header(request_msg, "Date", &ret_header->date);
	http_read_header(request_msg, "Pragma", &ret_header->pragma);
	http_read_header(request_msg, "Triler", &ret_header->triler);
	http_read_header(request_msg, "Transfe-Encoding", &ret_header->transfe_encoding);
	http_read_header(request_msg, "Upgrade", &ret_header->upgrade);
	http_read_header(request_msg, "Via", &ret_header->via);
	http_read_header(request_msg, "Warning", &ret_header->warning);
	return 0;
}

int http_parse_request_header(const char* request_msg, HTTP_REQUEST_HEADER_SET_t* ret_header)
{
	http_read_header(request_msg, "Accept", &ret_header->accept);
	http_read_header(request_msg, "Accept-Charset", &ret_header->accept_charset);
	http_read_header(request_msg, "Accept-Encoding", &ret_header->accept_encoding);
	http_read_header(request_msg, "Accept-Language", &ret_header->accept_language);
	http_read_header(request_msg, "Authorization", &ret_header->authorization);
	http_read_header(request_msg, "cookie", &ret_header->cookie);
	http_read_header(request_msg, "Expect", &ret_header->expect);
	http_read_header(request_msg, "From", &ret_header->from);
	http_read_header(request_msg, "Host", &ret_header->host);
	http_read_header(request_msg, "If-Match", &ret_header->if_match);
	http_read_header(request_msg, "If-Modified-Since", &ret_header->if_modified_since);
	http_read_header(request_msg, "If-None-Match", &ret_header->if_none_match);
	http_read_header(request_msg, "If-Range", &ret_header->if_range);
	http_read_header(request_msg, "If -Unmodified-Since", &ret_header->if_unmodified_since);
	http_read_header(request_msg, "Max-Forwards", &ret_header->max_forwards);
	http_read_header(request_msg, "Proxy-Authorization", &ret_header->proxy_authorization);
	http_read_header(request_msg, "Range", &ret_header->range);
	http_read_header(request_msg, "Referer", &ret_header->referer);
	http_read_header(request_msg, "TE", &ret_header->te);
	http_read_header(request_msg, "User-Agent", &ret_header->user_agent);
	return 0;
}

int http_parse_entity_header(const char* request_msg, HTTP_ENTITY_HEADER_SET_t* ret_header)
{
	http_read_header(request_msg, "Allow", &ret_header->allow);
	http_read_header(request_msg, "Content-Encoding", &ret_header->content_encoding);
	http_read_header(request_msg, "Content-Language", &ret_header->content_language);
	http_read_header(request_msg, "Content-Length", &ret_header->content_length);
	http_read_header(request_msg, "Content-Location", &ret_header->content_location);
	http_read_header(request_msg, "Content-MD5", &ret_header->content_md5);
	http_read_header(request_msg, "Content-Range", &ret_header->content_range);
	http_read_header(request_msg, "Expires", &ret_header->expires);
	http_read_header(request_msg, "Last_Modified", &ret_header->last_modified);
	http_read_header(request_msg, "Warning", &ret_header->warning);
	return 0;
}



#define NON_NUM '0'
static char char2num(char ch){
	if(ch>='0' && ch<='9') return (char)(ch-'0');
	if(ch>='a' && ch<='f') return (char)(ch-'a'+10);
	if(ch>='A' && ch<='F') return (char)(ch-'A'+10);
	return NON_NUM;
}

int http_url_encode(const char* in_str, ssize_t const in_size, char* out_str, ssize_t const out_size)
{
	int i;
	int j = 0; /* for out_str index */
	char ch;

	if ((in_str == NULL) || (out_str == NULL) || (in_size <= 0) || (out_size <= 0)) {
		return 0;
	}

	for (i=0; (i<in_size) && (j<out_size); i++) {
		ch = in_str[i];
		if ((ch >= 'A') && (ch <= 'Z')) {
			out_str[j++] = ch;
		} else if ((ch >= 'a') && (ch <= 'z')) {
			out_str[j++] = ch;
		} else if ((ch >= '0') && (ch <= '9')) {
			out_str[j++] = ch;
		} else if(ch == ' '){
			out_str[j++] = '+';
		} else {
			if (j + 3 < out_size) {
				sprintf(out_str+j, "%%%02X", (unsigned char)ch);
				j += 3;
			} else {
				return 0;
			}
		}
	}

	out_str[j] = '\0';
	return j;
}

int http_url_decode(const char* in_str, ssize_t const in_size, char* out_str, ssize_t const out_size)
{
	char ch, ch1, ch2;
	int i;
	int j = 0; /* for out_str index */

	if ((in_str == NULL) || (out_str == NULL) || (in_size <= 0) || (out_size <= 0)) {
		return 0;
	}

	for (i=0; (i<in_size) && (j<out_size); i++) {
		ch = in_str[i];
		switch (ch) {
			case '+':
			out_str[j++] = ' ';
			break;

			case '%':
			if (i+2 < in_size) {
				ch1 = char2num(in_str[i+1]);
				ch2 = char2num(in_str[i+2]);
				if ((ch1 != NON_NUM) && (ch2 != NON_NUM)) {
					out_str[j++] = (char)((ch1<<4) | ch2);

					i += 2;
					break;
				}
			}

			/* goto default */
			default:
			out_str[j++] = ch;
			break;
		}
	}

	out_str[j] = '\0';
	return j;
}

int http_read_query_string(const char* query_string, const char* key, AVal* ret_val)
{
	if(key && strlen(key) > 0 && ret_val){
		char* key_with_equal = alloca(strlen(key) + 2);
		char* str_ptr = NULL;
		// reset the value first
		ret_val->av_val = NULL;
		ret_val->av_len = 0;
		sprintf(key_with_equal, "%s=%c", key, '\0');
		str_ptr = strcasestr(query_string, key_with_equal);
		if(str_ptr){
			char former_ch = *(str_ptr - 1);
			if('?' == former_ch || '&' == former_ch || str_ptr == query_string){
				ret_val->av_val = str_ptr + strlen(key_with_equal);
				ret_val->av_len = strcspn(ret_val->av_val, "&\r\n");
				return 0;
			}
		}
	}
	return -1;
}





