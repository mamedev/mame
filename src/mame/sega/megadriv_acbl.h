// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_SEGA_MEGADRIV_ACBL_H
#define MAME_SEGA_MEGADRIV_ACBL_H

#include "megadriv.h"
#include "cpu/pic16c5x/pic16c5x.h"


class md_boot_state : public md_ctrl_state
{
public:
	md_boot_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_ctrl_state(mconfig, type, tag),
		m_io_exp(*this, "EXP")
	{ }

	void megadrvb(machine_config &config);
	void md_bootleg(machine_config &config);

	void init_aladmdb();
	void init_srmdb();
	void init_barek2ch();
	void init_barek3();
	void init_barek3a();
	void init_sonic2mb();
	void init_twinktmb();
	void init_jparkmb();

protected:
	uint16_t dsw_r(offs_t offset);

	void md_bootleg_map(address_map &map) ATTR_COLD;

private:
	void aladmdb_w(uint16_t data);
	uint16_t aladmdb_r();
	uint16_t barek3mba_r();
	uint16_t jparkmb_r();
	uint16_t twinktmb_r();

	optional_ioport m_io_exp;

	// bootleg specific
	int m_aladmdb_mcu_port = 0;
};

// for games with emulated PIC microcontroller
class md_boot_mcu_state : public md_boot_state
{
public:
	md_boot_mcu_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_boot_state(mconfig, type, tag),
		m_mcu(*this, "mcu"),
		m_dsw(*this, "DSW")
	{ }

	void md_boot_mcu(machine_config &config);

private:
	void md_boot_mcu_map(address_map &map) ATTR_COLD;

	uint16_t mcu_r();
	void mcu_w(uint16_t data);

	void mcu_porta_w(uint8_t data);
	uint8_t mcu_portc_r();
	void mcu_portb_w(uint8_t data);
	void mcu_portc_w(uint8_t data);

	required_device<pic16c57_device> m_mcu;
	required_ioport m_dsw;

	uint8_t m_mcu_porta = 0;
	uint8_t m_mcu_portc = 0;
	uint8_t m_mcu_in_latch_msb = 0;
	uint8_t m_mcu_in_latch_lsb = 0;
	uint8_t m_mcu_out_latch_msb = 0;
	uint8_t m_mcu_out_latch_lsb = 0;
};

class md_sonic3bl_state : public md_boot_state
{
public:
	md_sonic3bl_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_boot_state(mconfig, type, tag),
		m_in_coin(*this, "COIN"),
		m_in_mcu(*this, "MCU")
	{ }

	void init_sonic3mb();

private:
	void prot_w(u8 data);
	uint16_t prot_r();

	required_ioport m_in_coin;
	required_ioport m_in_mcu;

	u8 m_prot_cmd = 0;
};

class md_boot_6button_state : public md_boot_state
{
public:
	md_boot_6button_state(const machine_config& mconfig, device_type type, const char* tag) :
		md_boot_state(mconfig, type, tag)
	{
	}

	void megadrvb_6b(machine_config &config);
	void ssf2mdb(machine_config &config);

	void init_mk3mdb();
	void init_bk3ssrmb();
	void init_barekch();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void ssf2mdb_68k_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SEGA_MEGADRIV_ACBL_H
