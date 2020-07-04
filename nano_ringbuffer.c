#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "nano_ringbuffer.h"


/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })


static int is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

int nano_ringbuffer_init(struct nano_ringbuffer_t *ringbuffer, unsigned int size)
{
	/* size must be a power of 2 */
	assert(is_power_of_2(size));

	ringbuffer->buffer = malloc(size);
	if (!ringbuffer->buffer)
		return -1;

	ringbuffer->size = size;
	ringbuffer->in = ringbuffer->out = 0;

	pthread_mutex_init(&ringbuffer->lock, NULL);

	return size;
}

/**
 * __nano_ringbuffer_reset - removes the entire ringbuffer contents
 * @ringbuffer: the ringbuffer to be emptied.
 */
static void __nano_ringbuffer_reset(struct nano_ringbuffer_t *ringbuffer)
{
	ringbuffer->in = ringbuffer->out = 0;
}

/**
 * nano_ringbuffer_reset - removes the entire ringbuffer contents
 * @ringbuffer: the fifo to be emptied.
 */
void nano_ringbuffer_reset(struct nano_ringbuffer_t *ringbuffer)
{

	pthread_mutex_lock(&ringbuffer->lock);

	__nano_ringbuffer_reset(ringbuffer);

	pthread_mutex_unlock(&ringbuffer->lock);
}


int nano_ringbuffer_is_empty(struct nano_ringbuffer_t *ringbuffer){
	return ringbuffer->in == ringbuffer->out ;
}

static unsigned int ___nano_ringbuffe_size(struct nano_ringbuffer_t *ringbuffer)
{
	return ringbuffer->size;
}


/**
 * __nano_ringbuffer_len - returns the number of bytes available in the ringbuffer
 * @ringbuffer: the ringbuffer to be used.
 */
static unsigned int __nano_ringbuffer_len(struct nano_ringbuffer_t *ringbuffer)
{
	return ringbuffer->in - ringbuffer->out;
}

/**
 * __nano_ringbuffer_avail - returns the number of bytes available in the ringbuffer
 * @ringbuffer: the ringbuffer to be used.
 */
static unsigned int __nano_ringbuffer_avail(struct nano_ringbuffer_t *ringbuffer)
{
	return ___nano_ringbuffe_size(ringbuffer) - __nano_ringbuffer_len(ringbuffer);
}


int nano_ringbuffer_is_full(struct nano_ringbuffer_t *ringbuffer)
{
	return __nano_ringbuffer_len(ringbuffer) == ___nano_ringbuffe_size(ringbuffer);
}


void nano_ringbuffer_free(struct nano_ringbuffer_t *ringbuffer){
	free(ringbuffer->buffer);
}

/**
 * nano_ringbuffer_len - returns the number of bytes available in the ringbuffer
 * @ringbuffer: the ringbuffer to be used.
 */
unsigned int nano_ringbuffer_len(struct nano_ringbuffer_t *ringbuffer)
{
	unsigned int ret;

	pthread_mutex_lock(&ringbuffer->lock);

	ret = __nano_ringbuffer_len(ringbuffer);

	pthread_mutex_unlock(&ringbuffer->lock);

	return ret;
}

static int __nano_ringbuffer_put(struct nano_ringbuffer_t *ringbuffer,
			 const unsigned char *buffer, unsigned int len)
{
	unsigned int l;

	if(__nano_ringbuffer_avail(ringbuffer) < len){
		return -1 ;
	}

	len = min(len, ringbuffer->size - ringbuffer->in + ringbuffer->out);


	/* first put the data starting from fifo->in to buffer end */
	l = min(len, ringbuffer->size - (ringbuffer->in & (ringbuffer->size - 1)));
	memcpy(ringbuffer->buffer + (ringbuffer->in & (ringbuffer->size - 1)), buffer, l);

	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(ringbuffer->buffer, buffer + l, len - l);


	ringbuffer->in += len;

	return len;
}

int nano_ringbuffer_put(struct nano_ringbuffer_t *ringbuffer,
				     const unsigned char *buffer, unsigned int len)
{
	int ret;

	pthread_mutex_lock(&ringbuffer->lock);

	ret = __nano_ringbuffer_put(ringbuffer, buffer, len);

	pthread_mutex_unlock(&ringbuffer->lock);

	return ret;
}


static int __nano_ringbuffer_get(struct nano_ringbuffer_t *ringbuffer,
			 unsigned char *buffer, unsigned int *len)
{
	unsigned int l;

	if(ringbuffer->in == ringbuffer->out){
		return -1 ;
	}

	*len = min(*len, ringbuffer->in - ringbuffer->out);


	/* first get the data from fifo->out until the end of the buffer */
	l = min(*len, ringbuffer->size - (ringbuffer->out & (ringbuffer->size - 1)));
	memcpy(buffer, ringbuffer->buffer + (ringbuffer->out & (ringbuffer->size - 1)), l);

	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(buffer + l, ringbuffer->buffer, *len - l);

	ringbuffer->out += *len;

	/*
	 * optimization: if the FIFO is empty, set the indices to 0
	 * so we don't wrap the next time
	 */
	if (ringbuffer->in == ringbuffer->out)
		ringbuffer->in = ringbuffer->out = 0;


	return 0;
}

int nano_ringbuffer_get(struct nano_ringbuffer_t *ringbuffer,
				     unsigned char *buffer, unsigned int *len)
{
	int ret;

	pthread_mutex_lock(&ringbuffer->lock);

	ret = __nano_ringbuffer_get(ringbuffer, buffer, len);


	pthread_mutex_unlock(&ringbuffer->lock);

	return ret;
}
