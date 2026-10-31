// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB) with the Watzman ROM

****************************************************************************/

#include "emu.h"
#include "watz.h"

#include "igc.h"


namespace bus::heathzenith::h19 {

namespace {

/**
 * Heath TLB with Watzman (HUG) ROM
 */
class heath_watz_tlb_device : public heath_tlb_device
{
public:
	heath_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/**
 * Heath TLB with the Watzman ROM plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_watz_tlb_device : public heath_igc_tlb_device
{
public:
	heath_igc_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


static INPUT_PORTS_START( watz19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x0f, 0x0c, "Baud Rate")   PORT_DIPLOCATION("SW401:1,2,3,4")
	PORT_DIPSETTING(    0x00, "75")
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
	PORT_DIPSETTING(    0x0f, "134.5")
	PORT_DIPNAME( 0x40, 0x00, "Word Size")   PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, "8-bit Word")
	PORT_DIPSETTING(    0x40, "7-bit Word")
	PORT_DIPNAME( 0x80, 0x80, "Duplex")      PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")
INPUT_PORTS_END

static INPUT_PORTS_START( igc_watz )
	PORT_INCLUDE( watz19 )

	PORT_INCLUDE( igc_common )
INPUT_PORTS_END


ROM_START( watz19 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("watzman-a")

	// Watzman ROM
	ROM_SYSTEM_BIOS(0, "watzman", "Watzman")
	ROMX_LOAD("watzman.u437",                     0x0000, 0x1000, CRC(8168b6dc) SHA1(bfaebb9d766edbe545d24bc2b6630be4f3aa0ce9), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "watzman-a", "Watzman w/ clock persists after reset")
	ROMX_LOAD("watzman-a.u437",                   0x0000, 0x1000, CRC(1f7553e9) SHA1(ac6ddb12b4fb46c1a0ad08ee43978ad3153b51aa), ROM_BIOS(1))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",         0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Watzman keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "keybd.u445",                       0x0000, 0x0800, CRC(58dc8217) SHA1(1b23705290bdf9fc6342065c6a528c04bff67b13))
ROM_END


/**
 * Watzman ROM
 *
 * Developed by Barry Watzman, sold by HUG (Heath Users Group)
 */
heath_watz_tlb_device::heath_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_tlb_device(mconfig, HEATH_WATZ, tag, owner, clock)
{
}

const tiny_rom_entry *heath_watz_tlb_device::device_rom_region() const
{
	return ROM_NAME(watz19);
}

ioport_constructor heath_watz_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(watz19);
}


/**
 * SigmaSoft and Systems IGC plus TLB with Watzman ROM
 */
heath_igc_watz_tlb_device::heath_igc_watz_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_igc_tlb_device(mconfig, HEATH_IGC_WATZ, tag, owner, clock)
{
}

const tiny_rom_entry *heath_igc_watz_tlb_device::device_rom_region() const
{
	return ROM_NAME(watz19);
}

ioport_constructor heath_igc_watz_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(igc_watz);
}

} // anonymous namespace

} // namespace bus::heathzenith::h19

DEFINE_DEVICE_TYPE_PRIVATE(HEATH_WATZ,     device_heath_tlb_card_interface, bus::heathzenith::h19::heath_watz_tlb_device,     "heath_watz_tlb",     "Heath Terminal Logic Board w/ Watzman ROM")
DEFINE_DEVICE_TYPE_PRIVATE(HEATH_IGC_WATZ, device_heath_tlb_card_interface, bus::heathzenith::h19::heath_igc_watz_tlb_device, "heath_igc_watz_tlb", "Heath Terminal Logic Board w/ Watzman ROM plus SigmaSoft Interactive Graphics Controller")
