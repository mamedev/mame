// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#pragma once

#ifndef __S32COMM_H__
#define __S32COMM_H__

#define __S32COMM_SIMULATION__

#define MCFG_S32COMM_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, S32COMM, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s32comm_device : public device_t
{
public:
	// construction/destruction
	s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	// single bit registers (74LS74)
	uint8_t zfg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void zfg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// shared memory 2k
	uint8_t share_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void share_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// public API - stuff that gets called from host
	// shared memory 2k
	// reads/writes at I/O 0x800xxx
	// - share_r
	// - share_w
	// single bit registers (74LS74)
	// reads/writes at I/O 0x801000
	uint8_t cn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// reads/writes at I/O 0x801002
	uint8_t fg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fg_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();
#ifdef __S32COMM_SIMULATION__
	void set_linktype(uint16_t linktype);
#endif

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_shared[0x800];  // 2k shared memory
	uint8_t   m_zfg;                      // z80 flip gate? purpose unknown, bit0 is stored
	uint8_t   m_cn;                           // bit0 is used to enable/disable the comm board
	uint8_t   m_fg;                           // flip gate? purpose unknown, bit0 is stored, bit7 is connected to ZFG bit 0

	emu_file m_line_rx;    // rx line - can be either differential, simple serial or toslink
	emu_file m_line_tx;    // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];
	uint8_t m_buffer[0x800];

#ifdef __S32COMM_SIMULATION__
	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;

	uint16_t m_linktype;

	void comm_tick();

	void comm_tick_14084();
	void comm_tick_15033();
	void comm_tick_15612();
#endif
};

// device type definition
extern const device_type S32COMM;

#endif  /* __S32COMM_H__ */
