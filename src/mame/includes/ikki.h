/*************************************************************************

    Ikki

*************************************************************************/

class ikki_state : public driver_device
{
public:
	ikki_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub")
		{ }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_scroll;
	size_t     m_videoram_size;
	size_t     m_spriteram_size;

	/* video-related */
	bitmap_ind16 m_sprite_bitmap;
	UINT8      m_flipscreen;
	int        m_punch_through_pen;
	UINT8      m_irq_source;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	DECLARE_READ8_MEMBER(ikki_e000_r);
	DECLARE_WRITE8_MEMBER(ikki_coin_counters);
	DECLARE_WRITE8_MEMBER(ikki_scrn_ctrl_w);
};


/*----------- defined in video/ikki.c -----------*/


PALETTE_INIT( ikki );
VIDEO_START( ikki );
SCREEN_UPDATE_IND16( ikki );
