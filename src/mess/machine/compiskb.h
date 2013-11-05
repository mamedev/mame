// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Telenova Compis keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __COMPIS_KEYBOARD__
#define __COMPIS_KEYBOARD__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define COMPIS_KEYBOARD_TAG "compiskb"
#define SPEAKER_TAG         "speaker"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_COMPIS_KEYBOARD_ADD(_irq) \
	MCFG_DEVICE_ADD(COMPIS_KEYBOARD_TAG, COMPIS_KEYBOARD, 0) \
	downcast<compis_keyboard_device *>(device)->set_irq_callback(DEVCB2_##_irq);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> compis_keyboard_device

class compis_keyboard_device :  public device_t
{
public:
	// construction/destruction
	compis_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _irq> void set_irq_callback(_irq irq) { m_write_irq.set_callback(irq); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ_LINE_MEMBER( so_r );
	DECLARE_WRITE_LINE_MEMBER( si_w );

	DECLARE_READ8_MEMBER( bus_r );
	DECLARE_WRITE8_MEMBER( bus_w );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_READ8_MEMBER( p2_r );

protected:
	// device-level overrides
	virtual void device_start();

private:
	enum
	{
		LED_CAPS
	};

	devcb2_write_line   m_write_irq;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_special;

	int m_so;

	UINT8 m_bus;
	UINT8 m_keylatch;
};


// device type definition
extern const device_type COMPIS_KEYBOARD;



#endif
