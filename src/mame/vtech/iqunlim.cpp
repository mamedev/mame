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
#include "machine/mc68328.h"
#include "machine/ram.h"
#include "video/mc68328lcd.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

#include "screen.h"
#include "softlist_dev.h"

#define LOG_CARD_A_READ     (1U << 1)
#define LOG_CARD_A_WRITE    (1U << 2)
#define LOG_CARD_B_READ     (1U << 3)
#define LOG_CARD_B_WRITE    (1U << 4)
#define LOG_ALL             (LOG_CARD_A_READ | LOG_CARD_A_WRITE | LOG_CARD_B_READ | LOG_CARD_B_WRITE)

#define VERBOSE (LOG_ALL)
#include "logmacro.h"

namespace
{

class iqunlim_state : public driver_device
{
public:
	iqunlim_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_lcdctrl(*this, "lcdctrl"),
		m_screen(*this, "screen"),
		m_cart(*this, "cartslot")
	{ }

	void iqunlim(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void card4x_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t card4x_r(offs_t offset, uint16_t mem_mask = 0xffff);
	void card5x_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t card5x_r(offs_t offset, uint16_t mem_mask = 0xffff);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<mc68ez328_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<mc68328_lcd_device> m_lcdctrl;
	required_device<screen_device> m_screen;
	required_device<generic_slot_device> m_cart;
};

void iqunlim_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0x00000000, m_ram->size() - 1, m_ram->pointer());
}

void iqunlim_state::machine_reset()
{
	// Copy ROM vectors into RAM
	memcpy(m_ram->pointer(), memregion("ipl")->base(), 0x100);
}

void iqunlim_state::mem_map(address_map &map)
{
	map(0x02000000, 0x023fffff).rom().region("ipl", 0); // 68EZ328 /CSA0 pin selects System ROM after bootup
	map(0x03000000, 0x0307ffff).ram(); // Region used by the internal flash memory
	map(0x04000000, 0x04ffffff).rw(FUNC(iqunlim_state::card4x_r), FUNC(iqunlim_state::card4x_w)); // Region used by Card B
	map(0x05000000, 0x05ffffff).rw(FUNC(iqunlim_state::card5x_r), FUNC(iqunlim_state::card5x_w)); // Region used by Card A
}

uint32_t iqunlim_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_lcdctrl->video_update(bitmap, cliprect);
	return 0;
}

uint16_t iqunlim_state::card4x_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;
	LOGMASKED(LOG_CARD_B_READ, "card4x_read[%08x]: %04x\n", offset << 1, data);
	return data;
}

void iqunlim_state::card4x_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_CARD_B_WRITE, "card4x_write[%08x]: %04x\n", offset << 1, data);
}

uint16_t iqunlim_state::card5x_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;
	if (m_cart)
	{
		data = m_cart->read16_rom(offset, mem_mask);
	}
	LOGMASKED(LOG_CARD_A_READ, "card5x_read[%08x]: %04x\n", offset << 1, data);
	return data;
}

void iqunlim_state::card5x_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_CARD_A_WRITE, "card5x_write[%08x]: %04x\n", offset << 1, data);
}

static INPUT_PORTS_START( iqunlim )
INPUT_PORTS_END

DEVICE_IMAGE_LOAD_MEMBER(iqunlim_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}


void iqunlim_state::iqunlim(machine_config &config)
{
	// Basic machine hardware
	MC68EZ328(config, m_maincpu, 32768*506);
	m_maincpu->set_addrmap(AS_PROGRAM, &iqunlim_state::mem_map);

	m_maincpu->out_flm().set(m_lcdctrl, FUNC(mc68328_lcd_device::flm_w));
	m_maincpu->out_llp().set(m_lcdctrl, FUNC(mc68328_lcd_device::llp_w));
	m_maincpu->out_lsclk().set(m_lcdctrl, FUNC(mc68328_lcd_device::lsclk_w));
	m_maincpu->out_ld().set(m_lcdctrl, FUNC(mc68328_lcd_device::ld_w));
	m_maincpu->set_lcd_info_changed(m_lcdctrl, FUNC(mc68328_lcd_device::lcd_info_changed));

	RAM(config, RAM_TAG).set_default_size("2M");

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(480, 260);
	m_screen->set_visarea(0, 480 - 1, 0, 260 - 1);
	m_screen->set_screen_update(FUNC(iqunlim_state::screen_update));

	MC68328_LCD(config, m_lcdctrl, 0);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "iqunlim_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(iqunlim_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("iqunlim_cart");
}

ROM_START( iqunlim )
	ROM_REGION16_BE(0x400000, "ipl", 0)
	ROM_LOAD16_WORD_SWAP( "27-06122-006.bin", 0x000000, 0x400000, CRC(811b1b19) SHA1(bac99ce408ed0a3b6449db88b363293b46ce69b9) )
ROM_END

ROM_START( iqunlimgr )
	ROM_REGION16_BE(0x400000, "ipl", 0)
	ROM_LOAD16_WORD_SWAP( "27-06126-007.bin", 0x000000, 0x400000, CRC(2e99cfef) SHA1(790869ffcf7fd666def8ff57fce0691062b3cec5) )
ROM_END

} // anonymous namespace

COMP( 1995, iqunlim,         0, 0, iqunlim, iqunlim, iqunlim_state, empty_init, "VTech / Integrated Systems Inc.", "IQ Unlimited",           MACHINE_IS_SKELETON) // COPYRIGHT 1995 INTERGRATED SYSTEMS, INC.
COMP( 1995, iqunlimgr, iqunlim, 0, iqunlim, iqunlim, iqunlim_state, empty_init, "VTech / Integrated Systems Inc.", "IQ Unlimited (Germany)", MACHINE_IS_SKELETON) // COPYRIGHT 1995 INTERGRATED SYSTEMS, INC.
