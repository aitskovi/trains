#ifndef REQUEST_H_
#define REQUEST_H_

typedef struct Request {
	unsigned int request;
	void *args[4];
	int response;
} Request;

#endif
