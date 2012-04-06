/***************************************************************************

    Blomby Car

***************************************************************************/

class blmbycar_state : public driver_device
{
public:
	blmbycar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_vram_0;
	UINT16 *    m_scroll_0;
	UINT16 *    m_vram_1;
	UINT16 *    m_scroll_1;
	UINT16 *    m_spriteram;
	UINT16 *    m_paletteram;
	size_t      m_spriteram_size;

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
