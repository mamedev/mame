// license:BSD-3-Clause
// copyright-holders: David Haywood

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {

class vegaplus_state : public driver_device
{
public:
	vegaplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void vegaplus(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map) ATTR_COLD;
};

uint32_t vegaplus_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vegaplus_state::machine_start()
{
}

void vegaplus_state::machine_reset()
{
	//m_maincpu->set_state_int(arm7_cpu_device::ARM7_R15, 0x08000000);
}

static INPUT_PORTS_START( vegaplus )
INPUT_PORTS_END


void vegaplus_state::arm_map(address_map &map)
{
}

void vegaplus_state::vegaplus(machine_config &config)
{
	ARM9(config, m_maincpu, 454000000); // Freescale MCIMX233DJM4C
	m_maincpu->set_addrmap(AS_PROGRAM, &vegaplus_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(vegaplus_state::screen_update));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( vegaplus )
	ROM_REGION( 0x4000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "vegaplus.bin", 0x000000, 0x4000000, CRC(d557819c) SHA1(18160118e1b5538f2341213c0ded14317be6f0b3) )
ROM_END

} // anonymous namespace

CONS( 2018, vegaplus,      0,              0,      vegaplus, vegaplus, vegaplus_state, empty_init, "Retro Computers Ltd", "ZX Spectrum Vega+", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
