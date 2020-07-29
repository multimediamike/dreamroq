/*
**
** (C) Josh 'PH3NOM' Pearson 2011
**
*/
/*
** To anyone looking at this code:
**
** This driver runs in its own thread on the sh4.
**
** When the AICA driver requests more samples,
** it will signal sndbuf_status=SNDDRV_STATUS_NEEDBUF
** and assign the number of requested samples to snddrv.pcm_needed.
**
** The decoders need to check sndbuf_status,
** when more samples are requested by the driver ** the decoders will loop
** decoding into pcm_buffer untill pcm_bytes==snddrv.pcm_needed
** at that point the decoder signals sndbuf_status=SNDDRV_STATUS_HAVEBUF
**
*/
/*
**
** August 2012
** This module has been updated to call an external callback, rather than
** using the internal snddrv_callback.  Use the function snddrv_start_cb().
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <kos/thread.h>

#include "snddrv.h"

static snd_stream_hnd_t shnd;
static kthread_t * snddrv_thd;
static int snddrv_vol = 255;
static snddrv_cb *drv_callback = NULL;

/* Increase the Sound Driver volume */
int snddrv_volume_up()
{
    if( snddrv_vol <= 245 )
    {
        snddrv_vol += 10;
  	    snd_stream_volume(shnd, snddrv_vol);
    }
    return snddrv_vol;
}

/* Decrease the Sound Driver volume */
int snddrv_volume_down()
{
    if( snddrv_vol >= 10 )
    {
        snddrv_vol -= 10;
  	    snd_stream_volume(shnd, snddrv_vol);
    }
    return snddrv_vol;
}

/* Exit the Sound Driver */
int snddrv_exit()
{
    if( snddrv.drv_status != SNDDRV_STATUS_NULL ) {
        snddrv.drv_status = SNDDRV_STATUS_DONE;
        snddrv.buf_status = SNDDRV_STATUS_BUFEND;

        while( snddrv.drv_status != SNDDRV_STATUS_NULL )
          thd_pass();


        printf("SNDDRV: Exited\n");
    }

    memset( snddrv.pcm_buffer, 0, 65536+16384);
    snddrv.pcm_bytes = 0;
    snddrv.pcm_needed = 0;

    SNDDRV_FREE_STRUCT();

    return snddrv.drv_status;
}

/* Signal how many samples the AICA needs, then wait for the deocder to produce them */
static void *snddrv_callback(snd_stream_hnd_t hnd, int len, int * actual)
{
    /* Signal the Decoder thread how many more samples are needed */
    snddrv.pcm_needed = len;
    snddrv.buf_status = SNDDRV_STATUS_NEEDBUF;

    /* Wait for the samples to be ready */
    while( snddrv.buf_status != SNDDRV_STATUS_HAVEBUF && snddrv.buf_status != SNDDRV_STATUS_BUFEND )
           thd_pass();

    snddrv.pcm_ptr = snddrv.pcm_buffer;
    snddrv.pcm_bytes = 0;
    *actual = len;

    return snddrv.pcm_ptr;
}

void *snddrv_thread(void *ptr)
{
    printf("SNDDRV: Rate - %i, Channels - %i\n", snddrv.rate, snddrv.channels);

	shnd = snd_stream_alloc((void*)drv_callback, SND_STREAM_BUFFER_MAX/4);

    snd_stream_start(shnd, snddrv.rate, snddrv.channels-1);
    snddrv.drv_status = SNDDRV_STATUS_STREAMING;

	while( snddrv.drv_status != SNDDRV_STATUS_DONE && snddrv.drv_status != SNDDRV_STATUS_ERROR )
    {
		snd_stream_poll(shnd);
		thd_sleep(20);
	}

    snd_stream_destroy(shnd);
	snd_stream_shutdown();

    snddrv.drv_status = SNDDRV_STATUS_NULL;

    printf("SNDDRV: Finished\n");

}

/* Start the AICA Sound Stream Thread */
int snddrv_start( int rate, int chans )
{
    snddrv.rate = rate;
    snddrv.channels = chans;
    if( snddrv.channels > 2) {
        printf("SNDDRV: ERROR - Exceeds maximum channels\n");
        return -1;
    }

    printf("SNDDRV: Creating Driver Thread\n");

    drv_callback = (void *)snddrv_callback;

    snddrv.drv_status = SNDDRV_STATUS_INITIALIZING;

    snd_stream_init();

    //snddrv_thd = thd_create( snddrv_thread, NULL );
    snddrv_thd = thd_create(0, snddrv_thread, NULL );

    return snddrv.drv_status;
}

/* Start the AICA Sound Stream Thread */
int snddrv_start_cb( int rate, int chans, snddrv_cb cb )
{
    snddrv.rate = rate;
    snddrv.channels = chans;
    if( snddrv.channels > 2) {
        printf("SNDDRV: ERROR - Exceeds maximum channels\n");
        return -1;
    }

    printf("SNDDRV: Creating Driver Thread\n");

    drv_callback = (void*)cb;

    snddrv.drv_status = SNDDRV_STATUS_INITIALIZING;

    snd_stream_init();

    //snddrv_thd = thd_create( snddrv_thread, NULL );
    snddrv_thd = thd_create(0, snddrv_thread, NULL );

    return snddrv.drv_status;
}
