// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INCLUDES_MIPS_H
#define MAME_INCLUDES_MIPS_H

#pragma once

// processors and memory
#include "cpu/mips/r3000.h"
#include "cpu/nec/v53.h"
#include "machine/ram.h"

// i/o devices
#include "machine/am79c90.h"
#include "machine/at_keybc.h"
#include "machine/mc146818.h"
#include "machine/ncr5390.h"
#include "machine/upd765.h"
#include "machine/wd33c93.h"
#include "machine/z80scc.h"

// busses and connectors
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/rs232/rs232.h"

// video
#include "screen.h"
#include "video/bt45x.h"

#include "formats/pc_dsk.h"

class rx2030_state : public driver_device
{
public:
	rx2030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "cpu")
		, m_iop(*this, "iop")
		, m_ram(*this, "ram")
		, m_rtc(*this, "rtc")
		, m_kbdc(*this, "kbdc")
		, m_kbd(*this, "kbd")
		, m_scc(*this, "scc")
		, m_tty(*this, "tty%u", 0U)
		, m_fdc(*this, "fdc")
		, m_scsi(*this, "scsi")
		, m_net(*this, "net")
		, m_screen(*this, "screen")
		, m_ramdac(*this, "ramdac")
		, m_vram(*this, "vram")
	{
	}

	// machine config
	void rx2030(machine_config &config);
	void rs2030(machine_config &config);
	void rc2030(machine_config &config);

	void rx2030_init();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);
	void iop_program_map(address_map &map);
	void iop_io_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	// processors and memory
	required_device<r2000_device> m_maincpu;
	required_device<v53_device> m_iop;
	required_device<ram_device> m_ram;

	// i/o devices
	required_device<mc146818_device> m_rtc;
	required_device<at_keyboard_controller_device> m_kbdc;
	required_device<pc_kbdc_slot_device> m_kbd;
	required_device<z80scc_device> m_scc;
	required_device_array<rs232_port_device, 2> m_tty;
	required_device<wd37c65c_device> m_fdc;
	required_device<wd33c93_device> m_scsi;
	required_device<am7990_device> m_net;

	// optional video board
	optional_device<screen_device> m_screen;
	optional_device<bt458_device> m_ramdac;
	optional_device<ram_device> m_vram;

	// machine state
	u16 m_mmu[32];
};

#endif // MAME_INCLUDES_MIPS_H
