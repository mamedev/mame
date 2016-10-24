// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    Epson LX-800 dot matrix printer emulation

**********************************************************************/

#pragma once

#ifndef __EPSON_LX800__
#define __EPSON_LX800__

#include "emu.h"
#include "ctronics.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/e05a03.h"
#include "sound/beep.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> epson_lx800_t

class epson_lx800_t :  public device_t,
						public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	epson_lx800_t(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	epson_lx800_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t centronics_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void centronics_pe_w(int state);
	void paperempty_led_w(int state);
	void reset_w(int state);
	int an0_r();
	int an1_r();
	int an2_r();
	int an3_r();
	int an4_r();
	int an5_r();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
};



// device type definition
extern const device_type EPSON_LX800;



#endif
