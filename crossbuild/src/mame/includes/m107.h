/*************************************************************************

    Irem M107 hardware

*************************************************************************/


/*----------- defined in video/m107.c -----------*/

extern UINT16 *m107_vram_data;
extern UINT8 m107_spritesystem;
extern UINT16 m107_raster_irq_position;

WRITE16_HANDLER( m107_spritebuffer_w );
VIDEO_UPDATE( m107 );
VIDEO_START( m107 );
WRITE16_HANDLER( m107_control_w );
WRITE16_HANDLER( m107_vram_w );
