// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_scsiext.c
 *
 * Sharp CZ-6BS1 SCSI-1 controller
 *
 *  Created on: 5/06/2012
 */

#include "emu.h"
#include "x68k_scsiext.h"

#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "machine/mb89352.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(X68K_SCSIEXT, x68k_scsiext_device, "x68k_cz6bs1", "Sharp CZ-6BS1 SCSI-1")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( x68k_cz6bs1 )
	ROM_REGION( 0x10000, "scsiexrom", 0 )
	ROM_LOAD16_WORD_SWAP( "scsiexrom.bin",   0x0000, 0x2000, CRC(7be488de) SHA1(49616c09a8986ffe6a12ad600febe512f7ba8ae4) )
ROM_END

const tiny_rom_entry *x68k_scsiext_device::device_rom_region() const
{
	return ROM_NAME( x68k_cz6bs1 );
}

// device machine config
void x68k_scsiext_device::device_add_mconfig(machine_config &config)
{
	SCSI_PORT(config, m_scsibus);
	m_scsibus->set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	m_scsibus->set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	m_scsibus->set_slot_device(3, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_2));
	m_scsibus->set_slot_device(4, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_3));
	m_scsibus->set_slot_device(5, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));
	m_scsibus->set_slot_device(6, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_5));
	m_scsibus->set_slot_device(7, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));

	MB89352A(config, m_spc);
	m_spc->set_scsi_port(m_scsibus);
	m_spc->irq_cb().set(FUNC(x68k_scsiext_device::irq_w));
	m_spc->drq_cb().set(FUNC(x68k_scsiext_device::drq_w));
}

x68k_scsiext_device::x68k_scsiext_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X68K_SCSIEXT, tag, owner, clock)
	, device_x68k_expansion_card_interface(mconfig, *this)
	, m_slot(nullptr),
	m_scsibus(*this, "scsi"),
	m_spc(*this, "mb89352")
{
}

void x68k_scsiext_device::device_start()
{
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());
	m_slot->space().install_read_bank(0xea0020,0xea1fff,"scsi_ext");
	m_slot->space().unmap_write(0xea0020,0xea1fff);

	uint8_t *ROM = machine().root_device().memregion(subtag("scsiexrom").c_str())->base();
	machine().root_device().membank("scsi_ext")->set_base(ROM);

m_slot->space().install_readwrite_handler(0xea0000,0xea001f,read8_delegate(FUNC(x68k_scsiext_device::register_r),this),write8_delegate(FUNC(x68k_scsiext_device::register_w),this),0x00ff00ff);
}

void x68k_scsiext_device::device_reset()
{
}

void x68k_scsiext_device::irq_w(int state)
{
	m_slot->irq2_w(state);  // correct?  Or perhaps selectable?
}

uint8_t x68k_scsiext_device::iack2()
{
	return 0xf6;
}

void x68k_scsiext_device::drq_w(int state)
{
	// TODO
}

READ8_MEMBER(x68k_scsiext_device::register_r)
{
	return m_spc->mb89352_r(space,offset);
}

WRITE8_MEMBER(x68k_scsiext_device::register_w)
{
	m_spc->mb89352_w(space,offset,data);
}
