// license:BSD-3-Clause
// copyright-holders:David Haywood, Charles MacDonald
#ifndef MAME_DATAEAST_DECO146_H
#define MAME_DATAEAST_DECO146_H

#pragma once

#include "machine/gen_latch.h"

#define BLK (0xff)
#define INPUT_PORT_A (-1)
#define INPUT_PORT_B (-2)
#define INPUT_PORT_C (-3)


#define NIB3__ 0xc, 0xd, 0xe, 0xf
#define NIB3R1 0xd, 0xe, 0xf, 0xc
#define NIB3R2 0xe, 0xf, 0xc, 0xd
#define NIB3R3 0xf, 0xc, 0xd, 0xe

#define NIB2__ 0x8, 0x9, 0xa, 0xb
#define NIB2R1 0x9, 0xa, 0xb, 0x8
#define NIB2R2 0xa, 0xb, 0x8, 0x9
#define NIB2R3 0xb, 0x8, 0x9, 0xa

#define NIB1__ 0x4, 0x5, 0x6, 0x7
#define NIB1R1 0x5, 0x6, 0x7, 0x4
#define NIB1R2 0x6, 0x7, 0x4, 0x5
#define NIB1R3 0x7, 0x4, 0x5, 0x6

#define NIB0__ 0x0, 0x1, 0x2, 0x3
#define NIB0R1 0x1, 0x2, 0x3, 0x0
#define NIB0R2 0x2, 0x3, 0x0, 0x1
#define NIB0R3 0x3, 0x0, 0x1, 0x2

#define BLANK_ BLK, BLK, BLK, BLK

struct deco146port_xx
{
	int write_offset;
	u8 mapping[16];
	int use_xor;
	int use_nand;
};


/* Data East 146 protection chip */

class deco_146_base_device : public device_t
{
public:
	void write_data(u16 address, u16 data, u16 mem_mask, u8 &csflags);
	u16 read_data(u16 address, u8 &csflags);

	auto port_a_cb() { return m_port_a_r.bind(); }
	auto port_b_cb() { return m_port_b_r.bind(); }
	auto port_c_cb() { return m_port_c_r.bind(); }

	// there are some standard ways the chip gets hooked up, so have them here ready to use
	void set_interface_scramble(u8 a9, u8 a8, u8 a7, u8 a6, u8 a5, u8 a4, u8 a3,u8 a2,u8 a1,u8 a0)
	{
		m_external_addrswap[9] = a9;
		m_external_addrswap[8] = a8;
		m_external_addrswap[7] = a7;
		m_external_addrswap[6] = a6;
		m_external_addrswap[5] = a5;
		m_external_addrswap[4] = a4;
		m_external_addrswap[3] = a3;
		m_external_addrswap[2] = a2;
		m_external_addrswap[1] = a1;
		m_external_addrswap[0] = a0;
	}
	void set_interface_scramble_reverse() { set_interface_scramble(0,1,2,3,4,5,6,7,8,9); }
	void set_interface_scramble_interleave() { set_interface_scramble(4, 5, 3, 6, 2, 7, 1, 8, 0, 9); }
	void set_use_magic_read_address_xor(bool use_xor) { m_magic_read_address_xor_enabled = use_xor; }

	auto soundlatch_irq_cb() { return m_soundlatch_irq_cb.bind(); }

	u8 soundlatch_r();

	devcb_read16 m_port_a_r;
	devcb_read16 m_port_b_r;
	devcb_read16 m_port_c_r;

	u8 m_bankswitch_swap_read_address;
	u16 m_magic_read_address_xor;
	bool m_magic_read_address_xor_enabled;
	u8 m_xor_port;
	u8 m_mask_port;
	u8 m_soundlatch_port;

	u8 m_external_addrswap[10];

	deco146port_xx const *m_lookup_table;

protected:
	deco_146_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u16 read_protport(u16 address);
	virtual void write_protport(u16 address, u16 data, u16 mem_mask);
	virtual u16 read_data_getloc(u16 address, int& location);

	std::unique_ptr<u16[]> m_rambank[2];

	int m_current_rambank;

	u16 m_nand;
	u16 m_xor;

	u16 m_latchaddr;
	u16 m_latchdata;

	u8 m_configregion; // which value of upper 4 address lines accesses the config region
	int m_latchflag;

private:
	TIMER_CALLBACK_MEMBER(write_soundlatch);

	u8 region_selects[6];

	u8 m_soundlatch;
	devcb_write_line m_soundlatch_irq_cb;
};

class deco146_device : public deco_146_base_device
{
public:
	deco146_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(DECO146PROT, deco146_device)

#endif // MAME_DATAEAST_DECO146_H
