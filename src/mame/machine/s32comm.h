// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_MACHINE_S32COMM_H
#define MAME_MACHINE_S32COMM_H

#pragma once

#define S32COMM_SIMULATION

#include "osdcore.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s32comm_device : public device_t
{
public:
	// construction/destruction
	s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// single bit registers (74LS74)
	DECLARE_READ8_MEMBER(zfg_r);
	DECLARE_WRITE8_MEMBER(zfg_w);
	// shared memory 2k
	DECLARE_READ8_MEMBER(share_r);
	DECLARE_WRITE8_MEMBER(share_w);

	// public API - stuff that gets called from host
	// shared memory 2k
	// reads/writes at I/O 0x800xxx
	// - share_r
	// - share_w
	// single bit registers (74LS74)
	// reads/writes at I/O 0x801000
	DECLARE_READ8_MEMBER(cn_r);
	DECLARE_WRITE8_MEMBER(cn_w);
	// reads/writes at I/O 0x801002
	DECLARE_READ8_MEMBER(fg_r);
	DECLARE_WRITE8_MEMBER(fg_w);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();
#ifdef S32COMM_SIMULATION
	void set_linktype(uint16_t linktype);
#endif

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	uint8_t m_shared[0x800]; // 2k shared memory
	uint8_t m_zfg;           // z80 flip gate? purpose unknown, bit0 is stored
	uint8_t m_cn;            // bit0 is used to enable/disable the comm board
	uint8_t m_fg;            // flip gate? purpose unknown, bit0 is stored, bit7 is connected to ZFG bit 0

	osd_file::ptr m_line_rx; // rx line - can be either differential, simple serial or toslink
	osd_file::ptr m_line_tx; // tx line - is differential, simple serial and toslink
	char m_localhost[256];
	char m_remotehost[256];
	uint8_t m_buffer0[0x100];
	uint8_t m_buffer1[0x100];
	uint8_t m_framesync;

#ifdef S32COMM_SIMULATION
	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;
	uint16_t m_linktype;

	void comm_tick();
	int read_frame(int dataSize);
	void send_data(uint8_t frameType, int frameStart, int frameSize, int dataSize);
	void send_frame(int dataSize);

	void comm_tick_14084();
	void comm_tick_15033();
	void comm_tick_15612();
#endif
};

// device type definition
DECLARE_DEVICE_TYPE(S32COMM, s32comm_device)

#endif // MAME_MACHINE_S32COMM_H
