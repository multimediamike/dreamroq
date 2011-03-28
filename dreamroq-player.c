/*
 * Dreamroq by Mike Melanson
 *
 * This is the sample Dreamcast player app, designed to be run under
 * the KallistiOS operating system.
 */

#include "kos.h"

#include "dreamroqlib.h"

static pvr_ptr_t textures[2];
static int current_frame = 0;

static int render_cb(unsigned short *buf, int width, int height, int stride,
    int texture_height)
{
    pvr_poly_cxt_t cxt;
    static pvr_poly_hdr_t hdr[2];
    static pvr_vertex_t vert[4];

    float ratio;
    /* screen coordinates of upper left and bottom right corners */
    static int ul_x, ul_y, br_x, br_y;
    static int graphics_initialized = 0;

    /* on first call, initialize textures and drawing coordinates */
    if (!graphics_initialized)
    {
        textures[0] = pvr_mem_malloc(stride * texture_height * 2);
        textures[1] = pvr_mem_malloc(stride * texture_height * 2);
        if (!textures[0] || !textures[1])
        {
            return ROQ_RENDER_PROBLEM;
        }

        /* Precompile the poly headers */
        pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, stride, texture_height, textures[0], PVR_FILTER_NONE);
        pvr_poly_compile(&hdr[0], &cxt);
        pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565 | PVR_TXRFMT_NONTWIDDLED, stride, texture_height, textures[1], PVR_FILTER_NONE);
        pvr_poly_compile(&hdr[1], &cxt);

        /* this only works if width ratio <= height ratio */
        ratio = 640.0 / width;
        ul_x = 0;
        br_x = (ratio * stride);
        ul_y = ((480 - ratio * height) / 2);
        br_y = ul_y + ratio * texture_height;

        /* Things common to vertices */
        vert[0].z     = vert[1].z     = vert[2].z     = vert[3].z     = 1.0f; 
        vert[0].argb  = vert[1].argb  = vert[2].argb  = vert[3].argb  = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);    
        vert[0].oargb = vert[1].oargb = vert[2].oargb = vert[3].oargb = 0;  
        vert[0].flags = vert[1].flags = vert[2].flags = PVR_CMD_VERTEX;         
        vert[3].flags = PVR_CMD_VERTEX_EOL; 

        vert[0].x = ul_x;
        vert[0].y = ul_y;
        vert[0].u = 0.0;
        vert[0].v = 0.0;

        vert[1].x = br_x;
        vert[1].y = ul_y;
        vert[1].u = 1.0;
        vert[1].v = 0.0;

        vert[2].x = ul_x;
        vert[2].y = br_y;
        vert[2].u = 0.0;
        vert[2].v = 1.0;

        vert[3].x = br_x;
        vert[3].y = br_y;
        vert[3].u = 1.0;
        vert[3].v = 1.0;

        graphics_initialized = 1;
    }

    /* send the video frame as a texture over to video RAM */
    pvr_txr_load(buf, textures[current_frame], stride * texture_height * 2);

    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);

    pvr_prim(&hdr[current_frame], sizeof(pvr_poly_hdr_t));
    pvr_prim(&vert[0], sizeof(pvr_vertex_t));
    pvr_prim(&vert[1], sizeof(pvr_vertex_t));
    pvr_prim(&vert[2], sizeof(pvr_vertex_t));
    pvr_prim(&vert[3], sizeof(pvr_vertex_t));

    pvr_list_finish();
    pvr_scene_finish();

    if (current_frame)
        current_frame = 0;
    else
        current_frame = 1;

    return ROQ_SUCCESS;
}

static int quit_cb()
{
    cont_cond_t cont;

    /* check controller state */
    if (cont_get_cond(maple_first_controller(), &cont))
    {
        /* controller read error */
        return 1;
    }
    cont.buttons = ~cont.buttons;
    return (cont.buttons & CONT_START);
}

int main()
{
    int status;

    vid_set_mode(DM_640x480_NTSC_IL, PM_RGB565);
    pvr_init_defaults();

    status = dreamroq_play("/cd/venuscubes.roq", 1, render_cb, quit_cb);
    printf("dreamroq_play() status = %d\n", status);

    return 0;
}

