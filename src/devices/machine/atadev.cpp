// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    ATA Device implementation.

***************************************************************************/

#include "emu.h"
#include "atadev.h"

//-------------------------------------------------
//  device_ata_interface - constructor
//-------------------------------------------------

device_ata_interface::device_ata_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_irq_handler(device),
	m_dmarq_handler(device),
	m_dasp_handler(device),
	m_pdiag_handler(device)
{
}


READ16_MEMBER( device_ata_interface::read16_cs0 )
{
	return read16_cs0(offset, mem_mask);
}

READ16_MEMBER( device_ata_interface::read16_cs1 )
{
	return read16_cs1(offset, mem_mask);
}

WRITE16_MEMBER( device_ata_interface::write16_cs0 )
{
	write16_cs0(offset, data, mem_mask);
}

WRITE16_MEMBER( device_ata_interface::write16_cs1 )
{
	write16_cs1(offset, data, mem_mask);
}
