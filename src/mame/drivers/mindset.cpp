// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"

#include "screen.h"
#include "speaker.h"

class mindset_state: public driver_device
{
public:
	mindset_state(const machine_config &mconfig, device_type type, const char *tag);
	virtual ~mindset_state() = default;

	void mindset(machine_config &config);

protected:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<i8042_device> m_syscpu, m_soundcpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u16> m_vram;

	void maincpu_mem(address_map &map);
	void maincpu_io(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

mindset_state::mindset_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_syscpu(*this, "syscpu"),
	m_soundcpu(*this, "soundcpu"),
	m_screen(*this, "screen"),
	m_vram(*this, "vram")
{
}

u32 mindset_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const u32 pal[4] = { 0x000000, 0x555555, 0xaaaaaa, 0xffffff };

	for(u32 y=0; y<200; y++) {
		// Interleaved
		const u16 *src = m_vram + 40*(y >> 1) + 4096*(y & 1);
		u32 *dest = &bitmap.pix32(y);
		for(u32 x=0; x<320; x+=8) {
			u16 sv = *src++;
			*dest++ = pal[(sv >>  6) & 3];
			*dest++ = pal[(sv >>  4) & 3];
			*dest++ = pal[(sv >>  2) & 3];
			*dest++ = pal[(sv >>  0) & 3];
			*dest++ = pal[(sv >> 14) & 3];
			*dest++ = pal[(sv >> 12) & 3];
			*dest++ = pal[(sv >> 10) & 3];
			*dest++ = pal[(sv >>  8) & 3];
		}
	}
	return 0;
}


void mindset_state::maincpu_mem(address_map &map)
{
	map(0x00000, 0x07fff).ram();
	map(0xb8000, 0xbffff).ram().share("vram");
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}

void mindset_state::maincpu_io(address_map &map)
{
}

void mindset_state::mindset(machine_config &config)
{
	I80186(config, m_maincpu, 6000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mindset_state::maincpu_mem);
	m_maincpu->set_addrmap(AS_IO,      &mindset_state::maincpu_io);

	I8042(config, m_syscpu, 6000000);

	I8042(config, m_soundcpu, 6000000);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 319, 0, 199);
	m_screen->set_screen_update(FUNC(mindset_state::screen_update));
}

static INPUT_PORTS_START(mindset)
INPUT_PORTS_END

ROM_START(mindset)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD16_BYTE("1.7_lo.u60", 0, 0x4000, CRC(00474dc1) SHA1(676f30f170c14174dbff3b5cbf98d0f23472b7c4))
	ROM_LOAD16_BYTE("1.7_hi.u59", 1, 0x4000, CRC(1434af10) SHA1(39105eacdd7ddc13e449e2c32743e828bef33595))

	ROM_REGION(0x0800, "syscpu", 0)
	ROM_LOAD("253002-001.u17", 0, 0x800, CRC(69da82c9) SHA1(2f0bf5b134dc703cbc72e0c6df5b7beda1b39e70))

	ROM_REGION(0x0800, "soundcpu", 0)
	ROM_LOAD("253006-001.u16", 0, 0x800, CRC(7bea5edd) SHA1(30cdc0dedaa5246f4952df452a99ca22e3cd0636))
ROM_END

COMP( 1984, mindset, 0, 0, mindset, mindset, mindset_state, empty_init, "Mindset Corporation", "Mindset Video Production System", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)

