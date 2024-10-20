// license:BSD-3-Clause
// copyright-holders:David Haywood
/*******************************************************************************

 __________________________________IR___
 |                          ____  RX/TX |
 |   _____      ________    |   |    __ |
 |   |    |     |       |   |   |    | ||
 |   |IC3 |     | IC2   |   |   |CN2->_||
 |   |    |     |       |   |   |       |
 |   |____|     |_______|   |   |       |
 |   ____                   |   |       |
 |   |   |       CART SLOT->|   |       |
 |   |IC5|                  |   |       |
 |   |___|      ________    |   |       |
 |   ____       |       |   |   |       |
 |   |   |      | IC1   |   |   |       |
 |   |IC4|      |       |   |   |       |
 |   |___|      |_______|   |___|       |
 |                                      |
 | SW2                                  |
 |______________________________________|

 IC1 = Hitachi H8/328 (24K-byte internal ROM + 1K-byte RAM)
 IC2 = Hitachi HG62G010R21FBN Gate Array (low gatecount and low I/O-count packages)
 IC3 = Hitachi HM62256LFP-10T 256kbit CMOS SRAM
 IC4 = BA10324AF Ground Sense Operational Amplifier
 IC5 = Hitachi 74HC00 (5B2T HC00)


 TODO:
 - cartridge pinouts / information
 - needs H8/329 family emulation

 NOTE: cartridge dumps contain boot vectors so Internal ROM likely only used when no cartridge is present

*******************************************************************************/

#include "emu.h"
#include "cpu/h8/h83337.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "screen.h"
#include "softlist_dev.h"
//#include "speaker.h"


namespace {

class bdsm_state : public driver_device
{
public:
	bdsm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cartslot(*this, "cartslot"),
		m_cartslot_region(nullptr),
		m_bank(*this, "cartbank"),
		m_screen(*this, "screen")
	{ }

	void bdesignm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	uint16_t io_p7_r();

	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	[[maybe_unused]] DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load_bdesignm);

	required_device<h83334_device> m_maincpu;
	required_device<generic_slot_device> m_cartslot;
	memory_region *m_cartslot_region;
	required_memory_bank m_bank;
	required_device<screen_device> m_screen;
};

void bdsm_state::machine_start()
{
	if (m_cartslot && m_cartslot->exists())
	{
		std::string region_tag;
		m_cartslot_region = memregion(region_tag.assign(m_cartslot->tag()).append(GENERIC_ROM_REGION_TAG).c_str());
		m_bank->configure_entries(0, (m_cartslot_region->bytes() / 0x8000), m_cartslot_region->base(), 0x8000);

		// Only the first bank seems to contain a valid reset vector '0x50' which points at the first code in the ROM.
		// The other banks contain 0x5a00 as the reset vector.  IRQ vector seems valid in all banks.
		m_bank->set_entry(0);
	}
	else
		m_bank->set_base(memregion("roms")->base());
}

DEVICE_IMAGE_LOAD_MEMBER(bdsm_state::cart_load_bdesignm)
{
	uint32_t size = m_cartslot->common_get_size("rom");

	m_cartslot->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
	m_cartslot->common_load_rom(m_cartslot->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void bdsm_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("cartbank");
	map(0x8000, 0x895f).ram().share("unkram");
}

uint16_t bdsm_state::io_p7_r()
{
	return machine().rand();
}

static INPUT_PORTS_START( bdesignm )
INPUT_PORTS_END

uint32_t bdsm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void bdsm_state::bdesignm(machine_config &config)
{
	// basic machine hardware
	H83334(config, m_maincpu, 16_MHz_XTAL / 2); // H8/328 (24kbytes internal ROM, 1kbyte internal ROM)
	m_maincpu->set_addrmap(AS_PROGRAM, &bdsm_state::mem_map);
	m_maincpu->read_port7().set(FUNC(bdsm_state::io_p7_r));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(160, 150); // resolution unknown
	m_screen->set_visarea(0, 160-1, 0, 150-1);
	m_screen->set_screen_update(FUNC(bdsm_state::screen_update));
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));

	// TODO: this should be a custom bus type with capability to plug the 'design' carts into it
	GENERIC_CARTSLOT(config, m_cartslot, generic_linear_slot, "bdesignm_cart");
	//m_cartslot->set_must_be_loaded(true);

	// Game carts, these appear to disable the Internal ROM
	SOFTWARE_LIST(config, "cart_list_game").set_original("bdesignm_game_cart");

	// You can also plug a design cart directly into the unit for use by the Internal ROM program (they contain no program)
	SOFTWARE_LIST(config, "cart_list_design").set_original("bdesignm_design_cart");
}

ROM_START( bdesignm )
	ROM_REGION16_BE(0x88000, "roms", ROMREGION_ERASE00)
	// Internal ROM (When the console is booted up without a cart it enters the default (builtin) art / drawing program,
	// otherwise probably not used as carts contain boot vectors etc.)
	ROM_LOAD( "h8_328_hd6433288f8_l04.ic1", 0x00000, 0x6000, CRC(2c6b8fb0) SHA1(b958b0bc27f18b7dda4fe852b3fd070a66586edb) )
ROM_END

} // anonymous namespace


CONS( 1995, bdesignm, 0, 0, bdesignm, bdesignm, bdsm_state, empty_init, "Bandai", "Design Master Denshi Mangajuku", MACHINE_IS_SKELETON )
