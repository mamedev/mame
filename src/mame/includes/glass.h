/*************************************************************************

    Glass

*************************************************************************/

class glass_state : public driver_device
{
public:
	glass_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_videoram;
	UINT16 *    m_vregs;
	UINT16 *    m_spriteram;
//      UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t     *m_pant[2];
	bitmap_ind16    *m_screen_bitmap;

	/* misc */
	int         m_current_bit;
	int         m_current_command;
	int         m_cause_interrupt;
	int         m_blitter_serial_buffer[5];
	DECLARE_WRITE16_MEMBER(clr_int_w);
	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(glass_coin_w);
};


/*----------- defined in video/glass.c -----------*/

WRITE16_HANDLER( glass_vram_w );
WRITE16_HANDLER( glass_blitter_w );

VIDEO_START( glass );
SCREEN_UPDATE_IND16( glass );
