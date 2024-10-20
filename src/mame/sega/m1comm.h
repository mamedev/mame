// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_SEGA_M1COMM_H
#define MAME_SEGA_M1COMM_H

#pragma once

#define M1COMM_SIMULATION

#include "osdfile.h"
#include "cpu/z80/z80.h"
#include "machine/am9517a.h"
#include "machine/mb89374.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m1comm_device : public device_t
{
public:
	// construction/destruction
	m1comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	void m1comm_io(address_map &map) ATTR_COLD;
	void m1comm_mem(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<z80_device> m_cpu;
	required_device<am9517a_device> m_dma;
	required_device<mb89374_device> m_dlc;

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

	uint8_t m_shared[0x1000]{}; // 2x 2k = 4k; model1 accesses this with 16bit data and 11bit address (A0 to A10)
	uint8_t m_syn = 0;            // bit0 is used to trigger DOP line on VINT, bit1 is used to enable/disable VINT/IRQ5
	uint8_t m_zfg = 0;            // z80 flip gate, bit0 is stored
	uint8_t m_cn = 0;             // bit0 is used to enable/disable the comm board
	uint8_t m_fg = 0;             // flip gate, bit0 is stored, bit7 is connected to ZFG bit 0

#ifdef M1COMM_SIMULATION
	osd_file::ptr m_line_rx;  // rx line - can be either differential, simple serial or toslink
	osd_file::ptr m_line_tx;  // tx line - is differential, simple serial and toslink
	char m_localhost[256]{};
	char m_remotehost[256]{};
	uint8_t m_buffer0[0x200]{};
	uint8_t m_buffer1[0x200]{};
	uint8_t m_framesync;

	uint8_t m_linkenable = 0;
	uint16_t m_linktimer = 0;
	uint8_t m_linkalive = 0;
	uint8_t m_linkid = 0;
	uint8_t m_linkcount = 0;

	void comm_tick();
	int read_frame(int dataSize);
	void send_data(uint8_t frameType, int frameStart, int frameSize, int dataSize);
	void send_frame(int dataSize);
#endif
};

// device type definition
DECLARE_DEVICE_TYPE(M1COMM, m1comm_device)

#endif  // MAME_SEGA_M1COMM_H
