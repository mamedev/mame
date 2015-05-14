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
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"
#include "machine/mb89352.h"
#include "x68k_scsiext.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type X68K_SCSIEXT = &device_creator<x68k_scsiext_device>;

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( x68k_cz6bs1 )
	ROM_REGION( 0x10000, "scsiexrom", 0 )
	ROM_LOAD16_WORD_SWAP( "scsiexrom.bin",   0x0000, 0x2000, CRC(7be488de) SHA1(49616c09a8986ffe6a12ad600febe512f7ba8ae4) )
ROM_END

const rom_entry *x68k_scsiext_device::device_rom_region() const
{
	return ROM_NAME( x68k_cz6bs1 );
}

// device machine config
static MACHINE_CONFIG_FRAGMENT( x68k_scsiext )
	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_1)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE3, "harddisk", SCSIHD, SCSI_ID_2)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE4, "harddisk", SCSIHD, SCSI_ID_3)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE5, "harddisk", SCSIHD, SCSI_ID_4)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE6, "harddisk", SCSIHD, SCSI_ID_5)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE7, "harddisk", SCSIHD, SCSI_ID_6)

	MCFG_DEVICE_ADD("mb89352", MB89352A, 0)
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_MB89352A_IRQ_CB(WRITELINE(x68k_scsiext_device, irq_w))
	MCFG_MB89352A_DRQ_CB(WRITELINE(x68k_scsiext_device, drq_w))
MACHINE_CONFIG_END

machine_config_constructor x68k_scsiext_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( x68k_scsiext );
}

x68k_scsiext_device::x68k_scsiext_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, X68K_SCSIEXT, "Sharp CZ-6BS1 SCSI-1", tag, owner, clock, "x68k_cz6bs1", __FILE__),
		device_x68k_expansion_card_interface(mconfig, *this),
		m_spc(*this, "mb89352")
{
}

void x68k_scsiext_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	UINT8* ROM;
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());
	space.install_read_bank(0xea0020,0xea1fff,0,0,"scsi_ext");
	space.unmap_write(0xea0020,0xea1fff,0,0);
	ROM = machine().root_device().memregion(subtag("scsiexrom").c_str())->base();
	machine().root_device().membank("scsi_ext")->set_base(ROM);
	space.install_readwrite_handler(0xea0000,0xea001f,0,0,read8_delegate(FUNC(x68k_scsiext_device::register_r),this),write8_delegate(FUNC(x68k_scsiext_device::register_w),this),0x00ff00ff);
}

void x68k_scsiext_device::device_reset()
{
}

void x68k_scsiext_device::irq_w(int state)
{
	m_slot->irq2_w(state);  // correct?  Or perhaps selectable?
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
