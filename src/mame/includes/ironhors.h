/*************************************************************************

    IronHorse

*************************************************************************/

class ironhors_state : public driver_device
{
public:
	ironhors_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_interrupt_enable(*this, "int_enable"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_interrupt_enable;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_palettebank;
	int        m_charbank;
	int        m_spriterambank;

	/* devices */
	device_t *m_maincpu;
	device_t *m_soundcpu;
	DECLARE_WRITE8_MEMBER(ironhors_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(ironhors_videoram_w);
	DECLARE_WRITE8_MEMBER(ironhors_colorram_w);
	DECLARE_WRITE8_MEMBER(ironhors_charbank_w);
	DECLARE_WRITE8_MEMBER(ironhors_palettebank_w);
	DECLARE_WRITE8_MEMBER(ironhors_flipscreen_w);
	DECLARE_WRITE8_MEMBER(ironhors_filter_w);
	DECLARE_READ8_MEMBER(farwest_soundlatch_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(farwest_get_bg_tile_info);
};


/*----------- defined in video/ironhors.c -----------*/


PALETTE_INIT( ironhors );
VIDEO_START( ironhors );
SCREEN_UPDATE_IND16( ironhors );
VIDEO_START( farwest );
SCREEN_UPDATE_IND16( farwest );
