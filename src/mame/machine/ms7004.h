// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#pragma once

#ifndef __MS7004_H__
#define __MS7004_H__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "sound/beep.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MS7004_TX_HANDLER(_cb) \
	devcb = &ms7004_device::set_tx_handler(*device, DEVCB_##_cb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ms7004_device

class ms7004_device : public device_t //, public device_serial_interface
{
public:
	// construction/destruction
	ms7004_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_tx_handler(device_t &device, _Object wr) { return downcast<ms7004_device &>(device).m_tx_handler.set_callback(wr); }

	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( t1_r );
	DECLARE_WRITE8_MEMBER( i8243_port_w );

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_speaker;
	required_device<i8243_device> m_i8243;

	required_ioport m_kbd0;
	required_ioport m_kbd1;
	required_ioport m_kbd2;
	required_ioport m_kbd3;
	required_ioport m_kbd4;
	required_ioport m_kbd5;
	required_ioport m_kbd6;
	required_ioport m_kbd7;
	required_ioport m_kbd8;
	required_ioport m_kbd9;
	required_ioport m_kbd10;
	required_ioport m_kbd11;
	required_ioport m_kbd12;
	required_ioport m_kbd13;
	required_ioport m_kbd14;
	required_ioport m_kbd15;

	int m_keylatch;                 // keyboard row latch
	UINT8 m_p1;
	UINT8 m_p2;

	devcb_write_line m_tx_handler;
};

// device type definition
extern const device_type MS7004;

#endif
