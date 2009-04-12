#ifndef SHTTPD_TYPE_H
#define SHTTPD_TYPE_H

#define HTTP_OK_ ENCODE_("HTTP/1.1 200 OK\r\n")

#define HTTP_NOT_FOUND_ ENCODE_("HTTP/1.1 404 NOT FOUND\r\n")

#define HTTP_TYPE_ ENCODE_("Content-Type:%S\r\n")

#define NEW_LINE_ ENCODE_("\r\n")

typedef enum request_type
{
	GET=0,
	POST=1
}REQUEST_TYPE;

typedef struct http_request
{
	REQUEST_TYPE type;
	C_STRING path;

	C_STRING host;
	BOOL_ alive;
	
	/*common cgi var*/
	C_STRING accept;
	C_STRING accept_charset;
	C_STRING accept_encoding;
	C_STRING accept_language;
	C_STRING cookies;
	C_STRING referer;
	C_STRING user_agent;
	C_STRING query_string;
	C_STRING remote_addr;

	/*maybe post data?*/
	C_STRING content_type;
	C_STRING content_length;
}HTTP_REQUEST;

#endif
