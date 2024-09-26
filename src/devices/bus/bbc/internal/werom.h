// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics 12 ROM Board
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/WE_12ROMboard.html

    Watford Electronics 13 ROM Board
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/WE_13ROMBoard.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_WEROM_H
#define MAME_BUS_BBC_INTERNAL_WEROM_H

#include "internal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_werom_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_werom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_werom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	optional_device_array<bbc_romslot_device, 16> m_rom;

	virtual bool overrides_rom() override { return true; }
	virtual void romsel_w(offs_t offset, uint8_t data) override { m_romsel = data & 0x0f; }

	uint8_t m_romsel;
};


class bbc_we12rom_device : public bbc_werom_device
{
public:
	// construction/destruction
	bbc_we12rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;

private:
	required_ioport m_wp;
};


class bbc_we13rom_device : public bbc_werom_device
{
public:
	// construction/destruction
	bbc_we13rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t paged_r(offs_t offset) override;
	virtual void paged_w(offs_t offset, uint8_t data) override;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_WE12ROM, bbc_we12rom_device);
DECLARE_DEVICE_TYPE(BBC_WE13ROM, bbc_we13rom_device);



#endif /* MAME_BUS_BBC_INTERNAL_WEROM_H */
