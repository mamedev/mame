// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    RetroClinic DataCentre - USB, IDE and RAMdisc for the BBC Model B, B+, and Master 128

    http://www.retroclinic.com/acorn/datacentre/datacentre.htm

**********************************************************************/


#ifndef MAME_BUS_BBC_1MHZBUS_DATACENTRE_H
#define MAME_BUS_BBC_1MHZBUS_DATACENTRE_H

#include "1mhzbus.h"
#include "machine/i2cmem.h"
#include "bus/ata/ataintf.h"
#include "imagedev/snapquik.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_datacentre_device :
	public device_t,
	public device_bbc_1mhzbus_interface
{
public:
	// construction/destruction
	bbc_datacentre_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(import_nvrest);
	template<int Drive> DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t fred_r(offs_t offset) override;
	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	void irq_w(int state);

	required_ioport m_links;
	required_device<ata_interface_device> m_ide;
	required_device<i2cmem_device> m_nvram;
	required_memory_region m_nvrest;

	std::unique_ptr<uint8_t[]> m_ram;
	uint16_t m_page_ram;

	uint16_t m_ide_data;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_DATACENTRE, bbc_datacentre_device);


#endif /* MAME_BUS_BBC_1MHZBUS_DATACENTRE_H */
