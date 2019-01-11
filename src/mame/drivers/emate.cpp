// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Apple eMate 300 skeleton driver

    CPU: ARM 710a (32 bit RISC Processor)
    Memory: 1Mbyte of RAM, 2Mbytes of flash ROM
    Graphics: 480x320 16-level grayscale LCD with backlight
    Sound: Unknown
    Input: Keyboard, touchscreen
    Other: 2 serial ports, 1 IR port, 1 PCMCIA slot

****************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/ram.h"
#include "emupal.h"
#include "screen.h"

class emate_state : public driver_device
{
public:
	emate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
	{ }

	void emate(machine_config &config);

protected:
	void palette_init(palette_device &palette);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<arm710a_cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};

void emate_state::mem_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( emate )
INPUT_PORTS_END

void emate_state::palette_init(palette_device &palette)
{
	for (int i = 0; i < 16; i++)
	{
		const uint8_t gray = (i << 4) | i;
		palette.set_pen_color(15 - i, rgb_t(gray, gray, gray));
	}
}

uint32_t emate_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0);
	return 0;
}

void emate_state::emate(machine_config &config)
{
	ARM710A(config, m_maincpu, XTAL(25'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &emate_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(480, 320);
	screen.set_visarea(0, 480-1, 0, 320-1);
	screen.set_screen_update(FUNC(emate_state::screen_update));

	PALETTE(config, "palette", FUNC(emate_state::palette_init), 16);

	RAM(config, m_ram);
	m_ram->set_default_size("2M");
}

/* ROM definition */
ROM_START( emate )
	ROM_REGION32_LE( 0x800000, "maincpu", 0 )
	ROM_LOAD( "ematev22.bin", 0x000000, 0x800000, CRC(b16a453c) SHA1(f21006fa0d7350b743acffdfe26a151778f87b3b) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY           FULLNAME     FLAGS
CONS( 1997, emate, 0,      0,      emate,   emate, emate_state, empty_init, "Apple Computer", "eMate 300", MACHINE_IS_SKELETON )
