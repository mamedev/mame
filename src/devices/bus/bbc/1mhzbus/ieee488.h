// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn IEEE-488 Interface

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANK01_IEEE488IF.html

    Aries-B488

    http://www.beebmaster.co.uk/CheeseWedges/AriesB488.html

    CST Procyon IEEE Interface

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_IEEE488_H
#define MAME_BUS_BBC_1MHZBUS_IEEE488_H

#include "1mhzbus.h"
#include "bus/ieee488/ieee488.h"
#include "machine/tms9914.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_ieee488_device:
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	required_device<ieee488_device> m_ieee;
	required_device<tms9914_device> m_tms9914;
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
};


class bbc_b488_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_b488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;

private:
	required_device<ieee488_device> m_ieee;
	required_device<tms9914_device> m_tms9914;
};


//class bbc_procyon_device :
//  public device_t,
//  public device_bbc_1mhzbus_interface
//{
//public:
//  // construction/destruction
//  bbc_procyon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
//
//protected:
//  // device-level overrides
//  virtual void device_start() override ATTR_COLD;
//
//  // optional information overrides
//  virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
//  virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
//
//  virtual uint8_t fred_r(offs_t offset) override;
//  virtual void fred_w(offs_t offset, uint8_t data) override;
//
//private:
//  required_device<ieee488_device> m_ieee;
//};


// device type definition
DECLARE_DEVICE_TYPE(BBC_IEEE488, bbc_ieee488_device);
DECLARE_DEVICE_TYPE(BBC_B488,    bbc_b488_device);
//DECLARE_DEVICE_TYPE(BBC_PROCYON, bbc_procyon_device);


#endif /* MAME_BUS_BBC_1MHZBUS_IEEE488_H */
