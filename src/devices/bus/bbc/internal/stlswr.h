// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Solidisk Sideways RAM

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Solidisk_128KSWE.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_STLSWR_H
#define MAME_BUS_BBC_INTERNAL_STLSWR_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_stlswr_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_stlswr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_stlswr_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool overrides_rom() override { return true; }
	virtual void romsel_w(offs_t offset, uint8_t data) override { m_romsel = data & 0x0f; }
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;
	virtual void latch_fe60_w(uint8_t data) override { m_ramsel = (data & 0x07) | 0x08; }

	optional_device_array<bbc_romslot_device, 16> m_rom;

	uint8_t m_romsel;
	uint8_t m_ramsel;
};


class bbc_stlswr16_device : public bbc_stlswr_device
{
public:
	// construction/destruction
	bbc_stlswr16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class bbc_stlswr32_device : public bbc_stlswr_device
{
public:
	// construction/destruction
	bbc_stlswr32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class bbc_stlswr64_device : public bbc_stlswr_device
{
public:
	// construction/destruction
	bbc_stlswr64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class bbc_stlswr128_device : public bbc_stlswr_device
{
public:
	// construction/destruction
	bbc_stlswr128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_STLSWR16, bbc_stlswr16_device);
DECLARE_DEVICE_TYPE(BBC_STLSWR32, bbc_stlswr32_device);
DECLARE_DEVICE_TYPE(BBC_STLSWR64, bbc_stlswr64_device);
DECLARE_DEVICE_TYPE(BBC_STLSWR128, bbc_stlswr128_device);



#endif /* MAME_BUS_BBC_INTERNAL_STLSWR_H */
