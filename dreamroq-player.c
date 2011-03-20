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
    pvr_poly_hdr_t hdr;
    pvr_vertex_t vert;

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

        /* this only works if width ratio <= height ratio */
        ratio = 640.0 / width;
        ul_x = 0;
        br_x = (ratio * stride);
        ul_y = ((480 - ratio * height) / 2);
        br_y = ul_y + ratio * texture_height;

        graphics_initialized = 1;
    }

    /* send the video frame as a texture over to video RAM */
    pvr_txr_load_ex(buf, textures[current_frame], stride, texture_height,
        PVR_TXRLOAD_16BPP);

    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);

    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, stride, 
      texture_height, textures[current_frame], PVR_FILTER_NONE);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.argb = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f);
    vert.oargb = 0;
    vert.flags = PVR_CMD_VERTEX;

    vert.x = ul_x;
    vert.y = ul_y;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = br_x;
    vert.y = ul_y;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 0.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = ul_x;
    vert.y = br_y;
    vert.z = 1;
    vert.u = 0.0;
    vert.v = 1.0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = br_x;
    vert.y = br_y;
    vert.z = 1;
    vert.u = 1.0;
    vert.v = 1.0;
    vert.flags = PVR_CMD_VERTEX_EOL;
    pvr_prim(&vert, sizeof(vert));

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

