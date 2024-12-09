// license:BSD-3-Clause
// copyright-holders:smf

#ifndef MAME_BUS_JVS_CYBERLEAD_H
#define MAME_BUS_JVS_CYBERLEAD_H

#pragma once

#include "jvs.h"
#include "bus/rs232/rs232.h"
#include "cpu/h8/c77.h"
#include "machine/intelfsh.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class namco_cyberlead_device :
	public device_t,
	public device_jvs_interface
{
public:
	namco_cyberlead_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto &rs232() const { if (m_rs232) return *m_rs232; return *subdevice<rs232_port_device>(m_rs232.finder_tag()); }

protected:
	namco_cyberlead_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_jvs_interface
	virtual void rxd(int state) override;

private:
	void iocpu_program_map(address_map &map);

	uint8_t iocpu_in_r(offs_t offset);
	uint8_t iocpu_port_4_r();
	void iocpu_port_4_w(uint8_t data);
	uint8_t iocpu_port_5_r();
	void iocpu_port_5_w(uint8_t data);
	uint8_t iocpu_port_6_r();
	void iocpu_port_6_w(uint8_t data);
	uint8_t iocpu_port_7_r();

	required_device<namco_c77_device> m_iocpu;
	required_ioport m_dsw;
	required_ioport m_port7;
	required_device<rs232_port_device> m_rs232;
};

class namco_cyberlead_led_device :
	public device_t,
	public device_rs232_port_interface
{
public:
	namco_cyberlead_led_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_rs232_port_interface
	virtual void input_txd(int state) override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void ledcpu_program_map(address_map &map);
	uint8_t ledcpu_banked_flash_r(offs_t offset);
	void ledcpu_banked_flash_w(offs_t offset, uint8_t data);
	void ledcpu_flash_bank_w(offs_t offset, uint8_t data);
	uint8_t ledcpu_port_6_r();

	required_device<namco_c77_device> m_ledcpu;
	required_device<intelfsh8_device> m_flash;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_sed1351f_ram;
	required_shared_ptr<uint16_t> m_sed1351f_control;
	required_ioport m_dsw;
	rgb_t m_palette[4][4];
	int m_flash_bank;
};

DECLARE_DEVICE_TYPE(NAMCO_CYBERLEAD, namco_cyberlead_device)
DECLARE_DEVICE_TYPE(NAMCO_CYBERLEADA, device_jvs_interface)
DECLARE_DEVICE_TYPE(NAMCO_CYBERLEAD_LED, namco_cyberlead_led_device)

#endif
