// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#ifndef MAME_INCLUDES_XEVIOUS
#define MAME_INCLUDES_XEVIOUS

#pragma once

#include "machine/timer.h"
#include "tilemap.h"

class xevious_state : public galaga_state
{
public:
	xevious_state(const machine_config &mconfig, device_type type, const char *tag) :
		galaga_state(mconfig, type, tag),
		m_xevious_sr1(*this, "xevious_sr1"),
		m_xevious_sr2(*this, "xevious_sr2"),
		m_xevious_sr3(*this, "xevious_sr3"),
		m_xevious_fg_colorram(*this, "fg_colorram"),
		m_xevious_bg_colorram(*this, "bg_colorram"),
		m_xevious_fg_videoram(*this, "fg_videoram"),
		m_xevious_bg_videoram(*this, "bg_videoram"),
		m_samples(*this, "samples"),
		m_subcpu3(*this, "sub3")
	{ }

	void xevious(machine_config &config);

	void init_xevious();
	void init_xevios();

protected:
	required_shared_ptr<uint8_t> m_xevious_sr1;
	required_shared_ptr<uint8_t> m_xevious_sr2;
	required_shared_ptr<uint8_t> m_xevious_sr3;
	required_shared_ptr<uint8_t> m_xevious_fg_colorram;
	required_shared_ptr<uint8_t> m_xevious_bg_colorram;
	required_shared_ptr<uint8_t> m_xevious_fg_videoram;
	required_shared_ptr<uint8_t> m_xevious_bg_videoram;
	optional_device<samples_device> m_samples;

	int32_t m_xevious_bs[2];

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_VIDEO_START(xevious);
	void xevious_palette(palette_device &palette) const;
	DECLARE_MACHINE_RESET(xevios);
	uint32_t screen_update_xevious(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER( xevious_fg_videoram_w );
	DECLARE_WRITE8_MEMBER( xevious_fg_colorram_w );
	DECLARE_WRITE8_MEMBER( xevious_bg_videoram_w );
	DECLARE_WRITE8_MEMBER( xevious_bg_colorram_w );
	DECLARE_WRITE8_MEMBER( xevious_vh_latch_w );
	DECLARE_WRITE8_MEMBER( xevious_bs_w );
	DECLARE_READ8_MEMBER( xevious_bb_r );

	optional_device<cpu_device> m_subcpu3;

	void xevious_map(address_map &map);
};

class battles_state : public xevious_state
{
public:
	battles_state(const machine_config &mconfig, device_type type, const char *tag)
		: xevious_state(mconfig, type, tag),
		m_nmi_timer(*this, "nmi")
	{
	}

	void driver_init() override;

	void battles(machine_config &config);

protected:
	void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER(interrupt_4);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_generate);

	void battles_mem4(address_map &map);

	// Custom I/O
	DECLARE_READ8_MEMBER( customio0_r );
	DECLARE_READ8_MEMBER( customio_data0_r );
	DECLARE_READ8_MEMBER( customio3_r );
	DECLARE_READ8_MEMBER( customio_data3_r );
	DECLARE_READ8_MEMBER( input_port_r );

	DECLARE_WRITE8_MEMBER( customio0_w );
	DECLARE_WRITE8_MEMBER( customio_data0_w );
	DECLARE_WRITE8_MEMBER( customio3_w );
	DECLARE_WRITE8_MEMBER( customio_data3_w );
	DECLARE_WRITE8_MEMBER( cpu4_coin_w );
	DECLARE_WRITE8_MEMBER( noise_sound_w );

	required_device<timer_device> m_nmi_timer;

	uint8_t m_customio[16];
	char m_customio_command;
	char m_customio_prev_command;
	char m_customio_command_count;
	char m_customio_data;
	char m_sound_played;
};

#endif // MAME_INCLUDES_XEVIOUS
