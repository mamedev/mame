// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Konami Target Panic (cabinet test PCB)

    driver by Phil Bennett

    TODO: Determine correct clock frequencies, fix 'stuck' inputs in
    test mode

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "screen.h"


class tgtpanic_state : public driver_device
{
public:
	tgtpanic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ram(*this, "ram") { }

	void tgtpanic(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_ram;

	uint8_t m_color;

	DECLARE_WRITE8_MEMBER(color_w);

	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map);
	void prg_map(address_map &map);
};


void tgtpanic_state::machine_start()
{
	save_item(NAME(m_color));
}

/*************************************
 *
 *  Video hardware
 *
 *************************************/

uint32_t tgtpanic_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t colors[4];
	uint32_t offs;
	uint32_t x, y;

	colors[0] = 0;
	colors[1] = 0xffffffff;
	colors[2] = rgb_t(pal1bit(m_color >> 2), pal1bit(m_color >> 1), pal1bit(m_color >> 0));
	colors[3] = rgb_t(pal1bit(m_color >> 6), pal1bit(m_color >> 5), pal1bit(m_color >> 4));

	for (offs = 0; offs < 0x2000; ++offs)
	{
		uint8_t val = m_ram[offs];

		y = (offs & 0x7f) << 1;
		x = (offs >> 7) << 2;

		/* I'm guessing the hardware doubles lines */
		bitmap.pix32(y + 0, x + 0) = colors[val & 3];
		bitmap.pix32(y + 1, x + 0) = colors[val & 3];
		val >>= 2;
		bitmap.pix32(y + 0, x + 1) = colors[val & 3];
		bitmap.pix32(y + 1, x + 1) = colors[val & 3];
		val >>= 2;
		bitmap.pix32(y + 0, x + 2) = colors[val & 3];
		bitmap.pix32(y + 1, x + 2) = colors[val & 3];
		val >>= 2;
		bitmap.pix32(y + 0, x + 3) = colors[val & 3];
		bitmap.pix32(y + 1, x + 3) = colors[val & 3];
	}

	return 0;
}

WRITE8_MEMBER(tgtpanic_state::color_w)
{
	m_screen->update_partial(m_screen->vpos());
	m_color = data;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void tgtpanic_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).ram().share("ram");
}

void tgtpanic_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0").w(FUNC(tgtpanic_state::color_w));
	map(0x01, 0x01).portr("IN1");
}


/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( tgtpanic )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void tgtpanic_state::tgtpanic(machine_config &config)
{
	/* basic machine hardware */
	Z80(config,m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &tgtpanic_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &tgtpanic_state::io_map);
	m_maincpu->set_periodic_int(FUNC(tgtpanic_state::irq0_line_hold), attotime::from_hz(20)); /* Unverified */

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60); /* Unverified */
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* Unverified */
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 192 - 1, 0, 192 - 1);
	m_screen->set_screen_update(FUNC(tgtpanic_state::screen_update));
}


	/*************************************
	*
	*  ROM definition
	*
	*************************************/

ROM_START( tgtpanic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "601_ja_a01.13e", 0x0000, 0x8000, CRC(ece71952) SHA1(0f9cbd8adac2b1950bc608d51f0f122399c8f00f) )
ROM_END


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1996, tgtpanic, 0, tgtpanic, tgtpanic, tgtpanic_state, empty_init, ROT0, "Konami", "Target Panic", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
