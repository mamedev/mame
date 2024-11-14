// license:LGPL-2.1+
// copyright-holders: Samuele Zannoli, R. Belmont, ElSemi, David Haywood, Angelo Salese, Olivier Galibert, MetalliC

#ifndef MAME_SEGA_NAOMI_H
#define MAME_SEGA_NAOMI_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/x76f100.h"
#include "machine/eepromser.h"
//#include "machine/intelfsh.h"
#include "maple-dc.h"
#include "dc-ctrl.h"
#include "mie.h"
#include "naomirom.h"
#include "naomigd.h"
#include "naomim1.h"
#include "naomim2.h"
#include "naomim4.h"
#include "machine/nvram.h"
#include "machine/aicartc.h"
#include "machine/jvsdev.h"
#include "jvs13551.h"
#include "m3comm.h"
#include "gunsense.h"
#include "segashiobd.h"
#include "sound/aica.h"
#include "dc.h"


class naomi_state : public dc_state
{
public:
	naomi_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag)
		, m_eeprom(*this, "main_eeprom")
		, m_rombase(*this, "maincpu")
		, m_mp(*this, "KEY%u", 1U)
		, m_p1_kb(*this, "P1.ROW%u", 0U)
		, m_p2_kb(*this, "P2.ROW%u", 0U)
	{ }

	void naomi_base(machine_config &config);
	void naomim2(machine_config &config);
	void naomim2_kb(machine_config &config);
	void naomim2_gun(machine_config &config);
	void naomi(machine_config &config);
	void naomim1(machine_config &config);
	void naomim1_hop(machine_config &config);
	void naomigd(machine_config &config);
	void naomigd_kb(machine_config &config);
	void naomim4(machine_config &config);

	void init_naomi();
	void init_naomi_mp();
	void init_hotd2();
	void init_naomigd();
	void init_naomigd_mp();

	ioport_value naomi_mp_r();
	ioport_value suchie3_mp_r();
	template <int P> ioport_value naomi_kb_r();
	DECLARE_INPUT_CHANGED_MEMBER(naomi_mp_w);

	uint64_t naomi2_biose_idle_skip_r();

protected:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_region_ptr<uint64_t> m_rombase;
	optional_ioport_array<5> m_mp;
	optional_ioport_array<5> m_p1_kb;
	optional_ioport_array<5> m_p2_kb;

	DECLARE_MACHINE_RESET(naomi);
	void external_reset(int state);

	uint16_t naomi_g2bus_r(offs_t offset);
	uint64_t eeprom_93c46a_r();
	void eeprom_93c46a_w(uint64_t data);

	uint8_t m_mp_mux = 0;

	uint8_t asciihex_to_dec(uint8_t in);
	void create_pic_from_retdat();

	void naomi_map(address_map &map) ATTR_COLD;
	void naomi_port(address_map &map) ATTR_COLD;

	void set_drc_options();
};

class naomi2_state : public naomi_state
{
public:
	naomi2_state(const machine_config &mconfig, device_type type, const char *tag)
		: naomi_state(mconfig, type, tag),
		m_pvr2_texture_ram(*this, "textureram2"),
		m_pvr2_framebuffer_ram(*this, "frameram2"),
		m_elan_ram(*this, "elan_ram"),
		m_powervr2_slave(*this, "powervr2_slave") { }

	void naomi2_base(machine_config &config);
	void naomi2m2(machine_config &config);
	void naomi2gd(machine_config &config);
	void naomi2m1(machine_config &config);
	void naomi2m4(machine_config &config);

	void init_naomi2();

private:
	required_shared_ptr<uint64_t> m_pvr2_texture_ram;
	required_shared_ptr<uint64_t> m_pvr2_framebuffer_ram;
	required_shared_ptr<uint64_t> m_elan_ram;
	required_device<powervr2_device> m_powervr2_slave;

	void naomi2_map(address_map &map) ATTR_COLD;

	void both_pvr2_ta_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t elan_regs_r(offs_t offset);
	void elan_regs_w(offs_t offset, uint32_t data);
};

#endif // MAME_SEGA_NAOMI_H
