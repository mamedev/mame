// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INCLUDES_MIPS_H
#define MAME_INCLUDES_MIPS_H

#pragma once

// processors and memory
#include "cpu/mips/mips1.h"
#include "cpu/nec/v5x.h"
#include "machine/ram.h"

// i/o devices (common)
#include "machine/at_keybc.h"
#include "machine/z80scc.h"
#include "machine/upd765.h"
#include "machine/am79c90.h"

// i/o devices (rx2030)
#include "machine/mc146818.h"
#include "machine/z8038.h"
#include "machine/aic6250.h"

// i/o devices (rx3230)
#include "machine/timekpr.h"
#include "machine/ncr5390.h"
#include "machine/mips_rambo.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"

#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/rs232/rs232.h"

// video and audio
#include "screen.h"
#include "video/bt45x.h"
#include "video/bt459.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "formats/pc_dsk.h"

class rx2030_state : public driver_device
{
public:
	rx2030_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_iop(*this, "iop")
		, m_ram(*this, "ram")
		, m_rom(*this, "rx2030")
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

	u16 lance_r(offs_t offset, u16 mem_mask = 0xffff);
	void lance_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

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
	required_region_ptr<u16> m_rom;

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

class rx3230_state : public driver_device
{
public:
	rx3230_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_rom(*this, "rx3230")
		, m_rambo(*this, "rambo")
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:ncr53c94")
		, m_net(*this, "net")
		, m_scc(*this, "scc")
		, m_tty(*this, "tty%u", 0U)
		, m_rtc(*this, "rtc")
		, m_fdc(*this, "fdc")
		, m_kbdc(*this, "kbdc")
		, m_kbd(*this, "kbd")
		, m_buzzer(*this, "buzzer")
		, m_screen(*this, "screen")
		, m_ramdac(*this, "ramdac")
		, m_vram(*this, "vram")
	{
	}

	// machine config
	void rx3230(machine_config &config);
	void rs3230(machine_config &config);
	void rc3230(machine_config &config);

	void rx3230_init();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void rx3230_map(address_map &map);
	void rs3230_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	u16 lance_r(offs_t offset, u16 mem_mask = 0xffff);
	void lance_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	template <u8 Source> WRITE_LINE_MEMBER(irq_w);

private:
	// processors and memory
	required_device<r3000a_device> m_cpu;
	required_device<ram_device> m_ram;
	required_region_ptr<u32> m_rom;

	// i/o devices
	required_device<mips_rambo_device> m_rambo;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c94_device> m_scsi;
	required_device<am7990_device> m_net;
	required_device<z80scc_device> m_scc;
	required_device_array<rs232_port_device, 2> m_tty;
	required_device<m48t02_device> m_rtc;
	required_device<i82072_device> m_fdc;
	required_device<at_keyboard_controller_device> m_kbdc;
	required_device<pc_kbdc_slot_device> m_kbd;
	required_device<speaker_sound_device> m_buzzer;

	// optional colour video board
	optional_device<screen_device> m_screen;
	optional_device<bt459_device> m_ramdac;
	optional_device<ram_device> m_vram;

	enum int_reg_mask : u8
	{
		INT_SLOT = 0x01, // expansion slot
		INT_KBD  = 0x02, // keyboard controller
		INT_SCC  = 0x04, // serial controller
		INT_SCSI = 0x08, // scsi controller
		INT_NET  = 0x10, // ethernet controller
		INT_DRS  = 0x20, // data rate select
		INT_DSR  = 0x40, // data set ready
		INT_CEB  = 0x80, // modem call indicator

		INT_CLR  = 0xff,
	};

	enum gfx_reg_mask : u8
	{
		GFX_H_BLANK   = 0x10,
		GFX_V_BLANK   = 0x20,
		GFX_COLOR_RSV = 0xce, // reserved
	};

	u8 m_int_reg;
	int m_int0_state;
	int m_int1_state;
};

#endif // MAME_INCLUDES_MIPS_H
