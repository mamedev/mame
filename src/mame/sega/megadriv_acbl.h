// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_SEGA_MEGADRIV_ACBL_H
#define MAME_SEGA_MEGADRIV_ACBL_H

#include "megadriv.h"

#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"


class md_boot_state : public md_ctrl_state
{
public:
	md_boot_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_ctrl_state(mconfig, type, tag),
		m_io_exp(*this, "EXP")
	{ }

	void megadrvb(machine_config &config);
	void md_bootleg(machine_config &config);

	void init_srmdb();
	void init_barek2ch();
	void init_barek3();
	void init_barek3a();
	void init_sonic2mb();
	void init_twinktmb();

protected:
	uint16_t dsw_r(offs_t offset);

	void md_bootleg_map(address_map &map) ATTR_COLD;

private:
	void aladmdb_w(uint16_t data);
	uint16_t barek3mba_r();
	uint16_t twinktmb_r();

	optional_ioport m_io_exp;
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
	void init_srssf2mb();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void ssf2mdb_68k_map(address_map &map) ATTR_COLD;
};


class puckpkmn_state : public md_boot_state
{
public:
	puckpkmn_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_boot_state(mconfig, type, tag)
	{
	}

	void puckpkmn(machine_config &config) ATTR_COLD;
	void jingling(machine_config &config) ATTR_COLD;
	void puckpkmnb(machine_config &config) ATTR_COLD;

	void init_puckpkmn() ATTR_COLD;

protected:
	void puckpkmn_base_map(address_map &map) ATTR_COLD;

private:
	void puckpkmn_map(address_map &map) ATTR_COLD;
	void jingling_map(address_map &map) ATTR_COLD;
	void puckpkmnb_map(address_map &map) ATTR_COLD;
};


class jzth_state : public puckpkmn_state
{
public:
	using puckpkmn_state::puckpkmn_state;

	void jzth(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void bl_710000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t bl_710000_r();

	void jzth_map(address_map &map) ATTR_COLD;

	int m_protcount = 0;
};

class songjang_state : public puckpkmn_state
{
public:
	songjang_state(const machine_config &mconfig, device_type type, const char *tag) :
		puckpkmn_state(mconfig, type, tag)
	{
	}

	void songjang(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint16_t unhandled_protval_r();
	void unhandled_protval_w(offs_t offset, uint16_t data);

	uint16_t protval_r();
	void protval_w(uint16_t data);

	virtual uint16_t sj_70001c_r();

	void songjang_map(address_map &map) ATTR_COLD;

	uint16_t m_protval;
};

class shuifeng_state : public songjang_state
{
public:
	shuifeng_state(const machine_config &mconfig, device_type type, const char *tag) :
		songjang_state(mconfig, type, tag)
	{
	}

private:
	virtual uint16_t sj_70001c_r() override;
};



#endif // MAME_SEGA_MEGADRIV_ACBL_H
