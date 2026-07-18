// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_GENERALPLUS_GPL162XX_B_SOC_H
#define MAME_MACHINE_GENERALPLUS_GPL162XX_B_SOC_H

#pragma once

#include "generalplus_gpl162xx_soc.h"


class generalplus_gpl162xx_b_base : public generalplus_gpl162xx_base_device
{
protected:
	generalplus_gpl162xx_b_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void gpl162xx_b_extended_sprite_map(address_map &map) ATTR_COLD;
	void gpl162xx_b_byteswap_map(address_map &map) ATTR_COLD;

private:
	void esp_ctrl_w(u16 data);
	u16 esp_ctrl_r();
	void byteswap_w(u16 data);
	u16 byteswap_r();

	u16 m_esp_ctrl;
	u16 m_byteswap;
};


class generalplus_gpl16218b_device : public generalplus_gpl162xx_b_base
{
public:
	template <typename T> generalplus_gpl16218b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
		generalplus_gpl16218b_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	generalplus_gpl16218b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	generalplus_gpl16218b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	//virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD; // uncertain

private:
	void gpl16218b_map(address_map &map) ATTR_COLD;
};


class generalplus_gpl16238b_device : public generalplus_gpl16218b_device
{
public:
	template <typename T> generalplus_gpl16238b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
	generalplus_gpl16238b_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	generalplus_gpl16238b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	generalplus_gpl16238b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void gpl16238b_map(address_map &map) ATTR_COLD;
};


class generalplus_gpl16248vb_device : public generalplus_gpl16238b_device
{
public:
	template <typename T> generalplus_gpl16248vb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
	generalplus_gpl16248vb_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	generalplus_gpl16248vb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	generalplus_gpl16248vb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void gpl16248vb_map(address_map &map) ATTR_COLD;
};

class generalplus_gpl16258vb_device : public generalplus_gpl16248vb_device
{
public:
	template <typename T> generalplus_gpl16258vb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
	generalplus_gpl16258vb_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	generalplus_gpl16258vb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void gpl16258vb_map(address_map &map) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(GPL16218B, generalplus_gpl16218b_device)
DECLARE_DEVICE_TYPE(GPL16238B, generalplus_gpl16238b_device)
DECLARE_DEVICE_TYPE(GPL16248VB, generalplus_gpl16248vb_device)
DECLARE_DEVICE_TYPE(GPL16258VB, generalplus_gpl16258vb_device)


#endif // MAME_MACHINE_GENERALPLUS_GPL162XX_B_SOC_H
