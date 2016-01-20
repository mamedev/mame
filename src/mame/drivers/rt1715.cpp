// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Robotron PC-1715

        10/06/2008 Preliminary driver.


    Notes:

    - keyboard connected to sio channel a
    - sio channel a clock output connected to ctc trigger 0

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80pio.h"
#include "machine/z80dma.h"
#include "machine/ram.h"
#include "video/i8275.h"


class rt1715_state : public driver_device
{
public:
	rt1715_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG) { }

	int m_led1_val;
	int m_led2_val;
	DECLARE_WRITE8_MEMBER(rt1715_floppy_enable);
	DECLARE_READ8_MEMBER(k7658_led1_r);
	DECLARE_READ8_MEMBER(k7658_led2_r);
	DECLARE_READ8_MEMBER(k7658_data_r);
	DECLARE_WRITE8_MEMBER(k7658_data_w);
	DECLARE_WRITE8_MEMBER(rt1715_rom_disable);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(rt1715);
	I8275_DRAW_CHARACTER_MEMBER( crtc_display_pixels );
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};


/***************************************************************************
    FLOPPY
***************************************************************************/

WRITE8_MEMBER(rt1715_state::rt1715_floppy_enable)
{
	logerror("%s: rt1715_floppy_enable %02x\n", machine().describe_context(), data);
}


/***************************************************************************
    KEYBOARD
***************************************************************************/

/* si/so led */
READ8_MEMBER(rt1715_state::k7658_led1_r)
{
	m_led1_val ^= 1;
	logerror("%s: k7658_led1_r %02x\n", machine().describe_context(), m_led1_val);
	return 0xff;
}

/* caps led */
READ8_MEMBER(rt1715_state::k7658_led2_r)
{
	m_led2_val ^= 1;
	logerror("%s: k7658_led2_r %02x\n", machine().describe_context(), m_led2_val);
	return 0xff;
}

/* read key state */
READ8_MEMBER(rt1715_state::k7658_data_r)
{
	UINT8 result = 0xff;

	if (BIT(offset,  0)) result &= ioport("row_00")->read();
	if (BIT(offset,  1)) result &= ioport("row_10")->read();
	if (BIT(offset,  2)) result &= ioport("row_20")->read();
	if (BIT(offset,  3)) result &= ioport("row_30")->read();
	if (BIT(offset,  4)) result &= ioport("row_40")->read();
	if (BIT(offset,  5)) result &= ioport("row_50")->read();
	if (BIT(offset,  6)) result &= ioport("row_60")->read();
	if (BIT(offset,  7)) result &= ioport("row_70")->read();
	if (BIT(offset,  8)) result &= ioport("row_08")->read();
	if (BIT(offset,  9)) result &= ioport("row_18")->read();
	if (BIT(offset, 10)) result &= ioport("row_28")->read();
	if (BIT(offset, 11)) result &= ioport("row_38")->read();
	if (BIT(offset, 12)) result &= ioport("row_48")->read();

	return result;
}

/* serial output on D0 */
WRITE8_MEMBER(rt1715_state::k7658_data_w)
{
	logerror("%s: k7658_data_w %02x\n", machine().describe_context(), BIT(data, 0));
}


/***************************************************************************
    MEMORY HANDLING
***************************************************************************/

void rt1715_state::machine_start()
{
	membank("bank2")->set_base(m_ram->pointer() + 0x0800);
	membank("bank3")->set_base(m_ram->pointer());
}

void rt1715_state::machine_reset()
{
	/* on reset, enable ROM */
	membank("bank1")->set_base(memregion("ipl")->base());
}

WRITE8_MEMBER(rt1715_state::rt1715_rom_disable)
{
	logerror("%s: rt1715_set_bank %02x\n", machine().describe_context(), data);

	/* disable ROM, enable RAM */
	membank("bank1")->set_base(m_ram->pointer());
}

/***************************************************************************
    VIDEO EMULATION
***************************************************************************/

