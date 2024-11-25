// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

   VTech InnoTV / InnoTab MAX
   'Android' based platforms

   The InnoTV outputs 1280x720, the InnoTab MAX uses the same cartridges
   (both also support games downloaded from VTech to the internal storage)

   These are NOT compatible with the original InnoTab 1/2/3, and although
   some games for the platform claims compatibility with the older platforms
   on the box, this is done through a download code rather than the cartridge
   being compatible.

   This file exists so that the Software List has a place to hook up to for
   the time being.

   InnoTV details

   Rockchip RK3168 (Main CPU / SoC)
   Rockchip RK616 ('Partner Chip for Rockchip mobile application processor'
   2x EtronTech EM6GE16EWXD-12H (RAM)
   Ricoh RC5T619 (Power Management)
   MicroSDHC 8GB card in internal slot
   Realtek RTL8188EU (Wireless)

   There don't appear to be any ROM / SPI / NAND devices onboard, so must either
   boot directly from the SD, or have some boot program internal to the SoC

   The following pinout was used for the InnoTV / InnoTab MAX cartridges

    +---------------------+
    |                     |--+
    |                     |--| I/O8
    |                     |--| I/O7
    |  +---------------+  |--| I/O6
    |  |||||||||||||||||  |--| I/O5
    |  |               |  |--| ?
    |  |     NAND      |  |--| I/O1
    |  |               |  |--| I/O2
    |  |               |  |--| I/O3
    |  |               |  |--| I/O4
    |  |TC58NVG1S3HTA00|  |--| GND
    |  |               |  |--| GND
    |  |               |  |--| CLE
    |  |               |  |--| ALE
    |  |               |  |--| WE
    |  |               |  |--| WP
    |  |               |  |--| VCC
    |  |||||||||||||||||  |--| VCC
    |  +---------------+  |--| CE
    |                     |--| RE
    |                     |--| RY/BY
    |                     |--+
    +---------------------+

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "screen.h"


namespace {

class vtech_innotv_innotabmax_state : public driver_device
{
public:
	vtech_innotv_innotabmax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_cart_region(nullptr)
	{ }

	void vtech_innotv_innotabmax(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;
	memory_region *m_cart_region;
};



void vtech_innotv_innotabmax_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void vtech_innotv_innotabmax_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(vtech_innotv_innotabmax_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( vtech_innotv_innotabmax )
INPUT_PORTS_END

uint32_t vtech_innotv_innotabmax_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vtech_innotv_innotabmax_state::main_map(address_map &map)
{
}

void vtech_innotv_innotabmax_state::vtech_innotv_innotabmax(machine_config &config)
{
	ARM9(config, m_maincpu, 240000000); // unknown core type / frequency, but confirmed as ARM based
	m_maincpu->set_addrmap(AS_PROGRAM, &vtech_innotv_innotabmax_state::main_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(10));
	m_screen->set_size(1280, 720);
	m_screen->set_visarea(0, 1280-1, 0, 720-1);
	m_screen->set_screen_update(FUNC(vtech_innotv_innotabmax_state::screen_update));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "vtech_innotv_innotabmax_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(vtech_innotv_innotabmax_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("vtech_innotv_cart");
}

ROM_START( innotv )
	DISK_REGION( "internalsd" )
	DISK_IMAGE( "8gb_sdhc_internal", 0, SHA1(443a0a9cc830387317d3218955b72295ee5a88eb) )
ROM_END

} // anonymous namespace


CONS( 2015, innotv,      0,         0,      vtech_innotv_innotabmax,    vtech_innotv_innotabmax,  vtech_innotv_innotabmax_state,  empty_init, "VTech", "InnoTV", MACHINE_IS_SKELETON )
