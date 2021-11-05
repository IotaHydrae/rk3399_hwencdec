#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <drm.h>
#include <drm_mode.h>
#include <string.h>

int main()
{
    //------------------------------------------------------------------------------
    //Opening the DRI device
    //------------------------------------------------------------------------------

    int dri_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);

    //------------------------------------------------------------------------------
    //Kernel Mode Setting (KMS)
    //------------------------------------------------------------------------------

    uint64_t res_fb_buf[10] = {0},
             res_crtc_buf[10] = {0},
             res_conn_buf[10] = {0},
             res_enc_buf[10] = {0};

    struct drm_mode_card_res res = {0};

    //Become the "master" of the DRI device
    ioctl(dri_fd, DRM_IOCTL_SET_MASTER, 0);

    //Get resource counts
    ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res);
    res.fb_id_ptr = (uint64_t)res_fb_buf;
    res.crtc_id_ptr = (uint64_t)res_crtc_buf;
    res.connector_id_ptr = (uint64_t)res_conn_buf;
    res.encoder_id_ptr = (uint64_t)res_enc_buf;
    //Get resource IDs
    ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res);

    printf("fb: %d, crtc: %d, conn: %d, enc: %d\n", res.count_fbs, res.count_crtcs, res.count_connectors, res.count_encoders);

    void *fb_base;
    long fb_w;
    long fb_h;

    //------------------------------------------------------------------------------
    //Creating a dumb buffer
    //------------------------------------------------------------------------------
    struct drm_mode_create_dumb create_dumb = {0};
    struct drm_mode_map_dumb map_dumb = {0};
    struct drm_mode_fb_cmd cmd_dumb = {0};

    //If we create the buffer later, we can get the size of the screen first.
    //This must be a valid mode, so it's probably best to do this after we find
    //a valid crtc with modes.
    create_dumb.width = 1280;
    create_dumb.height = 720;
    create_dumb.bpp = 32;
    create_dumb.flags = 0;
    create_dumb.pitch = 0;
    create_dumb.size = 0;
    create_dumb.handle = 0;
    ioctl(dri_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);

    // cmd_dumb.width = create_dumb.width;
    // cmd_dumb.height = create_dumb.height;
    // cmd_dumb.bpp = create_dumb.bpp;
    // cmd_dumb.pitch = create_dumb.pitch;
    // cmd_dumb.depth = 24;
    // cmd_dumb.handle = create_dumb.handle;
    // ioctl(dri_fd, DRM_IOCTL_MODE_ADDFB, &cmd_dumb);

    map_dumb.handle = create_dumb.handle;
    ioctl(dri_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb);

    fb_base = mmap64(0, create_dumb.size, PROT_READ | PROT_WRITE, MAP_SHARED, dri_fd, map_dumb.offset);
    fb_w = create_dumb.width;
    fb_h = create_dumb.height;

    //------------------------------------------------------------------------------
    //Kernel Mode Setting (KMS)
    //------------------------------------------------------------------------------

    printf("modes: %dx%d FB: %p\n", 1280, 720, fb_base);
    memcpy(fb_base, "hello", 6);

    //Stop being the "master" of the DRI device
    ioctl(dri_fd, DRM_IOCTL_DROP_MASTER, 0);

    // int x, y;
    // for (int i = 0; i < 100; i++)
    // {
    //     int j;
    //     for (j = 0; j < res.count_connectors; j++)
    //     {
    //         int col = (rand() % 0x00ffffff) & 0x00ff00ff;
    //         for (y = 0; y < fb_h; y++)
    //             for (x = 0; x < fb_w; x++)
    //             {
    //                 int location = y * (fb_w) + x;
    //                 *(((uint32_t *)fb_base) + location) = col;
    //             }
    //     }
    //     // usleep(100000);
    // }

    return 0;
}