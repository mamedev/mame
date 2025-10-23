// license:BSD-3-Clause
// copyright-holders: 

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {

class gw35th_state : public driver_device
{
public:
	gw35th_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void gw35th(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map) ATTR_COLD;
};

uint32_t gw35th_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void gw35th_state::machine_start()
{
}

void gw35th_state::machine_reset()
{
}

static INPUT_PORTS_START( gw35th )
INPUT_PORTS_END


void gw35th_state::arm_map(address_map &map)
{
}

void gw35th_state::gw35th(machine_config &config)
{
	ARM9(config, m_maincpu, 480000000); // STM32H780
	m_maincpu->set_addrmap(AS_PROGRAM, &gw35th_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(gw35th_state::screen_update));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( gwsmb35 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mx25u8035e.bin", 0x000000, 0x100000, CRC(e261a5bf) SHA1(4ce600f725f166e9f0dfd9e3b2f61ef95fa6383c) )
ROM_END

} // anonymous namespace

CONS( 2020, gwsmb35,      0,              0,      gw35th, gw35th, gw35th_state, empty_init, "Nintendo", "Game & Watch: Super Mario Bros. 35th Anniversary", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
