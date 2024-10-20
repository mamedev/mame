// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    RetroClinic BBC 8-bit IDE Interface

    http://www.retroclinic.com/acorn/bbcide/bbcide.htm

    Sprow BeebIDE 16-bit IDE Interface for the BBC series

    http://www.sprow.co.uk/bbc/beebide.htm

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_IDE_H
#define MAME_BUS_BBC_1MHZBUS_IDE_H

#include "1mhzbus.h"
#include "bus/ata/ataintf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_ide8_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_ide8_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config& config) override;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;

private:
	required_device<ata_interface_device> m_ide;
};


class bbc_beebide_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_beebide_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config& config) override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	void irq_w(int state);

	required_device<ata_interface_device> m_ide;
	required_device<bbc_1mhzbus_slot_device> m_1mhzbus;
	required_ioport m_links;

	uint16_t m_ide_data;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_IDE8, bbc_ide8_device);
DECLARE_DEVICE_TYPE(BBC_BEEBIDE, bbc_beebide_device);


#endif /* MAME_BUS_BBC_1MHZBUS_IDE_H */
