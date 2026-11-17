// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    DEC VT100 Advanced Video Option (AVO), board 5413097 (VT1XX-AB)

    Plugs onto the terminal controller board's J1.  Adds 1K x 8 of screen
    RAM at 2C00-2FFF, 4K x 4 of attribute RAM at 3000-3FFF, and four 2K
    program ROM sockets, E8 also taking an 8K ROM.

    The terminal controller decodes only A10-A13, so its ROM and RAM repeat
    every 16K.  The AVO decodes A13-A15 (E12, a 74LS138 enabled on every
    cycle) and asserts MEM DISABLE for all of 8000-BFFF (E16, open collector
    gates on its 8000-9FFF and A000-BFFF outputs), so that area is the AVO's
    whether or not its sockets are filled; the patch switches add blocks
    below 2000 only.  TM-003 6.2.5.3 reads as if MEM DISABLE depended on the
    jumpers and covered all memory above 4000; the schematic does neither
    (E12's other outputs are unconnected).  Its RAM is selected from the
    terminal controller's decode, so it repeats with the base RAM outside
    8000-BFFF.

    Jumpers W5-W7 (switch S1 on later boards) place the 8K ROM at 0000
    (overlay), 8000 or A000; the 2K sockets answer in the 8000/A000 block it
    doesn't claim (E16, E23, E24).  Only W6 is modelled, the setting the
    VT1XX-AC ROMs and the VT132's 23-099E2/23-100E2 use alone: 8K socket
    (E8) at A000, 2K ROMs A-C at 8000-97FF.  ROM D at 9800 also needs W8,
    W11 and W14.  No 8K ROM is dumped, so A000-BFFF is always an empty
    socket.  Patch jumpers W1-W4/W8-W14, which put 2K ROMs over the base
    firmware or add ROM D, and the other 8K placements aren't modelled: no
    ROMs that use them are dumped.

    TM 6.2.3 names the 2K sockets E19, E17, E13 and E8; the schematic and
    its legend have E21 for ROM A (E19 is the S1 switch pack).

    Sources: VT100 technical manual EK-VT100-TM-003, 4.2.5 and 6.2.3,
    tables 5-8 to 5-10; MP-00633 print set, AVO schematic D-CS-5413097.

    TODO:
    - patch jumpers and the W5/W7 8K ROM placements, once ROMs for them
      are dumped

***************************************************************************/

#include "emu.h"
#include "vt100_avo.h"

#include <algorithm>
#include <iterator>


//**************************************************************************
//  SLOT
//**************************************************************************

DEFINE_DEVICE_TYPE(VT100_AVO_SLOT, vt100_avo_slot_device, "vt100_avo_slot", "DEC VT100 AVO connector")

vt100_avo_slot_device::vt100_avo_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VT100_AVO_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_vt100_avo_interface>(mconfig, *this)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
	, m_card(nullptr)
{
}

void vt100_avo_slot_device::device_start()
{
	m_card = get_card_device();
}

u8 vt100_avo_slot_device::video_r(offs_t offset)
{
	// nothing answers without the AVO: attributes all inactive
	return m_card ? m_card->video_r(offset) : 0xff;
}


device_vt100_avo_interface::device_vt100_avo_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "vt100avo")
	, m_slot(dynamic_cast<vt100_avo_slot_device *>(device.owner()))
{
}


//**************************************************************************
//  CARD
//**************************************************************************

namespace {

class vt100_avo_device : public device_t, public device_vt100_avo_interface
{
public:
	vt100_avo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: vt100_avo_device(mconfig, VT100_AVO, tag, owner, clock)
	{
	}

	// device_vt100_avo_interface implementation
	virtual u8 video_r(offs_t offset) override;

protected:
	vt100_avo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_vt100_avo_interface(mconfig, *this)
		, m_rom(*this, "rom")
	{
	}

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	u8 attr_r(offs_t offset) { return m_attr[offset] | 0xf0; }   // 4 bits wide; D4-D7 undriven, assumed to read high
	void attr_w(offs_t offset, u8 data) { m_attr[offset] = data & 0x0f; }

	optional_region_ptr<u8> m_rom;      // 2K sockets A-D (E21, E17, E13, E8); D unmapped under W6

	u8 m_screen[0x400];
	u8 m_attr[0x1000];
};

class vt100_avo_ac_device : public vt100_avo_device
{
public:
	vt100_avo_ac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: vt100_avo_device(mconfig, VT100_AVO_AC, tag, owner, clock)
	{
	}

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


ROM_START( vt100_avo_ac )
	ROM_REGION(0x2000, "rom", ROMREGION_ERASEFF)
	// VT1XX-AC printer port kit ROMs; table 5-9 also lists 23-284E2 for A and
	// 23-185E2 for B.  On these masks pin 18 is CE, pins 20 and 21 /CE1 and /CE2.
	// Same ROMs seen on a "54-13097-00 // PN2280402L" AVO and on a
	// "PN1030385J-F" AVO, which has a jumper in place of the switches.
	ROM_LOAD( "23-186e2.avo.e21", 0x0000, 0x0800, CRC(1592dec1) SHA1(c4b8fc9fc0514e0cd46ad2de03abe72271ce460b)) // Label: "S 8218 // C69063 // 23186E2" @E21
	ROM_LOAD( "23-187e2.avo.e17", 0x0800, 0x0800, CRC(c6d72a41) SHA1(956f9eb945a250fd05c76100b38c0ba381ab8fde)) // Label: "S 8228 // C69062 // 23187E2" @E17
ROM_END

const tiny_rom_entry *vt100_avo_ac_device::device_rom_region() const
{
	return ROM_NAME(vt100_avo_ac);
}


void vt100_avo_device::device_start()
{
	std::fill_n(m_screen, std::size(m_screen), 0);
	std::fill_n(m_attr, std::size(m_attr), 0x0f);

	save_item(NAME(m_screen));
	save_item(NAME(m_attr));

	address_space &mem = space();

	// RAM: selected through the terminal controller's decode, so it repeats
	// every 16K with the base RAM
	mem.install_ram(0x2c00, 0x2fff, 0xc000, m_screen);
	mem.install_readwrite_handler(0x3000, 0x3fff, 0, 0xc000, 0,
			read8sm_delegate(*this, FUNC(vt100_avo_device::attr_r)),
			write8sm_delegate(*this, FUNC(vt100_avo_device::attr_w)));

	// MEM DISABLE: 8000-BFFF is the AVO's, empty sockets read open bus.
	// Must follow the RAM installs: it removes their mirrors at AC00-BFFF.
	// (assumed all ones, as on vt101_mem)
	mem.nop_readwrite(0x8000, 0xbfff);
	if (m_rom.found())
		mem.install_rom(0x8000, 0x97ff, &m_rom[0]);
}

u8 vt100_avo_device::video_r(offs_t offset)
{
	if (offset < 0x1000)
		return m_screen[offset & 0x3ff];
	else if (offset < 0x2000)
		return attr_r(offset & 0xfff);
	else
		return 0xff;
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(VT100_AVO, device_vt100_avo_interface, vt100_avo_device, "vt100_avo", "DEC VT1XX-AB Advanced Video Option")
DEFINE_DEVICE_TYPE_PRIVATE(VT100_AVO_AC, device_vt100_avo_interface, vt100_avo_ac_device, "vt100_avo_ac", "DEC VT1XX-AB Advanced Video Option (VT1XX-AC ROMs)")

void vt100_avo_options(device_slot_interface &device)
{
	device.option_add("avo", VT100_AVO);
	device.option_add("avo_ac", VT100_AVO_AC);
}
