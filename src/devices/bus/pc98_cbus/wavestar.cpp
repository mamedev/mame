// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

QVision WaveStar sound card

-86 and -118 clone with an on-board H8 for waveblaster MIDI connection.

References:
- http://www.retropc.net/yasuma/V2/PC/SOUND/wavestar.html
- https://sammargh.github.io/pc98/ext_card_doc/wavestar.txt

TODO:
- ROM placeholder;

**************************************************************************************************/

#include "emu.h"
#include "wavestar.h"

DEFINE_DEVICE_TYPE(QVISION_WAVESTAR, qvision_wavestar_device, "qvision_wavestar", "QVision WaveStar sound card")

qvision_wavestar_device::qvision_wavestar_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, QVISION_WAVESTAR, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
{
}

ROM_START( wavestar )
	ROM_REGION( 0x8000, "cpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0,  "wst07a", "WST07A" )
	ROMX_LOAD( "wst07a.bin",    0x0000, 0x8000, CRC(4dd1a543) SHA1(0b32c371cc0fe17acbd7c566e707fd6687f5e7ae), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *qvision_wavestar_device::device_rom_region() const
{
	return ROM_NAME( wavestar );
}

static INPUT_PORTS_START( wavestar )
	// near the waveblaster connector
	// TODO: check activeness, if the H8 cares
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "PnP function" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "MPU MIDI enable" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "86 compatible sound" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "WSS DMA" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "Ch. 0" )
	PORT_DIPSETTING(    0x08, "Ch. 3" )
	PORT_DIPNAME( 0x10, 0x00, "FM port base" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "0x0188" )
	PORT_DIPSETTING(    0x10, "0x0288" )
	PORT_DIPNAME( 0x20, 0x00, "FM interrupt" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "INT5 (IRQ12)" )
	PORT_DIPSETTING(    0x20, "INT6 (IRQ13)" )
	PORT_DIPNAME( 0x40, 0x00, "MPU port base" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "0xe0d0" )
	PORT_DIPSETTING(    0x40, "0xe4d0" )
	PORT_DIPNAME( 0x80, 0x00, "MPU interrupt" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "INT2 (IRQ6)" )
	PORT_DIPSETTING(    0x80, "INT4 (IRQ10)" )
INPUT_PORTS_END

ioport_constructor qvision_wavestar_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( wavestar );
}

void qvision_wavestar_device::device_add_mconfig(machine_config &config)
{
	// TODO: unreadable XTAL near the H8
	// irqs 0 to 2 looks valid
	H83040(config, m_cpu, 8'000'000);
	m_cpu->set_addrmap(AS_PROGRAM, &qvision_wavestar_device::h8_map);
}

void qvision_wavestar_device::device_start()
{
}

void qvision_wavestar_device::device_reset()
{
}

void qvision_wavestar_device::h8_map(address_map &map)
{
	map(0x00000, 0x07fff).rom();
	map(0x10000, 0x17fff).ram(); // initialized to 0x80 at startup
//  map(0x20000, 0x2000f) C-Bus DAC host connection?
//  map(0x20030, 0x20031) address/data pair
}

