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
	m2comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	// single bit registers (74LS74)
	DECLARE_READ8_MEMBER(zfg_r);
	DECLARE_WRITE8_MEMBER(zfg_w);
	// shared memory 2k
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
#ifdef __M2COMM_SIMULATION__
	void set_linktype(UINT16 linktype);
#endif

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	UINT8 m_shared[0x4000]; // 16k shared memory
	UINT8   m_zfg;                      // z80 flip gate? purpose unknown, bit0 is stored
	UINT8   m_cn;                           // bit0 is used to enable/disable the comm board
	UINT8   m_fg;                           // flip gate? purpose unknown, bit0 is stored, bit7 is connected to ZFG bit 0

	emu_file m_line_rx;    // rx line - can be either differential, simple serial or toslink
	emu_file m_line_tx;    // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];
	UINT8 m_buffer[0x4000];

#ifdef __M2COMM_SIMULATION__
	UINT8 m_linkenable;
	UINT16 m_linktimer;
	UINT8 m_linkalive;
	UINT8 m_linkid;
	UINT8 m_linkcount;

	UINT16 m_linktype;

	void comm_tick();

	void comm_tick_16726();
#endif
};

// device type definition
extern const device_type M2COMM;

#endif  /* __M2COMM_H__ */
