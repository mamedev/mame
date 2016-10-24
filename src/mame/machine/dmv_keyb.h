// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Decision Mate V keyboard emulation

*********************************************************************/

#pragma once

#ifndef __DMV_KEYBOARD__
#define __DMV_KEYBOARD__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DMV_KEYBOARD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DMV_KEYBOARD, 0)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_keyboard_device

class dmv_keyboard_device : public device_t
{
public:
	// construction/destruction
	dmv_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	void sd_poll_w(int state);
	int sd_poll_r();

	uint8_t port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override { m_shortname = "dmv_keyb"; }
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<upi41_cpu_device> m_maincpu;
	required_ioport_array<16> m_keyboard;

	uint8_t   m_col;
	int     m_sd_data_state;
	int     m_sd_poll_state;
};


// device type definition
extern const device_type DMV_KEYBOARD;


#endif
