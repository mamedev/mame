// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    basic information
    https://gbatemp.net/threads/the-c2-color-game-console-an-obscure-chinese-handheld.509320/

    "The C2 is a glorious console with a D-Pad, Local 2.4GHz WiFi, Cartridge slot, A, B, and C buttons,
     and has micro usb power! Don't be fooled though, there is no lithium battery, so you have to put in
     3 AA batteries if you don't want to play with it tethered to a charger.

     It comes with a built in game based on the roco kingdom characters.

     In addition, there is a slot on the side of the console allowing cards to be swiped through. Those
     cards can add characters to the game. The console scans the barcode and a new character or item appears in the game for you to use.

     The C2 comes with 9 holographic game cards that will melt your eyes."

    also includes a link to the following video
    https://www.youtube.com/watch?v=D3XO4aTZEko

    TODO:
    identify CPU type - It's an i8051 derived CPU, and seems to be "Mars Semiconductor Corp" related, there is a MARS-PCCAM string
                        amongst other things, this is a known USB identifier for the "Discovery Kids Digital Camera"
                        Possibly a MR97327B, which is listed as RISC-51 in places, but little information can be found in English


*******************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "emupal.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class c2_color_state : public driver_device
{
public:
	c2_color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
	{ }

	void c2_color(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	u8 cart_r(offs_t offset);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

uint32_t c2_color_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}




void c2_color_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		std::string region_tag;
		m_cart_region = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
	}
}

void c2_color_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(c2_color_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

u8 c2_color_state::cart_r(offs_t offset)
{
	// skip past 32-byte header
	return m_cart->read_rom(offset + 32);
}

void c2_color_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(c2_color_state::cart_r));
}

void c2_color_state::ext_map(address_map &map)
{
	map(0x2400, 0x2400).nopr();
}

static INPUT_PORTS_START( c2_color )
INPUT_PORTS_END

void c2_color_state::c2_color(machine_config &config)
{
	I8032(config, m_maincpu, 12'000'000); // exact type and clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &c2_color_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &c2_color_state::ext_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(c2_color_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "c2color_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(c2_color_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("c2color_cart");
}

ROM_START( c2color )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bootloader", 0x0000, 0x4000, NO_DUMP )

	// As with the cartridges, each of these has a 0x20 byte header before the i8051
	// code starts.  This suggests it is unlikely the game runs directly from the SPI
	// ROM and more likely a bootloader copies the code into RAM.
	// The Mainboard has a 2MByte DRAM on it

	// This, the larger of the 2 ROMs contains unique code, it appears to be the base
	// game, and system functions.  It also has some 16-bit signed PCM samples.
	ROM_REGION( 0x800000, "spi1", ROMREGION_ERASEFF )
	ROM_LOAD( "spi.u7", 0x000000, 0x800000, CRC(6a4d2cd2) SHA1(46e109bbd5db206911716919ad13efc080cbdf34) )

	// The smaller ROM is much more similar to the cartridges (actually identical up
	// until the first MRDB resource block at 0x26000 aside from a few bytes in the
	// 0x20 header, and the game number at the 0x20000 mark being 0)
	//
	// This ROM also has a 2nd MRDB resource block, whereas the cartridges only have
	// a single block
	//
	// The code still contains a lot of generic 'firmware' like functions, but it is
	// unclear if any of the code is used, or if these ROMs are used more like skins
	// for the base game, accessing the resource table only
	//
	// A large number of graphics, which are assumed to be JPEG compressed are indexed
	// by the MRDB tables, no sound effects or non-graphical resources have been
	// identified

	ROM_REGION( 0x400000, "spi2", ROMREGION_ERASEFF )
	ROM_LOAD( "spi.u16", 0x000000, 0x400000, CRC(9101b02a) SHA1(8c31e7641f4667bd8d5d7cc991cd5976828a0628) )
ROM_END

} // anonymous namespace


//    year, name,         parent,  compat, machine,      input,        class,              init,       company,  fullname,                             flags
CONS( 201?, c2color,      0,       0,      c2_color,   c2_color, c2_color_state, empty_init, "Baiyi Animation", "C2 Color (China)", MACHINE_IS_SKELETON )
