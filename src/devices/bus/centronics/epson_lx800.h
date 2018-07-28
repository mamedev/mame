// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    Epson LX-800 dot matrix printer emulation

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_EPSON_LX800_H
#define MAME_BUS_CENTRONICS_EPSON_LX800_H

#pragma once

#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/e05a03.h"
#include "sound/beep.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_lx800_device

class epson_lx800_device :  public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_lx800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	epson_lx800_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_READ8_MEMBER(portc_r);
	DECLARE_WRITE8_MEMBER(portc_w);

	DECLARE_READ_LINE_MEMBER(an0_r);
	DECLARE_READ_LINE_MEMBER(an1_r);
	DECLARE_READ_LINE_MEMBER(an2_r);
	DECLARE_READ_LINE_MEMBER(an3_r);
	DECLARE_READ_LINE_MEMBER(an4_r);
	DECLARE_READ_LINE_MEMBER(an5_r);

	DECLARE_READ8_MEMBER(centronics_data_r);
	DECLARE_WRITE_LINE_MEMBER(centronics_pe_w);
	DECLARE_WRITE_LINE_MEMBER(paperempty_led_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);

	void lx800_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
};



// device type definition
DECLARE_DEVICE_TYPE(EPSON_LX800, epson_lx800_device)

#endif // MAME_BUS_CENTRONICS_EPSON_LX800_H
