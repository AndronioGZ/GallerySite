#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <fstream>

using namespace std;

#define HTTP_HOST "Host: "
#define HTTP_CONTENT_TYPE "Content-Type: "
#define HTTP_CONTENT_LENGTH "Content-Length: "
#define HTTP_CACHE_CONTROL "Cache-Control: "
#define HTTP_CONNECTION "Connection: "

class HTTPBuf{
	private:
		string m_sbuf;
		string m_boundary;
		string m_filename;
		//string m_body;
		//string m_scrollpreview;
		char *m_buf;
		int m_size;
		int m_page;
		bool m_binary;
		bool m_connection;
		bool m_is_auth;
		bool m_upload;
	public:
		bool GetConnection();
		int GetPage();
		void SetPage(int page);
		bool GetUpload();
		void SetUpload(bool upload);
		bool HandleReqwest(char reqstm[1024*1024*8], int size, bool is_auth);
		void AddHeader(string header);
		void AddHeader(char *header_type, int value);
		void AddHeader(char *header_type, string value);
		char * GetBuf();
		int GetSize();
		void AddContent(char* file);
		void AddContentBinary(char* file);
		void AddContentStr(string content);
		void SetError(int error);
		void ClearBuf();
		string GetBoundary();
		string GetFileName();
		void SetFileName(string filename);
		void SetBoundary(string boundary);
		int FindStrIn(char* buf, string str, int pos1, int pos2);
		HTTPBuf();
		HTTPBuf(bool is_upload, string boundary);
		HTTPBuf(int error);
		HTTPBuf(char reqstm[1024*1024*8], int size, bool is_auth);
		~HTTPBuf();
};
