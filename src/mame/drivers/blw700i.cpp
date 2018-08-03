// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Brother LW-700i and friends word processor/typewriters

    Preliminary driver by R. Belmont

    Main CPU: Hitachi H8/3003
    FDC: HD63266F (uPD765 derivative)
    256KiB RAM
    Dot-matrix LCD (480x128)

****************************************************************************/

#include "emu.h"
#include "cpu/h8/h83003.h"
#include "machine/nvram.h"
#include "machine/at28c16.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"

class lw700i_state : public driver_device
{
public:
	lw700i_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_mainram(*this, "mainram"),
			m_screen(*this, "screen")
	{ }

	void lw700i(machine_config &config);

	DECLARE_READ16_MEMBER(status_r) { return 0x8080; }  // "ready"
	DECLARE_WRITE16_MEMBER(data_w) { printf("%c", data & 0x7f); }

	DECLARE_READ8_MEMBER(p7_r);
	DECLARE_READ8_MEMBER(pb_r);
	DECLARE_WRITE8_MEMBER(pb_w);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void io_map(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(vbl_interrupt);

	// devices
	required_device<h83003_device> m_maincpu;
	required_shared_ptr<uint16_t> m_mainram;
	required_device<screen_device> m_screen;

	// driver_device overrides
	virtual void video_start() override;

	uint8_t m_keyrow;
};

READ8_MEMBER(lw700i_state::p7_r)
{
	// must be non-zero; f0 = French, fe = German, ff = English
	return 0xf0;
}

READ8_MEMBER(lw700i_state::pb_r)
{
	if (m_keyrow == 0xf)    // config
	{
		return 0;
	}

	return 0xffff;
}

WRITE8_MEMBER(lw700i_state::pb_w)
{
	printf("%x to keyboard row\n", data);
	m_keyrow = data;
}

TIMER_DEVICE_CALLBACK_MEMBER(lw700i_state::vbl_interrupt)
{
	int scanline = m_screen->vpos();

	if (scanline == 1)
	{
		m_maincpu->set_input_line(5, ASSERT_LINE);
		// not sure where this is coming from, fix hang
		m_maincpu->space(0).write_byte(0xffffff05, 0);
	}
	else if (scanline == 2)
	{
		m_maincpu->set_input_line(5, CLEAR_LINE);
	}
}

void lw700i_state::machine_reset()
{
}

void lw700i_state::machine_start()
{
}

void lw700i_state::video_start()
{
}

uint32_t lw700i_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;
	static const uint32_t palette[2] = { 0, 0xffffff };
	uint8_t *pVRAM = (uint8_t *)m_mainram.target();

	pVRAM += 0x3e200;

	for (y = 0; y < 128; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 480/8; x++)
		{
			pixels = pVRAM[(y * (480/8)) + (BYTE_XOR_BE(x))];

			*scanline++ = palette[(pixels>>7)&1];
			*scanline++ = palette[(pixels>>6)&1];
			*scanline++ = palette[(pixels>>5)&1];
			*scanline++ = palette[(pixels>>4)&1];
			*scanline++ = palette[(pixels>>3)&1];
			*scanline++ = palette[(pixels>>2)&1];
			*scanline++ = palette[(pixels>>1)&1];
			*scanline++ = palette[(pixels&1)];
		}
	}

	return 0;
}

void lw700i_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0x0000);
	map(0x600000, 0x63ffff).ram().share("mainram"); // 256K of main RAM
	map(0xe00000, 0xe00001).rw(FUNC(lw700i_state::status_r), FUNC(lw700i_state::data_w));
	map(0xf00048, 0xf00049).ram();
}

void lw700i_state::io_map(address_map &map)
{
	map(h83003_device::PORT_7, h83003_device::PORT_7).r(FUNC(lw700i_state::p7_r));
	map(h83003_device::PORT_B, h83003_device::PORT_B).rw(FUNC(lw700i_state::pb_r), FUNC(lw700i_state::pb_w));
}

static INPUT_PORTS_START( lw700i )
INPUT_PORTS_END

MACHINE_CONFIG_START(lw700i_state::lw700i)
	MCFG_DEVICE_ADD("maincpu", H83003, XTAL(16'000'000))
	MCFG_DEVICE_PROGRAM_MAP(main_map)
	MCFG_DEVICE_IO_MAP(io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", lw700i_state, vbl_interrupt, "screen", 0, 1)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(lw700i_state, screen_update)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 480, 0, 128)
MACHINE_CONFIG_END

ROM_START(blw700i)
	ROM_REGION(0x200000, "maincpu", 0)      /* H8/3003 program ROM */
	ROM_LOAD16_WORD_SWAP( "mx24969b.bin", 0x000000, 0x200000, CRC(78d88d04) SHA1(3cda632c7190257abd20e121575767e8e9a18b1c) )
ROM_END

SYST( 1995, blw700i,    0, 0, lw700i, lw700i, lw700i_state, empty_init, "Brother", "LW-700i", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
