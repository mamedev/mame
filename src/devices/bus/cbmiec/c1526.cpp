// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1526/MPS-802/4023 Printer emulation

**********************************************************************/

#include "emu.h"
#include "c1526.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6504_TAG "u7d"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1526, c1526_device, "c1526", "Commodore 1526/MPS802 Printer")
DEFINE_DEVICE_TYPE(C4023, c4023_device, "c4023", "Commodore 4023 Printer")


//-------------------------------------------------
//  ROM( c1526 )
//-------------------------------------------------

ROM_START( c1526 )
	ROM_REGION( 0x2000, M6504_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "r05", "Revision 5" )
	ROMX_LOAD( "325341-05.u8d", 0x0000, 0x2000, CRC(3ef63c59) SHA1(a71be83a476d2777d33dddb0103c036a047975ba), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r07c", "Revision 7c" )
	ROMX_LOAD( "325341-08.u8d", 0x0000, 0x2000, CRC(38f85b4a) SHA1(25880091979b21fdaf713b53ef2f1cb8063a3505), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r07b", "Revision 7b (Swe/Fin)" )
	ROMX_LOAD( "cbm 1526 vers. 1.0 skand.gen.u8d", 0x0000, 0x2000, CRC(21051f69) SHA1(7e622fc39985ebe9333d2b546b3c85fd6ab17a53), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "grafik", "MPS802 GrafikROM II v60.12" )
	ROMX_LOAD( "mps802 grafikrom ii v60.12.u8d", 0x0000, 0x2000, CRC(9f5e6b18) SHA1(8b7f620a8f85e250b142d72b812a67fd0e292d68), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1526_device::device_rom_region() const
{
	return ROM_NAME( c1526 );
}


//-------------------------------------------------
//  ROM( c4023 )
//-------------------------------------------------

ROM_START( c4023 )
	ROM_REGION( 0x2000, M6504_TAG, 0 )
	ROM_LOAD( "325360-03.u8d", 0x0000, 0x2000, CRC(c6bb0977) SHA1(7a8c43d2e205f58d83709c04bc7795602a892ddd) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c4023_device::device_rom_region() const
{
	return ROM_NAME( c4023 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1526_mem )
//-------------------------------------------------

void c1526_device_base::c1526_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region(M6504_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1526_device::device_add_mconfig(machine_config &config)
{
	m6504_device &cpu(M6504(config, M6504_TAG, XTAL(4'000'000)/4));
	cpu.set_addrmap(AS_PROGRAM, &c1526_device::c1526_mem);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c4023_device::device_add_mconfig(machine_config &config)
{
	m6504_device &cpu(M6504(config, M6504_TAG, XTAL(4'000'000)/4));
	cpu.set_addrmap(AS_PROGRAM, &c4023_device::c1526_mem);
}


//-------------------------------------------------
//  INPUT_PORTS( c1526 )
//-------------------------------------------------

static INPUT_PORTS_START( c1526 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1526_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1526 );
}


//-------------------------------------------------
//  INPUT_PORTS( c4023 )
//-------------------------------------------------

static INPUT_PORTS_START( c4023 )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c4023_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c4023 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1526_device_base - constructor
//-------------------------------------------------

c1526_device_base::c1526_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
{
}


//-------------------------------------------------
//  c1526_device - constructor
//-------------------------------------------------

c1526_device::c1526_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1526_device_base(mconfig, C1526, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  c4023_device - constructor
//-------------------------------------------------

c4023_device::c4023_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c1526_device_base(mconfig, C4023, tag, owner, clock),
	device_ieee488_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1526_device_base::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1526_device_base::device_reset()
{
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void c1526_device::cbm_iec_atn(int state)
{
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void c1526_device::cbm_iec_data(int state)
{
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void c1526_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  ieee488_atn_w -
//-------------------------------------------------

void c4023_device::ieee488_atn(int state)
{
}


//-------------------------------------------------
//  ieee488_ifc_w -
//-------------------------------------------------

void c4023_device::ieee488_ifc(int state)
{
	if (!state)
	{
		device_reset();
	}
}
