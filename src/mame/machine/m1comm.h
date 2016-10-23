// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#pragma once

#ifndef __M1COMM_H__
#define __M1COMM_H__

#define __M1COMM_SIMULATION__

#include "cpu/z80/z80.h"

#define MCFG_M1COMM_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, M1COMM, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m1comm_device : public device_t
{
public:
	// construction/destruction
	m1comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	required_device<z80_device> m_commcpu;

	// internal API - stuff that happens on the comm board
	// MB89374 registers
	uint8_t dlc_reg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dlc_reg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// MB89237A registers
	uint8_t dma_reg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dma_reg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// single bit registers (74LS74)
	uint8_t syn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void syn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t zfg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zfg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// shared memory 4k
	uint8_t share_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void share_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// public API - stuff that gets called from the model1
	// shared memory 4k
	// reads/writes at I/O 0xB00xxx
	// - share_r
	// - share_w
	// single bit registers (74LS74)
	// reads/writes at I/O 0xB01000
	uint8_t cn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// reads/writes at I/O 0xB01002
	uint8_t fg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	uint8_t m_shared[0x1000]; // 2x 2k = 4k; model1 accesses this with 16bit data and 11bit address (A0 to A10)
	uint8_t m_dlc_reg[0x20];  // MB89374 registers
	uint8_t m_dma_reg[0x20];  // MB89237A registers
	uint8_t   m_syn;                      // bit0 is stored; purpose unknown, bit1 is used to enable/disable VINT/IRQ5
	uint8_t   m_zfg;                      // z80 flip gate? purpose unknown, bit0 is stored
	uint8_t   m_cn;                           // bit0 is used to enable/disable the comm board
	uint8_t   m_fg;                           // flip gate? purpose unknown, bit0 is stored, bit7 is connected to ZFG bit 0

	emu_file m_line_rx;    // rx line - can be either differential, simple serial or toslink
	emu_file m_line_tx;    // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];
	uint8_t m_buffer[0x1000];

#ifdef __M1COMM_SIMULATION__
	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;

	void comm_tick();
#endif
};

// device type definition
extern const device_type M1COMM;

#endif  /* __M1COMM_H__ */
