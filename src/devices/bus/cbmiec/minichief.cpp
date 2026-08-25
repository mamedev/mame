// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ICT Mini Chief MC-20 Hard Disk Drive emulation

**********************************************************************/

/*

    TODO:

    - WD1002A-WX1 ISA controller card
    - Seagate ST225 (-chs 615,4,17 -ss 512)

*/

#include "emu.h"
#include "minichief.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "u1"
#define M6522_0_TAG     "u9"
#define M6526_TAG       "u20"
#define WD1770_TAG      "u11"
#define ISA_BUS_TAG     "isabus"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MINI_CHIEF, mini_chief_device, "minichif", "ICT Mini Chief Disk Drive")


//-------------------------------------------------
//  ROM( minichief )
//-------------------------------------------------

ROM_START( minichief )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "ictdos710.u2", 0x0000, 0x8000, CRC(aaacf7e9) SHA1(c1296995238ef23f18e7fec70a144a0566a25a27) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mini_chief_device::device_rom_region() const
{
	return ROM_NAME( minichief );
}


//-------------------------------------------------
//  ADDRESS_MAP( mini_chief_mem )
//-------------------------------------------------

void mini_chief_device::mini_chief_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x1800, 0x180f).mirror(0x03f0).m(M6522_0_TAG, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x03f0).rw(FUNC(mini_chief_device::via1_r), FUNC(mini_chief_device::via1_w));
	map(0x2000, 0x2003).mirror(0x1ffc).rw(WD1770_TAG, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x4000, 0x400f).mirror(0xff0).rw(M6526_TAG, FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	map(0x5000, 0x5fff).mirror(0x2000).ram();
	map(0x6000, 0x6fff).ram();
	map(0x8000, 0xffff).rom().region(M6502_TAG, 0);
}


//-------------------------------------------------
//  MOS6526_INTERFACE( mini_chief_cia_intf )
//-------------------------------------------------

uint8_t mini_chief_device::cia_pa_r()
{
	// TODO read from ISA bus @ 0x320 | A2 A1 A0

	return 0;
}

void mini_chief_device::cia_pa_w(uint8_t data)
{
	// TODO write to ISA bus @ 0x320 | A2 A1 A0
}

void mini_chief_device::cia_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0       ISA A0
	    1       ISA A1
	    2       ISA A2
	    3       ISA /SMEMR
	    4       ISA /SMEMW
	    5       ISA RESET
	    6
	    7

	*/
}


//-------------------------------------------------
//  isa8bus_interface isabus_intf
//-------------------------------------------------

static void mini_chief_isa8_cards(device_slot_interface &device)
{
	device.option_add("wd1002a_wx1", ISA8_WD1002A_WX1);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mini_chief_device::device_add_mconfig(machine_config &config)
{
	add_base_mconfig(config);
	add_cia_mconfig(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mini_chief_device::mini_chief_mem);

	// FIXME: determine ISA bus clock
	isa8_device &isa8(ISA8(config, ISA_BUS_TAG));
	isa8.set_memspace(m_maincpu, AS_PROGRAM);
	isa8.set_iospace(m_maincpu, AS_PROGRAM);
	ISA8_SLOT(config, "isa1", 0, ISA_BUS_TAG, mini_chief_isa8_cards, "wd1002a_wx1", false);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mini_chief_device - constructor
//-------------------------------------------------

mini_chief_device::mini_chief_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1571_device(mconfig, MINI_CHIEF, tag, owner, clock)
{
}
