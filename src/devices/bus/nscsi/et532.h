// license:BSD-3-Clause
// copyright-holders:Dave Rand

#ifndef MAME_BUS_NSCSI_ET532_H
#define MAME_BUS_NSCSI_ET532_H

#pragma once

#include "machine/nscsi_bus.h"

#include "bus/rs232/rs232.h"
#include "cpu/ns32000/ns32000.h"
#include "machine/dp8390.h"
#include "machine/eepromser.h"
#include "machine/input_merger.h"
#include "machine/ncr5380.h"   // dp8490
#include "machine/ns32081.h"
#include "machine/scc2698b.h"

// ET532 board core (NS32532 + FPU + DRAM + EPROM + COPS + 2x SCC2698 + 16 rs232 +
// DP8390 Ethernet + DP8490 SCSI + CPU-driven pseudo-DMA), shared by BOTH the
// stand-alone et532 system driver and the et532 SCSI slot card.  The board's
// DP8490 is exposed (tag "dp8490") for whatever bus the owner attaches it to:
// the stand-alone driver puts it on a private bus with disk/tape (initiator role);
// the slot card hands it to a host nscsi bus (target role).  The role is a firmware
// choice (MODE_TARGET), not the wiring.
class et532_board_device : public device_t
{
public:
	et532_board_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;

private:
	enum dma_state : u8
	{
		IDLE,
		WR1, WR2, WR3,
		RD1, RD2, RD3, RD4,
	};

	void drq_w(int state);
	void irq_w(int state);
	u32 dma_r(offs_t offset, u32 mem_mask);
	void dma_w(offs_t offset, u32 data, u32 mem_mask);

	u8 net_ram_r(offs_t offset);
	void net_ram_w(offs_t offset, u8 data);
	u8 cops_r(offs_t offset);
	void cops_w(offs_t offset, u8 data);

	required_device<ns32532_device> m_cpu;
	required_device<ns32381_device> m_fpu;
	required_device<dp8490_device> m_scsi;
	required_device<dp8390d_device> m_net;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device_array<scc2698b_device, 2> m_duart;
	required_device_array<rs232_port_device, 16> m_serial;
	required_device<input_merger_device> m_irqs;

	u8 m_net_ram[0x2000]; // 8KB DP8390 packet buffer (6264 SRAM)

	bool m_drq;
	bool m_irq;
	u32 m_dma;
	dma_state m_state;
};

// ET532 as an intelligent SCSI slot card: drops into an NSCSI_CONNECTOR on a host
// nscsi bus (e.g. the PC532 532SC/DP8490 expansion bus), presenting the board's
// DP8490 as the SCSI device while the on-board NS32532 runs the SFP firmware.
class et532_scsi_device : public device_t, public nscsi_slot_card_interface
{
public:
	et532_scsi_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<et532_board_device> m_board;
};

DECLARE_DEVICE_TYPE(ET532_BOARD, et532_board_device)
DECLARE_DEVICE_TYPE(ET532_SCSI, et532_scsi_device)

#endif // MAME_BUS_NSCSI_ET532_H
