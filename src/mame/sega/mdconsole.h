// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_SEGACONS_MDCONSOLE_H
#define MAME_SEGACONS_MDCONSOLE_H

#pragma once

#include "megadriv.h"

#include "mega32x.h"
#include "megacd.h"

#include "bus/generic/slot.h"
#include "bus/megadrive/md_slot.h"
#include "bus/megadrive/md_carts.h"
#include "bus/sms_ctrl/smsctrl.h"


class md_cons_state : public md_base_state
{
public:
	md_cons_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_base_state(mconfig, type, tag),
		m_ctrl_ports(*this, { "ctrl1", "ctrl2", "exp" }),
		m_32x(*this,"sega32x"),
		m_segacd(*this,"segacd"),
		m_cart(*this, "mdslot"),
		m_tmss(*this, "tmss")
	{ }

	void init_genesis();
	void init_md_eur();
	void init_md_jpn();

	void md_32x(machine_config &config);
	void genesis_32x(machine_config &config);
	void mdj_32x(machine_config &config);
	void dcat16_megadriv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void screen_vblank_console(int state);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( _32x_cart );

	void install_cartslot();
	void install_tmss();

	void md_ctrl_ports(machine_config &config);
	void md_exp_port(machine_config &config);

	optional_device_array<sms_control_port_device, 3> m_ctrl_ports;
	optional_device<sega_32x_device> m_32x;
	optional_device<sega_segacd_device> m_segacd;
	optional_device<md_cart_slot_device> m_cart;
	optional_region_ptr<uint16_t> m_tmss;

private:
	void _32x_scanline_callback(int x, uint32_t priority, uint32_t &lineptr);
	void _32x_interrupt_callback(int scanline, int irq6);
	void _32x_scanline_helper_callback(int scanline);

	uint16_t tmss_r(offs_t offset);
	void tmss_swap_w(uint16_t data);

	void dcat16_megadriv_base(machine_config &config);

	void dcat16_megadriv_map(address_map &map) ATTR_COLD;
};


class md_cons_slot_state : public md_cons_state
{
public:
	md_cons_slot_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_cons_state(mconfig, type, tag)
	{ }

	void ms_megadriv(machine_config &config);
	void ms_megadrivj(machine_config &config);
	void ms_megadpal(machine_config &config);
	void ms_megadriv2(machine_config &config);
	void ms_nomad(machine_config &config);
	void ms_megajet(machine_config &config);

	void genesis_tmss(machine_config &config);


protected:
	virtual void machine_start() override ATTR_COLD;
};


class md_cons_cd_state : public md_cons_state
{
public:
	md_cons_cd_state(const machine_config &mconfig, device_type type, const char *tag) :
		md_cons_state(mconfig, type, tag)
	{ }

	void genesis_scd(machine_config &config);
	void genesis2_scd(machine_config &config);
	void md_scd(machine_config &config);
	void md2_scd(machine_config &config);
	void mdj_scd(machine_config &config);

	void mdj_32x_scd(machine_config &config);
	void md2j_scd(machine_config &config);
	void genesis_32x_scd(machine_config &config);
	void md_32x_scd(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
};

#endif // MAME_SEGACONS_MDCONSOLE_H
