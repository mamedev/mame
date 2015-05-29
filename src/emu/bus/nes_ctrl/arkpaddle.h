// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System -
    Arkanoid Paddle input device

**********************************************************************/

#pragma once

#ifndef __NES_ARKPADDLE__
#define __NES_ARKPADDLE__


#include "emu.h"
#include "ctrl.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> nes_vaus_device

class nes_vaus_device : public device_t,
							public device_nes_control_port_interface
{
public:
	// construction/destruction
	nes_vaus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	nes_vaus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_bit34();
	virtual void write(UINT8 data);

	required_ioport m_paddle;
	required_ioport m_button;
	UINT8 m_start_conv;
	UINT32 m_latch;
};


// ======================> nes_vaus_device

class nes_vausfc_device : public nes_vaus_device
{
public:
	// construction/destruction
	nes_vausfc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual UINT8 read_bit34() { return 0; }
	virtual UINT8 read_exp(offs_t offset);
};


// device type definition
extern const device_type NES_ARKPADDLE;
extern const device_type NES_ARKPADDLE_FC;


#endif
