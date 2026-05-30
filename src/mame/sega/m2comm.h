// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_SEGA_M2COMM_H
#define MAME_SEGA_M2COMM_H

#pragma once

#define M2COMM_SIMULATION


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_m2comm_device : public device_t
{
public:
	sega_m2comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// single bit registers (74LS74)
	uint8_t zfg_r(offs_t offset);
	void zfg_w(uint8_t data);
	// shared memory 16k (these are actually 2x 16k bank switched)
	uint8_t share_r(offs_t offset);
	void share_w(offs_t offset, uint8_t data);

	// public API - stuff that gets called from host
	// shared memory 16k
	// reads/writes at I/O 0x01a10000-0x01a13fff
	// - share_r
	// - share_w
	// single bit registers (74LS74)
	// reads/writes at I/O 0x01a14000
	uint8_t cn_r();
	void cn_w(uint8_t data);
	// reads/writes at I/O 0x01a14002
	uint8_t fg_r();
	void fg_w(uint8_t data);

	// IRQ logic - 5 = VINT, 7 = DLC
	void check_vint_irq();

	void set_frameoffset(uint16_t offset) { m_frameoffset = offset; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t m_shared[0x4000];  // 16k shared memory
	uint8_t m_zfg;             // z80 flip gate - bit 0 switches memory banks, bit7 is connected to FG bit 0
	uint8_t m_cn;              // bit0 is used to enable/disable the comm board
	uint8_t m_fg;              // i960 flip gate - bit0 is stored, bit7 is connected to ZFG bit 0

	uint16_t m_frameoffset;

#ifdef M2COMM_SIMULATION
	class context;
	std::unique_ptr<context> m_context;

	uint8_t m_buffer[0x1000];
	uint8_t m_framesync;

	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;
	uint8_t m_zfg_delay;

	void comm_tick();
	void read_fg();
	unsigned read_frame(unsigned data_size);
	void send_data(uint8_t frame_type, unsigned frame_start, unsigned frame_size, unsigned data_size);
	void send_frame(unsigned data_size);
#endif
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_MODEL2_COMM, sega_m2comm_device)

#endif  // MAME_SEGA_M2COMM_H
