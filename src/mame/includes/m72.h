/*************************************************************************

    Irem M72 hardware

*************************************************************************/

class m72_state : public driver_device
{
public:
	m72_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 irqvector;
	UINT32 sample_addr;
	UINT16 *protection_ram;
	emu_timer *scanline_timer;
	UINT8 irq_base;
	UINT8 mcu_snd_cmd_latch;
	UINT8 mcu_sample_latch;
	UINT32 mcu_sample_addr;
	const UINT8 *protection_code;
	const UINT8 *protection_crc;
	UINT8 *soundram;
	int prev[4];
	int diff[4];
	UINT16 *videoram1;
	UINT16 *videoram2;
	UINT16 *majtitle_rowscrollram;
	UINT32 raster_irq_position;
	UINT16 *spriteram;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;
	INT32 scrollx1;
	INT32 scrolly1;
	INT32 scrollx2;
	INT32 scrolly2;
	INT32 video_off;
	int majtitle_rowscroll;
};


/*----------- defined in video/m72.c -----------*/

VIDEO_START( m72 );
VIDEO_START( rtype2 );
VIDEO_START( majtitle );
VIDEO_START( hharry );
VIDEO_START( poundfor );

READ16_HANDLER( m72_palette1_r );
READ16_HANDLER( m72_palette2_r );
WRITE16_HANDLER( m72_palette1_w );
WRITE16_HANDLER( m72_palette2_w );
WRITE16_HANDLER( m72_videoram1_w );
WRITE16_HANDLER( m72_videoram2_w );
WRITE16_HANDLER( m72_irq_line_w );
WRITE16_HANDLER( m72_scrollx1_w );
WRITE16_HANDLER( m72_scrollx2_w );
WRITE16_HANDLER( m72_scrolly1_w );
WRITE16_HANDLER( m72_scrolly2_w );
WRITE16_HANDLER( m72_dmaon_w );
WRITE16_HANDLER( m72_port02_w );
WRITE16_HANDLER( rtype2_port02_w );
WRITE16_HANDLER( majtitle_gfx_ctrl_w );

SCREEN_UPDATE( m72 );
SCREEN_UPDATE( majtitle );
