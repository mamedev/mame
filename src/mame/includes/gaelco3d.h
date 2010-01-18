/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

**************************************************************************/


/*----------- defined in video/gaelco3d.c -----------*/

extern UINT8 *gaelco3d_texture;
extern UINT8 *gaelco3d_texmask;
extern offs_t gaelco3d_texture_size;
extern offs_t gaelco3d_texmask_size;

void gaelco3d_render(running_device *screen);
WRITE32_HANDLER( gaelco3d_render_w );

WRITE16_HANDLER( gaelco3d_paletteram_w );
WRITE32_HANDLER( gaelco3d_paletteram_020_w );

VIDEO_START( gaelco3d );
VIDEO_UPDATE( gaelco3d );
