// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aries B12 Sideways ROM Expansion
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Aries_B12.html

    Aries B20 20K RAM expansion
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Aries_B20.html

    Aries-B32 32K RAM expansion
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Aries_B32.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_ARIES_H
#define MAME_BUS_BBC_INTERNAL_ARIES_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_ariesb12_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_ariesb12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_ariesb12_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual bool overrides_rom() override { return true; }
	virtual void romsel_w(offs_t offset, uint8_t data) override { m_romsel = data & 0x0f; }
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

	optional_device_array<bbc_romslot_device, 16> m_rom;

	uint8_t m_romsel;
};


class bbc_ariesb20_device : public bbc_ariesb12_device
{
public:
	// construction/destruction
	bbc_ariesb20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool overrides_ram() override { return true; }
	virtual uint8_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint8_t data) override;

private:
	void control_w(offs_t offset, uint8_t data);

	bool m_shadow;
	std::unique_ptr<uint8_t[]> m_ram;
};


class bbc_ariesb32_device : public bbc_ariesb12_device
{
public:
	// construction/destruction
	bbc_ariesb32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual bool overrides_ram() override { return true; }
	virtual uint8_t ram_r(offs_t offset) override;
	virtual void ram_w(offs_t offset, uint8_t data) override;
	virtual void romsel_w(offs_t offset, uint8_t data) override;
	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

private:
	required_memory_region m_aries_rom;

	bool m_shadow;
	std::unique_ptr<uint8_t[]> m_ram;
	uint8_t m_ramsel;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_ARIESB12, bbc_ariesb12_device);
DECLARE_DEVICE_TYPE(BBC_ARIESB20, bbc_ariesb20_device);
DECLARE_DEVICE_TYPE(BBC_ARIESB32, bbc_ariesb32_device);


#endif /* MAME_BUS_BBC_INTERNAL_ARIES_H */
