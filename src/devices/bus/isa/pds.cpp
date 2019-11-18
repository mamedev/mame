// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * isa_pds.c - Programmers Development System 8-bit ISA card
 *
 * Used to connect up to two 8-bit systems to the PC, allowing the download of assembled code directly to the
 * target 8-bit system (Spectrum, CPC, MSX, C64 and maybe the BBC?)
 *
 * The editor software requires the ISA card to be present.
 *
 * The PC end hardware consists of an 8-bit ISA card containing an 8255 PPI hooked up to the two connectors on the
 * back of the card.
 *
 * The 8-bit end hardware consists of an expansion device containing a Z80PIO.
 *
 *  Created on: 31/01/2014
 */

#include "emu.h"
#include "pds.h"

DEFINE_DEVICE_TYPE(ISA8_PDS, isa8_pds_device, "isa_pds", "Programmers Development System (host)")

isa8_pds_device::isa8_pds_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA8_PDS, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_ppi(*this,"pds_ppi")
{
}


READ8_MEMBER(isa8_pds_device::ppi_r)
{
	if(!(offset & 0x01))
		return m_ppi->read(offset/2);
	return 0xff;
}

WRITE8_MEMBER(isa8_pds_device::ppi_w)
{
	if(!(offset & 0x01))
		m_ppi->write(offset/2,data);
}

void isa8_pds_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0300, 0x0307, read8_delegate(*this, FUNC(isa8_pds_device::ppi_r)), write8_delegate(*this, FUNC(isa8_pds_device::ppi_w)));
}

void isa8_pds_device::device_reset()
{
}

void isa8_pds_device::device_stop()
{
}

void isa8_pds_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_ppi);
}
