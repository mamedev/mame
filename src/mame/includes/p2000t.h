// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/*****************************************************************************
 *
 * includes/p2000t.h
 *
 ****************************************************************************/

#ifndef P2000T_H_
#define P2000T_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "video/saa5050.h"


class p2000t_state : public driver_device
{
public:
	p2000t_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_keyboard(*this, "KEY.%u", 0)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	optional_device<gfxdecode_device> m_gfxdecode;
	optional_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport_array<10> m_keyboard;
	uint8_t p2000t_port_000f_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t p2000t_port_202f_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p2000t_port_101f_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2000t_port_303f_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2000t_port_505f_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2000t_port_707f_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2000t_port_888b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2000t_port_8c90_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2000t_port_9494_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m_port_101f;
	uint8_t m_port_202f;
	uint8_t m_port_303f;
	uint8_t m_port_707f;
	int8_t m_frame_count;
	void video_start_p2000m();
	void palette_init_p2000m(palette_device &palette);
	uint32_t screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void p2000_interrupt(device_t &device);
};

#endif /* P2000T_H_ */
