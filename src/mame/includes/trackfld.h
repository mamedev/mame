/***************************************************************************

    Track'n'Field

***************************************************************************/


class trackfld_state : public driver_device
{
public:
	trackfld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram2(*this, "spriteram2"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_scroll2(*this, "scroll2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_bg_bank;
	int      m_sprite_bank1;
	int      m_sprite_bank2;
	int      m_old_gfx_bank;					// needed by atlantol
	int		 m_sprites_gfx_banked;

	UINT8    m_irq_mask;
	UINT8    m_yieartf_nmi_mask;
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(questions_bank_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(yieartf_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(trackfld_videoram_w);
	DECLARE_WRITE8_MEMBER(trackfld_colorram_w);
	DECLARE_WRITE8_MEMBER(trackfld_flipscreen_w);
	DECLARE_WRITE8_MEMBER(atlantol_gfxbank_w);
	DECLARE_READ8_MEMBER(trackfld_SN76496_r);
	DECLARE_READ8_MEMBER(trackfld_speech_r);
	DECLARE_WRITE8_MEMBER(trackfld_VLM5030_control_w);

};


/*----------- defined in video/trackfld.c -----------*/


PALETTE_INIT( trackfld );
VIDEO_START( trackfld );
SCREEN_UPDATE_IND16( trackfld );
VIDEO_START( atlantol );

