// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    https://elinux.org/Pollux

    "The Pollux is a System on a Chip (SoC) that was manufactured by MagicEyes,
    whose intellectual property is now owned by Core Logic. Originally designed
    for LeapFrog as the LF1000, the 533MHz ARM926EJ core VR3520F has now made
    its way into several products running WinCE and Linux."

    (there are also links to datasheets etc.)

    used by
    Leapfrog Didj (as LF1000)
    GP2X Wiz (as VR3520F)
    GP2X Caanoo (as VR3520F)
     + more?

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/arm7.h"

#include "softlist_dev.h"
#include "speaker.h"
#include "screen.h"


namespace {

class magiceyes_vr3520f_game_state : public driver_device
{
public:
	magiceyes_vr3520f_game_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "arm9")
		, m_cart(*this, "cartslot")
	{ }

	void leapfrog_didj(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void didj_arm9_map(address_map &map) ATTR_COLD;

	required_device<arm9_cpu_device> m_maincpu;
	optional_device<generic_slot_device> m_cart;
};

void magiceyes_vr3520f_game_state::didj_arm9_map(address_map &map)
{
}

void magiceyes_vr3520f_game_state::machine_start()
{
}

void magiceyes_vr3520f_game_state::machine_reset()
{
}

DEVICE_IMAGE_LOAD_MEMBER(magiceyes_vr3520f_game_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

static INPUT_PORTS_START( leapfrog_didj )
INPUT_PORTS_END


uint32_t magiceyes_vr3520f_game_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void magiceyes_vr3520f_game_state::leapfrog_didj(machine_config &config)
{
	ARM9(config, m_maincpu, 533000000); // 533 MHz ARM926TEJ (clocked at @ 393 MHz for the Didj?)
	m_maincpu->set_addrmap(AS_PROGRAM, &magiceyes_vr3520f_game_state::didj_arm9_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(magiceyes_vr3520f_game_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "leapfrog_didj_cart");
	m_cart->set_width(GENERIC_ROM16_WIDTH);
	m_cart->set_device_load(FUNC(magiceyes_vr3520f_game_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("leapfrog_didj_cart");
}

ROM_START( didj )
	ROM_REGION32_BE( 0x10800000, "bios", 0 ) // external NAND
	ROM_LOAD16_WORD_SWAP( "didj_29f2g08aac_2cda.bin", 0x000000, 0x10800000, CRC(3df8c3ee) SHA1(6dc4044be10da48b6dd37e40f8a112fc4314c87d) )

	// is there an internal bootloader beyond copying code from NAND into RAM?
ROM_END

} // anonymous namespace


CONS( 2008, didj,      0,       0,      leapfrog_didj, leapfrog_didj, magiceyes_vr3520f_game_state, empty_init, "LeapFrog", "Didj", MACHINE_IS_SKELETON )
