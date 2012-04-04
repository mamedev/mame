class nemesis_state : public driver_device
{
public:
	nemesis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_videoram1;
	UINT16 *  m_videoram2;
	UINT16 *  m_colorram1;
	UINT16 *  m_colorram2;
	UINT16 *  m_charram;
	UINT16 *  m_spriteram;
	UINT16 *  m_paletteram;
	UINT16 *  m_xscroll1;
	UINT16 *  m_xscroll2;
	UINT16 *  m_yscroll1;
	UINT16 *  m_yscroll2;
	UINT8 *   m_gx400_shared_ram;

	size_t    m_charram_size;
	size_t    m_spriteram_size;

	/* video-related */
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	int       m_spriteram_words;
	int       m_tilemap_flip;
	int       m_flipscreen;
	UINT8     m_irq_port_last;
	UINT8     m_blank_tile[8*8];

	/* misc */
	int       m_irq_on;
	int       m_irq1_on;
	int       m_irq2_on;
	int       m_irq4_on;
	UINT16    m_selected_ip; /* Copied from WEC Le Mans 24 driver, explicity needed for Hyper Crash */
	int       m_gx400_irq1_cnt;
	UINT8     m_frame_counter;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_vlm;
	DECLARE_WRITE16_MEMBER(gx400_irq1_enable_word_w);
	DECLARE_WRITE16_MEMBER(gx400_irq2_enable_word_w);
	DECLARE_WRITE16_MEMBER(gx400_irq4_enable_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_irq_enable_word_w);
	DECLARE_WRITE16_MEMBER(konamigt_irq_enable_word_w);
	DECLARE_WRITE16_MEMBER(konamigt_irq2_enable_word_w);
	DECLARE_READ16_MEMBER(gx400_sharedram_word_r);
	DECLARE_WRITE16_MEMBER(gx400_sharedram_word_w);
	DECLARE_READ16_MEMBER(konamigt_input_word_r);
	DECLARE_WRITE16_MEMBER(selected_ip_word_w);
	DECLARE_READ16_MEMBER(selected_ip_word_r);
	DECLARE_WRITE16_MEMBER(nemesis_soundlatch_word_w);
	DECLARE_READ8_MEMBER(wd_r);
};


/*----------- defined in video/nemesis.c -----------*/

WRITE16_HANDLER( nemesis_gfx_flipx_word_w );
WRITE16_HANDLER( nemesis_gfx_flipy_word_w );
WRITE16_HANDLER( salamand_control_port_word_w );
WRITE16_HANDLER( salamander_palette_word_w );
WRITE16_HANDLER( nemesis_palette_word_w );

WRITE16_HANDLER( nemesis_videoram1_word_w );
WRITE16_HANDLER( nemesis_videoram2_word_w );
WRITE16_HANDLER( nemesis_colorram1_word_w );
WRITE16_HANDLER( nemesis_colorram2_word_w );
WRITE16_HANDLER( nemesis_charram_word_w );

VIDEO_START( nemesis );
SCREEN_UPDATE_IND16( nemesis );
