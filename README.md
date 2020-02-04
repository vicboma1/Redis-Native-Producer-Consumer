# Redis Native C/C++ | POF (non-optimize)

Connection
```c
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
```

Runnable
```c
void threadRun(_In_ _beginthread_proc_type _StartAddress) {
	HANDLE hThread;
	hThread = (HANDLE)_beginthread(_StartAddress, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
}
```

Producer
```c
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
```

Consumer
```c
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
```

# References
  * [Commands](http://antsnote.club/2019/02/16/Redis-Redis%E5%88%97%E8%A1%A8/)
