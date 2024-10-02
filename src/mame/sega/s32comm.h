// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_SEGA_S32COMM_H
#define MAME_SEGA_S32COMM_H

#pragma once

#define S32COMM_SIMULATION

#include "osdfile.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class s32comm_device : public device_t
{
public:
	// construction/destruction
	s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// single bit registers (74LS74)
	uint8_t zfg_r(offs_t offset);
	void zfg_w(uint8_t data);
	// shared memory 2k
	uint8_t share_r(offs_t offset);
	void share_w(offs_t offset, uint8_t data);

	// public API - stuff that gets called from host
	// shared memory 2k
	// reads/writes at I/O 0x800xxx
	// - share_r
	// - share_w
	// single bit registers (74LS74)
	// reads/writes at I/O 0x801000
	uint8_t cn_r();
	void cn_w(uint8_t data);
	// reads/writes at I/O 0x801002
	uint8_t fg_r();
	void fg_w(uint8_t data);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();
#ifdef S32COMM_SIMULATION
	void set_linktype(uint16_t linktype);
#endif

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t m_shared[0x800]{}; // 2k shared memory
	uint8_t m_zfg = 0;           // z80 flip gate? purpose unknown, bit0 is stored
	uint8_t m_cn = 0;            // bit0 is used to enable/disable the comm board
	uint8_t m_fg = 0;            // flip gate? purpose unknown, bit0 is stored, bit7 is connected to ZFG bit 0

	osd_file::ptr m_line_rx; // rx line - can be either differential, simple serial or toslink
	osd_file::ptr m_line_tx; // tx line - is differential, simple serial and toslink
	char m_localhost[256]{};
	char m_remotehost[256]{};
	uint8_t m_buffer0[0x100]{};
	uint8_t m_buffer1[0x100]{};
	uint8_t m_framesync = 0;

#ifdef S32COMM_SIMULATION
	uint8_t m_linkenable = 0;
	uint16_t m_linktimer = 0;
	uint8_t m_linkalive = 0;
	uint8_t m_linkid = 0;
	uint8_t m_linkcount = 0;
	uint16_t m_linktype = 0;

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

#endif // MAME_SEGA_S32COMM_H
