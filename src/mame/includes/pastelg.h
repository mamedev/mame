/*----------- defined in video/pastelg.c -----------*/

PALETTE_INIT( pastelg );
VIDEO_UPDATE( pastelg );
VIDEO_START( pastelg );

WRITE8_HANDLER( pastelg_clut_w );
WRITE8_HANDLER( pastelg_romsel_w );
WRITE8_HANDLER( threeds_romsel_w );
WRITE8_HANDLER( threeds_output_w );
WRITE8_HANDLER( pastelg_blitter_w );
READ8_HANDLER( threeds_rom_readback_r );

int pastelg_blitter_src_addr_r(void);
