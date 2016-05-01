// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

class xevious_state : public galaga_state
{
public:
	xevious_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state(mconfig, type, tag),
		m_xevious_sr1(*this, "xevious_sr1"),
		m_xevious_sr2(*this, "xevious_sr2"),
		m_xevious_sr3(*this, "xevious_sr3"),
		m_xevious_fg_colorram(*this, "fg_colorram"),
		m_xevious_bg_colorram(*this, "bg_colorram"),
		m_xevious_fg_videoram(*this, "fg_videoram"),
		m_xevious_bg_videoram(*this, "bg_videoram"),
		m_samples(*this, "samples"),
		m_subcpu3(*this, "sub3") { }

	required_shared_ptr<UINT8> m_xevious_sr1;
	required_shared_ptr<UINT8> m_xevious_sr2;
	required_shared_ptr<UINT8> m_xevious_sr3;
	required_shared_ptr<UINT8> m_xevious_fg_colorram;
	required_shared_ptr<UINT8> m_xevious_bg_colorram;
	required_shared_ptr<UINT8> m_xevious_fg_videoram;
	required_shared_ptr<UINT8> m_xevious_bg_videoram;
	optional_device<samples_device> m_samples;

	INT32 m_xevious_bs[2];
	DECLARE_DRIVER_INIT(xevious);
	DECLARE_DRIVER_INIT(xevios);
	DECLARE_DRIVER_INIT(battles);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_VIDEO_START(xevious);
	DECLARE_PALETTE_INIT(xevious);
	DECLARE_MACHINE_RESET(xevios);
	DECLARE_PALETTE_INIT(battles);
	DECLARE_MACHINE_RESET(battles);
	UINT32 screen_update_xevious(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(battles_interrupt_4);
	TIMER_DEVICE_CALLBACK_MEMBER(battles_nmi_generate);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER( xevious_fg_videoram_w );
	DECLARE_WRITE8_MEMBER( xevious_fg_colorram_w );
	DECLARE_WRITE8_MEMBER( xevious_bg_videoram_w );
	DECLARE_WRITE8_MEMBER( xevious_bg_colorram_w );
	DECLARE_WRITE8_MEMBER( xevious_vh_latch_w );
	DECLARE_WRITE8_MEMBER( xevious_bs_w );
	DECLARE_READ8_MEMBER( xevious_bb_r );

	// Custom I/O
	void battles_customio_init();

	DECLARE_READ8_MEMBER( battles_customio0_r );
	DECLARE_READ8_MEMBER( battles_customio_data0_r );
	DECLARE_READ8_MEMBER( battles_customio3_r );
	DECLARE_READ8_MEMBER( battles_customio_data3_r );
	DECLARE_READ8_MEMBER( battles_input_port_r );

	DECLARE_WRITE8_MEMBER( battles_customio0_w );
	DECLARE_WRITE8_MEMBER( battles_customio_data0_w );
	DECLARE_WRITE8_MEMBER( battles_customio3_w );
	DECLARE_WRITE8_MEMBER( battles_customio_data3_w );
	DECLARE_WRITE8_MEMBER( battles_CPU4_coin_w );
	DECLARE_WRITE8_MEMBER( battles_noise_sound_w );

	UINT8 m_customio[16];
	char m_battles_customio_command;
	char m_battles_customio_prev_command;
	char m_battles_customio_command_count;
	char m_battles_customio_data;
	char m_battles_sound_played;

	optional_device<cpu_device> m_subcpu3;
};
