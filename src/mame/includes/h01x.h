// license:BSD-3-Clause
/***************************************************************************
        NF500A (TRS80 Level II Basic)
        09/01/2019
****************************************************************************/

#ifndef MAME_INCLUDES_H01X_H
#define MAME_INCLUDES_H01X_H

#pragma once

#include "emu.h"
#include "screen.h"
#include "speaker.h"
#include "emupal.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "imagedev/cassette.h"
#include "formats/trs_cas.h"

class h01x_state : public driver_device
{
public:
	h01x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_vram(*this, "vram")
		, m_rom(*this, "maincpu")
		, m_hzrom(*this, "hzrom")
		, m_exrom(*this, "exrom")
		, m_p_videoram(*this, "videoram")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "LINE%u", 0)
	{ }

	void init_h01x();

	void nf500a(machine_config &config);
	void h01jce(machine_config &config);

//private:
	virtual void video_start() override;

	void h01x_mem_map(address_map &map);
	void h01x_io_map(address_map &map);

	uint32_t screen_update_h01x(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(mem_0000_r);
	DECLARE_WRITE8_MEMBER(mem_0000_w);
	DECLARE_READ8_MEMBER(mem_4000_r);
	DECLARE_WRITE8_MEMBER(mem_4000_w);
	DECLARE_READ8_MEMBER(mem_8000_r);
	DECLARE_WRITE8_MEMBER(mem_8000_w);
	DECLARE_READ8_MEMBER(mem_c000_r);
	DECLARE_WRITE8_MEMBER(mem_c000_w);

	DECLARE_WRITE8_MEMBER(port_60_w);
	DECLARE_WRITE8_MEMBER(port_64_w);
	DECLARE_WRITE8_MEMBER(port_70_w);
	DECLARE_READ8_MEMBER(port_50_r);

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<ram_device> m_vram;
	required_memory_region m_rom;
	required_memory_region m_hzrom;
	optional_memory_region m_exrom;
	optional_shared_ptr<u8> m_p_videoram;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<8> m_io_keyboard;

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	uint8_t m_bank;

	uint8_t *m_ram_ptr, *m_rom_ptr, *m_hzrom_ptr, *m_vram_ptr;
	int m_ram_size;

	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	bool m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
};

#endif // MAME_INCLUDES_H01X_H
