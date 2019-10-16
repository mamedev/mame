// license:BSD-3-Clause
// copyright-holders: Dirk Best
/****************************************************************************

    Termtek TK-635 terminal

    Hardware:
    - AMD N80C188-25
    - TERMTEK TKA-200
	- 128k + 32k RAM
    - 256 KB flash memory

    Features:
    - 31.5khz or 48.1khz horizontal
	- 70/72 hz vertical
    - 16 background/foreground/border colors
    - 24x80/132, 25x80/132, 42x80/132, 43x80/132
	- Standard PC/AT keyboard

	Notes:
	- Identical to the Qume QVT-72 Plus?
	- Character shape defintions start at 0x20000 in the ROM

	TODO:
	- Everything

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "emupal.h"
#include "screen.h"

class tk635_state : public driver_device
{
public:
	tk635_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void tk635(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;

	void mem_map(address_map &map);
	void io_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

void tk635_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0xc0000, 0xfffff).rom().region("maincpu", 0);
}

void tk635_state::io_map(address_map &map)
{
}

static INPUT_PORTS_START( tk635 )
INPUT_PORTS_END

uint32_t tk635_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tk635_state::machine_start()
{
}

void tk635_state::machine_reset()
{
}

void tk635_state::tk635(machine_config &config)
{
	I80188(config, m_maincpu, 25000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tk635_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &tk635_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(70);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_screen_update(FUNC(tk635_state::screen_update));
	screen.set_size(800, 600);
	screen.set_visarea(0, 800-1, 0, 600-1);

	PALETTE(config, "palette").set_entries(16);
}

ROM_START( tk635 )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD("fw_v0_23.bin", 0x00000, 0x40000, CRC(bec6fdae) SHA1(37dc46f6b761d874bd1627a1137bc4082e364698))
ROM_END

COMP( 199?, tk635, 0, 0, tk635, tk635, tk635_state, empty_init, "Termtek", "TK-635", MACHINE_IS_SKELETON )
