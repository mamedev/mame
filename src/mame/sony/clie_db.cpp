// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Sony Clie PDAs running on Motorola Dragonball CPU.
Later series ran on ARM based SoCs and should go in a separate driver.

Sony Clie PEG T650C main components (PCB YSX-1230 MP-43):
- Motorola Super VZ DragonBall MC68SZ328AVH66
- MediaQ MQ1100-CBC
- Sony CXD3523AGG
- Mosel-Vitelic V54C3128164VAT7
- Sony CXD1859GA
*/

#include "emu.h"

#include "machine/mc68328.h"

#include "screen.h"
#include "speaker.h"


namespace {

class clie_db_state : public driver_device
{
public:
	clie_db_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void t650c(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


uint32_t clie_db_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void clie_db_state::program_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom();
}


static INPUT_PORTS_START( t650c )
INPUT_PORTS_END

void clie_db_state::t650c(machine_config &config)
{
	MC68EZ328(config, m_maincpu, 66'000'000); // unknown clock, 66 MHz according to flyer
	m_maincpu->set_addrmap(AS_PROGRAM, &clie_db_state::program_map);

	screen_device&screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(320, 320); // 320 x 320 according to flyer
	screen.set_visarea(0, 320 - 1, 0, 320 - 1);
	screen.set_screen_update(FUNC(clie_db_state::screen_update));

	// TODO: MediaQ MQ1100-CB LCD controller

	SPEAKER(config, "speaker").front_center();
	// TODO: CXD1859GA audio chip
}


ROM_START( t650c )
	ROM_REGION16_BE( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "sony_clie_peg_t650c_flashrom.bin", 0x000000, 0x800000, CRC(60855a64) SHA1(e08350e64438c62401041aaa335def08aa0decb7) ) // verified factory defaulted image
ROM_END

} // anonymous namespace


SYST( 2002, t650c, 0, 0, t650c, t650c, clie_db_state, empty_init, "Sony", "Clie PEG-T650C", MACHINE_IS_SKELETON )