I8275_DRAW_CHARACTER_MEMBER( rt1715_state::crtc_display_pixels )
{
}

/* F4 Character Displayer */
static const gfx_layout rt1715_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*128, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8, 8*128*8, 9*128*8, 10*128*8, 11*128*8, 12*128*8, 13*128*8, 14*128*8, 15*128*8 },
	8                   /* every char takes 1 x 16 bytes */
};

static GFXDECODE_START( rt1715 )
	GFXDECODE_ENTRY("gfx", 0x0000, rt1715_charlayout, 0, 1)
	GFXDECODE_ENTRY("gfx", 0x0800, rt1715_charlayout, 0, 1)
GFXDECODE_END


/***************************************************************************
    PALETTE
***************************************************************************/

PALETTE_INIT_MEMBER(rt1715_state, rt1715)
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00)); /* black */
	palette.set_pen_color(1, rgb_t(0x00, 0x7f, 0x00)); /* low intensity */
	palette.set_pen_color(2, rgb_t(0x00, 0xff, 0x00)); /* high intensitiy */
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( rt1715_mem, AS_PROGRAM, 8, rt1715_state )
	AM_RANGE(0x0000, 0x07ff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank3")
	AM_RANGE(0x0800, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( rt1715_io, AS_IO, 8, rt1715_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("a71", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("a72", z80pio_device, read_alt, write_alt)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("a30", z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("a29", z80sio0_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x18, 0x19) AM_DEVREADWRITE("a26", i8275_device, read, write)
	AM_RANGE(0x20, 0x20) AM_WRITE(rt1715_floppy_enable)
	AM_RANGE(0x28, 0x28) AM_WRITE(rt1715_rom_disable)
ADDRESS_MAP_END

static ADDRESS_MAP_START( k7658_mem, AS_PROGRAM, 8, rt1715_state )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(k7658_data_w)
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0xf800) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( k7658_io, AS_IO, 8, rt1715_state )
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x8000) AM_READ(k7658_led1_r)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x8000) AM_READ(k7658_led2_r)
	AM_RANGE(0x8000, 0x9fff) AM_READ(k7658_data_r)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( k7658 )
	PORT_START("row_00")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_20")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_30")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_40")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_50")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_60")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_70")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_08")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_18")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_28")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_38")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("row_48")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

/* priority unknown */
static const z80_daisy_config rt1715_daisy_chain[] =
{
	{ "a71" },
	{ "a72" },
	{ "a30" },
	{ "a29" },
	{ nullptr }
};

