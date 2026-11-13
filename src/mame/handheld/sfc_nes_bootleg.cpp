// license:BSD-3-Clause
// copyright-holders:

// SFC-style shell, with bootleg NES games
// seems to be emulation based as there's an ARM ROM

// main SoC is marked MPK3B18.00 1852
// PCB is marked SFC-36-H4 20181116 on front
//               LCX-136P2N4502B1 on back

#include "emu.h"

#include "screen.h"
#include "speaker.h"

#include "cpu/arm7/arm7.h"

namespace {

class sfc_nes_bootleg : public driver_device
{
public:
	sfc_nes_bootleg(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{
	}

	void sfcnes(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};

u32 sfc_nes_bootleg::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void sfc_nes_bootleg::machine_start()
{
}

static INPUT_PORTS_START(sfcnes)
INPUT_PORTS_END


void sfc_nes_bootleg::arm_map(address_map &map)
{
}

void sfc_nes_bootleg::sfcnes(machine_config &config)
{
	ARM7(config, m_maincpu, 100000000); // unknown ARM
	m_maincpu->set_addrmap(AS_PROGRAM, &sfc_nes_bootleg::arm_map);

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(640, 480);
	m_screen->set_visarea(0, 640 - 1, 0, 480 - 1);
	m_screen->set_screen_update(FUNC(sfc_nes_bootleg::screen_update));

	SPEAKER(config, "speaker").front_center();
}

ROM_START( sfcnes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "at25f512.u6", 0x000000, 0x10000, CRC(5d10a597) SHA1(37d7d43b86d66507264b28bd3f26e06a62384041) )

	ROM_REGION( 0x4000000, "maincpu2", 0 )
	ROM_LOAD( "s29gl512n11tfi02.u3", 0x000000, 0x4000000, CRC(206405a4) SHA1(f3280ef5b5dc95d222c856ce907293e18498a77d) )
ROM_END

} // anonymous namespace

// there are 600, 620 and 621 game versions at least, unsure which this is
GAME( 2018, sfcnes, 0, sfcnes, sfcnes, sfc_nes_bootleg, empty_init, ROT0, "<unknown>", "Super Mini SFC (NES/Famicom bootleg)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
