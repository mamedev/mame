// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

#ifndef MAME_NAMCO_XEVIOUS_H
#define MAME_NAMCO_XEVIOUS_H

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
	virtual void video_start() override ATTR_COLD;

	required_shared_ptr<uint8_t> m_xevious_sr1;
	required_shared_ptr<uint8_t> m_xevious_sr2;
	required_shared_ptr<uint8_t> m_xevious_sr3;
	required_shared_ptr<uint8_t> m_xevious_fg_colorram;
	required_shared_ptr<uint8_t> m_xevious_bg_colorram;
	required_shared_ptr<uint8_t> m_xevious_fg_videoram;
	required_shared_ptr<uint8_t> m_xevious_bg_videoram;
	optional_device<samples_device> m_samples;
	optional_device<cpu_device> m_subcpu3;

	int32_t m_xevious_bs[2];

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void xevious_palette(palette_device &palette) const;
	uint32_t screen_update_xevious(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void xevious_fg_videoram_w(offs_t offset, uint8_t data);
	void xevious_fg_colorram_w(offs_t offset, uint8_t data);
	void xevious_bg_videoram_w(offs_t offset, uint8_t data);
	void xevious_bg_colorram_w(offs_t offset, uint8_t data);
	void xevious_vh_latch_w(offs_t offset, uint8_t data);
	void xevious_bs_w(offs_t offset, uint8_t data);
	uint8_t xevious_bb_r(offs_t offset);

	void xevious_map(address_map &map) ATTR_COLD;
};

class battles_state : public xevious_state
{
public:
	battles_state(const machine_config &mconfig, device_type type, const char *tag)
		: xevious_state(mconfig, type, tag),
		m_nmi_timer(*this, "nmi")
	{
	}

	void battles(machine_config &config);

protected:
	virtual void driver_start() override;
	virtual void machine_reset() override ATTR_COLD;

private:
	void interrupt_4(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_generate);

	void battles_mem4(address_map &map) ATTR_COLD;

	// Custom I/O
	uint8_t customio0_r();
	uint8_t customio_data0_r(offs_t offset);
	uint8_t customio3_r();
	uint8_t customio_data3_r(offs_t offset);
	uint8_t input_port_r(offs_t offset);

	void customio0_w(uint8_t data);
	void customio_data0_w(offs_t offset, uint8_t data);
	void customio3_w(uint8_t data);
	void customio_data3_w(offs_t offset, uint8_t data);
	void cpu4_coin_w(uint8_t data);
	void noise_sound_w(offs_t offset, uint8_t data);

	required_device<timer_device> m_nmi_timer;

	uint8_t m_customio[16]{};
	char m_customio_command = 0;
	char m_customio_prev_command = 0;
	char m_customio_command_count = 0;
	char m_customio_data = 0;
	char m_sound_played = 0;
};

#endif // MAME_NAMCO_XEVIOUS_H
