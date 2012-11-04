
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

// PIN����
static int input_pin(void *userobj, const char *url, char *pin)
{
	// �u���E�U���J��
	ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOW);

	printf("PIN:");
	scanf("%s", pin);

	return 1;
}

int main(int argc, char **argv)
{
	TwitAcount twit;
	int result;

	// ������
	twit_initialize();

	// �L�[��o�^
	twit_setup(&twit, TWITTER_CKEY, TWITTER_CSECRET, TWITTER_TKEY, TWITTER_TSECRET);

	// OAuth�F�؏���
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

	// �c�C�[�g�i���{���UTF-8����Ȃ��Ǝ��s����̂Œ��Ӂj
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

	// �I������
	twit_finalize();
	
	return 0;
}
