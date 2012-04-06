

class fuuki16_state : public driver_device
{
public:
	fuuki16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_vram[4];
	UINT16 *    m_vregs;
	UINT16 *    m_priority;
	UINT16 *    m_unknown;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* video-related */
	tilemap_t     *m_tilemap[4];

	/* misc */
	emu_timer   *m_raster_interrupt_timer;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(fuuki16_vregs_w);
	DECLARE_WRITE16_MEMBER(fuuki16_sound_command_w);
	DECLARE_WRITE8_MEMBER(fuuki16_sound_rombank_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_0_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_1_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_2_w);
	DECLARE_WRITE16_MEMBER(fuuki16_vram_3_w);
};


/*----------- defined in video/fuukifg2.c -----------*/


VIDEO_START( fuuki16 );
SCREEN_UPDATE_IND16( fuuki16 );
