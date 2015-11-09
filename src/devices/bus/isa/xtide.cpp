// license:BSD-3-Clause
// copyright-holders:Phill Harvey-Smith
/***************************************************************************

The XT-IDE project is a Vintage Computer forum driven project to develop and manufacturer an 8-bit ISA IDE controller.
It allows any PC/XT class machine to use modern IDE hard drives or Compact Flash devices for long term storage.

http://www.vintage-computer.com/vcforum/showwiki.php?title=XTIDE+project

Card has jumpers for I/O base address, and ROM base address, the default
rom images being we'll emulate an I/O base of 0x300 and a ROM base of 0xC8000.

If the I/O address is changed then you will need to use XTIDECFG to configure the ROM.
The opensource bios is available from :
http://code.google.com/p/xtideuniversalbios/

The data high register is connected to a pair of latches that have the MSB of
the 16 bit data word latched into so that 16 bit IO may be performmed, this
involves the following :

A Data read will read the data register first, and obtain the bottom 8 bits
of the data word, the top 8 bits will be latched at the same time these are
then read from the latch to the processor.

A data write will first write the top 8 bits to the latch, and then the bottom
8 bits to the normal data register, this will also transfer the top 8 bits to
the drive.

A modded r1 has A0 & A3 swapped, AKA chuckmod, which puts the high & low bytes together.
This also affects the eeprom, so the BIOS would need to be shuffled (or the plain images flashed using XTIDECFG).

IDE Register                XTIDE rev 1     rev 2 or modded rev 1
Data (XTIDE Data Low)       0               0
Error (in), Features (out)  1               8
Sector Count                2               2
Sector Number,
LBA bits 0...7,
LBA48 bits 24...31          3               10
Low Cylinder,
LBA bits 8...15,
LBA48 bits 32...39          4               4
High Cylinder,
LBA bits 16...23,
LBA48 bits 40...47          5               12
Drive and Head Select,
LBA28 bits 24...27          6               6
Status (in), Command (out)  7               14
XTIDE Data High             8               1
Alternative Status (in),
Device Control (out)        14              7

***************************************************************************/



#include "xtide.h"


READ8_MEMBER( xtide_device::read )
{
	UINT8 result;

	if (offset == 0)
	{
		UINT16 data16 = m_ata->read_cs0(space, offset & 7, 0xffff);
		result = data16 & 0xff;
		m_d8_d15_latch = data16 >> 8;
	}
	else if (offset < 8)
	{
		result = m_ata->read_cs0(space, offset & 7, 0xff);
	}
	else if (offset == 8)
	{
		result = m_d8_d15_latch;
	}
	else
	{
		result = m_ata->read_cs1(space, offset & 7, 0xff);
	}

//  logerror("%s xtide_device::read: offset=%d, result=%2X\n",device->machine().describe_context(),offset,result);

	return result;
}

WRITE8_MEMBER( xtide_device::write )
{
//  logerror("%s xtide_device::write: offset=%d, data=%2X\n",device->machine().describe_context(),offset,data);

	if (offset == 0)
	{
		// Data register transfer low byte and latched high
		UINT16 data16 = (m_d8_d15_latch << 8) | data;
		m_ata->write_cs0(space, offset & 7, data16, 0xffff);
	}
	else if (offset < 8)
	{
		m_ata->write_cs0(space, offset & 7, data, 0xff);
	}
	else if (offset == 8)
	{
		m_d8_d15_latch = data;
	}
	else
	{
		m_ata->write_cs1(space, offset & 7, data, 0xff);
	}
}


WRITE_LINE_MEMBER(xtide_device::ide_interrupt)
{
	switch (m_irq_number)
	{
		case 0x02: m_isa->irq2_w(state); break;
		case 0x03: m_isa->irq3_w(state); break;
		case 0x04: m_isa->irq4_w(state); break;
		case 0x05: m_isa->irq5_w(state); break;
		case 0x07: m_isa->irq7_w(state); break;
	}
}

static MACHINE_CONFIG_FRAGMENT( xtide_config )
	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", NULL, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(xtide_device, ide_interrupt))

	MCFG_EEPROM_2864_ADD("eeprom")
MACHINE_CONFIG_END

