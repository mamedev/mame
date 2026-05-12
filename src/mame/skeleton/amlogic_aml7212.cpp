// license:BSD-3-Clause
// copyright-holders:David Haywood

// has an @mlogic AML7212, unknown CPU core

#include "emu.h"

#include "screen.h"
#include "speaker.h"

namespace {

class amlogic_aml7212_popstarz_state : public driver_device
{
public:
	amlogic_aml7212_popstarz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
	{ }

	void popstarz(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
};

uint32_t amlogic_aml7212_popstarz_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void amlogic_aml7212_popstarz_state::machine_start()
{
}

void amlogic_aml7212_popstarz_state::machine_reset()
{
}

static INPUT_PORTS_START( popstarz )
INPUT_PORTS_END

void amlogic_aml7212_popstarz_state::popstarz(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 256); // unknown resolution
	m_screen->set_visarea(0, 320-1, 0, 255-1);
	m_screen->set_screen_update(FUNC(amlogic_aml7212_popstarz_state::screen_update));

	SPEAKER(config, "mono").front_center();
}

ROM_START( popstarz )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	// contains compressed(?) code/data
	ROM_LOAD( "kh29lv160cb.ic903", 0x000000, 0x200000, CRC(95c79782) SHA1(eda0dda4ee0f108f8259e7805505a4e5d29dc5d7) )

	ROM_REGION( 0x42000000, "nand", ROMREGION_ERASEFF )
	ROM_LOAD( "k9g8g08u0a_tsop48.ic902", 0x000000, 0x42000000, CRC(4a0a22cf) SHA1(eba69c5432c6b730e9ab50b196bedc1928890f5f) )
ROM_END

} // anonymous namespace

CONS( 201?, popstarz,       0,              0,      popstarz,  popstarz, amlogic_aml7212_popstarz_state, empty_init, "<unknown>", "Popstarz Karaoke (GK8000)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
