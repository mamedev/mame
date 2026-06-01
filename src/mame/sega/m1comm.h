// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_SEGA_M1COMM_H
#define MAME_SEGA_M1COMM_H

#pragma once

#define M1COMM_SIMULATION

#include "cpu/z80/z80.h"
#include "machine/am9517a.h"
#include "machine/mb89374.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class sega_m1comm_device : public device_t
{
public:
	sega_m1comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// public API - stuff that gets called from the model1
	// shared memory 4k
	// reads/writes at I/O 0xB00xxx
	uint8_t share_r(offs_t offset);
	void share_w(offs_t offset, uint8_t data);

	// single bit registers (74LS74)
	// reads/writes at I/O 0xB01000
	uint8_t cn_r();
	void cn_w(uint8_t data);

	// reads/writes at I/O 0xB01002
	uint8_t fg_r();
	void fg_w(uint8_t data);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<z80_device> m_cpu;
	required_device<am9517a_device> m_dma;
	required_device<mb89374_device> m_dlc;

	void m1comm_io(address_map &map) ATTR_COLD;
	void m1comm_mem(address_map &map) ATTR_COLD;

	// MB89374 handler
	void dlc_int7_w(int state);

	// MB89237A handler
	void dma_hreq_w(int state);
	uint8_t dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, uint8_t data);

	// single bit registers (74LS74)
	uint8_t syn_r();
	void syn_w(uint8_t data);
	uint8_t zfg_r();
	void zfg_w(uint8_t data);

	// shared memory 4k
	// reads/writes at 0xC000-FFFF
	// - share_r
	// - share_w

	uint8_t m_shared[0x1000]; // 2x 2k = 4k; model1 accesses this with 16bit data and 11bit address (A0 to A10)
	uint8_t m_syn;            // bit0 is used to trigger DOP line on VINT, bit1 is used to enable/disable VINT/IRQ5
	uint8_t m_zfg;            // z80 flip gate, bit0 is stored
	uint8_t m_cn;             // bit0 is used to enable/disable the comm board
	uint8_t m_fg;             // flip gate, bit0 is stored, bit7 is connected to ZFG bit 0

#ifdef M1COMM_SIMULATION
	class context;
	std::unique_ptr<context> m_context;

	uint8_t m_buffer[0x200];
	uint8_t m_framesync;

	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;

	void comm_tick();
	unsigned read_frame(unsigned data_size);
	void send_data(uint8_t frame_type, unsigned frame_start, unsigned frame_size, unsigned data_size);
	void send_frame(unsigned data_size);
#endif
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_MODEL1_COMM, sega_m1comm_device)

#endif  // MAME_SEGA_M1COMM_H
