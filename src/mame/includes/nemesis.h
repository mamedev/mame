class nemesis_state : public driver_device
{
public:
	nemesis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_charram(*this, "charram"),
		m_xscroll1(*this, "xscroll1"),
		m_xscroll2(*this, "xscroll2"),
		m_yscroll2(*this, "yscroll2"),
		m_yscroll1(*this, "yscroll1"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_colorram1(*this, "colorram1"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_gx400_shared_ram(*this, "gx400_shared"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_charram;
	required_shared_ptr<UINT16> m_xscroll1;
	required_shared_ptr<UINT16> m_xscroll2;
	required_shared_ptr<UINT16> m_yscroll2;
	required_shared_ptr<UINT16> m_yscroll1;
	required_shared_ptr<UINT16> m_videoram1;
	required_shared_ptr<UINT16> m_videoram2;
	required_shared_ptr<UINT16> m_colorram1;
	required_shared_ptr<UINT16> m_colorram2;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT8> m_gx400_shared_ram;

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
	DECLARE_WRITE16_MEMBER(nemesis_gfx_flipx_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_gfx_flipy_word_w);
	DECLARE_WRITE16_MEMBER(salamand_control_port_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_palette_word_w);
	DECLARE_WRITE16_MEMBER(salamander_palette_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_videoram1_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_videoram2_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_colorram1_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_colorram2_word_w);
	DECLARE_WRITE16_MEMBER(nemesis_charram_word_w);
	DECLARE_WRITE8_MEMBER(gx400_speech_start_w);
	DECLARE_WRITE8_MEMBER(salamand_speech_start_w);
	DECLARE_READ8_MEMBER(nemesis_portA_r);
	DECLARE_WRITE8_MEMBER(city_sound_bank_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_nemesis(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/nemesis.c -----------*/





