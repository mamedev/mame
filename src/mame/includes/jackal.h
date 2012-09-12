
#define MASTER_CLOCK         XTAL_18_432MHz
#define SOUND_CLOCK          XTAL_3_579545MHz



class jackal_state : public driver_device
{
public:
	jackal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoctrl(*this, "videoctrl"),
		m_paletteram(*this, "paletteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoctrl;
	UINT8 *  m_scrollram;
	required_shared_ptr<UINT8> m_paletteram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* misc */
	int      m_irq_enable;
	UINT8    *m_rambank;
	UINT8    *m_spritebank;

	/* devices */
	cpu_device *m_mastercpu;
	cpu_device *m_slavecpu;
	DECLARE_READ8_MEMBER(topgunbl_rotary_r);
	DECLARE_WRITE8_MEMBER(jackal_flipscreen_w);
	DECLARE_READ8_MEMBER(jackal_zram_r);
	DECLARE_READ8_MEMBER(jackal_voram_r);
	DECLARE_READ8_MEMBER(jackal_spriteram_r);
	DECLARE_WRITE8_MEMBER(jackal_rambank_w);
	DECLARE_WRITE8_MEMBER(jackal_zram_w);
	DECLARE_WRITE8_MEMBER(jackal_voram_w);
	DECLARE_WRITE8_MEMBER(jackal_spriteram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/jackal.c -----------*/

void jackal_mark_tile_dirty(running_machine &machine, int offset);

PALETTE_INIT( jackal );
VIDEO_START( jackal );
SCREEN_UPDATE_IND16( jackal );
