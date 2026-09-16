// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_EEPROM_H
#define MAME_BUS_MEGADRIVE_CART_EEPROM_H

#pragma once

#include "machine/i2cmem.h"

#include "rom.h"
#include "slot.h"

class megadrive_eeprom_device : public megadrive_rom_device
{
public:
	megadrive_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	megadrive_eeprom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<i2cmem_device> m_i2cmem;
};

class megadrive_eeprom_nbajam_device : public megadrive_eeprom_device
{
public:
	megadrive_eeprom_nbajam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class megadrive_eeprom_nbajamte_device : public megadrive_eeprom_device
{
public:
	megadrive_eeprom_nbajamte_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	megadrive_eeprom_nbajamte_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class megadrive_eeprom_nflqb96_device : public megadrive_eeprom_nbajamte_device
{
public:
	megadrive_eeprom_nflqb96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class megadrive_eeprom_collslam_device : public megadrive_eeprom_nbajamte_device
{
public:
	megadrive_eeprom_collslam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class megadrive_eeprom_nhlpa_device : public megadrive_eeprom_device
{
public:
	megadrive_eeprom_nhlpa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};



class megadrive_eeprom_blara95_device : public megadrive_eeprom_device
{
public:
	megadrive_eeprom_blara95_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void cart_map(address_map &map) override ATTR_COLD;
protected:
	megadrive_eeprom_blara95_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class megadrive_eeprom_blara96_device : public megadrive_eeprom_blara95_device
{
public:
	megadrive_eeprom_blara96_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};




DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM,           megadrive_eeprom_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM_NBAJAM,    megadrive_eeprom_nbajam_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM_NBAJAMTE,  megadrive_eeprom_nbajamte_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM_NFLQB96,   megadrive_eeprom_nflqb96_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM_COLLSLAM,  megadrive_eeprom_collslam_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM_NHLPA,     megadrive_eeprom_nhlpa_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM_BLARA95,   megadrive_eeprom_blara95_device)
DECLARE_DEVICE_TYPE(MEGADRIVE_EEPROM_BLARA96,   megadrive_eeprom_blara96_device)


#endif // MAME_BUS_MEGADRIVE_CART_EEPROM_H
