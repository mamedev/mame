// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2diskii.h

    Apple II Disk II Controller Card

*********************************************************************/

#ifndef __A2BUS_DISKII__
#define __A2BUS_DISKII__

#include "a2bus.h"
#include "machine/applefdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_floppy_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_floppy_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(address_space &space, uint8_t offset) override;

	required_device<applefdc_base_device> m_fdc;

private:
	uint8_t *m_rom;
};

class a2bus_diskii_device: public a2bus_floppy_device
{
public:
	a2bus_diskii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class a2bus_iwmflop_device: public a2bus_floppy_device
{
public:
	a2bus_iwmflop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual machine_config_constructor device_mconfig_additions() const override;
};

// device type definition
extern const device_type A2BUS_DISKII;
extern const device_type A2BUS_IWM_FDC;

#endif  /* __A2BUS_DISKII__ */
