#ifndef _H_NANO_RRINGBUFFER_H_
#define _H_NANO_RRINGBUFFER_H_

struct nano_ringbuffer_t {
	unsigned char *buffer;	/* the buffer holding the data */
	unsigned int size;	/* the size of the allocated buffer */
	unsigned int in;	/* data is added at offset (in % size) */
	unsigned int out;	/* data is extracted from off. (out % size) */
	pthread_mutex_t lock;	/* protects concurrent modifications */
};

unsigned int nano_ringbuffer_init(struct nano_ringbuffer_t *ringbuffer,unsigned int size);

void nano_ringbuffer_reset(struct nano_ringbuffer_t *ringbuffer);

int nano_ringbuffer_is_empty(struct nano_ringbuffer_t *ringbuffer);

int nano_ringbuffer_is_full(struct nano_ringbuffer_t *ringbuffer);

void nano_ringbuffer_free(struct nano_ringbuffer_t *ringbuffer);

unsigned int nano_ringbuffer_len(struct nano_ringbuffer_t *ringbuffer);

int nano_ringbuffer_put(struct nano_ringbuffer_t *ringbuffer,const unsigned char *buffer, unsigned int len);

int nano_ringbuffer_get(struct nano_ringbuffer_t *ringbuffer,unsigned char *buffer, unsigned int *len);

#endif
