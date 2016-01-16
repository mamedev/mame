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
#include "rendlay.h"

class ti630_state : public driver_device
{
public:
	ti630_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
	{ }

	DECLARE_WRITE8_MEMBER(ti630_io_w);
	DECLARE_READ8_MEMBER(ti630_io_r);
	DECLARE_DRIVER_INIT(ti630);
	DECLARE_PALETTE_INIT(ti630);
private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
};

#define LOG_IO_PORTS 0

static ADDRESS_MAP_START(i80c31_prg, AS_PROGRAM, 8, ti630_state)
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

DRIVER_INIT_MEMBER( ti630_state, ti630 )
{
}

static ADDRESS_MAP_START(i80c31_io, AS_IO, 8, ti630_state)
	AM_RANGE(0x0000,0x0000) /*AM_MIRROR(?)*/ AM_DEVWRITE("hd44780", hd44780_device, control_write)
	AM_RANGE(0x1000,0x1000) /*AM_MIRROR(?)*/ AM_DEVWRITE("hd44780", hd44780_device, data_write)
	AM_RANGE(0x2000,0x2000) /*AM_MIRROR(?)*/ AM_DEVREAD("hd44780", hd44780_device, control_read)
	AM_RANGE(0x8000,0xffff) AM_RAM /*TODO: verify the ammont of RAM and the correct address range to which it is mapped. This is just a first reasonable guess that apparently yields good results in the emulation */

	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P3) AM_READWRITE(ti630_io_r, ti630_io_w)
ADDRESS_MAP_END

void ti630_state::machine_start()
{
}

void ti630_state::machine_reset()
{
}

READ8_MEMBER(ti630_state::ti630_io_r)
{
	switch (offset)
	{
		case 0x01:
		{
			UINT8 value = 0;
#if LOG_IO_PORTS
			printf("P1 read value:%02X\n", value);
#endif
			return value;
		}
		default:
#if LOG_IO_PORTS
			printf("Unhandled I/O Read at offset 0x%02X (return 0)\n", offset);
#endif
			return 0;
	}
}

WRITE8_MEMBER(ti630_state::ti630_io_w)
{
	static UINT8 p0=0, p1=0, p2=0, p3=0;
	switch (offset)
	{
		case 0x00:
		{
			if (data != p0)
			{
				p0=data;
#if LOG_IO_PORTS
				printf("Write to P0: %02X\n", data);
#endif
			}
			break;
		}
		case 0x01:
		{
			if (data != p1)
			{
				p1=data;
#if LOG_IO_PORTS
				printf("Write to P1: %02X\n", data);
#endif
			}
			break;
		}
		case 0x02:
		{
			if (data != p2)
			{
				p2=data;
#if LOG_IO_PORTS
				printf("Write to P2: %02X\n", data);
#endif
			}
			break;
		}
		case 0x03:
		{
			if (data != p3)
			{
				p3=data;
#if LOG_IO_PORTS
				printf("Write to P3: %02X\n", data);
#endif
			}
			break;
		}
	}
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

static GFXDECODE_START( ti630 )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, ti630_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( ti630, ti630_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80C31, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(i80c31_prg)
	MCFG_CPU_IO_MAP(i80c31_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_SIZE(6*16, 9*2)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16-1, 0, 9*2-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(ti630_state, ti630)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ti630)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)
MACHINE_CONFIG_END

ROM_START( ti630 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ti630.ci11",  0x00000, 0x10000, CRC(2602cbdc) SHA1(98266bea52a5893e0af0b5872eca0a0a1e0c5f9c) )
ROM_END

/*    YEAR  NAME      PARENT  COMPAT  MACHINE     INPUT     CLASS         INIT    COMPANY  FULLNAME                       FLAGS */
COMP( 1999, ti630,   0,      0,      ti630,     0,   ti630_state, ti630, "Intelbras", "TI630 telephone",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND)