static INPUT_PORTS_START( xtide_port )
	PORT_START("BIOS_BASE")
	PORT_DIPNAME( 0x0F, 0x02, "XT-IDE ROM base segment")
	PORT_DIPSETTING(    0x00, "C000" )
	PORT_DIPSETTING(    0x01, "C400" )
	PORT_DIPSETTING(    0x02, "C800" )
	PORT_DIPSETTING(    0x03, "CC00" )
	PORT_DIPSETTING(    0x04, "D000" )
	PORT_DIPSETTING(    0x05, "D400" )
	PORT_DIPSETTING(    0x06, "D800" )
	PORT_DIPSETTING(    0x07, "DC00" )
	PORT_DIPSETTING(    0x08, "E000" )
	PORT_DIPSETTING(    0x09, "E400" )
	PORT_DIPSETTING(    0x0A, "E800" )
	PORT_DIPSETTING(    0x0B, "EC00" )
	PORT_DIPSETTING(    0x0C, "F000" )
	PORT_DIPSETTING(    0x0D, "F400" )
	PORT_DIPSETTING(    0x0E, "F800" )
	PORT_DIPSETTING(    0x0F, "FC00" )

	PORT_START("IO_ADDRESS")
	PORT_DIPNAME( 0x0F, 0x08, "XT-IDE I/O address")
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPSETTING(    0x01, "220" )
	PORT_DIPSETTING(    0x02, "240" )
	PORT_DIPSETTING(    0x03, "260" )
	PORT_DIPSETTING(    0x04, "280" )
	PORT_DIPSETTING(    0x05, "2A0" )
	PORT_DIPSETTING(    0x06, "2C0" )
	PORT_DIPSETTING(    0x07, "2E0" )
	PORT_DIPSETTING(    0x08, "300" )
	PORT_DIPSETTING(    0x09, "320" )
	PORT_DIPSETTING(    0x0A, "340" )
	PORT_DIPSETTING(    0x0B, "360" )
	PORT_DIPSETTING(    0x0C, "380" )
	PORT_DIPSETTING(    0x0D, "3A0" )
	PORT_DIPSETTING(    0x0E, "3C0" )
	PORT_DIPSETTING(    0x0F, "3E0" )

	PORT_START("IRQ")
	PORT_DIPNAME( 0x07, 0x05, "XT-IDE IRQ")
	PORT_DIPSETTING(    0x02, "IRQ 2" )
	PORT_DIPSETTING(    0x03, "IRQ 3" )
	PORT_DIPSETTING(    0x04, "IRQ 4" )
	PORT_DIPSETTING(    0x05, "IRQ 5" )
	PORT_DIPSETTING(    0x07, "IRQ 7" )
INPUT_PORTS_END

