// license:GPL-2.0+
// copyright-holders:smf
#ifndef MAME_BUS_PCCARD_LINFLASH_H
#define MAME_BUS_PCCARD_LINFLASH_H

#pragma once

#include "machine/intelfsh.h"
#include "pccard.h"

class linear_flash_pccard_device :
	public device_t,
	public device_pccard_interface,
	public device_memory_interface
{
public:
	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

	void update_wp(int state);

protected:
	linear_flash_pccard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor amap);

	// device_t
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_space_config;
	address_space *m_space;
	int8_t m_wp;
};

template<unsigned N>
class linear_flash_pccard_8bit_device :
	public linear_flash_pccard_device
{
protected:
	linear_flash_pccard_8bit_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void amap(address_map &map) ATTR_COLD;

	required_device_array<intelfsh8_device, N> m_l;
	required_device_array<intelfsh8_device, N> m_u;
};

template<unsigned N>
class linear_flash_pccard_16bit_device :
	public linear_flash_pccard_device
{
protected:
	linear_flash_pccard_16bit_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void amap(address_map &map) ATTR_COLD;

	required_device_array<intelfsh16_device, N> m_flash;
};

template<unsigned N>
class linear_flash_pccard_29f017a_device :
	public linear_flash_pccard_8bit_device<N>
{
protected:
	linear_flash_pccard_29f017a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

template<unsigned N>
class linear_flash_pccard_lh28f016s_device :
	public linear_flash_pccard_8bit_device<N>
{
protected:
	linear_flash_pccard_lh28f016s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

template<unsigned N>
class linear_flash_pccard_28f640j5_device :
	public linear_flash_pccard_16bit_device<N>
{
protected:
	linear_flash_pccard_28f640j5_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class fujitsu_16mb_flash_card_device : public linear_flash_pccard_29f017a_device<4>
{
public:
	fujitsu_16mb_flash_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class fujitsu_32mb_flash_card_device : public linear_flash_pccard_29f017a_device<8>
{
public:
	fujitsu_32mb_flash_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class id245p01_device : public linear_flash_pccard_lh28f016s_device<8>
{
public:
	id245p01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class pm24276_device : public linear_flash_pccard_28f640j5_device<4>
{
public:
	pm24276_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(FUJITSU_16MB_FLASH_CARD, fujitsu_16mb_flash_card_device)
DECLARE_DEVICE_TYPE(FUJITSU_32MB_FLASH_CARD, fujitsu_32mb_flash_card_device)
DECLARE_DEVICE_TYPE(ID245P01, id245p01_device)
DECLARE_DEVICE_TYPE(PM24276, pm24276_device)

#endif // MAME_BUS_PCCARD_LINFLASH_H
