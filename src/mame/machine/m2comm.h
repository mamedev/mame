// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#pragma once

#ifndef __M2COMM_H__
#define __M2COMM_H__

#define __M2COMM_SIMULATION__


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
	DECLARE_READ8_MEMBER(zfg_r);
	DECLARE_WRITE8_MEMBER(zfg_w);
	// shared memory 16k (these are actually 2x 16k bank switched)
	DECLARE_READ8_MEMBER(share_r);
	DECLARE_WRITE8_MEMBER(share_w);

	// public API - stuff that gets called from host
	// shared memory 16k
	// reads/writes at I/O 0x01a10000-0x01a13fff
	// - share_r
	// - share_w
	// single bit registers (74LS74)
	// reads/writes at I/O 0x01a14000
	DECLARE_READ8_MEMBER(cn_r);
	DECLARE_WRITE8_MEMBER(cn_w);
	// reads/writes at I/O 0x01a14002
	DECLARE_READ8_MEMBER(fg_r);
	DECLARE_WRITE8_MEMBER(fg_w);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t m_shared[0x4000]; // 16k shared memory
	uint8_t m_zfg;            // z80 flip gate - bit 0 switches memory banks, bit7 is connected to FG bit 0
	uint8_t m_cn;             // bit0 is used to enable/disable the comm board
	uint8_t m_fg;             // i960 flip gate - bit0 is stored, bit7 is connected to ZFG bit 0

	emu_file m_line_rx;       // rx line - can be either differential, simple serial or toslink
	emu_file m_line_tx;       // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];
	uint8_t m_buffer[0x4000];

#ifdef __M2COMM_SIMULATION__
	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;

	void comm_tick();
#endif
};

// device type definition
extern const device_type M2COMM;

#endif  /* __M2COMM_H__ */
