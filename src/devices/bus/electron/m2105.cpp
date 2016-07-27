// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BT Merlin M2105

**********************************************************************/


#include "m2105.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ELECTRON_M2105 = &device_creator<electron_m2105_device>;


//-------------------------------------------------
//  ROM( m2105 )
//-------------------------------------------------

ROM_START( m2105 )
	ROM_REGION(0x40000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("350")
	ROM_SYSTEM_BIOS(0, "350", "v3.50")
	ROMX_LOAD("ic22-sm-35l-1.ic22", 0x30000, 0x4000, CRC(e8f8a639) SHA1(eb7fa1e884be9c072ae0c1e598507b802422127f), ROM_BIOS(1))
	ROMX_LOAD("ic23-sm-35l-1.ic23", 0x34000, 0x4000, CRC(b1bb1d83) SHA1(07ca3a93744519b8d03bbf1c3c3537c0a0a3c6fe), ROM_BIOS(1))
	ROMX_LOAD("sk01-pc-35l-1.ic24", 0x38000, 0x4000, CRC(54fd4c09) SHA1(9588296306581580ba223cf6bce4be61476f14c4), ROM_BIOS(1))
	ROMX_LOAD("sk02-pc-35l-1.ic24", 0x3c000, 0x4000, CRC(c08de988) SHA1(86f2da5f8e9a5301ad40360e286f841f42e94a99), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "340", "v3.40")
	ROMX_LOAD("ic22-sm-34l-1.ic22", 0x30000, 0x4000, CRC(b514b15f) SHA1(a9c6c20b5a4f860b000511dde2f54497bcdd97b0), ROM_BIOS(2))
	ROMX_LOAD("ic23-sm-34l-1.ic23", 0x34000, 0x4000, CRC(18875889) SHA1(d1a7dd87c4d99869a1961becec5e9d567d8fad53), ROM_BIOS(2))
	ROMX_LOAD("sk01-pc-34l-1.ic24", 0x38000, 0x4000, CRC(a8796c9e) SHA1(29bc01b8f7617b252e4b243d13b1bbd3cd32cc3b), ROM_BIOS(2))
	ROMX_LOAD("sk02-pc-34l-1.ic24", 0x3c000, 0x4000, CRC(fa74063c) SHA1(cdc31c606e69e7a6d221b7340a310d475d487fc9), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "207", "v2.07")
	ROMX_LOAD("ic22-sm-207l-1.ic22", 0x30000, 0x4000, CRC(0c431547) SHA1(13d2eab49b9c79f507b7dd8436d1e56cf43be412), ROM_BIOS(3))
	ROMX_LOAD("ic23-sm-207l-1.ic23", 0x34000, 0x4000, CRC(15044d49) SHA1(e75fe4321579a9027527a0e256050d1444b3fe82), ROM_BIOS(3))
	ROMX_LOAD("sk01-pc-207l-1.ic24", 0x38000, 0x4000, CRC(0850bcea) SHA1(270e7a31e69e1454cfb70ced23a50f5d97efe4d5), ROM_BIOS(3))
	ROMX_LOAD("sk02-pc-207l-1.ic24", 0x3c000, 0x4000, CRC(d8b9143f) SHA1(4e132c7a6dae4caf7203139b51882706d508c449), ROM_BIOS(3))

	ROM_REGION(0x8000, "vsm", 0) /* system speech PHROM */
	ROM_LOAD("phroma.bin", 0x0000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


//-------------------------------------------------
//  ADDRESS_MAP( m2105 )
//-------------------------------------------------

//static ADDRESS_MAP_START( m2105_mem, AS_IO, 8, electron_m2105_device )
//  AM_RANGE(0x30000, 0x3ffff) AM_MIRROR(0x4000) AM_ROM AM_REGION("m2105_rom", 0)
//ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( m2105 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( m2105 )
	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	/* system via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 1000000)
	/*MCFG_VIA6522_READPA_HANDLER(READ8(electron_m2105_device, m2105_via_system_read_porta))
	MCFG_VIA6522_READPB_HANDLER(READ8(electron_m2105_device, m2105_via_system_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(electron_m2105_device, m2105_via_system_write_porta))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(electron_m2105_device, m2105_via_system_write_portb))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(electron_m2105_device, m2105_via_system_irq_w))*/

	/* user via */
	MCFG_DEVICE_ADD("via6522_1", VIA6522, 1000000)
	//MCFG_VIA6522_READPB_HANDLER(READ8(electron_m2105_device, m2105_via_user_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8("cent_data_out", output_latch_device, write))
	//MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(electron_m2105_device, m2105_via_user_write_portb))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE("centronics", centronics_device, write_strobe))
	//MCFG_VIA6522_IRQ_HANDLER(WRITELINE(electron_m2105_device, m2105_via_user_irq_w))

	/* speech hardware */
	MCFG_DEVICE_ADD("vsm", SPEECHROM, 0)
	MCFG_SOUND_ADD("tms5220", TMS5220, 640000)
	MCFG_TMS52XX_SPEECHROM("vsm")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	/* duart */
	MCFG_MC68681_ADD("sc2681", XTAL_3_6864MHz)
	//MCFG_MC68681_IRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, int0_w))
	//MCFG_MC68681_A_TX_CALLBACK(DEVWRITELINE("rs232_1", rs232_port_device, write_txd))
	//MCFG_MC68681_B_TX_CALLBACK(DEVWRITELINE("rs232_2", rs232_port_device, write_txd))
	//MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(electron_m2105_device, sio_out_w))

	//MCFG_RS232_PORT_ADD("rs232_1", default_rs232_devices, "terminal")
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sc2681", mc68681_device, rx_a_w))
	//MCFG_RS232_PORT_ADD("rs232_2", default_rs232_devices, nullptr)
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sc2681", mc68681_device, rx_b_w))

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("via6522_1", via6522_device, write_ca1)) MCFG_DEVCB_INVERT /* ack seems to be inverted? */
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor electron_m2105_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m2105 );
}

const rom_entry *electron_m2105_device::device_rom_region() const
{
	return ROM_NAME( m2105 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_m2105_device - constructor
//-------------------------------------------------

electron_m2105_device::electron_m2105_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ELECTRON_M2105, "Acorn M2105 Expansion", tag, owner, clock, "electron_m2105", __FILE__),
		device_electron_expansion_interface(mconfig, *this),
		m_exp_rom(*this, "exp_rom"),
		m_via6522_0(*this, "via6522_0"),
		m_via6522_1(*this, "via6522_1"),
		m_tms(*this, "tms5220"),
		m_centronics(*this, "centronics")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_m2105_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_m2105_device::device_reset()
{
}
