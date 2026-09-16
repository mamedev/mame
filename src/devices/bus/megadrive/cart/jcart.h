// license: BSD-3-Clause
// copyright-holders: Angelo Salese, Fabio Priuli

#ifndef MAME_BUS_MEGADRIVE_CART_JCART_H
#define MAME_BUS_MEGADRIVE_CART_JCART_H

#pragma once

#include "bus/sms_ctrl/smsctrl.h"
#include "machine/i2cmem.h"

#include "rom.h"
#include "slot.h"

class megadrive_rom_jcart_sampras_device : public megadrive_rom_device
{
public:
	megadrive_rom_jcart_sampras_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	megadrive_rom_jcart_sampras_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u16 jcart_r(offs_t offset, u16 mem_mask = ~0);
	void jcart_w(offs_t offset, u16 data, u16 mem_mask = ~0);

private:
	template <unsigned N> void th_in(int state);

	required_device_array<sms_control_port_device, 2> m_ctrl_ports;
	uint8_t m_th_in[2];
	uint8_t m_th_out;
};

class megadrive_rom_jcart_sskid_device : public megadrive_rom_jcart_sampras_device
{
public:
	megadrive_rom_jcart_sskid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
};

class megadrive_rom_jcart_micromac2_device : public megadrive_rom_jcart_sampras_device
{
public:
	megadrive_rom_jcart_micromac2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;

protected:
	megadrive_rom_jcart_micromac2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<i2cmem_device> m_i2cmem;
};

class megadrive_rom_jcart_micromac96_device : public megadrive_rom_jcart_micromac2_device
{
public:
	megadrive_rom_jcart_micromac96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_SAMPRAS,    megadrive_rom_jcart_sampras_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_SSKID,      megadrive_rom_jcart_sskid_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_MICROMAC2,  megadrive_rom_jcart_micromac2_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_ROM_JCART_MICROMAC96, megadrive_rom_jcart_micromac96_device)


#endif // MAME_BUS_MEGADRIVE_CART_JCART_H
