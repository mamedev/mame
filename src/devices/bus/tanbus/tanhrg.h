// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine High Resolution Graphics Card (MT0055 Iss2)

**********************************************************************/


#ifndef MAME_BUS_TANBUS_TANHRG_H
#define MAME_BUS_TANBUS_TANHRG_H

#pragma once

#include "bus/tanbus/tanbus.h"
#include "emupal.h"
#include "screen.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_tanhrg_device :
	public device_t,
	public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_tanhrg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;
	virtual void set_inhibit_lines(offs_t offset, int &inhram, int &inhrom) override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_ioport m_dsw;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	std::unique_ptr<uint8_t[]> m_videoram;
};


class tanbus_tanhrgc_device :
	public device_t,
	public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_tanhrgc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;
	virtual void set_inhibit_lines(offs_t offset, int &inhram, int &inhrom) override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_ioport_array<3> m_dsw;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	std::unique_ptr<uint8_t[]> m_videoram;
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_TANHRG, tanbus_tanhrg_device)
DECLARE_DEVICE_TYPE(TANBUS_TANHRGC, tanbus_tanhrgc_device)


#endif // MAME_BUS_TANBUS_TANHRG_H
