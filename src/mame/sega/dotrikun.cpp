// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi, hap
/***************************************************************************

Dottori Kun (Head On's mini game)
(c)1990 SEGA

Driver by Takahiro Nogi 1999/12/15 -


CPU   : Z-80 (4MHz)
SOUND : (none)

14479.MPR  ; PRG (FIRST VER)
14479A.MPR ; PRG (NEW VER)

* This game is only for the test of cabinet
* BackRaster = WHITE on the FIRST version.
* BackRaster = BLACK on the NEW version.
* On the NEW version, push COIN-SW as TEST MODE.
* 0000-3FFF:ROM 8000-85FF:VRAM(128x96) 8600-87FF:WORK-RAM


TODO:
- improve video timing more (see http://www.chrismcovell.com/dottorikun.html)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"


namespace {

#define MASTER_CLOCK XTAL(4'000'000)


class dotrikun_state : public driver_device
{
public:
	dotrikun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_vram(*this, "vram"),
		m_interrupt_timer(*this, "interrupt"),
		m_scanline_off_timer(*this, "scanline_off")
	{ }

	void dotrikun(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_vram;
	required_device<timer_device> m_interrupt_timer;
	required_device<timer_device> m_scanline_off_timer;

	uint8_t m_vram_latch = 0U;
	uint8_t m_color = 0U;

	void vram_w(offs_t offset, uint8_t data);
	void color_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_off);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_on);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void dotrikun_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};

TIMER_DEVICE_CALLBACK_MEMBER(dotrikun_state::interrupt)
{
	m_maincpu->pulse_input_line(0, attotime::from_hz(MASTER_CLOCK/128));
}

TIMER_DEVICE_CALLBACK_MEMBER(dotrikun_state::scanline_off)
{
	m_maincpu->set_unscaled_clock(MASTER_CLOCK);
}

TIMER_DEVICE_CALLBACK_MEMBER(dotrikun_state::scanline_on)
{
	// on vram fetch(every 8 pixels during active display), z80 is stalled for 2 clocks
	if (param < 192)
	{
		m_maincpu->set_unscaled_clock(MASTER_CLOCK * 0.75);
		m_scanline_off_timer->adjust(m_screen->time_until_pos(param, 128));
	}

	// vblank interrupt
	if (param == 191)
		m_interrupt_timer->adjust(m_screen->time_until_pos(param, 128+64));
}


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void dotrikun_state::vram_w(offs_t offset, uint8_t data)
{
	m_screen->update_now();
	m_vram[offset] = data;
}

void dotrikun_state::color_w(uint8_t data)
{
	// d0-d2: fg palette
	// d3-d5: bg palette
	// d6,d7: N/C
	m_screen->update_now();
	m_color = data & 0x3f;
}


uint32_t dotrikun_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			// vram fetch
			if ((x & 7) == 0)
				m_vram_latch = m_vram[x >> 3 | y >> 1 << 4];

			bitmap.pix(y, x) = (m_vram_latch >> (~x & 7) & 1) ? m_color & 7 : m_color >> 3;
		}
	}

	return 0;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void dotrikun_state::dotrikun_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x8000, 0x85ff).ram().w(FUNC(dotrikun_state::vram_w)).share("vram");
	map(0x8600, 0x87ff).ram();
}

void dotrikun_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0xff).portr("INPUTS").w(FUNC(dotrikun_state::color_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( dotrikun )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void dotrikun_state::machine_start()
{
	save_item(NAME(m_vram_latch));
	save_item(NAME(m_color));
}

void dotrikun_state::machine_reset()
{
	m_vram_latch = 0;
	m_color = 0;
}

void dotrikun_state::dotrikun(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &dotrikun_state::dotrikun_map);
	m_maincpu->set_addrmap(AS_IO, &dotrikun_state::io_map);

	TIMER(config, "scanline_on").configure_scanline(FUNC(dotrikun_state::scanline_on), "screen", 0, 1);
	TIMER(config, m_scanline_off_timer).configure_generic(FUNC(dotrikun_state::scanline_off));
	TIMER(config, m_interrupt_timer).configure_generic(FUNC(dotrikun_state::interrupt));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK, 128+128, 0, 128, 192+64, 0, 192);
	m_screen->set_physical_aspect(1, 1); // large border area
	m_screen->set_screen_update(FUNC(dotrikun_state::screen_update));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_palette("palette");
	PALETTE(config, "palette", palette_device::RGB_3BIT);

	/* no sound hardware */
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dotrikun )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mpr-14479a.ic2", 0x0000, 0x4000, CRC(b77a50db) SHA1(2a5d812d39f0f58f5c3e1b46f80aca75aa225115) )
ROM_END

ROM_START( dotrikun2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mpr-14479.ic2",  0x0000, 0x4000, CRC(a6aa7fa5) SHA1(4dbea33fb3541fdacf2195355751078a33bb30d5) ) // also seen with EPR-13141 (same ROM contents)
ROM_END

ROM_START( dotriman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dotriman.bin", 0x0000, 0x4000, CRC(4ba6d2f5) SHA1(db805e9121ecbd41fac4593b58d7f071e7dbc720) )
ROM_END

} // anonymous namespace


GAME( 1990, dotrikun,  0,        dotrikun, dotrikun, dotrikun_state, empty_init, ROT0, "Sega", "Dottori Kun (new version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
GAME( 1990, dotrikun2, dotrikun, dotrikun, dotrikun, dotrikun_state, empty_init, ROT0, "Sega", "Dottori Kun (old version)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )

GAME( 2016, dotriman,  dotrikun, dotrikun, dotrikun, dotrikun_state, empty_init, ROT0, "hack (Chris Covell)", "Dottori-Man Jr.", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
