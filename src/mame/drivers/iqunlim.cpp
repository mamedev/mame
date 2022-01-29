// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

/*

IQ Unlimited - GERMAN:
      +------------------------------------------------------------------------------+
      |                                                                              |
+-----+                                                                              |
|                                                                                    |
|                                                                                    |
|                                                                                    |
|  +----+                                                +---+                       |
|  | A2 |                +----+                          |   |                       |
|  |    |                | A4 |                          |A1 |        +-+            |
|  +----+                +----+                          |   |        | |            |
|                                                        |   |        +-+            |
|                                                        |   |                       |
|                                                        +---+                    +--+
|                                                                                 |
|                                                       +-------+                 |
+--+                                                    |65C5L5K|            +----+
   |                                                    | HC374 |            |
+--+                         +----------+               +-------+            +--+
|                            |DragonBall|                                       |
| C         +----+           |EZ        |               +-------+             C |
| A         | A3 |           |          |               |65C5L5K|             A |
| R         +----+           |LSC414328P|               | HC374 |             R |
| T                          |U16  IJ75C|               +-------+             T |
| R                          | HHAV984S |                                     R |
| I                          +----------+                                     I |
| D  CARD 1 +------------+                                            CARD 0  D |
| G         | AM29F0400  |                                                    G |
| E         |            |     +------+                +--------+             E |
|           +------------+     | LGS  |                |LHMN5KR7|               |
| S                            |      |                |        |             S |
| L                            |GM71C1|                |  1998  |             L |
| O       GER                  |8163CJ|                |        |             O |
| T       038                  |6     |                |27-06126|             T |
|                              |      |                |-007    |               |
+--+                           |      |                |        |            +--+
   |                           |      |                |  VTECH |            |
   |                           +------+                +--------+            |
   |                                                   35-19600-200  703139-G|
   +-------------------------------------------------------------------------+

A1 = 98AHCLT / 27-05992-0-0 / VTech
A2 = 9932 HBL / C807U-1442 / 35016B / Japan
A3 = ACT139
A4 = MAX232

*/

#include "emu.h"
#include "screen.h"
#include "cpu/m68000/m68000.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist_dev.h"

class iqunlim_state : public driver_device
{
public:
	iqunlim_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cart(*this, "cartslot")
		{ }

	void iqunlim(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void iqunlim_mem(address_map &map);
};

uint32_t iqunlim_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void iqunlim_state::iqunlim_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1FFFFF).rom();
}

static INPUT_PORTS_START( iqunlim )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(iqunlim_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


void iqunlim_state::iqunlim(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, XTAL(32'000'000)/2); // DragonBall EZ (MC68EZ328, 68k core) (Is the xtal correct? This was from the other hardware)
	m_maincpu->set_addrmap(AS_PROGRAM, &iqunlim_state::iqunlim_mem);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // Not accurate
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(iqunlim_state::screen_update));

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "iqunlim_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(iqunlim_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("iqunlim_cart");
}

ROM_START( iqunlim )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "27-06122-006.bin", 0x000000, 0x400000, CRC(811b1b19) SHA1(bac99ce408ed0a3b6449db88b363293b46ce69b9) )
ROM_END

ROM_START( iqunlimgr )
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "27-06126-007.bin", 0x000000, 0x400000, CRC(2e99cfef) SHA1(790869ffcf7fd666def8ff57fce0691062b3cec5) )
ROM_END

COMP( 1995, iqunlim,         0, 0, iqunlim, iqunlim, iqunlim_state, empty_init, "VTech / Integrated Systems Inc.", "IQ Unlimited",           MACHINE_IS_SKELETON) // COPYRIGHT 1995 INTERGRATED SYSTEMS, INC.
COMP( 1995, iqunlimgr, iqunlim, 0, iqunlim, iqunlim, iqunlim_state, empty_init, "VTech / Integrated Systems Inc.", "IQ Unlimited (Germany)", MACHINE_IS_SKELETON) // COPYRIGHT 1995 INTERGRATED SYSTEMS, INC.
