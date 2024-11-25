// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

  Intelbras TI630 telephone
  Driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>

  http://images.quebarato.com.br/T440x/telefone+ks+ti+630+seminovo+intelbras+sao+paulo+sp+brasil__2E255D_1.jpg

  Changelog:

   2014 JUN 17 [Felipe Sanches]:
   * Initial driver skeleton
   * LCD works

================
    Messages displayed on screen are in brazilian portuguese.
    During boot, it says:

"TI auto-test."
"Wait!"

    Then it says:

"Initializing..."
"Wait!"

    And finally:

"TI did not receive"
"the dial tone"

It means we probably would have to emulate a modem device for it to treat communications with a PABX phone hub.
================
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

#define LOG_IO_PORTS (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

class ti630_state : public driver_device
{
public:
	ti630_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
	{ }

	void ti630(machine_config &config);

	void init_ti630();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void i80c31_p1_w(uint8_t data);
	void i80c31_p3_w(uint8_t data);
	uint8_t i80c31_p1_r();
	void ti630_palette(palette_device &palette) const;
	void i80c31_io(address_map &map) ATTR_COLD;
	void i80c31_prg(address_map &map) ATTR_COLD;

	required_device<i80c31_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

void ti630_state::i80c31_prg(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void ti630_state::init_ti630()
{
}

void ti630_state::i80c31_io(address_map &map)
{
	map(0x0000, 0x0000) /*.mirror(?)*/ .w(m_lcdc, FUNC(hd44780_device::control_w));
	map(0x1000, 0x1000) /*.mirror(?)*/ .w(m_lcdc, FUNC(hd44780_device::data_w));
	map(0x2000, 0x2000) /*.mirror(?)*/ .r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x8000, 0xffff).ram(); /*TODO: verify the ammont of RAM and the correct address range to which it is mapped. This is just a first reasonable guess that apparently yields good results in the emulation */
}

void ti630_state::machine_start()
{
}

void ti630_state::machine_reset()
{
}

uint8_t ti630_state::i80c31_p1_r()
{
	uint8_t value = 0;
	LOGMASKED(LOG_IO_PORTS, "P1 read value:%02X\n", value);
	return value;
}

void ti630_state::i80c31_p1_w(uint8_t data)
{
	LOGMASKED(LOG_IO_PORTS, "Write to P1: %02X\n", data);
}

void ti630_state::i80c31_p3_w(uint8_t data)
{
	LOGMASKED(LOG_IO_PORTS, "Write to P3: %02X\n", data);
}

void ti630_state::ti630_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static const gfx_layout ti630_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( gfx_ti630 )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, ti630_charlayout, 0, 1 )
GFXDECODE_END

void ti630_state::ti630(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ti630_state::i80c31_prg);
	m_maincpu->set_addrmap(AS_IO, &ti630_state::i80c31_io);
	m_maincpu->port_in_cb<1>().set(FUNC(ti630_state::i80c31_p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(ti630_state::i80c31_p1_w));
	m_maincpu->port_out_cb<3>().set(FUNC(ti630_state::i80c31_p3_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 9*2);
	screen.set_visarea(0, 6*16-1, 0, 9*2-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(ti630_state::ti630_palette), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_ti630);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 16);
}

ROM_START( ti630 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ti630.ci11",  0x00000, 0x10000, CRC(2602cbdc) SHA1(98266bea52a5893e0af0b5872eca0a0a1e0c5f9c) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME           FLAGS
COMP( 1999, ti630, 0,      0,      ti630,   0,     ti630_state, init_ti630, "Intelbras", "TI630 telephone", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
