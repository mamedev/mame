class gsword_state : public driver_device
{
public:
	gsword_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_coins;
	int m_fake8910_0;
	int m_fake8910_1;
	int m_nmi_enable;
	UINT8 *m_cpu2_ram;
	int m_protect_hack;
	size_t m_spritexy_size;
	UINT8 *m_spritexy_ram;
	UINT8 *m_spritetile_ram;
	UINT8 *m_spriteattrib_ram;
	int m_charbank;
	int m_charpalbank;
	int m_flipscreen;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/gsword.c -----------*/

WRITE8_HANDLER( gsword_charbank_w );
WRITE8_HANDLER( gsword_videoctrl_w );
WRITE8_HANDLER( gsword_videoram_w );
WRITE8_HANDLER( gsword_scroll_w );

PALETTE_INIT( josvolly );
PALETTE_INIT( gsword );
VIDEO_START( gsword );
SCREEN_UPDATE( gsword );
