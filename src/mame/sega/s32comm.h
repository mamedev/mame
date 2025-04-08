// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_SEGA_S32COMM_H
#define MAME_SEGA_S32COMM_H

#pragma once

#define S32COMM_SIMULATION


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_s32comm_device : public device_t
{
public:
	sega_s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t m_shared[0x800];  // 2k shared memory
	uint8_t m_zfg;            // z80 flip gate? purpose unknown, bit0 is stored
	uint8_t m_cn;             // bit0 is used to enable/disable the comm board
	uint8_t m_fg;             // flip gate? purpose unknown, bit0 is stored, bit7 is connected to ZFG bit 0

#ifdef S32COMM_SIMULATION
	class context;
	std::unique_ptr<context> m_context;

	uint8_t m_buffer[0x100];
	uint8_t m_framesync;

	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;
	uint16_t m_linktype;

	void comm_tick();
	unsigned read_frame(unsigned data_size);
	void send_data(uint8_t frame_type, unsigned frame_start, unsigned frame_size, unsigned data_size);
	void send_frame(unsigned data_size);

	void comm_tick_14084();
	void comm_tick_15033();
	void comm_tick_15612();
#endif
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_SYSTEM32_COMM, sega_s32comm_device)

#endif // MAME_SEGA_S32COMM_H
