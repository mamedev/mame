// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    GX800 (Konami Test board)

    driver by Angelo Salese

    Notes:
    Z80, 8KB RAM, 2 * SN76489AN for sound, TTLs for video/misc
    There are 5 x 005273 (Konami custom resistor array (SIL10)) on the PCB,
    also seen on Jail Break HW

    menu translation:
    * screen distortion adjustment normal
    * screen distortion adjustment wide
    * input/output check
    * color check
    * sound check

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class kontest_state : public driver_device
{
public:
	kontest_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_palette(*this, "palette")
	{ }

	void kontest(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	required_device<palette_device> m_palette;

	// driver state
	uint8_t m_control = 0;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// member functions
	void control_w(uint8_t data);
	void vblank(int state);

	void kontest_io(address_map &map) ATTR_COLD;
	void kontest_map(address_map &map) ATTR_COLD;

	void kontest_palette(palette_device &palette) const;
};


/***************************************************************************

  Video

***************************************************************************/

void kontest_state::kontest_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1, bit2;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = 0x52 * bit0 + 0xad * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

uint32_t kontest_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = (cliprect.top() >> 3); y <= (cliprect.bottom() >> 3); y++)
	{
		for (int x = (cliprect.left() >> 3); x <= (cliprect.right() >> 3); x++)
		{
			uint8_t const tile = m_ram[x | (y << 6) | 0x820];
			uint8_t const attr = m_ram[x | ((y >> 1) << 6) | 0x20] & 7;

			for (int yi = 0; yi < 8; yi++)
			{
				int const res_y = (y << 3) | yi;
				for (int xi = 0; xi < 8; xi++)
				{
					int const res_x = (x << 3) | xi;
					if (cliprect.contains(res_x, res_y))
					{
						uint8_t const pen = m_ram[(yi << 1) | (xi >> 2) | (tile << 4) | 0x1000];
						uint8_t const color = bitswap<2>(pen, (~xi & 3) | 4, ~xi & 3);

						bitmap.pix(res_y, res_x) = m_palette->pen(color | (attr << 2));
					}
				}
			}
		}
	}

	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

void kontest_state::vblank(int state)
{
	if (state && BIT(m_control, 3))
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void kontest_state::control_w(uint8_t data)
{
	// d0: coin counter
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));

	// d3: irq enable
	if (!BIT(data, 3))
		m_maincpu->set_input_line(0, CLEAR_LINE);

	// d2: ? (reset during 1st grid test and color test)
	// other bits: ?
	m_control = data;
}

void kontest_state::kontest_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xffff).ram().share("ram");
}

void kontest_state::kontest_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("sn1", FUNC(sn76489a_device::write));
	map(0x04, 0x04).w("sn2", FUNC(sn76489a_device::write));
	map(0x08, 0x08).w(FUNC(kontest_state::control_w));
	map(0x0c, 0x0c).portr("IN0");
	map(0x0d, 0x0d).portr("IN1");
	map(0x0e, 0x0e).portr("IN2");
	map(0x0f, 0x0f).portr("IN3");
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( kontest )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x80, 0x80, "Orientation" )
	PORT_DIPSETTING(    0x80, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

void kontest_state::machine_start()
{
	save_item(NAME(m_control));
}

void kontest_state::machine_reset()
{
	control_w(0);
}

void kontest_state::kontest(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 24_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &kontest_state::kontest_map);
	m_maincpu->set_addrmap(AS_IO, &kontest_state::kontest_io);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 4, 384, 0, 256, 264, 16, 240); // guessed
	screen.set_screen_update(FUNC(kontest_state::screen_update));
	screen.screen_vblank().set(FUNC(kontest_state::vblank));

	PALETTE(config, m_palette, FUNC(kontest_state::kontest_palette), 32);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	SN76489A(config, "sn1", 24_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "speaker", 0.50, 1);
	SN76489A(config, "sn2", 24_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "speaker", 0.50, 0);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kontest )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "800b01.10d",   0x0000, 0x8000, CRC(520f83dc) SHA1(abc23c586864c2ecbc5b16614e27faafc93287de) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "800a02.4f",    0x0000, 0x0020, CRC(6d604171) SHA1(6b1366fb53cecbde6fb651142a77917dd16daf69) )
ROM_END

} // anonymous namespace


GAME( 1987?, kontest, 0, kontest, kontest, kontest_state, empty_init, ROT0, "Konami", "Konami Test Board (GX800, Japan)", MACHINE_SUPPORTS_SAVE ) // late 1987 or early 1988
