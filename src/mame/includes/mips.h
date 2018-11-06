// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INCLUDES_MIPS_H
#define MAME_INCLUDES_MIPS_H

#pragma once

// processors and memory
#include "cpu/mips/mips1.h"
#include "cpu/nec/v5x.h"
#include "machine/ram.h"

// i/o devices
#include "machine/mc146818.h"
#include "machine/z8038.h"
#include "machine/at_keybc.h"
#include "machine/z80scc.h"
#include "machine/upd765.h"
#include "machine/aic6250.h"
#include "machine/am79c90.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"

#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/rs232/rs232.h"

// video and audio
#include "screen.h"
#include "video/bt45x.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "formats/pc_dsk.h"

class rx2030_state : public driver_device
{
public:
	rx2030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_iop(*this, "iop")
		, m_ram(*this, "ram")
		, m_rtc(*this, "rtc")
		, m_fio(*this, "fio")
		, m_kbdc(*this, "kbdc")
		, m_kbd(*this, "kbd")
		, m_scc(*this, "scc")
		, m_tty(*this, "tty%u", 0U)
		, m_fdc(*this, "fdc")
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:aic6250")
		, m_net(*this, "net")
		, m_buzzer(*this, "buzzer")
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
	void rx2030_map(address_map &map);
	void rs2030_map(address_map &map);

	void iop_program_map(address_map &map);
	void iop_io_map(address_map &map);

	u16 mmu_r(offs_t offset, u16 mem_mask = 0xffff);
	void mmu_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	enum iop_interface_status_mask : u8
	{
		FPU_ABSENT = 0x01, // clear if FPU installed
		DBG_ABSENT = 0x02, // clear if debug board installed
		IOP_IRQ    = 0x04, // set when CPU interrupts IOP
		VID_ABSENT = 0x10, // clear if video board installed
		IOP_IACK   = 0x40, // set when IOP acknowledges CPU interrupt
		IOP_NERR   = 0x80, // clear when IOP receives parity error
	};

	// processors and memory
	required_device<r2000a_device> m_cpu;
	required_device<v50_device> m_iop;
	required_device<ram_device> m_ram;

	// i/o devices
	required_device<mc146818_device> m_rtc;
	required_device<z8038_device> m_fio;
	required_device<at_keyboard_controller_device> m_kbdc;
	required_device<pc_kbdc_slot_device> m_kbd;
	required_device<z80scc_device> m_scc;
	required_device_array<rs232_port_device, 2> m_tty;
	required_device<wd37c65c_device> m_fdc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<aic6250_device> m_scsi;
	required_device<am7990_device> m_net;
	required_device<speaker_sound_device> m_buzzer;

	// optional video board
	optional_device<screen_device> m_screen;
	optional_device<bt458_device> m_ramdac;
	optional_shared_ptr<u32> m_vram;

	// machine state
	u16 m_mmu[32];

	u8 m_iop_interface;
};

#endif // MAME_INCLUDES_MIPS_H