ROM_START( xtide )
	ROM_REGION(0x02000,"eeprom", 0)

	ROM_DEFAULT_BIOS("xub200b3xt")

	ROM_SYSTEM_BIOS( 0, "xtide_010", "Hargle's Bios v0.10" )
	ROMX_LOAD( "oprom.bin(v0.10)", 0x000000, 0x002000, CRC(56075ac2) SHA1(f55285a1ed8414c8ddf2364421552e0548cf548f), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "xtide_011", "Hargle's Bios v0.11" )
	ROMX_LOAD( "oprom.bin(v0.11)", 0x000000, 0x002000, CRC(c5fee6c5) SHA1(cc3a015d8d36208d99de8500c962828d2daea939), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "xub110xt", "XTIDE_Universal_BIOS_v1.1.0 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v1.1.0)", 0x000000, 0x002000, CRC(d13f6ae7) SHA1(42c7e7cbf949af718abbd279e9a33680b8428400), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "xub110xtp", "XTIDE_Universal_BIOS_v1.1.0 (XT 80186+)" )
	ROMX_LOAD( "ide_xtp.bin(v1.1.0)", 0x000000, 0x002000, CRC(4dd9124b) SHA1(af9e5742f57cccd16a580efcbda519314afd272d), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS( 4, "xub110at", "XTIDE_Universal_BIOS_v1.1.0 (AT)" )
	ROMX_LOAD( "ide_at.bin(v1.1.0)", 0x000000, 0x002000, CRC(673ebf69) SHA1(3960c0be39a787e740d14c8667fc09437bd56ff7), ROM_BIOS(5) )

	ROM_SYSTEM_BIOS( 5, "xub111xt", "XTIDE_Universal_BIOS_v1.1.1 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v1.1.1)", 0x000000, 0x002000, CRC(6c15f095) SHA1(007db7dc16ccbbd9d297e13b81dee4785ac9fa9b), ROM_BIOS(6) )

	ROM_SYSTEM_BIOS( 6, "xub111xtp", "XTIDE_Universal_BIOS_v1.1.1 (XT 80186+)" )
	ROMX_LOAD( "ide_xtp.bin(v1.1.1)", 0x000000, 0x002000, CRC(3eb1210d) SHA1(1d2e1cd20d548f794c889cdcfa7ebf224d073052), ROM_BIOS(7) )

	ROM_SYSTEM_BIOS( 7, "xub111at", "XTIDE_Universal_BIOS_v1.1.1 (AT)" )
	ROMX_LOAD( "ide_at.bin(v1.1.1)", 0x000000, 0x002000, CRC(c808b718) SHA1(215903c68784c886a3117662c735a84d203b7858), ROM_BIOS(8) )

	ROM_SYSTEM_BIOS( 8, "xub113xt", "XTIDE_Universal_BIOS_v1.1.3 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v1.1.3)", 0x000000, 0x002000, CRC(3158452f) SHA1(1363f370196a12c6770de5a76e8daf283b561625), ROM_BIOS(9) )

	ROM_SYSTEM_BIOS( 9, "xub113xtp", "XTIDE_Universal_BIOS_v1.1.3 (XT 80186+)" )
	ROMX_LOAD( "ide_xtp.bin(v1.1.3)", 0x000000, 0x002000, CRC(d994fa2f) SHA1(68bdc24cc9878a09a77d6420b9565e51bb08e9b1), ROM_BIOS(10) )

	ROM_SYSTEM_BIOS( 10, "xub113at", "XTIDE_Universal_BIOS_v1.1.3 (AT)" )
	ROMX_LOAD( "ide_at.bin(v1.1.3)", 0x000000, 0x002000, CRC(14ce1ced) SHA1(3eea39ffcb9a796c30f48d12ec8ff13572b3b9dc), ROM_BIOS(11) )

	ROM_SYSTEM_BIOS( 11, "xub114xt", "XTIDE_Universal_BIOS_v1.1.4 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v1.1.4)", 0x000000, 0x002000, CRC(c73d2dcc) SHA1(335a79be455ef856f2b0c7444fc0b1dfeccc649c), ROM_BIOS(12) )

	ROM_SYSTEM_BIOS( 12, "xub114at", "XTIDE_Universal_BIOS_v1.1.4 (AT)" )
	ROMX_LOAD( "ide_at.bin(v1.1.4)", 0x000000, 0x002000, CRC(ebb3deda) SHA1(bcab1743e37f5c0a252d7b127b13e64d5c65baf3), ROM_BIOS(13) )

	ROM_SYSTEM_BIOS( 13, "xub115xt", "XTIDE_Universal_BIOS_v1.1.5 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v1.1.5)", 0x000000, 0x002000, CRC(33a7e0ee) SHA1(b610fd8ea31f5b0568b8b3f2c3ef682be4897a3d), ROM_BIOS(14) )

	ROM_SYSTEM_BIOS( 14, "xub115xtp", "XTIDE_Universal_BIOS_v1.1.3 (XT 80186+)" )
	ROMX_LOAD( "ide_xtp.bin(v1.1.5)", 0x000000, 0x002000, CRC(44ad9ee9) SHA1(9cd275469703cadb85b6654c56e421a151324ac0), ROM_BIOS(15) )

	ROM_SYSTEM_BIOS( 15, "xub115at", "XTIDE_Universal_BIOS_v1.1.5 (AT)" )
	ROMX_LOAD( "ide_at.bin(v1.1.5)", 0x000000, 0x002000, CRC(434286ce) SHA1(3fc07d174924e7c48b4758a7ba76ecd5362bd75b), ROM_BIOS(16) )

	ROM_SYSTEM_BIOS( 16, "xub200b1xt", "XTIDE_Universal_BIOS_v2.0.0_beta1 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v2.0.0_beta1)", 0x000000, 0x002000, CRC(379579e7) SHA1(da5ee7b9c43a55592fe909451d31a6766d0ab977), ROM_BIOS(17) )

	ROM_SYSTEM_BIOS( 17, "xub200b1xtp", "XTIDE_Universal_BIOS_v2.0.0_beta1 (XT 80186+)" )
	ROMX_LOAD( "ide_xtp.bin(v2.0.0_beta1)", 0x000000, 0x002000, CRC(a887ed63) SHA1(fb33d9e8e8824f61a8d247610d7bd215b7e306b4), ROM_BIOS(18) )

	ROM_SYSTEM_BIOS( 18, "xub200b1at", "XTIDE_Universal_BIOS_v2.0.0_beta1 (AT)" )
	ROMX_LOAD( "ide_at.bin(v2.0.0_beta1)", 0x000000, 0x002000, CRC(cd2d8791) SHA1(2f831e7701d181d719a777b63dbd61d87036ee21), ROM_BIOS(19) )

	ROM_SYSTEM_BIOS( 19, "xub200b2xt", "XTIDE_Universal_BIOS_v2.0.0_beta2 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v2.0.0_beta2)", 0x000000, 0x002000, CRC(61ae1143) SHA1(de5f04b71f2614a0c3db6ec01a5dc7546205100a), ROM_BIOS(20) )

	ROM_SYSTEM_BIOS( 20, "xub200b2xtp", "XTIDE_Universal_BIOS_v2.0.0_beta2 (XT 80186+)" )
	ROMX_LOAD( "ide_xtp.bin(v2.0.0_beta2)", 0x000000, 0x002000, CRC(58883399) SHA1(582718d6dcd8a4367ee86da3201fb966dc4fffcd), ROM_BIOS(21) )

	ROM_SYSTEM_BIOS( 21, "xub200b2at", "XTIDE_Universal_BIOS_v2.0.0_beta2 (AT)" )
	ROMX_LOAD( "ide_at.bin(v2.0.0_beta2)", 0x000000, 0x002000, CRC(33fe9336) SHA1(723de092af44e2b709b620f3b591ec12bdca53cd), ROM_BIOS(22) )

	ROM_SYSTEM_BIOS( 22, "xub200b3xt", "XTIDE_Universal_BIOS_v2.0.0_beta3 (XT)" )
	ROMX_LOAD( "ide_xt.bin(v2.0.0_beta3)", 0x000000, 0x002000, CRC(0a8d4bb4) SHA1(509504c1c54842bcd24cdd318bcf6fb0ece09c33), ROM_BIOS(23) )

	ROM_SYSTEM_BIOS( 23, "xub200b3xtp", "XTIDE_Universal_BIOS_v2.0.0_beta3 (XT 80186+)" )
	ROMX_LOAD( "ide_xtp.bin(v2.0.0_beta3)", 0x000000, 0x002000, CRC(a58658f8) SHA1(f3a4c1dfc8e2b56eeaf0e39aa192125bc05af626), ROM_BIOS(24) )

	ROM_SYSTEM_BIOS( 24, "xub200b3at", "XTIDE_Universal_BIOS_v2.0.0_beta3 (AT)" )
	ROMX_LOAD( "ide_at.bin(v2.0.0_beta3)", 0x000000, 0x002000, CRC(fc228f41) SHA1(c0053710ebac15284e740889967d73a6657734c7), ROM_BIOS(25) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_XTIDE = &device_creator<xtide_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor xtide_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( xtide_config );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor xtide_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( xtide_port );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *xtide_device::device_rom_region() const
{
	return ROM_NAME( xtide );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  xtide_device - constructor
//-------------------------------------------------

xtide_device::xtide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ISA8_XTIDE, "XT-IDE Fixed Drive Adapter", tag, owner, clock, "isa8_xtide", __FILE__),
	device_isa8_card_interface( mconfig, *this ),
	m_ata(*this, "ata"),
	m_eeprom(*this, "eeprom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void xtide_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void xtide_device::device_reset()
{
	int base_address    = ((ioport("BIOS_BASE")->read() & 0x0F) * 16 * 1024) + 0xC0000;
	int io_address      = ((ioport("IO_ADDRESS")->read() & 0x0F) * 0x20) + 0x200;
	m_irq_number        = (ioport("IRQ")->read() & 0x07);

	m_isa->install_memory(base_address, base_address + 0x1fff, 0, 0, read8_delegate(FUNC(eeprom_parallel_28xx_device::read), &(*m_eeprom)), write8_delegate(FUNC(eeprom_parallel_28xx_device::write), &(*m_eeprom)));
	m_isa->install_device(io_address, io_address + 0xf, 0, 0, read8_delegate(FUNC(xtide_device::read), this), write8_delegate(FUNC(xtide_device::write), this));

	//logerror("xtide_device::device_reset(), bios_base=0x%5X to 0x%5X, I/O=0x%3X, IRQ=%d\n",base_address,base_address + (16*1024)  -1 ,io_address,irq);
}
