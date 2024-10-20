// license:BSD-3-Clause
// copyright-holders:

/*
Sharp Wizard series of electronic organizers

Currently only the IQ-7000 is dumped (sold in the US as OZ-7000)

Other known undumped models are:
IQ-7100M
OZ-7200 WIZARD
IQ-7300M
IQ-7520M
IQ-7700M
IQ-7720M
IQ-8100M
IQ-8200
IQ-8300M
IQ-8500M
IQ-8900G
IQ-8920G
OZ-9520 WIZARD
IQ-9000G
IQ-9200G

Sharp followed up with the Zaurus series (see drivers/zaurus.cpp)

Models IQ-7000, IQ-7100M, OZ-7200 WIZARD, IQ-7300M, IQ-7520M, IQ-7700M, and IQ-7720M
use the following screen layout (96x64 square pixels + custom segments at the right):
____________________________________
| 96 x 64 pixels LCD                |
| 16 cols x 8 lines (5 x 7 chars)   | BATT
| 12 cols x 4 lines (8 x 16 chars)  | CARD
|                                   | EDIT
|                                   | SHIFT
|                                   | CAPS
|                                   | ✱ 🅂
|                                   | � 🕭
|                                   | ↑ ↓
|___________________________________| ← →

IQ-8100M, IQ-8200, IQ-8300M and IQ-8500M models use a 240x64 screen (40 cols x 8 lines with
6x8 chars or 30 cols x 4 lines with 8x16 chars).
IQ-8900G, IQ-8920G and OZ-9520 WIZARD models use a 240x160 screen.
IQ-9000G and IQ-9200G models use a 320x240 screen.

IQ-700 hardware notes:
-32 Kbytes RAM.
-32,768 Hz clock crystal oscillation frecuency (as per manual).
-Supports the Sharp CE-50P external printer.
-Supports a cassette interface for saving and loading data (connected through the CE-50P printer).
-Supports PC connection through Sharp PC-LINK software.
-Supports data communication between two Organizers.

More info:
-SHARP PC-E500 CPU Instruction Table: http://www.andrewwoods3d.com/pce500/insttabl.html

*/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/arm7.h" // wrong, needs CPU core

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "screen.h"


namespace {

class wizard_state : public driver_device
{
public:
	wizard_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
		, m_cart_region(nullptr)
	{ }

	void iq7000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};


void wizard_state::machine_start()
{
	// if there's a cart, override the standard mapping
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	}
}

void wizard_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(wizard_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( iq7000 )
INPUT_PORTS_END

uint32_t wizard_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void wizard_state::main_map(address_map &map)
{
}

void wizard_state::iq7000(machine_config &config)
{
	ARM9(config, m_maincpu, 240000000); // actually Sharp SC62015B02, currently unemulated
	m_maincpu->set_addrmap(AS_PROGRAM, &wizard_state::main_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD)); // all wrong, TBD
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(wizard_state::screen_update));
	screen.set_size(96, 64);
	screen.set_visarea_full();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "wizard_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(wizard_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("wizard_cart");
}

ROM_START( iq7000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "iq7000.bin", 0x00000, 0x10000, CRC(04ba80ca) SHA1(fe25e7c892b1e57641ff75bcd703882e28627fda) )
ROM_END

} // Anonymous namespace


CONS( 1988, iq7000, 0, 0, iq7000, iq7000, wizard_state, empty_init, "Sharp", "IQ-7000", MACHINE_IS_SKELETON )
