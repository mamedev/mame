// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#pragma once

#ifndef __M2COMM_H__
#define __M2COMM_H__

#define __M2COMM_SIMULATION__

#include "emu.h"

#define MCFG_M2COMM_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, M2COMM, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class m2comm_device : public device_t
{
public:
	// construction/destruction
	m2comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// single bit registers (74LS74)
	uint8_t zfg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zfg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// shared memory 2k
	uint8_t share_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void share_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// public API - stuff that gets called from host
	// shared memory 16k
	// reads/writes at I/O 0x01a10000-0x01a13fff
	// - share_r
	// - share_w
	// single bit registers (74LS74)
	// reads/writes at I/O 0x01a14000
	uint8_t cn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// reads/writes at I/O 0x01a14002
	uint8_t fg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();
#ifdef __M2COMM_SIMULATION__
	void set_linktype(uint16_t linktype);
#endif

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_shared[0x4000]; // 16k shared memory
	uint8_t   m_zfg;                      // z80 flip gate? purpose unknown, bit0 is stored
	uint8_t   m_cn;                           // bit0 is used to enable/disable the comm board
	uint8_t   m_fg;                           // flip gate? purpose unknown, bit0 is stored, bit7 is connected to ZFG bit 0

	emu_file m_line_rx;    // rx line - can be either differential, simple serial or toslink
	emu_file m_line_tx;    // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];
	uint8_t m_buffer[0x4000];

#ifdef __M2COMM_SIMULATION__
	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;

	uint16_t m_linktype;

	void comm_init();
	void comm_tick();
#endif
};

// device type definition
extern const device_type M2COMM;

#endif  /* __M2COMM_H__ */
