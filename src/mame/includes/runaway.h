// license:BSD-3-Clause
// copyright-holders:Mike Balfour
class runaway_state : public driver_device
{
public:
	runaway_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_video_ram(*this, "video_ram"),
		m_sprite_ram(*this, "sprite_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	emu_timer *m_interrupt_timer;
	required_shared_ptr<UINT8> m_video_ram;
	required_shared_ptr<UINT8> m_sprite_ram;
	tilemap_t *m_bg_tilemap;
	int m_tile_bank;
	DECLARE_READ8_MEMBER(runaway_input_r);
	DECLARE_WRITE8_MEMBER(runaway_led_w);
	DECLARE_WRITE8_MEMBER(runaway_irq_ack_w);
	DECLARE_WRITE8_MEMBER(runaway_paletteram_w);
	DECLARE_WRITE8_MEMBER(runaway_video_ram_w);
	DECLARE_WRITE8_MEMBER(runaway_tile_bank_w);
	DECLARE_READ8_MEMBER(runaway_pot_r);
	TILE_GET_INFO_MEMBER(runaway_get_tile_info);
	TILE_GET_INFO_MEMBER(qwak_get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(qwak);
	UINT32 screen_update_runaway(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_qwak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
