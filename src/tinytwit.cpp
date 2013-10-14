#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include "oauth.h"
#include "socket.h"
#include "tinytwit.h"

static const char *url_request_token = "http://api.twitter.com/oauth/request_token";
static const char *url_authorize = "http://api.twitter.com/oauth/authorize";
static const char *uri_access_token = "http://api.twitter.com/oauth/access_token";
static const char *uri_update = "http://api.twitter.com/1.1/statuses/update.json";
static const char *uri_update_with_media = "http://api.twitter.com/1.1/statuses/update_with_media.json";

void twit_initialize(void)
{
	Socket::initialize();
}

void twit_finalize(void)
{
	Socket::finalize();
}

int twit_setup(TwitAcount *twit, 
	const char *c_key, const char *c_secret, 
	const char *t_key, const char *t_secret)
{
	strcpy(twit->c_key, c_key);
	strcpy(twit->c_secret, c_secret);
	if (t_key) {
		strcpy(twit->t_key, t_key);
	} else {
		strcpy(twit->t_key, "");
	}
	if (t_secret) {
		strcpy(twit->t_secret, t_secret);
	} else {
		strcpy(twit->t_secret, "");
	}

	return 1;
}

static bool parse_reply(const char *reply, char *token, char *secret)
{
	std::vector<std::string> arr;

	char const *end = reply + strlen(reply);
	char const *left = reply;
	char const *right = left;
	while (1) {
		int c = 0;
		if (right < end) {
			c = *right;
		}
		if (c == 0 || c == '&') {
			std::string str(left, right);
			arr.push_back(str);
			if (c == 0) {
				break;
			}
			right++;
			left = right;
		}
		right++;
	}

	char const *oauth_token = 0;
	char const *oauth_token_secret = 0;

	for (std::vector<std::string>::const_iterator it = arr.begin(); it != arr.end(); it++) {
		if (strncmp(it->c_str(), "oauth_token=", 12) == 0) {
			oauth_token = it->c_str() + 12;
		} else if (strncmp(it->c_str(), "oauth_token_secret=", 19) == 0) {
			oauth_token_secret = it->c_str() + 19;
		}
	}

	if (oauth_token && oauth_token_secret) {
		if (token) {
			strcpy(token, oauth_token);
		}
		if (secret) {
			strcpy(secret, oauth_token_secret);
		}
		return true;
	}
	return false;
}

int twit_request_token(TwitAcount *twit, 
	int (*inputpin)(void *userptr, const char *url, char *pin), void *userptr)
{
	std::string req_url;
	std::string postarg;
	std::string reply;
	char t_key[256];
	char t_secret[256];
	
	req_url = oauth_sign_url2(url_request_token, 
		postarg, OA_HMAC, 0, twit->c_key, twit->c_secret, 0, 0);
	reply = oauth_http_get(req_url.c_str(), postarg.c_str());

	if (!parse_reply(reply.c_str(), t_key, t_secret)) {
		return 0;
	}
	
	req_url = oauth_sign_url2(url_authorize, std::string(), OA_HMAC, 0, 
		twit->c_key, twit->c_secret, t_key, t_secret);
	
	char pin[256] = "";
	if (!inputpin(userptr, req_url.c_str(), pin)) {
		return 0;
	}
	
	char url[1024];
	sprintf(url, "%s?oauth_verifier=%s", uri_access_token, pin);

	req_url = oauth_sign_url2(url, std::string(), OA_HMAC, 0, twit->c_key, 0, t_key, 0);
	reply = oauth_http_get(req_url.c_str(), postarg.c_str());

	if (!parse_reply(reply.c_str(), twit->t_key, twit->t_secret)) {
		return 0;
	}
	
	return 1;
}

int twit_tweet(TwitAcount *twit, const char *text)
{
	std::string postargs;
	std::string reply;

	postargs = std::string("status=") + text;
	std::vector<std::string> header;
	header.push_back(oauth_sign_header(uri_update, postargs, OA_HMAC, NULL,
		twit->c_key, twit->c_secret, twit->t_key, twit->t_secret));
	
	postargs = std::string("status=") + oauth_url_escape(text);
	reply = oauth_http_post_with_header(uri_update, 
		header, postargs.c_str(), postargs.size(), NULL);

	return !reply.empty();
}

int twit_tweet_with_media(TwitAcount *twit, 
	const char *text, const char *filename, void *data, size_t size)
{
	const char* boundary = "asdfghjkl";

	std::vector<std::string> header;
	header.push_back(oauth_sign_header(uri_update_with_media, 
		std::string(), OA_HMAC, "POST",
		twit->c_key, twit->c_secret, twit->t_key, twit->t_secret));
	
	std::vector<unsigned char> content;
	content.reserve(size + 2048);

	std::string boundary_line = std::string("--") + boundary + "\r\n";
	std::string sub;

	sub = boundary_line;
	sub += "Content-Disposition: form-data; name=\"status\"\r\n";
	sub += "Content-Type: text/plain; charset=UTF-8\r\n";
	sub += "\r\n";
	sub += text;
	sub += "\r\n";
	content.insert(content.end(), sub.begin(), sub.end());
	
	sub = boundary_line;
	sub += "Content-Disposition: form-data; name=\"media[]\"; filename=\"";
	sub += filename;
	sub += "\"\r\n";
	sub += "Content-Type: application/octet-stream\r\n";
	sub += "\r\n";
	content.insert(content.end(), sub.begin(), sub.end());
	content.insert(content.end(), (unsigned char*)data, (unsigned char*)data + size);
	
	sub = "\r\n";
	sub += std::string("--") + boundary + "--";
	content.insert(content.end(), sub.begin(), sub.end());
	
	char content_type[128];
	sprintf(content_type, "multipart/form-data; boundary=%s", boundary);
	
	std::string reply = oauth_http_post_with_header(uri_update_with_media, 
		header, &content[0], content.size(), content_type);

	return !reply.empty();
}
