// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

NEC PC-9801-118 sound card  "CanBe Sound 2"

References:
- https://sammargh.github.io/pc98/ext_card_doc/9801-118.txt

YMF297 + some extra ports, apparently derived from -86.
Introduced around the same time as Windows 95 release, it has various compatibility issues
under DOS (especially when PnP is enabled).
Doesn't have a sound ROM, it also cannot be installed with an environment also sporting a -86.

TODO:
- Fix sound chip type (YMF297-F);
- Add CS-4232 support, it's an extended clone of the already emulated AD1848 used on the
  Windows Sound System;
- Understand what the obfuscated NEC "ANCHOR" and "MAZE" chips really are;
- PnP interface (missing BIOS);
- verify sound irq;
- test if driver can be installed under Windows 95;

**************************************************************************************************/

#include "emu.h"
#include "pc9801_118.h"

#include "sound/ymopn.h"
#include "speaker.h"


#define XTAL_5B 24.576_MHz_XTAL
#define XTAL_5D 33.8688_MHz_XTAL

DEFINE_DEVICE_TYPE(PC9801_118, pc9801_118_device, "pc9801_118", "NEC PC-9801-118 sound card")

pc9801_118_device::pc9801_118_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_opn3(*this, "opn3")
{
}

pc9801_118_device::pc9801_118_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_118_device(mconfig, PC9801_118, tag, owner, clock)
{
}

void pc9801_118_device::device_add_mconfig(machine_config &config)
{
	// TODO: "ANCHOR" & "MAZE" custom NEC chips
	// sourced by 5D clock

	SPEAKER(config, "speaker", 2).front();

	// actually YMF297-F (YMF288 + OPL3 compatible FM sources), unknown clock / divider
	// 5B is near both CS-4232 and this
	YM2608(config, m_opn3, XTAL_5B * 2 / 5);
	m_opn3->irq_handler().set([this] (int state) { m_bus->int_w(5, state); });
//  m_opn3->port_a_read_callback().set(FUNC(pc9801_118_device::opn_porta_r));
//  m_opn3->port_b_write_callback().set(FUNC(pc9801_118_device::opn_portb_w));
	m_opn3->add_route(ALL_OUTPUTS, "speaker", 1.00, 0);
	m_opn3->add_route(ALL_OUTPUTS, "speaker", 1.00, 1);

	// TODO: DA-15 PC gameport labeled "MIDI / Joystick"
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( pc9801_118 )
	// 12 line Jumper settings @ 8F
	// documented at https://sammargh.github.io/pc98/ext_card_doc/9801-118.txt
	// TODO: understand how SW can read these
	PORT_START("OPN3_JP_8F")
	PORT_CONFNAME( 0x001, 0x000, "PC-9801-118: Enable Plug and Play" ) // [1]
	PORT_CONFSETTING(    0x000, DEF_STR( No ) )
	PORT_CONFSETTING(    0x001, DEF_STR( Yes ) )
	// "group" is basically a obnoxious machine ID
	// details are in the aforementioned link, in a nutshell should be:
	// Group 1: later CanBe (Cb onward)
	// Group 2: early CanBe (Ce, Ce2, Cs2), 9821 MATE A
	// Group 4: several ValueStar models
	// Group 5: 9821 MATE B, Notebooks, 9801 BX, some H98 models
	// Group 3: anything not covered above (link also mentions BX4 here?)
	// In practice this should really be tested on field ...
	PORT_CONFNAME( 0x406, 0x000, "PC-9801-118: PCM Group select") // [2, 3, 11]
	PORT_CONFSETTING(     0x000, "Groups 2, 3, 4, 5" ) // uses -118 PCM
	PORT_CONFSETTING(     0x404, "Group 3" ) // uses PCM host
	PORT_CONFSETTING(     0x002, "Group 1" ) // ?
	// all other settings "prohibited"
	PORT_CONFNAME( 0x008, 0x008, "PC-9801-118: unknown [4]" ) // [4] "prohibited", always ON
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x008, DEF_STR( On ) )
	PORT_CONFNAME( 0x030, 0x000, "PC-9801-118: FM Interrupt setting" ) // [5,6]
	PORT_CONFSETTING(     0x000, "INT5 (IRQ12)" )
	PORT_CONFSETTING(     0x010, "INT6 (IRQ13)" )
	PORT_CONFSETTING(     0x020, "INT41 (IRQ10)" )
	PORT_CONFSETTING(     0x030, "INT0 (IRQ3)")
	PORT_CONFNAME( 0x040, 0x000, "PC-9801-118: DMA channel" ) // [7]
	PORT_CONFSETTING(     0x000, "1" )
	PORT_CONFSETTING(     0x040, "2" )
	PORT_CONFNAME( 0x180, 0x000, "PC-9801-118: PCM Interrupt setting" ) // [8,9]
	PORT_CONFSETTING(     0x000, "INT5 (IRQ12)" )
	PORT_CONFSETTING(     0x080, "INT1 (IRQ5)" )
	PORT_CONFSETTING(     0x100, "INT41 (IRQ10)" )
	PORT_CONFSETTING(     0x180, "INT0 (IRQ3)" )
	PORT_CONFNAME( 0x200, 0x000, "PC-9801-118: enable MIDI Interrupt" ) // [10]
	PORT_CONFSETTING(     0x000, DEF_STR( No ) )
	PORT_CONFSETTING(     0x200, DEF_STR( Yes ) ) // auto for PnP, INT41 for non-PnP
	PORT_CONFNAME( 0x800, 0x000, "PC-9801-118: unknown [12]" ) // [12] "prohibited", always OFF
	PORT_CONFSETTING(     0x000, DEF_STR( Off ) )
	PORT_CONFSETTING(     0x800, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor pc9801_118_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc9801_118 );
}

ROM_START( pc9801_118 )
	ROM_REGION( 0x20000, "pnp_bios", ROMREGION_ERASE00 )
	// NB: either socket is exclusively populated, earlier models populates 1E while later populates 2E.
	// Most likely contains same data or slight revision bump.
	// μPD27c1024 socket
	ROM_LOAD( "118 e316.1e", 0x00000, 0x20000, NO_DUMP )
	// LH531024N socket at .2e (unreadable label)

	ROM_REGION( 0x100000, "opn3", ROMREGION_ERASE00 )
ROM_END

const tiny_rom_entry *pc9801_118_device::device_rom_region() const
{
	return ROM_NAME( pc9801_118 );
}


void pc9801_118_device::device_start()
{
	save_item(NAME(m_ext_reg));
}

void pc9801_118_device::device_reset()
{
	// assume disabled on boot
	m_ext_reg = 0;
}

void pc9801_118_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &pc9801_118_device::io_map);
	}
}


void pc9801_118_device::io_map(address_map &map)
{
	// hardwired on this board
	map(0x0188, 0x018f).rw(m_opn3, FUNC(ym2608_device::read), FUNC(ym2608_device::write)).umask16(0x00ff);

	map(0xa460, 0xa460).rw(FUNC(pc9801_118_device::id_r), FUNC(pc9801_118_device::ext_w));
}

u8 pc9801_118_device::id_r(offs_t offset)
{
	return 0x80 | (m_ext_reg & 1);
}

void pc9801_118_device::ext_w(offs_t offset, u8 data)
{
	m_ext_reg = BIT(data, 0);
	if (m_ext_reg)
		popmessage("PC9801-118: extended CS4231 enable");
}
