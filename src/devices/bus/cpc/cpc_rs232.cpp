// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * cpc_rs232.c
 *
 *  Created on: 22/04/2014
 */

#include "cpc_rs232.h"
#include "includes/amstrad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_RS232 = &device_creator<cpc_rs232_device>;
const device_type CPC_RS232_AMS = &device_creator<cpc_ams_rs232_device>;

// device machine config
static MACHINE_CONFIG_FRAGMENT( cpc_rs232 )
	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(2000000)
	MCFG_PIT8253_CLK1(2000000)
	MCFG_PIT8253_CLK2(2000000)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(cpc_rs232_device, pit_out0_w))
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(cpc_rs232_device, pit_out1_w))
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(cpc_rs232_device, pit_out2_w))

	MCFG_Z80DART_ADD("dart", XTAL_4MHz, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232",default_rs232_devices,NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("dart",z80dart_device,rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("dart",z80dart_device,dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("dart",z80dart_device,ctsa_w))
//  MCFG_RS232_RI_HANDLER(DEVWRITELINE("dart",z80dart_device,ria_w))

	// pass-through
	MCFG_DEVICE_ADD("exp", CPC_EXPANSION_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(cpc_exp_cards, NULL, false)
	MCFG_CPC_EXPANSION_SLOT_OUT_IRQ_CB(DEVWRITELINE("^", cpc_expansion_slot_device, irq_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_NMI_CB(DEVWRITELINE("^", cpc_expansion_slot_device, nmi_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_ROMDIS_CB(DEVWRITELINE("^", cpc_expansion_slot_device, romdis_w))  // ROMDIS

MACHINE_CONFIG_END

ROM_START( cpc_rs232 )
	ROM_REGION( 0x8000, "exp_rom", 0 )
	ROM_LOAD( "comstar1.rom",   0x0000, 0x4000, CRC(ddcade50) SHA1(d09ee0bd51a8e1cafc5107a75fed839dda3d21e5) )
	ROM_LOAD( "comstar2.rom",   0x4000, 0x4000, CRC(664e788c) SHA1(13d033f2d1cad70140deb903d787ba514f236a59) )
ROM_END

ROM_START( cpc_rs232_ams )
	ROM_REGION( 0x4000, "exp_rom", 0 )
	ROM_SYSTEM_BIOS( 0, "amstrad", "Amstrad RS232C interface (v1)" )
	ROMX_LOAD( "rs232101.rom",   0x0000, 0x2000, CRC(c6eb52b2) SHA1(8a7e0a1183fdde8d07bc8827a3e159ca3022f93b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "mercitel", "Amstrad RS232C interface (v1) + Mercitel (v1.4)" )
	ROMX_LOAD( "rs232mercitel14.rom",   0x0000, 0x4000, CRC(8ffb114b) SHA1(145233fe8d4db9f5265eeac767d8ee8d45d14755), ROM_BIOS(2) )
ROM_END

machine_config_constructor cpc_rs232_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_rs232 );
}

const rom_entry *cpc_rs232_device::device_rom_region() const
{
	return ROM_NAME( cpc_rs232 );
}

const rom_entry *cpc_ams_rs232_device::device_rom_region() const
{
	return ROM_NAME( cpc_rs232_ams );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_rs232_device::cpc_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_RS232, "Pace RS232C interface", tag, owner, clock, "cpc_ser", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_pit(*this,"pit"),
	m_dart(*this,"dart"),
	m_rs232(*this,"rs232"), m_slot(nullptr)
{
}

cpc_rs232_device::cpc_rs232_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_cpc_expansion_card_interface(mconfig, *this),
	m_pit(*this,"pit"),
	m_dart(*this,"dart"),
	m_rs232(*this,"rs232"), m_slot(nullptr)
{
}

cpc_ams_rs232_device::cpc_ams_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	cpc_rs232_device(mconfig, CPC_RS232_AMS, "Amstrad RS232C interface", tag, owner, clock, "cpc_serams", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_rs232_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_IO);
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	space.install_readwrite_handler(0xfadc,0xfadf,0,0,read8_delegate(FUNC(cpc_rs232_device::dart_r),this),write8_delegate(FUNC(cpc_rs232_device::dart_w),this));
	space.install_readwrite_handler(0xfbdc,0xfbdf,0,0,read8_delegate(FUNC(cpc_rs232_device::pit_r),this),write8_delegate(FUNC(cpc_rs232_device::pit_w),this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_rs232_device::device_reset()
{
}


WRITE_LINE_MEMBER(cpc_rs232_device::pit_out0_w)
{
	m_dart->txca_w(state);
}

WRITE_LINE_MEMBER(cpc_rs232_device::pit_out1_w)
{
	m_dart->rxca_w(state);
}

WRITE_LINE_MEMBER(cpc_rs232_device::pit_out2_w)
{
	m_dart->txcb_w(state);
	m_dart->rxcb_w(state);
}

READ8_MEMBER(cpc_rs232_device::dart_r)
{
	return m_dart->ba_cd_r(space,offset);
}

WRITE8_MEMBER(cpc_rs232_device::dart_w)
{
	m_dart->ba_cd_w(space,offset,data);
}

READ8_MEMBER(cpc_rs232_device::pit_r)
{
	return m_pit->read(space,offset);
}

WRITE8_MEMBER(cpc_rs232_device::pit_w)
{
	m_pit->write(space,offset,data);
}
