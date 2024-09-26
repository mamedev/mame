// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro SD Card

**********************************************************************/


#ifndef MAME_BUS_BBC_USERPORT_SDCARD_H
#define MAME_BUS_BBC_USERPORT_SDCARD_H

#pragma once

#include "userport.h"
#include "machine/spi_sdcard.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_sdcard_device

class bbc_sdcard_device : public device_t, public device_bbc_userport_interface
{
public:
	// construction/destruction
	bbc_sdcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_sdcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void pb_w(uint8_t data) override;
	virtual void write_cb1(int state) override;

	required_device<spi_sdcard_device> m_sdcard;
};


// ======================> bbc_sdcardt_device

class bbc_sdcardt_device : public bbc_sdcard_device
{
public:
	// construction/destruction
	bbc_sdcardt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void pb_w(uint8_t data) override;
	virtual void write_cb1(int state) override;
	virtual void write_cb2(int state) override;

private:
	bool m_turbo;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_SDCARD, bbc_sdcard_device)
DECLARE_DEVICE_TYPE(BBC_SDCARDT, bbc_sdcardt_device)


#endif // MAME_BUS_BBC_USERPORT_SDCARD_H
