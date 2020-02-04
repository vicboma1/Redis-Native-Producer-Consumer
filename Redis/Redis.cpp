#include "pch.h"
#include <iostream>
#include <Windows.h>
#include "hiredis.h"
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <process.h>


//http://antsnote.club/2019/02/16/Redis-Redis%E5%88%97%E8%A1%A8/

constexpr char *hostname = "127.0.0.1";
constexpr int port = 6379;

redisReply *reply;
redisContext *c;

char* getFormattedTime(void) {
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	static char _retval[20];
	strftime(_retval, sizeof(_retval), "%Y-%m-%d_%H-%M-%S", timeinfo);

	return _retval;
}

void clearScreen(const char* str) {
	system("@cls||clear");
	printf("%s \n", str);
}

void printHelp() {
	printf("\nArgs: Redis {producer or consumer}");
}

void printArguments(int argc, char **argv) {
	printf("Have %d arguments: \n", argc);
	for (int i = 0; i < argc; ++i)
		printf("[%d]=%s\n", i, argv[i]);
}

void producer(void *param) {
	unsigned int j = 0;

	while (true) {
		clearScreen("PRODUCER \n");

		char buf[20];
		snprintf(buf, 20, "%s", getFormattedTime());
		reply = (redisReply*)redisCommand(c, "RPUSH list element-%s", buf);
		printf("RPUSH list element-%s\n", buf);
		freeReplyObject(reply);

		reply = (redisReply*)redisCommand(c, "LLEN list");
		printf("LLEN list: %lu\n",reply->integer);
		freeReplyObject(reply);

		reply = (redisReply*)redisCommand(c, "LRANGE list 0 -1");
		printf("LRANGE list");
		if (reply->type == REDIS_REPLY_ARRAY) {
			for (j = 0; j < reply->elements; j++) {
				printf("%s ",reply->element[j]->str);
			}
		}

		freeReplyObject(reply);
		Sleep(30);
	}
}

void consumer(void *param) {
	
	while (true) {
		clearScreen("CONSUMER \n");

		reply = (redisReply*)redisCommand(c, "LLEN list");
		unsigned int size = reply->integer;
		printf("LLEN list: %lu\n", size);
		freeReplyObject(reply);
		if (size > 0) {
			reply = (redisReply*)redisCommand(c, "BLPOP list 0");
			printf("BLPOP Element: \n");
			if (reply->type == REDIS_REPLY_ARRAY) {
				for (unsigned int j = 0; j < reply->elements; j++) {
					printf("%s ", reply->element[j]->str);
				}
			}
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "LRANGE list 0 -1");
			printf("LRANGE list: \n");
			if (reply->type == REDIS_REPLY_ARRAY) {
				for (unsigned int j = 0; j < reply->elements; j++) {
					printf("%s ", reply->element[j]->str);
				}
			}
			freeReplyObject(reply);

		}

		Sleep(27);
	}

}



void connection() {
	struct timeval timeout = { 2, 500000 }; // 2.5 segundos
	c = redisConnectWithTimeout(hostname, port, timeout);
	if (c == NULL || c->err) {
		if (c) {
			printf("Connection error: %s\n", c->errstr);
			redisFree(c);
		}
		else 
			printf("Connection error: can't allocate redis context\n");
		
		exit(1);
	}
}

void threadRun(_In_ _beginthread_proc_type _StartAddress) {
	HANDLE hThread;
	hThread = (HANDLE)_beginthread(_StartAddress, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
}

int main(int argc, char **argv) {

	printArguments(argc,argv);
	connection();
	
	if (argc == 2) {
		if (strcmp(argv[1], "producer") == 0)
			threadRun(producer);
		else 
			if (strcmp(argv[1], "consumer") == 0) 
				threadRun(consumer);
	}
	else
		if (argc == 1) {

			clearScreen("REDIS -> EXAMPLE.C \n");

			/* Example.c*/
			reply = (redisReply*)redisCommand(c, "PING");
			printf("PING: %s\n", reply->str);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "SET %s %s", "foo", "hello world");
			printf("SET: %s\n", reply->str);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "SET %b %b", "bar", (size_t)3, "hello", (size_t)5);
			printf("SET (binary API): %s\n", reply->str);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "GET foo");
			printf("GET foo: %s\n", reply->str);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "GET bar");
			printf("GET bar: %s\n", reply->str);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "GET hello");
			printf("GET hello: %s\n", reply->str);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "INCR counter");
			printf("INCR counter: %lld\n", reply->integer);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "INCR counter");
			printf("INCR counter: %lld\n", reply->integer);
			freeReplyObject(reply);

			reply = (redisReply*)redisCommand(c, "DEL mylist");
			printf("DEL mylist: %lld\n", reply->elements);
			freeReplyObject(reply);
			for (unsigned int j = 0; j < 10; j++) {
				char buf[64];

				snprintf(buf, 64, "%d", j);
				printf("mylist LPUSH element[%lu]-%s\n", j, buf);
				reply = (redisReply*)redisCommand(c, "LPUSH mylist element-%s", buf);
				freeReplyObject(reply);
			}

			reply = (redisReply*)redisCommand(c, "LRANGE mylist 0 -1");
			if (reply->type == REDIS_REPLY_ARRAY) {
				for (unsigned int j = 0; j < reply->elements; j++) {
					printf("%u) %s\n", j, reply->element[j]->str);
				}
			}
			freeReplyObject(reply);

			/* Disconnects and frees the context */
			redisFree(c);
		}
	else 
			printHelp();
	
	return 0;
}