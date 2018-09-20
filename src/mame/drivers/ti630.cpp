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

private:
	DECLARE_WRITE8_MEMBER(i80c31_p1_w);
	DECLARE_WRITE8_MEMBER(i80c31_p3_w);
	DECLARE_READ8_MEMBER(i80c31_p1_r);
	DECLARE_PALETTE_INIT(ti630);
	void i80c31_io(address_map &map);
	void i80c31_prg(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

#define LOG_IO_PORTS 0

void ti630_state::i80c31_prg(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void ti630_state::init_ti630()
{
}

void ti630_state::i80c31_io(address_map &map)
{
	map(0x0000, 0x0000) /*.mirror(?)*/ .w("hd44780", FUNC(hd44780_device::control_write));
	map(0x1000, 0x1000) /*.mirror(?)*/ .w("hd44780", FUNC(hd44780_device::data_write));
	map(0x2000, 0x2000) /*.mirror(?)*/ .r("hd44780", FUNC(hd44780_device::control_read));
	map(0x8000, 0xffff).ram(); /*TODO: verify the ammont of RAM and the correct address range to which it is mapped. This is just a first reasonable guess that apparently yields good results in the emulation */
}

void ti630_state::machine_start()
{
}

void ti630_state::machine_reset()
{
}

READ8_MEMBER(ti630_state::i80c31_p1_r)
{
	uint8_t value = 0;
	if (LOG_IO_PORTS)
		logerror("P1 read value:%02X\n", value);

	return value;
}

WRITE8_MEMBER(ti630_state::i80c31_p1_w)
{
	if (LOG_IO_PORTS)
		logerror("Write to P1: %02X\n", data);
}

WRITE8_MEMBER(ti630_state::i80c31_p3_w)
{
	if (LOG_IO_PORTS)
		logerror("Write to P3: %02X\n", data);
}

PALETTE_INIT_MEMBER(ti630_state, ti630)
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

MACHINE_CONFIG_START(ti630_state::ti630)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I80C31, XTAL(10'000'000))
	MCFG_DEVICE_PROGRAM_MAP(i80c31_prg)
	MCFG_DEVICE_IO_MAP(i80c31_io)
	MCFG_MCS51_PORT_P1_IN_CB(READ8(*this, ti630_state, i80c31_p1_r))
	MCFG_MCS51_PORT_P1_OUT_CB(WRITE8(*this, ti630_state, i80c31_p1_w))
	MCFG_MCS51_PORT_P3_OUT_CB(WRITE8(*this, ti630_state, i80c31_p3_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(6*16, 9*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16-1, 0, 9*2-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(ti630_state, ti630)
	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_ti630)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)
MACHINE_CONFIG_END

ROM_START( ti630 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ti630.ci11",  0x00000, 0x10000, CRC(2602cbdc) SHA1(98266bea52a5893e0af0b5872eca0a0a1e0c5f9c) )
ROM_END

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME           FLAGS
COMP( 1999, ti630, 0,      0,      ti630,   0,     ti630_state, init_ti630, "Intelbras", "TI630 telephone", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND )
