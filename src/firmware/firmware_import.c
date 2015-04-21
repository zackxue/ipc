
#include "firmware.h"
#include "md5sum.h"

#define FIRMWARE_IMPORT_BUFFER_SZ (256 * 1024)

#define FIRMWARE_IMPORT_HTTP_FILE "/latest/FW_HI3507_JAN72.rom"

#define FIRMWARE_IMPORT_FROM_FOLDER_FLAG (1<<0)
#define FIRMWARE_IMPORT_FROM_FTP_FLAG (1<<1)
#define FIRMWARE_IMPORT_FROM_HTTP_FLAG (1<<2)

struct FwImportFromHttp
{
	ssize_t file_size;
	ssize_t recv_size;
};

typedef struct FwImport
{
	uint32_t flag;
	struct FwImportFromHttp from_http;
}FwImport_t;
static FwImport_t* _fw_import = NULL;

void FIRMWARE_import_clear()
{
	unlink(FIRMWARE_IMPORT_FILE);
	remove(FIRMWARE_IMPORT_FILE);
}

int FIRMWARE_import_get_rate()
{
	if(_fw_import->flag & FIRMWARE_IMPORT_FROM_FOLDER_FLAG){
		;
	}else if(_fw_import->flag & FIRMWARE_IMPORT_FROM_FTP_FLAG){
		;
	}else if(_fw_import->flag & FIRMWARE_IMPORT_FROM_HTTP_FLAG){
		if(0 == _fw_import->from_http.recv_size){
			return 0;
		}
		return _fw_import->from_http.recv_size * 100 / _fw_import->from_http.file_size;
	}
	return 0;
}

int FIRMWARE_import_from_folder(const char* folder, FIRMWARE_IMPORT_MATCH match)
{
	// FIXME:
	return -1;
}

int FIRMWARE_import_from_ftp(const char* ftp_addr, const char* user, const char* password)
{
	// FIXME:
	return -1;
}

int FIRMWARE_import_from_http(const char* addr, uint16_t port)
{
	printf("%s start\n", __FUNCTION__);
	int ret = 0;
	int reuse_on = 1;
	int sock = -1;
	char buf[FIRMWARE_IMPORT_BUFFER_SZ];
	char* buf_ptr = NULL;
	struct sockaddr_in server_addr;
	ssize_t buf_sz = 0;
	FILE* fid = NULL;
	const char* http_get =
		"GET " FIRMWARE_IMPORT_HTTP_FILE " HTTP/1.0\r\n"
		"Connection: close\r\n"
		"\r\n";
//
// 1. setup sock -> connect to sever -> send 'GET' to request file
// 2. recevie http response -> analyse the header and the the filesize
// 3. loop to receive the whole file
// 4. success to import a file from http server
//

	// setup flag
	_fw_import->flag |= FIRMWARE_IMPORT_FROM_HTTP_FLAG;
	memset(&_fw_import->from_http, 0, sizeof(_fw_import->from_http));

///////////////////////////////////////////////////////////////////////////////
// step 1
///////////////////////////////////////////////////////////////////////////////
	// create socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		goto FIRMWARE_import_from_http_err1;
	}
	// port reuse active
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(int));
	assert(0 == ret);

	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(port);
	ret = connect(sock,(struct sockaddr*)&server_addr, sizeof(server_addr));
	if(ret < 0){
		// connect to server failed
		goto FIRMWARE_import_from_http_err2;
	}

	ret = send(sock, http_get, strlen(http_get), 0);
	assert(ret > 0);

///////////////////////////////////////////////////////////////////////////////
// step 2
///////////////////////////////////////////////////////////////////////////////
	// receive file
	memset(buf, 0, sizeof(buf));
	ret = recv(sock, buf, sizeof(buf), 0);
	buf[ret] = 0;
	// file not found or other problems
	buf_ptr = strstr(buf, "200 OK\r\n");
	if(!buf_ptr){
		goto FIRMWARE_import_from_http_err2;
	}

	buf_ptr = strstr(buf, "Content-Length:");
	if(!buf_ptr){
		goto FIRMWARE_import_from_http_err2;
	}

//	sscanf(buf, "Content-Length: %d\r\n", &fw_sz);
	_fw_import->from_http.file_size = atoi(buf_ptr + strlen("Content-Length:"));
	if(_fw_import->from_http.file_size > FIRMWARE_max_rom_size()){
		// file is too large
		goto FIRMWARE_import_from_http_err2;
	}

	// seek the end of http header
	buf_ptr = strstr(buf, "\r\n\r\n");
	if(!buf_ptr){
		goto FIRMWARE_import_from_http_err2;
	}

