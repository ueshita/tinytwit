
#ifndef _TINY_TWIT_H_
#define _TINY_TWIT_H_

// Twitter Acount Instance Structure
typedef struct TwitAcount_ {
	char c_key[256];
	char c_secret[256];
	char t_key[256];
	char t_secret[256];
} TwitAcount;

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Library
void twit_initialize(void);

// Finalize Library
void twit_finalize(void);

// Setup for account instance
// args:
//  c_key:    Consumer Key
//  c_secret: Consumer Secret
//  t_key:    Access Token Key (NULL OK)
//  t_secret: Access Token Secret (NULL OK)
// return:
//  1:SUCESS, 0:FAIL
int twit_setup(TwitAcount *twit, 
	const char *c_key, const char *c_secret, 
	const char *t_key, const char *t_secret);

// Signin and get access tokens
// args:
//  twit: Acount instance
//  inputpin: PIN Input callback function
//  userptr:  PIN Input callback user pointer
// return:
//  1:SUCESS, 0:FAIL
int twit_request_token(TwitAcount *twit, 
	int (*inputpin)(void *userptr, const char *url, char *pin), void *userptr);

// Tweet
// args:
//  twit: Acount instance
//  text: Tweet message(UTF8)
// return:
//  1:SUCESS, 0:FAIL
int twit_tweet(TwitAcount *twit, const char *text);

// Tweet with image data
// args:
//  twit: Acount instance
//  text: Tweet message (UTF8)
//  filename: Image file name
//  data: Image data (PNG, JPG and GIF)
//  size: Image data size (bytes)
// return:
//  1:SUCESS, 0:FAIL
int twit_tweet_with_media(TwitAcount *twit, 
	const char *text, const char *filename, void *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif	/* _TINY_TWIT_H_ */
