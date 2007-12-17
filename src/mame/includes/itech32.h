/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

/*----------- defined in drivers/itech32.c -----------*/

void itech32_update_interrupts(int vint, int xint, int qint);


/*----------- defined in video/itech32.c -----------*/

extern UINT16 *itech32_video;
extern UINT32 *drivedge_zbuf_control;
extern UINT8 itech32_planes;
extern UINT16 itech32_vram_height;

VIDEO_START( itech32 );

WRITE16_HANDLER( timekill_colora_w );
WRITE16_HANDLER( timekill_colorbc_w );
WRITE16_HANDLER( timekill_intensity_w );

WRITE16_HANDLER( bloodstm_color1_w );
WRITE16_HANDLER( bloodstm_color2_w );
WRITE16_HANDLER( bloodstm_plane_w );

WRITE32_HANDLER( drivedge_color0_w );

WRITE32_HANDLER( itech020_color1_w );
WRITE32_HANDLER( itech020_color2_w );
WRITE32_HANDLER( itech020_plane_w );

WRITE16_HANDLER( timekill_paletteram_w );
WRITE16_HANDLER( bloodstm_paletteram_w );
WRITE32_HANDLER( drivedge_paletteram_w );
WRITE32_HANDLER( itech020_paletteram_w );

WRITE16_HANDLER( itech32_video_w );
READ16_HANDLER( itech32_video_r );

WRITE16_HANDLER( bloodstm_video_w );
READ16_HANDLER( bloodstm_video_r );
WRITE32_HANDLER( itech020_video_w );
READ32_HANDLER( itech020_video_r );
WRITE32_HANDLER( drivedge_zbuf_control_w );

VIDEO_UPDATE( itech32 );