///////////////////////////////////////////////////////////////////////////////
// step 3
///////////////////////////////////////////////////////////////////////////////
	// clear the recv size stats
	_fw_import->from_http.recv_size = 0;

	fid = fopen(FIRMWARE_IMPORT_FILE, "wb");
	assert(fid);
	buf_sz = ret - (buf_ptr + strlen("\r\n\r\n") - buf); // receive data size - header
	ret = fwrite(buf_ptr + strlen("\r\n\r\n"), 1, buf_sz, fid);
	assert(ret == buf_sz);
	_fw_import->from_http.recv_size += buf_sz;

	while(1){
		ret = recv(sock, buf, sizeof(buf), 0);
		if(ret < 0){
			goto FIRMWARE_import_from_http_err3;
		}
		if(0 == ret){
			break;
		}
		buf_sz = ret;
		ret = fwrite(buf, 1, buf_sz, fid);
		assert(ret == buf_sz);
		_fw_import->from_http.recv_size += buf_sz;

		printf("download: %d%% from %s\r\n", FIRMWARE_import_get_rate(), addr);
	}
	fclose(fid);
	fid = NULL;

	close(sock);
	sock = -1;

//	system("ls -l " FIRMWARE_IMPORT_FILE);
	// success
	_fw_import->flag &= ~FIRMWARE_IMPORT_FROM_HTTP_FLAG;
	printf("%s end\n", __FUNCTION__);
	return 0;

FIRMWARE_import_from_http_err3:
	printf("FIRMWARE_import_from_http_err3\n");
	fclose(fid);
	fid = NULL;
	FIRMWARE_import_clear();
FIRMWARE_import_from_http_err2:
	printf("FIRMWARE_import_from_http_err2\n");
	close(sock);
	sock = -1;
FIRMWARE_import_from_http_err1:
	printf("FIRMWARE_import_from_http_err1\n");
	_fw_import->flag &= ~FIRMWARE_IMPORT_FROM_HTTP_FLAG;
	return -1;
}

#define FIRMWARE_MD5_CHECK_TAG "MD5 CHECK PASSED QC LAW!"
uint32_t FIRMWARE_import_check()
{
	printf("%s start\n", __FUNCTION__);
	int ret = 0;
	FwHeader_t header;
	char md5sum[64] = {""};
	char* p_md5 = NULL;
	FILE* fid = NULL;

	fid = fopen(FIRMWARE_IMPORT_FILE, "r+");
	if(fid){
		// get the md5 from firmware file
		ret = fread(&header, 1, sizeof(header), fid);
		assert(sizeof(header) == ret);

		strcpy(md5sum, header.md5_sum);
		if(0 == strcmp(md5sum, FIRMWARE_MD5_CHECK_TAG) && strlen(md5sum) == strlen(FIRMWARE_MD5_CHECK_TAG)){
			// has been check
			fclose(fid);
			return true;
		}else{
			// first time to check
			memset(header.md5_sum, 0, sizeof(header.md5_sum));
			ret = fseek(fid, 0, SEEK_SET);
			assert(0 == ret);
			ret = fwrite(&header, 1, sizeof(header), fid);
			assert(sizeof(header) == ret);
			p_md5 = md5sum_file(FIRMWARE_IMPORT_FILE);
			if(0 == strcmp(md5sum, p_md5) && strlen(md5sum) == strlen(p_md5)){
				// passed
				strcpy(header.md5_sum, FIRMWARE_MD5_CHECK_TAG);
				ret = fseek(fid, 0, SEEK_SET);
				assert(0 == ret);
				ret = fwrite(&header, 1, sizeof(header), fid);
				assert(sizeof(header) == ret);
				fclose(fid);
				return FIRMWARE_import_check();
			}else{
				fclose(fid);
				printf("md5 mismatch %s/%s\r\n", p_md5, md5sum);
				return false;
			}
		}
	}
	return false;
}

int FIRMWARE_import_init()
{
	if(!_fw_import){
		_fw_import = calloc(sizeof(FwImport_t), 1);
		return 0;
	}
	return -1;
}

void FIRMWARE_import_destroy()
{
	if(_fw_import){
		free(_fw_import);
		_fw_import = NULL;
	}
}


