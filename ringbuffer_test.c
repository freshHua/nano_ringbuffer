#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "nano_ringbuffer.h"


struct nano_ringbuffer_t s_ringbuffer;

int main(int argc, char*argv[]){
	unsigned char frame[8] = {"12345678"};
	unsigned char buffer[16]; 
	int iret ;
	unsigned int len;
	unsigned char a = 1 ;
	unsigned char b = 10 ;
	unsigned char c = b - a ;

	printf("Nano ringbuffer testing\n") ;

	nano_ringbuffer_init(&s_ringbuffer,16);
	iret = nano_ringbuffer_put(&s_ringbuffer, frame,sizeof(frame));
	printf("Ring buffer put %d used %d\n", len, nano_ringbuffer_len(&s_ringbuffer));
	len = sizeof(buffer);
	if( nano_ringbuffer_get(&s_ringbuffer, buffer, &len) >= 0){
		buffer[len] = '\0';
		printf(" Ring buffer get %d buffer %s\n", len, buffer);
	}
	return 0 ;
}
