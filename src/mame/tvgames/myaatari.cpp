// license:BSD-3-Clause
// copyright-holders: David Haywood

// CPU/SoC is simply marked "3805"

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {

class myaatari_state : public driver_device
{
public:
	myaatari_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void myaatari(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map) ATTR_COLD;
};

uint32_t myaatari_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void myaatari_state::machine_start()
{
}

void myaatari_state::machine_reset()
{
	m_maincpu->set_state_int(arm7_cpu_device::ARM7_R15, 0x08000000);
}

static INPUT_PORTS_START( myaatari )
INPUT_PORTS_END


void myaatari_state::arm_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).ram();
	map(0x03000000, 0x03001fff).ram();
	map(0x08000000, 0x08ffffff).rom().region("maincpu", 0);
	map(0x18f00000, 0x18f3ffff).ram();
}

void myaatari_state::myaatari(machine_config &config)
{
	ARM9(config, m_maincpu, 72000000); // unknown ARM core
	m_maincpu->set_addrmap(AS_PROGRAM, &myaatari_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(myaatari_state::screen_update));

	SPEAKER(config, "speaker", 2).front();
}

ROM_START( myaatari )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "atariarcade_s29gl128p11tfi02_0001227e.bin", 0x000000, 0x1000000, CRC(c838563c) SHA1(7b3a76d29556f5c30679efcece50e31ae5a5d489) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( kuniotv )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "s29gl064n90tfi04.u2", 0x000000, 0x0800000, CRC(f26cd4a2) SHA1(92b7af5ecb8b58065cfa39cac77e32242383af78) )
	ROM_IGNORE(0x100)
ROM_END

} // anonymous namespace

CONS( 2021, myaatari,      0,              0,      myaatari, myaatari, myaatari_state, empty_init, "dreamGEAR", "My Arcade Atari (DGUNL-7013, Micro Player Pro)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// HDMI stick, runs the Famicom titles using an emulator
CONS( 2021, kuniotv,       0,              0,      myaatari, myaatari, myaatari_state, empty_init, "Lithon", "Kunio-kun TV (5-in-1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
