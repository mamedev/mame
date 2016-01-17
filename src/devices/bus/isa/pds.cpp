// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * isa_pds.c - Programmers Development System 8-bit ISA card
 *
 * Used to connect up to two 8-bit systems to the PC, allowing the download of assembled code directly to the
 * target 8-bit system (Spectrum, CPC, MSX, C64 and maybe the BBC?)
 *
 * The editor software require the ISA card to be present.
 *
 * The PC end hardware consists of an 8-bit ISA card containing an 8255 PPI hooked up to the two connectors on the
 * back of the card.
 *
 * The 8-bit end hardware consists of an expansion device containing a Z80PIO.
 *
 *  Created on: 31/01/2014
 */

#include "pds.h"

const device_type ISA8_PDS = &device_creator<isa8_pds_device>;

isa8_pds_device::isa8_pds_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, ISA8_PDS, "Programmers Development System", tag, owner, clock, "isa_pds", __FILE__),
		device_isa8_card_interface( mconfig, *this ),
		m_ppi(*this,"pds_ppi")
{
}


READ8_MEMBER(isa8_pds_device::ppi_r)
{
	if(!(offset & 0x01))
		return m_ppi->read(space,offset/2);
	return 0xff;
}

WRITE8_MEMBER(isa8_pds_device::ppi_w)
{
	if(!(offset & 0x01))
		m_ppi->write(space,offset/2,data);
}

void isa8_pds_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0300, 0x0307, 0, 0, read8_delegate(FUNC(isa8_pds_device::ppi_r),this), write8_delegate(FUNC(isa8_pds_device::ppi_w),this) );
}

void isa8_pds_device::device_reset()
{
}

void isa8_pds_device::device_stop()
{
}

static MACHINE_CONFIG_FRAGMENT( pds_config )
	MCFG_DEVICE_ADD("pds_ppi", I8255, 0)
MACHINE_CONFIG_END

machine_config_constructor isa8_pds_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pds_config );
}
