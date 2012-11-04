
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "tinytwit.h"

#pragma comment (lib, "ws2_32.lib")

const char* TWITTER_CKEY	= "xxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* TWITTER_CSECRET	= "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
const char* TWITTER_TKEY	= NULL;
const char* TWITTER_TSECRET	= NULL;

// PIN入力
static int input_pin(void *userobj, const char *url, char *pin)
{
	// ブラウザを開く
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOW);

	printf("PIN:");
	scanf("%s", pin);

	return 1;
}

int main(int argc, char **argv)
{
	TwitAcount twit;
	int result;

	// 初期化
	twit_initialize();

	// キーを登録
	twit_setup(&twit, TWITTER_CKEY, TWITTER_CSECRET, TWITTER_TKEY, TWITTER_TSECRET);

	// OAuth認証処理
	if (TWITTER_TKEY == NULL) {
		printf("Request token.\n");
		result = twit_request_token(&twit, input_pin, NULL);
		if (result == 0) {
			printf("failed.\n");
			twit_finalize();
			return -1;
		}
		printf("ok.\n");
		printf("Token key:    %s\n", twit.t_key);
		printf("Token Secret: %s\n", twit.t_secret);
		printf("\n");
	}

	// ツイート（日本語はUTF-8じゃないと失敗するので注意）
	char message[1024];
	printf("Tweet message:");
	scanf("%s", message);
	
	printf("Tweet.\n");
	result = twit_tweet(&twit, message);
	if (result == 0) {
		printf("failed.\n");
		twit_finalize();
		return -1;
	}
	printf("ok.\n");

	// 終了処理
	twit_finalize();
	
	return 0;
}
