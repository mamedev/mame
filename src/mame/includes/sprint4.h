// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
class sprint4_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI
	};

	sprint4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	int m_da_latch;
	int m_steer_FF1[4];
	int m_steer_FF2[4];
	int m_gear[4];
	UINT8 m_last_wheel[4];
	int m_collision[4];
	tilemap_t* m_playfield;
	bitmap_ind16 m_helper;
	DECLARE_READ8_MEMBER(sprint4_wram_r);
	DECLARE_READ8_MEMBER(sprint4_analog_r);
	DECLARE_READ8_MEMBER(sprint4_coin_r);
	DECLARE_READ8_MEMBER(sprint4_collision_r);
	DECLARE_READ8_MEMBER(sprint4_options_r);
	DECLARE_WRITE8_MEMBER(sprint4_wram_w);
	DECLARE_WRITE8_MEMBER(sprint4_collision_reset_w);
	DECLARE_WRITE8_MEMBER(sprint4_da_latch_w);
	DECLARE_WRITE8_MEMBER(sprint4_lamp_w);
	DECLARE_WRITE8_MEMBER(sprint4_video_ram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(get_lever);
	DECLARE_CUSTOM_INPUT_MEMBER(get_wheel);
	DECLARE_CUSTOM_INPUT_MEMBER(get_collision);
	DECLARE_WRITE8_MEMBER(sprint4_screech_1_w);
	DECLARE_WRITE8_MEMBER(sprint4_screech_2_w);
	DECLARE_WRITE8_MEMBER(sprint4_screech_3_w);
	DECLARE_WRITE8_MEMBER(sprint4_screech_4_w);
	DECLARE_WRITE8_MEMBER(sprint4_bang_w);
	DECLARE_WRITE8_MEMBER(sprint4_attract_w);
	TILE_GET_INFO_MEMBER(sprint4_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(sprint4);
	UINT32 screen_update_sprint4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_sprint4(screen_device &screen, bool state);
	TIMER_CALLBACK_MEMBER(nmi_callback);
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
