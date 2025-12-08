// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

main SoC is marked

SONIX
SN73231M1N-000
215EATB1e^^e3  (^^ are some kind of graphic)

Dual-core ARM Cortex-M3 SoC (Sonix SNC7320 series)

other sources mention that the Tamagotchi Pix uses a GeneralPlus GP32 (ARM)
series CPU, so are there multiple hardware revisions or is that information
incorrect?

device has a camera

*******************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {

class tamagotchi_pix_state : public driver_device
{
public:
	tamagotchi_pix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void tamapix(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
};

uint32_t tamagotchi_pix_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tamagotchi_pix_state::mem_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rom().region("maincpu", 0);
}

void tamagotchi_pix_state::machine_start()
{
}

void tamagotchi_pix_state::machine_reset()
{
}

static INPUT_PORTS_START( tamapix )
INPUT_PORTS_END

void tamagotchi_pix_state::tamapix(machine_config &config)
{
	ARM9(config, m_maincpu, 12'000'000); // not correct, needs a core with Thumb-2 support
	m_maincpu->set_addrmap(AS_PROGRAM, &tamagotchi_pix_state::mem_map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 256); // unknown resolution
	m_screen->set_visarea(0, 255-1, 0, 255-1);
	m_screen->set_screen_update(FUNC(tamagotchi_pix_state::screen_update));

	SPEAKER(config, "mono").front_center();
}

ROM_START( tamapix )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	// this is an SPI ROM so there's probably an internal bootstrap at least
	ROM_LOAD( "25q64.u5", 0x000000, 0x800000, CRC(559d0cc8) SHA1(bd5510a38cd4b293bc89bced99718d2998c5b893) )
ROM_END

} // anonymous namespace

CONS( 2020, tamapix,       0,              0,      tamapix, tamapix, tamagotchi_pix_state, empty_init, "Bandai", "Tamagotchi Pix", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
