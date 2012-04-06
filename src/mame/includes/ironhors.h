/*************************************************************************

    IronHorse

*************************************************************************/

class ironhors_state : public driver_device
{
public:
	ironhors_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	UINT8 *    m_spriteram2;
	UINT8 *    m_scroll;
	UINT8 *    m_interrupt_enable;
	size_t     m_spriteram_size;

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
};


/*----------- defined in video/ironhors.c -----------*/


PALETTE_INIT( ironhors );
VIDEO_START( ironhors );
SCREEN_UPDATE_IND16( ironhors );
VIDEO_START( farwest );
SCREEN_UPDATE_IND16( farwest );
