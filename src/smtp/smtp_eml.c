
#include "smtp_debug.h"
#include "smtp_eml.h"
#include "base64.h"
#include "generic.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#define SMTP_EML_BOUNDARY  "=======JUAN_SMTP_V1_0======="

#define SMTP_EML_SUBJECT_TEMPLATE \
	"Subject:%s\r\n" \
	"MIME-Version:1.0;\r\n" \
	"Content-Type:multipart/mixed;boundary=\""SMTP_EML_BOUNDARY"\"\r\n" \
	"Content-Transfer-Encoding:7bit\r\n" \
	"\r\n" \


#define SMTP_EML_BODY_TEMPLATE \
	"--"SMTP_EML_BOUNDARY"\r\n" \
	"Content-Type: text/html;charset=\"gb2312\"\r\n\r\n" \
	"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">" \
	"<HTML>" \
	"<HEAD>" \
	"<META http-equiv=Content-Type content=\"text/html; charset=gb2312\">" \
	"<META content=\"MSHTML 6.00.2900.6266\" name=GENERATOR>" \
	"<STYLE>" \
	".border { border-radius:5px; background-color:#456654; text-align:center}" \
	"</STYLE>" \
	"</HEAD>" \
	"<BODY bgColor=#ffffff>" \
	"<DIV>" \
	"<FONT size=2>" \
	"%s" \
	"<br>" \
	"<IMG alt="" hspace=0 src=\"cid:ABCDEF0123456\" align=baseline border=0/>" \
	"</FONT>" \
	"</DIV>" \
	"</BODY>" \
	"</HTML>\r\n" \
	"\r\n" \


// with a content-type, file name, and attachment data with base64 encode
#define SMTP_EML_ATTACHMENT_TEMPLATE \
	"--"SMTP_EML_BOUNDARY"\r\n" \
	"Content-Type: %s;name=\"%s\"\r\n" \
	"Content-Transfer-Encoding:base64\r\n" \
	"Content-ID: <ABCDEF0123456>\r\n\r\n" \
	"%s\r\n" \
	"\r\n" \


#define SMTP_EML_ENDMARK "--\r\n\r\n.\r\n"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

static int smtp_eml_set_attachment(void* const buf, ssize_t buf_size, const char* attach_file, const char* file_name)
{
	FILE *fid = NULL;
	fid = fopen(attach_file, "rb");
	if(fid > 0){
		int ret = -1;
		ssize_t file_size = -1;
		void* file_origin = NULL;
		char* file_base64 = NULL;

		// set a default attach file name
		if(!file_name){
			file_name = basename(attach_file);
		}
		
		// get file size
		fseek(fid, 0, SEEK_END);
		file_size = ftell(fid);
		assert(file_size < 256 * 1024);
		fseek(fid, 0, SEEK_SET); // back to the beginning
		SMTP_TRACE("attach file size = %d", file_size);

		// alloc memory
		if(file_size > 32 * 1024){
			// more than 32k, alloc the memory on heap
			file_origin = calloc(file_size, 1);
			file_base64 = calloc(file_size * 3 / 2, 1);
		}else{
			file_origin = alloca(file_size);
			file_base64 = alloca(file_size * 3 / 2);
		}
		
		// load the file
		ret = fread(file_origin, 1, file_size, fid);
		assert(ret == file_size);
		// base64 encode attach file
		base64_encode(file_origin, file_base64, file_size);

		// make attach packet
		ret = snprintf(buf, buf_size,
			"--"SMTP_EML_BOUNDARY"\r\n"
			"Content-Type: %s;name=\"%s\"\r\n"
			"Content-Transfer-Encoding:base64\r\n"
			"Content-ID: <ABCDEF0123456>\r\n\r\n"
			"%s\r\n"
			"\r\n",
			SMTP_eml_attachment_mime(file_name),
			file_name,
			file_base64);

		if(file_size > 32 * 1024){
			free(file_origin);
			free(file_base64);
		}

		fclose(fid);
		fid = NULL;

		if(ret < buf_size){
			return 0;
		}
	}
	return -1;
}

const char* SMTP_eml_attachment_mime(const char* attach_file)
{
	if(strstr(attach_file, ".jpg") || strstr(attach_file, ".jpeg")){
		return "image/jpg";
	}else if(strstr(attach_file, ".html")){
		return "text/plain";
	}else if(strstr(attach_file, ".txt")){
		return "text/html";
	}
	return "";
}

ssize_t SMTP_eml_estimate(const char* subject, const char* body, const char* attach_file)
{
	ssize_t eml_size = 0;
	// calc mail size
	eml_size += strlen(SMTP_EML_SUBJECT_TEMPLATE) + strlen(subject);
	eml_size += strlen(SMTP_EML_BODY_TEMPLATE) + strlen(body);
	if(attach_file){
		ssize_t file_size = 0;
		GET_FILE_SIZE(attach_file, file_size);
		if(file_size > 0){
			const char* file_bname = strdupa(basename(attach_file));
			eml_size += strlen(SMTP_EML_ATTACHMENT_TEMPLATE) + \
				strlen(SMTP_eml_attachment_mime(file_bname)) + \
				strlen(file_bname) + \
				file_size * 2;
		}
	}
	eml_size += strlen(SMTP_EML_ENDMARK);
	return eml_size;
}

int SMTP_eml_make(char* const buf, ssize_t buf_size, const char* subject, const char* body, const char* attach_file, const char* file_name)
{
	int ret = 0;
	char* eml_offset = NULL;

	// put subject
	eml_offset = buf;
	ret = sprintf(eml_offset, SMTP_EML_SUBJECT_TEMPLATE, subject);
	// put body
	eml_offset = buf + strlen(buf);
	ret = sprintf(eml_offset, SMTP_EML_BODY_TEMPLATE, body);
	// put attachment
	if(attach_file){
		eml_offset = buf + strlen(buf);
		ret = smtp_eml_set_attachment(eml_offset, buf_size - (eml_offset - buf), attach_file, file_name);
	}
	// put end mark
	eml_offset = buf + strlen(buf);
	strcpy(eml_offset, SMTP_EML_ENDMARK);

	// printf out to check
//	printf("%s", buf);

	return 0;
}


