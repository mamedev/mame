// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_MULTIBUS_LABTAM_Z80SBC_H
#define MAME_BUS_MULTIBUS_LABTAM_Z80SBC_H

#pragma once

#include "multibus.h"

#include "cpu/z80/z80.h"
#include "machine/am9513.h"
#include "machine/am9519.h"
#include "machine/mm58167.h"
#include "machine/wd_fdc.h"
#include "machine/z80dma.h"
#include "machine/z80sio.h"

#include "machine/input_merger.h"

#include "imagedev/floppy.h"

class labtam_z80sbc_device
	: public device_t
	, public device_multibus_interface
{
public:
	labtam_z80sbc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	// device_t overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_mem(address_map &map) ATTR_COLD;
	void cpu_pio(address_map &map) ATTR_COLD;

	// cpu memory handlers
	u8 mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);

	// memory mapping handlers
	u8 map_r(unsigned map_num, offs_t offset);
	void map_w(unsigned map_num, offs_t offset, u8 data);
	template <unsigned M> u8 map_r(offs_t offset) { return map_r(M, offset); }
	template <unsigned M> void map_w(offs_t offset, u8 data) { map_w(M, offset, data); }

	void intswt_w(u8 data);
	void mapnum_w(u8 data);
	void drive_w(offs_t offset, u8 data);
	void fdcclr_w(u8 data);
	void netclr_w(u8 data);
	void fdcatn_w(u8 data);
	u8 fdcstatus_r();
	u8 drvstatus_r();

	required_device<z80_device> m_cpu;
	required_device<am9513_device> m_stc;
	required_device<am9519_device> m_uic;
	required_device<mm58167_device> m_rtc;
	required_device<wd2793_device> m_fdc;
	required_device_array<z80dma_device, 2> m_dma;
	required_device<z80sio_device> m_sio;
	required_device<input_merger_any_high_device> m_int;

	required_device_array<floppy_connector, 4> m_fdd;

	optional_region_ptr_array<u8, 2> m_eprom;

	required_ioport_array<5> m_e15; // multibus address assignments
	required_ioport m_e21;          // floppy drive option selection

	std::unique_ptr<u8[]> m_ram0;
	std::unique_ptr<u8[]> m_ram1;
	std::unique_ptr<u8[]> m_sram;

	std::unique_ptr<u8[]> m_map_lo;
	std::unique_ptr<u8[]> m_map_hi;

	u8 m_map_mux;
	u8 m_map_num;
	u8 m_map_cnt;

	u8 m_fdcstatus;
	std::optional<u8> m_drive;

	bool m_installed;
};

DECLARE_DEVICE_TYPE(LABTAM_Z80SBC, labtam_z80sbc_device)

#endif // MAME_BUS_MULTIBUS_LABTAM_Z80SBC_H
