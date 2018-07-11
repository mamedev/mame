// license:BSD-3-Clause
// copyright-holders:David Haywood, Charles MacDonald
#ifndef MAME_MACHINE_DECO146_H
#define MAME_MACHINE_DECO146_H

#pragma once

#include "machine/gen_latch.h"


#define MCFG_DECO146_IN_PORTA_CB(_devcb) \
	downcast<deco_146_base_device &>(*device).set_port_a_cb(DEVCB_##_devcb);

#define MCFG_DECO146_IN_PORTB_CB(_devcb) \
	downcast<deco_146_base_device &>(*device).set_port_b_cb(DEVCB_##_devcb);

#define MCFG_DECO146_IN_PORTC_CB(_devcb) \
	downcast<deco_146_base_device &>(*device).set_port_c_cb(DEVCB_##_devcb);

#define MCFG_DECO146_SOUNDLATCH_IRQ_CB(_devcb) \
	downcast<deco_146_base_device &>(*device).set_soundlatch_irq_callback(DEVCB_##_devcb);

// there are some standard ways the chip gets hooked up, so have them here ready to use
#define MCFG_DECO146_SET_INTERFACE_SCRAMBLE( a9,a8,a7,a6,a5,a4,a3,a2,a1,a0 ) \
	downcast<deco_146_base_device &>(*device).set_interface_scramble(a9,a8,a7,a6,a5,a4,a3,a2,a1,a0);

#define MCFG_DECO146_SET_INTERFACE_SCRAMBLE_REVERSE \
	downcast<deco_146_base_device &>(*device).set_interface_scramble_reverse();

#define MCFG_DECO146_SET_INTERFACE_SCRAMBLE_INTERLEAVE \
	downcast<deco_146_base_device &>(*device).set_interface_scramble_interleave();

#define MCFG_DECO146_SET_USE_MAGIC_ADDRESS_XOR \
	downcast<deco_146_base_device &>(*device).set_use_magic_read_address_xor(true);




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
	uint8_t mapping[16];
	int use_xor;
	int use_nand;
};



/* Data East 146 protection chip */

class deco_146_base_device : public device_t
{
public:
	void write_data(address_space &space, uint16_t address, uint16_t data, uint16_t mem_mask, uint8_t &csflags);
	uint16_t read_data(uint16_t address, uint16_t mem_mask, uint8_t &csflags);

	template<class Object> devcb_base &set_port_a_cb(Object &&object) { return m_port_a_r.set_callback(std::forward<Object>(object)); }
	template<class Object> devcb_base &set_port_b_cb(Object &&object) { return m_port_b_r.set_callback(std::forward<Object>(object)); }
	template<class Object> devcb_base &set_port_c_cb(Object &&object) { return m_port_c_r.set_callback(std::forward<Object>(object)); }
	auto port_a_cb() { return m_port_a_r.bind(); }
	auto port_b_cb() { return m_port_b_r.bind(); }
	auto port_c_cb() { return m_port_c_r.bind(); }
	void set_interface_scramble(uint8_t a9, uint8_t a8, uint8_t a7, uint8_t a6, uint8_t a5, uint8_t a4, uint8_t a3,uint8_t a2,uint8_t a1,uint8_t a0)
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

	template <class Object> devcb_base &set_soundlatch_irq_callback(Object &&cb) { return m_soundlatch_irq_cb.set_callback(std::forward<Object>(cb)); }
	auto soundlatch_irq_cb() { return m_soundlatch_irq_cb.bind(); }

	DECLARE_READ8_MEMBER( soundlatch_r );

	devcb_read16 m_port_a_r;
	devcb_read16 m_port_b_r;
	devcb_read16 m_port_c_r;

	uint8_t m_bankswitch_swap_read_address;
	uint16_t m_magic_read_address_xor;
	bool m_magic_read_address_xor_enabled;
	uint8_t m_xor_port;
	uint8_t m_mask_port;
	uint8_t m_soundlatch_port;

	uint8_t m_external_addrswap[10];

	deco146port_xx const *m_lookup_table;

protected:
	deco_146_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	uint16_t read_protport(uint16_t address, uint16_t mem_mask);
	virtual void write_protport(address_space &space, uint16_t address, uint16_t data, uint16_t mem_mask);
	virtual uint16_t read_data_getloc(uint16_t address, int& location);

	uint16_t m_rambank0[0x80];
	uint16_t m_rambank1[0x80];

	int m_current_rambank;

	uint16_t m_nand;
	uint16_t m_xor;

	uint16_t m_latchaddr;
	uint16_t m_latchdata;

	uint8_t m_configregion; // which value of upper 4 address lines accesses the config region
	int m_latchflag;

private:
	TIMER_CALLBACK_MEMBER(write_soundlatch);

	uint8_t region_selects[6];

	uint8_t m_soundlatch;
	devcb_write_line m_soundlatch_irq_cb;
};

class deco146_device : public deco_146_base_device
{
public:
	deco146_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(DECO146PROT, deco146_device)

#define MCFG_DECO146_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECO146PROT, 0)

#endif // MAME_MACHINE_DECO146_H
