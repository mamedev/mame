// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*****************************************************************************
 *
 * includes/primo.h
 *
 ****************************************************************************/

#ifndef MAME_MICROKEY_PRIMO_H
#define MAME_MICROKEY_PRIMO_H

#pragma once

#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"
#include "screen.h"

#include "bus/cbmiec/cbmiec.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class primo_state : public driver_device
{
public:
	primo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_iec(*this, CBM_IEC_TAG)
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_cart1(*this, "cartslot1")
		, m_cart2(*this, "cartslot2")
		, m_screen(*this, "screen")
		, m_mem_exp_port(*this, "MEMORY_EXPANSION")
		, m_clock_port(*this, "CPU_CLOCK")
		, m_keyboard_port(*this, "IN%u", 0U)
		, m_reset_port(*this, "RESET")
		, m_vram(*this, "videoram")
	{ }

	void init_primo();

	void primob32(machine_config &config);
	void primob64(machine_config &config);
	void primoa32(machine_config &config);
	void primob48(machine_config &config);
	void primoa64(machine_config &config);
	void primoc64(machine_config &config);
	void primoa48(machine_config &config);

private:
	uint8_t be_1_r(offs_t offset);
	uint8_t be_2_r();
	void ki_1_w(uint8_t data);
	void ki_2_w(uint8_t data);
	void FD_w(uint8_t data);
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	DECLARE_MACHINE_RESET(primob);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void update_memory();
	void common_machine_init();
	void setup_pss(uint8_t* snapshot_data, uint32_t snapshot_size);
	void setup_pp(uint8_t* quickload_data, uint32_t quickload_size);
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void primo32_mem(address_map &map) ATTR_COLD;
	void primo48_mem(address_map &map) ATTR_COLD;
	void primo64_mem(address_map &map) ATTR_COLD;
	void primoa_io(address_map &map) ATTR_COLD;
	void primob_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cbm_iec_device> m_iec;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart1;
	required_device<generic_slot_device> m_cart2;
	required_device<screen_device> m_screen;
	required_ioport m_mem_exp_port;
	required_ioport m_clock_port;
	required_ioport_array<4> m_keyboard_port;
	required_ioport m_reset_port;
	required_shared_ptr<uint8_t> m_vram;

	memory_region *m_cart1_rom = nullptr;
	memory_region *m_cart2_rom = nullptr;

	uint16_t m_video_memory_base = 0;
	uint8_t m_port_FD = 0;
	int m_nmi = 0;
};


#endif // MAME_MICROKEY_PRIMO_H
