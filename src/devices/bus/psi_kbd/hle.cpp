// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    PSI HLE Keyboard

***************************************************************************/

#include "emu.h"
#include "hle.h"
#include "machine/keyboard.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSI_HLE_KEYBOARD, psi_hle_keyboard_device, "psi_hle_kbd", "PSI HLE Keyboard")


MACHINE_CONFIG_MEMBER( psi_hle_keyboard_device::device_add_mconfig )
	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(psi_hle_keyboard_device, kbd_put))
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psi_hle_keyboard_device - constructor
//-------------------------------------------------

psi_hle_keyboard_device::psi_hle_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSI_HLE_KEYBOARD, tag, owner, clock),
	device_psi_keyboard_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psi_hle_keyboard_device::device_start()
{
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

void psi_hle_keyboard_device::kbd_put(uint8_t data)
{
	m_host->key_data_w(machine().dummy_space(), 0, data);
	m_host->key_strobe_w(1);
	m_host->key_strobe_w(0);
}
