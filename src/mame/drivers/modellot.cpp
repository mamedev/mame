// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************

General Processor Modello T

2012-12-10 Skeleton driver.
2013-09-27 Added keyboard and cursor.

Made in Italy, a single board with numerous small daughter boards.
The 3 units (keyboard, disk drives, main unit) had wooden cabinets.
It had an inbuilt small green-screen CRT, like a Kaypro, and the RAM could
be 16, 32, or 48k. The FDC is a FD1791.

All the articles and doco (what there is of it) is all in Italian.

Doco found...

Port 77 out (cassette control):
- d0 = recording signal #1
- d1 = relay #1 (0 = open)
- d2 = recording signal #2
- d3 = relay #2 (0 = open)

Port 77 in:
- d0 = free
- d1 = playback signal
- d2 = signal from the anti-glare circuit
- d3 = same as d2

Optional ports:
- 3c to 3f (FDC)
- 5c to 5f (PRT)
- 6c to 6f (US2)
- 78 to 7b (US1)
It's not clear if these are meant to be 3881 PIOs connected to the devices, or for
the devices themselves. An example shows a i8251 used as the US1 device.

There's a rom missing EC00-EFFF, it is used if you try to save to tape.
All input must in UPPER case.

***********************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/keyboard.h"

#include "emupal.h"
#include "screen.h"


namespace {

class modellot_state : public driver_device
{
public:
	modellot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void modellot(machine_config &config);

private:
	u8 port77_r();
	u8 portff_r();
	void kbd_put(u8 data);
	u32 screen_update_modellot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	u8 m_term_data = 0U;
	void machine_start() override;
	void machine_reset() override;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};

void modellot_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xbfff).ram().share("mainram"); // 48k ram
	map(0xc000, 0xc3ff).ram().share("videoram");
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}

void modellot_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x77, 0x77).r(FUNC(modellot_state::port77_r));
	map(0xff, 0xff).r(FUNC(modellot_state::portff_r));
}


/* Input ports */
static INPUT_PORTS_START( modellot )
INPUT_PORTS_END

u8 modellot_state::port77_r()
{
	return 4;
}

u8 modellot_state::portff_r()
{
	u8 data = (m_term_data) ? (m_term_data ^ 0x7f) : 0xff;
	m_term_data = 0;
	return data;
}

void modellot_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void modellot_state::machine_reset()
{
	m_term_data = 1;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xe000, 0xe7ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void modellot_state::machine_start()
{
	save_item(NAME(m_term_data));
}

const gfx_layout charlayout =
{
	8, 16,              /* 8x16 characters */
	128,                /* 128 characters */
	1,              /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0,1,2,3,4,5,6,7},
	{0, 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8,
		0x400*8, 0x401*8, 0x402*8, 0x403*8, 0x404*8, 0x405*8, 0x406*8, 0x407*8},
	8*8             /* space between characters */
};

static GFXDECODE_START( gfx_modellot )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END


u32 modellot_state::screen_update_modellot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 16; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = 0; x < 64; x++)
			{
				u8 inv = 0;

				u8 chr = m_p_videoram[x+ma];

				if (BIT(chr, 7)) inv = 0xff;

				chr &= 0x7f; // cursor

				u8 gfx;
				if (ra < 8)
					gfx = m_p_chargen[(chr<<3) | ra ];
				else
					gfx = m_p_chargen[(chr<<3) | (ra-8) | 0x400];

				gfx ^= inv;

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}

void modellot_state::modellot(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &modellot_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &modellot_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 16*16);
	screen.set_visarea(0, 64*8-1, 0, 16*16-1);
	screen.set_screen_update(FUNC(modellot_state::screen_update_modellot));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_modellot);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* Devices */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(modellot_state::kbd_put));
}

/* ROM definition */
ROM_START( modellot )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
	//ROM_LOAD( "fdc8119.u3", 0x0000, 0x0400, CRC(a8aee944) SHA1(f2cc598ed2e7a1a620e2f3f53c1a573965f6af26))
	ROM_LOAD( "dt49-48.u1", 0x0000, 0x0400, CRC(2441c438) SHA1(832994a4214a744b7e19e5f74000c95ae65e3759))
	ROM_LOAD( "ht20.u2",    0x0400, 0x0400, CRC(497c0495) SHA1(d03beebc4c31284729f6eac3bdf1fbf44adf7fff))

	ROM_REGION( 0x0800, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "gcem1.u3", 0x0000, 0x0200, CRC(e7739268) SHA1(091ef69282abe657d5f38c70a572964f5200a1d5))
	ROM_CONTINUE(0x400, 0x200)
	ROM_LOAD( "gcem2.u4", 0x0200, 0x0200, CRC(6614330e) SHA1(880a541fb0ef6f37ac89439f9ea75a313c3e53d6))
	ROM_CONTINUE(0x600, 0x200)
ROM_END

} // anonymous namespace

/* Driver */
COMP( 1979, modellot, 0, 0, modellot, modellot, modellot_state, empty_init, "General Processor", "Modello T", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )

