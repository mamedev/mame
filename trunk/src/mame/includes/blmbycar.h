/***************************************************************************

    Blomby Car

***************************************************************************/

class blmbycar_state : public driver_device
{
public:
	blmbycar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram_1(*this, "vram_1"),
		m_vram_0(*this, "vram_0"),
		m_scroll_1(*this, "scroll_1"),
		m_scroll_0(*this, "scroll_0"),
		m_paletteram(*this, "paletteram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_scroll_1;
	required_shared_ptr<UINT16> m_scroll_0;
	required_shared_ptr<UINT16> m_paletteram;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;

	/* input-related */
	UINT8       m_pot_wheel;	// blmbycar
	int         m_old_val;	// blmbycar
	int         m_retvalue;	// waterball
	DECLARE_WRITE16_MEMBER(blmbycar_okibank_w);
	DECLARE_WRITE16_MEMBER(blmbycar_pot_wheel_reset_w);
	DECLARE_WRITE16_MEMBER(blmbycar_pot_wheel_shift_w);
	DECLARE_READ16_MEMBER(blmbycar_pot_wheel_r);
	DECLARE_READ16_MEMBER(blmbycar_opt_wheel_r);
	DECLARE_READ16_MEMBER(waterball_unk_r);
	DECLARE_WRITE16_MEMBER(blmbycar_palette_w);
	DECLARE_WRITE16_MEMBER(blmbycar_vram_0_w);
	DECLARE_WRITE16_MEMBER(blmbycar_vram_1_w);
};


/*----------- defined in video/blmbycar.c -----------*/



VIDEO_START( blmbycar );
SCREEN_UPDATE_IND16( blmbycar );
