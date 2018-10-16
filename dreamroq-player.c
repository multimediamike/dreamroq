/*
* Dreamroq by Mike Melanson
* Updated by Josh Pearson to add audio support
*
* This is the sample Dreamcast player app, designed to be run under
* the KallistiOS operating system.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dc/maple.h>
#include <dc/maple/controller.h>
#include <kos/mutex.h>
#include <kos/thread.h>

#include "dreamroqlib.h"

#include "pvr_driver.h"
#include "snddrv.h"

/* Audio Global variables */
#define PCM_BUF_SIZE 1024*1024
static unsigned char *pcm_buf = NULL;
static int pcm_size = 0;
static int audio_init = 0;
static mutex_t * pcm_mut;
static long samples_done = 0;

/* Video Global variables */
static int graphics_initialized = 0;
static int frame=0;
static kthread_t * render_thread;        /* Video thread */
static volatile int render_thd = 0;      /* Video thread status */
static const float VIDEO_RATE = 30.0f;   /* Video FPS */

/* PVR Driver Handle */
static struct pvr_frame        roq_vram_ptr;
static struct pvr_frame_vertex roq_vram_vertex;

static float ATS = 0, VTS = 0; /* A/V Timestamps For Synchronization */

/* This is called by the AICA Sound Driver when it needs more samples */
void *audio_drv_cb ( snd_stream_hnd_t hnd, int pcm_needed, int * pcm_done )
{
    /* Wait for RoQ Decoder to produce enough samples */
    while( pcm_size < pcm_needed )
        thd_pass();

    /* Copy the Requested PCM Samples to the AICA Driver */
    mutex_lock( pcm_mut );
    memcpy( snddrv.pcm_buffer, pcm_buf, pcm_needed );
    pcm_size -= pcm_needed;
    memmove( pcm_buf, pcm_buf+pcm_needed, pcm_size );
    mutex_unlock( pcm_mut );

    samples_done += pcm_needed; /* Record the Audio Time Stamp */
    ATS = (samples_done/(double)snddrv.rate)/((double)snddrv.channels*2.0);

    snddrv.pcm_ptr = snddrv.pcm_buffer;
    *pcm_done = pcm_needed;

    return snddrv.pcm_ptr; /* Return the requested samples to the AICA driver */
}

static int audio_cb( unsigned char *buf, int size, int channels)
{
    if(!audio_init)
    {
        /* allocate PCM buffer */
        pcm_buf = malloc(PCM_BUF_SIZE);
        if( pcm_buf == NULL )
            return ROQ_NO_MEMORY;

        /* Start AICA Driver */
        snddrv_start_cb( 22050, channels, audio_drv_cb );
        snddrv.dec_status = SNDDEC_STATUS_STREAMING;

        /* Create a mutex to handle the double-threaded buffer */
        mutex_init(pcm_mut, MUTEX_TYPE_NORMAL);


        audio_init=1;
    }

    /* Copy the decoded PCM samples to our local PCM buffer */
    mutex_lock( pcm_mut );
    memcpy(pcm_buf+pcm_size, buf, size);
    pcm_size += size;
    mutex_unlock( pcm_mut );

    return ROQ_SUCCESS;
}

static void *video_thd(void *ptr)
{
    render_thd=1;  /* Signal Thread is active */

    /* Match the Auido and Video Time Stamps */
    VTS = ++frame / VIDEO_RATE;
    while( ATS < VTS ) thd_pass();

    /* Draw the frame using the PVR */
    pvr_draw_frame_dma(&roq_vram_ptr);
    printf("Rendered Frame %u\n", frame);
    render_thd=0;   /* Signal Thread is finished */
}

static int render_cb(unsigned short *buf, int width, int height, int stride,
    int texture_height)
{
     /* on first call, initialize textures and drawing coordinates */
    if (!graphics_initialized)
    {
        /* Allocate VRAM for current texture */
        pvr_malloc( &roq_vram_ptr, width, height );

        /* Compile the PVR Driver Handle */
        //pvr_resize_resolution( roq_vram_ptr, &roq_vram_vertex );
        pvr_set_resolution( roq_vram_ptr, 0, 0, 640, 480, &roq_vram_vertex );
        pvr_compile_poly(roq_vram_vertex, &roq_vram_ptr,
           PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, 10.0f);

        graphics_initialized = 1;
    }

    /* Wait for last frame to finish render */
    while(render_thd)
       thd_pass();

    /* Current decoded frame */
    pvr_dma_load( (unsigned char *)buf, &roq_vram_ptr);

    /* Create a thread to render the current frame */
    //Try 1 onr 0 for first arg
    render_thread = thd_create(0, video_thd, NULL);

    return ROQ_SUCCESS;
}

maple_device_t  *cont;     //Controller
cont_state_t    *state;    //State of inputs
static int quit_cb()
{
    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    /* check controller state */
    state = maple_dev_status(cont);

    // If the state/controller is unavailable
    if(!state) {
        printf("Error reading controller\n");
        return(0);
    }

    state->buttons = ~state->buttons;
    return (state->buttons & CONT_START);
}

int main()
{
    int status=0;

    vid_set_mode(DM_640x480, PM_RGB565);
    pvr_init_defaults();
    pvr_dma_init();

    printf("dreamroq_play(C) Multimedia Mike Melanson & Josh PH3NOM Pearson 2011\n");

    /* To disable a callback, simply replace the function name by 0 */
    status = dreamroq_play("/cd/sample.roq", 1, render_cb, 0, 0);

    printf("dreamroq_play() status = %d\n", status);

    if(audio_init)
    {
      free( pcm_buf );
      pcm_buf = NULL;
      pcm_size = 0;
      samples_done = 0;
      mutex_destroy(pcm_mut);                  /* Destroy the PCM mutex */
      snddrv_exit();                           /* Exit the AICA Driver */
    }

    if(graphics_initialized)
    {
        pvr_free( &roq_vram_ptr );             /* Free the PVR memory */
    }

    pvr_dma_shutdown();

    return 0;
}
