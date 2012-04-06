/*************************************************************************

    Venture Line Super Rider driver

**************************************************************************/

class suprridr_state : public driver_device
{
public:
	suprridr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_nmi_enable;
	UINT8 m_sound_data;
	UINT8 *m_fgram;
	UINT8 *m_bgram;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg_tilemap_noscroll;
	UINT8 m_flipx;
	UINT8 m_flipy;
	UINT8 *m_spriteram;
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_WRITE8_MEMBER(sound_irq_ack_w);
	DECLARE_WRITE8_MEMBER(coin_lock_w);
	DECLARE_WRITE8_MEMBER(suprridr_flipx_w);
	DECLARE_WRITE8_MEMBER(suprridr_flipy_w);
	DECLARE_WRITE8_MEMBER(suprridr_fgdisable_w);
	DECLARE_WRITE8_MEMBER(suprridr_fgscrolly_w);
	DECLARE_WRITE8_MEMBER(suprridr_bgscrolly_w);
	DECLARE_WRITE8_MEMBER(suprridr_bgram_w);
	DECLARE_WRITE8_MEMBER(suprridr_fgram_w);
};


/*----------- defined in video/suprridr.c -----------*/

VIDEO_START( suprridr );
PALETTE_INIT( suprridr );

int suprridr_is_screen_flipped(running_machine &machine);


SCREEN_UPDATE_IND16( suprridr );
