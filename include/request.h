#ifndef REQUEST_H_
#define REQUEST_H_

typedef struct Request {
	unsigned int request;
	void *args[5];
	int response;
} Request;

#endif
