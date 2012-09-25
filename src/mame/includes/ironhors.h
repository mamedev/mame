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
	cpu_device *m_maincpu;
	cpu_device *m_soundcpu;
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
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_VIDEO_START(farwest);
	UINT32 screen_update_ironhors(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_farwest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(ironhors_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(farwest_irq);
};
