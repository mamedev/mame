class snk68_state : public driver_device
{
public:
	snk68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_invert_controls;
	int m_sound_status;

	UINT16* m_pow_fg_videoram;

	UINT16 *m_spriteram;
	UINT16 *m_paletteram;
	int m_sprite_flip_axis;
	tilemap_t *m_fg_tilemap;
	int m_flipscreen;
	UINT32 m_fg_tile_offset;
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_status_w);
	DECLARE_READ16_MEMBER(control_1_r);
	DECLARE_READ16_MEMBER(control_2_r);
	DECLARE_READ16_MEMBER(rotary_1_r);
	DECLARE_READ16_MEMBER(rotary_2_r);
	DECLARE_READ16_MEMBER(rotary_lsb_r);
	DECLARE_READ16_MEMBER(protcontrols_r);
	DECLARE_WRITE16_MEMBER(protection_w);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_READ16_MEMBER(pow_spriteram_r);
	DECLARE_WRITE16_MEMBER(pow_spriteram_w);
	DECLARE_READ16_MEMBER(pow_fg_videoram_r);
	DECLARE_WRITE16_MEMBER(pow_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(searchar_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(pow_flipscreen16_w);
	DECLARE_WRITE16_MEMBER(searchar_flipscreen16_w);
	DECLARE_WRITE16_MEMBER(pow_paletteram16_word_w);
};


/*----------- defined in video/snk68.c -----------*/

VIDEO_START( pow );
VIDEO_START( searchar );
SCREEN_UPDATE_IND16( pow );
