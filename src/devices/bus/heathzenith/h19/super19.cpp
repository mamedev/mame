// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB) with the Super19 ROM

  Not Currently Implemented
    - Interlace video
    - Alternate Font19 alt 128 mode and extended 256 mode (missing ROM images)
    - ANSI character set selection (missing ROM image)
    - light pen support

****************************************************************************/

#include "emu.h"
#include "super19.h"

#include "igc.h"


namespace bus::heathzenith::h19 {

namespace {

/**
 * Heath TLB with Super19 ROM
 */
class heath_super19_tlb_device : public heath_tlb_device
{
public:
	heath_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/**
 * Heath TLB with the Super19 ROM plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_super19_tlb_device : public heath_igc_tlb_device
{
public:
	heath_igc_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

static INPUT_PORTS_START( super19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")          PORT_DIPLOCATION("SW401:1,2,3,4")
	PORT_DIPSETTING(    0x01, "110")
	PORT_DIPSETTING(    0x02, "150")
	PORT_DIPSETTING(    0x03, "300")
	PORT_DIPSETTING(    0x04, "600")
	PORT_DIPSETTING(    0x05, "1200")
	PORT_DIPSETTING(    0x06, "1800")
	PORT_DIPSETTING(    0x07, "2000")
	PORT_DIPSETTING(    0x08, "2400")
	PORT_DIPSETTING(    0x09, "3600")
	PORT_DIPSETTING(    0x0a, "4800")
	PORT_DIPSETTING(    0x0b, "7200")
	PORT_DIPSETTING(    0x0c, "9600")
	PORT_DIPSETTING(    0x0d, "19200")
	PORT_DIPSETTING(    0x0e, "38400")
	PORT_DIPNAME( 0x70, 0x00, "8 bit mode")         PORT_DIPLOCATION("SW401:5,6,7")
	PORT_DIPSETTING(    0x00, "Mode A/0 - 8th bit ignored, sent as 0")
	PORT_DIPSETTING(    0x10, "Mode B/1 - 8th bit ignored, sent as 1")
	PORT_DIPSETTING(    0x20, "Mode C/2 - 8 bit escape mode")
	PORT_DIPSETTING(    0x30, "Mode D/3 - 8 bit escape mode, invert 8th bit")
	PORT_DIPSETTING(    0x40, "Mode E/4 - 8 bit data mode")
	PORT_DIPSETTING(    0x50, "Mode F/5 - 8 bit data mode, invert 8th bit")
	PORT_DIPSETTING(    0x60, "7 bit data with odd parity, 8th bit ignored on input")
	PORT_DIPSETTING(    0x70, "7 bit data with even parity, 8th bit ignored on input")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")             PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x02, 0x00, "Transmit mode")      PORT_DIPLOCATION("SW402:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, "Slow")
	PORT_DIPNAME( 0x80, 0x00, "DEC Keypad Codes")   PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( igc_super19 )
	PORT_INCLUDE( super19 )

	PORT_INCLUDE( igc_common )
INPUT_PORTS_END


ROM_START( super19 )
	// Super19 ROM
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2732_super19_h447.u437",           0x0000, 0x1000, CRC(6c51aaa6) SHA1(5e368b39fe2f1af44a905dc474663198ab630117))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",         0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Original keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_444-37_h19keyb.u445",         0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
ROM_END


/**
 * Super19 ROM
 *
 * Developed by ATG Systems
 */
heath_super19_tlb_device::heath_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_tlb_device(mconfig, HEATH_SUPER19, tag, owner, clock)
{
}

const tiny_rom_entry *heath_super19_tlb_device::device_rom_region() const
{
	return ROM_NAME(super19);
}

ioport_constructor heath_super19_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(super19);
}


/**
 * SigmaSoft and Systems IGC plus TLB with Super19 ROM
 */
heath_igc_super19_tlb_device::heath_igc_super19_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_igc_tlb_device(mconfig, HEATH_IGC_SUPER19, tag, owner, clock)
{
}

const tiny_rom_entry *heath_igc_super19_tlb_device::device_rom_region() const
{
	return ROM_NAME(super19);
}

ioport_constructor heath_igc_super19_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(igc_super19);
}

} // anonymous namespace

} // namespace bus::heathzenith::h19

DEFINE_DEVICE_TYPE_PRIVATE(HEATH_SUPER19,     device_heath_tlb_card_interface, bus::heathzenith::h19::heath_super19_tlb_device,     "heath_super19_tlb",     "Heath Terminal Logic Board w/ Super19 ROM")
DEFINE_DEVICE_TYPE_PRIVATE(HEATH_IGC_SUPER19, device_heath_tlb_card_interface, bus::heathzenith::h19::heath_igc_super19_tlb_device, "heath_igc_super19_tlb", "Heath Terminal Logic Board w/ Super19 ROM plus SigmaSoft Interactive Graphics Controller")
