// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/m6502/st2204.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class bzyasmin_state : public driver_device
{
public:
	bzyasmin_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void bzyasmin(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<st2204_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	uint32_t screen_update_bzyasmin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void bzyasmin_mem(address_map &map) ATTR_COLD;
};


void bzyasmin_state::bzyasmin_mem(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x800000, 0x80ffff).ram();

}

static INPUT_PORTS_START( bzyasmin )
INPUT_PORTS_END

uint32_t bzyasmin_state::screen_update_bzyasmin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void bzyasmin_state::machine_start()
{
}


void bzyasmin_state::bzyasmin(machine_config &config)
{
	// all guessed / probably wrong
	ST2204(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_DATA, &bzyasmin_state::bzyasmin_mem);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(48, 32);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(bzyasmin_state::screen_update_bzyasmin));
	m_screen->set_palette(m_palette);

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x200);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, "dac", 0).add_route(0, "speaker", 1.0);
}

ROM_START(bzyasmin)
	// is there an internal ROM? it ends up jumping to 0x4000 with no code
	// could be wrong SoC?

	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD("sst39vf1681.u2", 0x00000, 0x200000, CRC(32dd95b0) SHA1(00c954fecbc569c9fd0615a10c19a943f7fba10e) )
ROM_END

} // anonymous namespace

CONS( 2006, bzyasmin, 0, 0, bzyasmin, bzyasmin,  bzyasmin_state, empty_init, "MGA", "Miuchiz Bratz Yasmin", MACHINE_NOT_WORKING )
