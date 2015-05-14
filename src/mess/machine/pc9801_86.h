// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-86

***************************************************************************/


#pragma once

#ifndef __PC9801_86DEV_H__
#define __PC9801_86DEV_H__

#include "machine/pic8259.h"
#include "sound/2608intf.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_86_device

class pc9801_86_device : public device_t
{
public:
	// construction/destruction
	pc9801_86_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER(opn_porta_r);
	DECLARE_WRITE8_MEMBER(opn_portb_w);
	DECLARE_READ8_MEMBER(pc9801_86_r);
	DECLARE_WRITE8_MEMBER(pc9801_86_w);
	DECLARE_WRITE_LINE_MEMBER(pc9801_sound_irq);
//  DECLARE_READ8_MEMBER(pc9801_86_ext_r);
//  DECLARE_WRITE8_MEMBER(pc9801_86_ext_w);

//  required_device<cpu_device>  m_maincpu;
	required_device<ym2608_device>  m_opna;
	virtual const rom_entry *device_rom_region() const;
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);

private:
	UINT8 m_joy_sel;

};


// device type definition
extern const device_type PC9801_86;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
