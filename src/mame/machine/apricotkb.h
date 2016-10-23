// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Apricot keyboard emulation

*********************************************************************/

#pragma once

#ifndef __APRICOT_KEYBOARD__
#define __APRICOT_KEYBOARD__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define APRICOT_KEYBOARD_TAG    "aprikb"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_APRICOT_KEYBOARD_TXD_CALLBACK(_write) \
	devcb = &apricot_keyboard_device::set_tcd_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_keyboard_device

class apricot_keyboard_device :  public device_t
{
public:
	// construction/destruction
	apricot_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_txd_wr_callback(device_t &device, _Object object) { return downcast<apricot_keyboard_device &>(device).m_write_txd.set_callback(object); }

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	uint8_t read_keyboard();

	uint8_t kb_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t kb_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t kb_p6_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kb_p3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kb_y0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kb_y4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kb_y8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kb_yc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	devcb_write_line   m_write_txd;

	required_ioport_array<13> m_y;
	required_ioport m_modifiers;

	uint16_t m_kb_y;
};


// device type definition
extern const device_type APRICOT_KEYBOARD;



#endif
