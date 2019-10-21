// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Schleicher MES

2010-08-30 Skeleton driver

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/keyboard.h"
#include "emupal.h"
#include "screen.h"


class mes_state : public driver_device
{
public:
	mes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void mes(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void kbd_put(u8 data);
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port08_r);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	u8 m_term_data;
	u8 m_port08;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};


READ8_MEMBER( mes_state::port00_r )
{
	u8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( mes_state::port08_r )
{
	return m_port08 | (m_term_data ? 0x80 : 0);
}

void mes_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("roms", 0);
	map(0x1000, 0xefff).ram();
	map(0xf000, 0xffff).ram().share("videoram");
}

void mes_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(mes_state::port00_r));
	map(0x08, 0x08).r(FUNC(mes_state::port08_r));
	map(0x0c, 0x0f).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x18, 0x1b).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

/* Input ports */
static INPUT_PORTS_START( mes )
INPUT_PORTS_END

void mes_state::machine_reset()
{
	m_port08 = 0;
	m_term_data = 0;
}

/* This system appears to have 2 screens. Not implemented.
    Also the screen dimensions are a guess. */
uint32_t mes_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[x];
					gfx = m_p_chargen[(chr<<4) | ra ];
				}

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
		ma+=80;
	}
	return 0;
}

void mes_state::kbd_put(u8 data)
{
	m_term_data = data;
}

void mes_state::mes(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(16'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &mes_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mes_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(mes_state::screen_update));
	screen.set_size(640, 250);
	screen.set_visarea(0, 639, 0, 249);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	Z80CTC(config, "ctc", 0);
	Z80PIO(config, "pio", 0);
	Z80SIO(config, "sio", 0);

	generic_keyboard_device &keybd(GENERIC_KEYBOARD(config, "keybd", 0));
	keybd.set_keyboard_callback(FUNC(mes_state::kbd_put));
}


/* ROM definition */
ROM_START( mes )
	ROM_REGION( 0x1000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "mescpu.bin",   0x0000, 0x1000, CRC(b6d90cf4) SHA1(19e608af5bdaabb00a134e1106b151b00e2a0b04))

	ROM_REGION( 0x2000, "xebec", ROMREGION_ERASEFF )
	ROM_LOAD( "mesxebec.bin", 0x0000, 0x2000, CRC(061b7212) SHA1(c5d600116fb7563c69ebd909eb9613269b2ada0f))

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

/* Driver */

//   YEAR   NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY       FULLNAME  FLAGS
COMP( 198?, mes,  0,      0,      mes,     mes,   mes_state, empty_init, "Schleicher", "MES",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
