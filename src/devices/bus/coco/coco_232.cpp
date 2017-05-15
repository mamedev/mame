// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco_232.cpp

    Code for emulating the CoCo RS-232 PAK

***************************************************************************/

#include "emu.h"
#include "coco_232.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UART_TAG        "uart"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

static MACHINE_CONFIG_FRAGMENT(coco_rs232)
	MCFG_DEVICE_ADD(UART_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL_1_8432MHz)
MACHINE_CONFIG_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(COCO_232, coco_232_device, "coco_232", "CoCo RS-232 PAK")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_232_device - constructor
//-------------------------------------------------

coco_232_device::coco_232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COCO_232, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_uart(*this, UART_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_232_device::device_start()
{
	install_readwrite_handler(0xFF68, 0xFF6F,
		read8_delegate(FUNC(mos6551_device::read), (mos6551_device *)m_uart),
		write8_delegate(FUNC(mos6551_device::write), (mos6551_device *)m_uart));
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_232_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( coco_rs232 );
}
