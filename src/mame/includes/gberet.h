/***************************************************************************

    Green Beret

***************************************************************************/

class gberet_state : public driver_device
{
public:
	gberet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_videoram;
	UINT8 *     m_colorram;
	UINT8 *     m_spriteram;
	UINT8 *     m_spriteram2;
	UINT8 *     m_scrollram;
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t * m_bg_tilemap;
	UINT8       m_spritebank;

	/* misc */
	UINT8 *     m_soundlatch;
	UINT8       m_interrupt_mask;
	UINT8       m_interrupt_ticks;
	DECLARE_WRITE8_MEMBER(gberet_coin_counter_w);
	DECLARE_WRITE8_MEMBER(mrgoemon_coin_counter_w);
	DECLARE_WRITE8_MEMBER(gberet_flipscreen_w);
	DECLARE_WRITE8_MEMBER(gberet_sound_w);
	DECLARE_WRITE8_MEMBER(gberetb_flipscreen_w);
	DECLARE_READ8_MEMBER(gberetb_irq_ack_r);
	DECLARE_WRITE8_MEMBER(gberetb_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(gberet_videoram_w);
	DECLARE_WRITE8_MEMBER(gberet_colorram_w);
	DECLARE_WRITE8_MEMBER(gberet_scroll_w);
	DECLARE_WRITE8_MEMBER(gberet_sprite_bank_w);
	DECLARE_WRITE8_MEMBER(gberetb_scroll_w);
};


/*----------- defined in video/gberet.c -----------*/


PALETTE_INIT( gberet );
VIDEO_START( gberet );
SCREEN_UPDATE_IND16( gberet );
SCREEN_UPDATE_IND16( gberetb );
