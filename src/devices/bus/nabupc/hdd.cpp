// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * NABU PC HDD Card
 *
 *******************************************************************/

#include "emu.h"
#include "hdd.h"

#include "machine/wd1000.h"

//**************************************************************************
//  NABU PC HDD DEVICE
//**************************************************************************


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* NABU PC HDD Device */

class hdd_device : public device_t, public bus::nabupc::device_option_expansion_interface
{
public:
	// construction/destruction
	hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_option_expansion_interface implementation
	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<wd1000_device> m_hdd;
};

//-------------------------------------------------
//  hdd_device - constructor
//-------------------------------------------------
hdd_device::hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NABUPC_OPTION_HDD, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	m_hdd(*this, "hdd")
{
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------
void hdd_device::device_add_mconfig(machine_config &config)
{
	WD1000(config, m_hdd, 0);

	HARDDISK(config, "hdd:0", 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void hdd_device::device_start()
{
}

uint8_t hdd_device::read(offs_t offset)
{
	uint8_t result = 0xff;

	switch(offset) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		result = m_hdd->read(offset);
		break;
	case 0x0f:
		result = 0xe8;
		break;
	}
	return result;
}

void hdd_device::write(offs_t offset, uint8_t data)
{
	switch(offset) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
		m_hdd->write(offset, data);
		break;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(NABUPC_OPTION_HDD, bus::nabupc::device_option_expansion_interface, hdd_device, "nabupc_option_hdd", "NABU PC Hard Disk Controller")
