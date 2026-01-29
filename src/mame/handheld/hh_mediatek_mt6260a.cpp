// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "softlist_dev.h"


namespace {

class sonicwtc_state : public driver_device
{
public:
	sonicwtc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void sonicwtc(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map) ATTR_COLD;
};

uint32_t sonicwtc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void sonicwtc_state::machine_start()
{
}

void sonicwtc_state::machine_reset()
{
}

static INPUT_PORTS_START( sonicwtc )
INPUT_PORTS_END

void sonicwtc_state::arm_map(address_map &map)
{
}

void sonicwtc_state::sonicwtc(machine_config &config)
{
	ARM7(config, m_maincpu, 72000000); // Mediatek ARM MT6260AH, ARM7EJ-STM core, unknown frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &sonicwtc_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(sonicwtc_state::screen_update));
}

ROM_START( sonicwtc )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jw.bin", 0x000000, 0x1000000, CRC(1d29c63c) SHA1(7e36b351dcbb8ecf0e5e709127db29e3a4779879) )
ROM_END

} // anonymous namespace

CONS( 201?, sonicwtc, 0, 0, sonicwtc, sonicwtc, sonicwtc_state, empty_init, "Accutime", "Sonic the Hedgehog Kids Smart Watch", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