static MACHINE_CONFIG_START( rt1715, rt1715_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_2_4576MHz)
	MCFG_CPU_PROGRAM_MAP(rt1715_mem)
	MCFG_CPU_IO_MAP(rt1715_io)
	MCFG_CPU_CONFIG(rt1715_daisy_chain)


	/* keyboard */
	MCFG_CPU_ADD("keyboard", Z80, 683000)
	MCFG_CPU_PROGRAM_MAP(k7658_mem)
	MCFG_CPU_IO_MAP(k7658_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DEVICE("a26", i8275_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(78*6, 30*10)
	MCFG_SCREEN_VISIBLE_AREA(0, 78*6-1, 0, 30*10-1)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rt1715)
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(rt1715_state, rt1715)

	MCFG_DEVICE_ADD("a26", I8275, XTAL_2_4576MHz)
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(rt1715_state, crtc_display_pixels)

	MCFG_DEVICE_ADD("a30", Z80CTC, XTAL_10MHz/4 /* ? */)

	MCFG_Z80SIO0_ADD("a29", XTAL_10MHz/4 /* ? */, 0, 0, 0, 0)

	/* floppy */
	MCFG_DEVICE_ADD("a71", Z80PIO, XTAL_10MHz/4 /* ? */)
	MCFG_DEVICE_ADD("a72", Z80PIO, XTAL_10MHz/4 /* ? */)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
	MCFG_RAM_DEFAULT_VALUE(0x00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( rt1715w, rt1715 )
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

ROM_START( rt1715 )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s500.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 90e7
	ROM_LOAD("s501.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 68da
	ROM_LOAD("s502.a25.3", 0x0000, 0x0800, CRC(7b6302e1) SHA1(e8f61763ff8841078a1939aa5e85a17f2af42163))

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("s619.a25.2", 0x0000, 0x0800, CRC(98647763) SHA1(93fba51ed26392ec3eff1037886576fa12443fe5))
	ROM_LOAD("s602.a25.1", 0x0800, 0x0800, NO_DUMP) // CCITT fd67

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s600.ic8", 0x0000, 0x0800, CRC(b7070122) SHA1(687056b822086ef0eee1e9b27e5b031bdbcade61))

	ROM_REGION(0x0800, "floppy", 0)
	ROM_LOAD("068.a8.2", 0x0000, 0x0400, CRC(5306d57b) SHA1(a12d025717b039a8a760eb9961365402f1f501f5)) // "read rom"
	ROM_LOAD("069.a8.1", 0x0400, 0x0400, CRC(319fa72c) SHA1(5f26af1e36339a934760a63e5975e9db09abeaaf)) // "write rom"
ROM_END

ROM_START( rt1715lc )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s500.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 90e7
	ROM_LOAD("s501.a25.3", 0x0000, 0x0800, NO_DUMP) // CCITT 68da
	ROM_LOAD("s502.a25.3", 0x0000, 0x0800, CRC(7b6302e1) SHA1(e8f61763ff8841078a1939aa5e85a17f2af42163))

	ROM_REGION(0x1000, "gfx", 0)
	ROM_LOAD("s643.a25.2", 0x0000, 0x0800, CRC(ea37f0e6) SHA1(357760974d944b9782734504b9820771e7e37645))
	ROM_LOAD("s605.a25.1", 0x0800, 0x0800, CRC(38062024) SHA1(798f62d4adeb7098b7dcbfe6caf28302853ee97d))

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s642.ic8", 0x0000, 0x0800, NO_DUMP) // CCITT 962e

	ROM_REGION(0x0800, "floppy", 0)
	ROM_LOAD("068.a8.2", 0x0000, 0x0400, CRC(5306d57b) SHA1(a12d025717b039a8a760eb9961365402f1f501f5)) // "read rom"
	ROM_LOAD("069.a8.1", 0x0400, 0x0400, CRC(319fa72c) SHA1(5f26af1e36339a934760a63e5975e9db09abeaaf)) // "write rom"
ROM_END

ROM_START( rt1715w )
	ROM_REGION(0x0800, "ipl", 0)
	ROM_LOAD("s550.bin", 0x0000, 0x0800, CRC(0a96c754) SHA1(4d9ad5b877353d91ba355044d2847e1d621e2b01))

	// loaded from floppy on startup
	ROM_REGION(0x1000, "gfx", ROMREGION_ERASEFF)

	ROM_REGION(0x0800, "keyboard", 0)
	ROM_LOAD("s600.ic8", 0x0000, 0x0800, CRC(b7070122) SHA1(687056b822086ef0eee1e9b27e5b031bdbcade61))

	ROM_REGION(0x0100, "prom", 0)
	ROM_LOAD("287.bin", 0x0000, 0x0100, CRC(8508360c) SHA1(d262a8c3cf2d284c67f23b853e0d59ae5cc1d4c8)) // /CAS decoder prom, 74S287
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT  INIT  COMPANY     FULLNAME                             FLAGS */
COMP( 1986, rt1715,   0,      0,      rt1715,  k7658, driver_device, 0,    "Robotron",  "Robotron PC-1715",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1986, rt1715lc, rt1715, 0,      rt1715,  k7658, driver_device, 0,    "Robotron",  "Robotron PC-1715 (latin/cyrillic)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1986, rt1715w,  rt1715, 0,      rt1715w, k7658, driver_device, 0,    "Robotron",  "Robotron PC-1715W",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
