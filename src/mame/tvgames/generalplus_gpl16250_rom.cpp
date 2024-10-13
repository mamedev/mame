// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    GPL16250 / GPAC800 / GMC384 / GCM420 related support

    GPL16250 is the GeneralPlus / SunPlus part number
    GPAC800 is the JAKKS Pacific codename
    GMC384 / GCM420 is what is printed on the die

    ----

    GPL16250 games using ROM (no extra RAM) configuration
*/

#include "emu.h"
#include "generalplus_gpl16250.h"


static INPUT_PORTS_START( smartfp )
	PORT_START("IN0")
	// entirely non-standard mat based controller (0-11 are where your feet are placed normally, row of selection places to step above those)
	// no sensible default mapping unless forced
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_Q) PORT_NAME("0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_W) PORT_NAME("1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_E) PORT_NAME("2")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_R) PORT_NAME("3")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_T) PORT_NAME("4")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_CODE(KEYCODE_Y) PORT_NAME("5")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_CODE(KEYCODE_U) PORT_NAME("6")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_CODE(KEYCODE_I) PORT_NAME("7")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_CODE(KEYCODE_O) PORT_NAME("8")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_CODE(KEYCODE_P) PORT_NAME("9")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("10")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("11")

	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_A) PORT_NAME("Circle / Red")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_S) PORT_NAME("Square / Orange")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_D) PORT_NAME("Triangle / Yellow")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_F) PORT_NAME("Star / Blue")

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("HOME")
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gormiti ) // DOWN with A+B+C for test mode?
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 ) // causes long delay in test mode?
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( tkmag220 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Important" ) // gets stuck in inf loop if this is wrong
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )   // set 0x0001 and 0x0002 on to get a test mode (some of the ROM banks fail their test, but dumps were repeatable, should be verified on another unit)
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( gameu )
	PORT_START("IN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2") // P2 inputs are listed in test mode, but unit has no 2nd set of controls
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( beijuehh )
	PORT_START("IN0")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN ) // battery
	PORT_BIT( 0x001e, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0e00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // this one must be kept in this state or the machine will freeze after a few seconds?
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("IN3") // is there a 4th button?
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Menu")
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



ROM_START( smartfp )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("smartfitparkuk.bin", 0x000000, 0x800000, CRC(2072d7d0) SHA1(eaa4f254d6dee3a7eac64ae2204dd6291e4d27cc) )
ROM_END

ROM_START( smartfps )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("smartfitpark.bin", 0x000000, 0x800000, CRC(ada84507) SHA1(a3a80bf71fae62ebcbf939166a51d29c24504428) )
ROM_END

ROM_START( smartfpf )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("smartfitpark_fr.bin", 0x000000, 0x800000, CRC(e6d3ba29) SHA1(14e4632997318329be3291f2c4e62f088181f3c8) )
ROM_END


ROM_START( gormiti )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION(0x800000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD16_WORD_SWAP("gormiti.bin", 0x000000, 0x800000, CRC(71b82d41) SHA1(169b35dc7bdd05b7b32176ddf901ace27736cb86) )
ROM_END

ROM_START( tkmag220 )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "taikee220.bin", 0x0000000, 0x8000000, CRC(02881534) SHA1(a0e0c9cfa3a6b1c6107f06abd3268b82bd663d06) )
ROM_END


ROM_START( imgame )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 ) // the 2nd half of this ROM required multiple attempts to get a matching dump, so could be suspect, might also be unused as this only has 120 games
	ROM_LOAD16_WORD_SWAP( "imgame.bin", 0x0000000, 0x8000000, CRC(6fba9021) SHA1(852f4c0aaed682aa8ff5b8cd52313ea2d3d920a1))
ROM_END

ROM_START( myac220 )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "myarcadegogamerportable.bin", 0x0000000, 0x8000000, BAD_DUMP CRC(c929a2fa) SHA1(e99007ccc45a268267b4ea0efaf22e3117f5a6bd) ) // several sections seemed to be erased, was repaired with data from tkmag220, likely good but should be verified
ROM_END

ROM_START( beijuehh )
	//ROM_REGION16_BE( 0x40000, "maincpu:internal", ROMREGION_ERASE00 ) // not on this model? (or at least not this size, as CS base is different)
	//ROM_LOAD16_WORD_SWAP( "internal.rom", 0x00000, 0x40000, NO_DUMP )

	ROM_REGION( 0x8000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "beijeu.bin", 0x0000000, 0x8000000, CRC(e7b968af) SHA1(a39a3a70e6e0827e4395e09e55983eb9e9348e4a) ) // some address lines might be swapped
ROM_END


ROM_START( gameu50 )
	ROM_REGION( 0x2000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "gameu.bin", 0x000000, 0x2000000, CRC(13c42bce) SHA1(f769ceabb8ab4e60c0d663dffd5cca91c6aec206) )
ROM_END



void tkmag220_game_state::tkmag220(machine_config &config)
{
	gcm394_game_state::base(config);

	m_maincpu->porta_in().set_ioport("IN0");
	m_maincpu->portb_in().set_ioport("IN1");
	m_maincpu->portc_in().set_ioport("IN2");

	m_maincpu->portd_out().set(FUNC(tkmag220_game_state::tkmag220_portd_w));
}

void tkmag220_game_state::tkmag220_portd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_maincpu->pc() < 0x10000)
	{
		logerror("%s: port write %04x\n", machine().describe_context().c_str(), data);

		int newbank = 0;

		if (data & 0x8000) newbank |= 1;
		if (data & 0x0002) newbank |= 2;
		if (data & 0x0004) newbank |= 4;

		m_upperbase = newbank * (0x1000000 / 2);
	}

}


uint16_t tkmag220_game_state::cs0_r(offs_t offset)
{
	// [:] installing cs0 handler start_address 00000000 end_address 007fffff
	return m_romregion[(offset & 0x07fffff) + m_upperbase];
}

void tkmag220_game_state::machine_reset()
{
	// as with the Family Sport multi-game versions on spg2xx hardware, there are actually 8 programs in here, externally banked
	// each of those programs is 16MBytes, so is either going to have the upper half banked externally like the older hardware types, os possibly using the CS registers on this hardware type
	m_upperbase = 0 * (0x1000000 / 2);
	gcm394_game_state::machine_reset();

	//m_maincpu->set_paldisplaybank_high_hack(0);
	//m_maincpu->set_pal_sprites_hack(0x000);
	//m_maincpu->set_pal_back_hack(0x000);
	m_maincpu->set_alt_tile_addressing_hack(1);
}




void beijuehh_game_state::beijuehh(machine_config &config)
{
	gcm394_game_state::base(config);

	m_maincpu->porta_in().set_ioport("IN0");
	m_maincpu->portb_in().set_ioport("IN1");
	m_maincpu->portc_in().set_ioport("IN2");
	m_maincpu->portd_in().set_ioport("IN3");

	m_maincpu->portb_out().set(FUNC(beijuehh_game_state::beijuehh_portb_w));
	m_maincpu->portd_out().set(FUNC(beijuehh_game_state::beijuehh_portd_w));
}


void beijuehh_game_state::beijuehh_portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_maincpu->pc() < 0xf000)
	{
		// 00a0 bits are banking
		logerror("%s: portb write %04x\n", machine().describe_context(), data);

		if (data & 0x0020)
			m_bank |= 0x04;
		else
			m_bank &= ~0x04;

		if (data & 0x0080)
			m_bank |= 0x08;
		else
			m_bank &= ~0x08;

		m_upperbase = m_bank * (0x400000);
	}
	m_portb_data = data;
}


void beijuehh_game_state::beijuehh_portd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_maincpu->pc() < 0xf000)
	{
		// c000 bits are banking
		logerror("%s: portd write %04x\n", machine().describe_context(), data);

		if (data & 0x4000)
			m_bank |= 0x02;
		else
			m_bank &= ~0x02;

		if (data & 0x8000)
			m_bank |= 0x01;
		else
			m_bank &= ~0x01;

		m_upperbase = m_bank * (0x400000);
	}
	m_portd_data = data;
}


uint16_t beijuehh_game_state::cs0_r(offs_t offset)
{
	// [:] installing cs0 handler start_address 00000000 end_address 003fffff
	return m_romregion[(offset & 0x03fffff) + m_upperbase];
}


void beijuehh_game_state::machine_reset()
{
	// this one seems to operate in a mode much closer to the older spg2xx hardware
	// presumably registers enable this behavior
	//
	// ROM is just banked 8MByte blocks
	// overall very similar to marc101 / marc250 units, seems to have the port based
	// 'timer' checks for protection(?) too

	m_portb_data = 0;
	m_portd_data = 0;
	m_bank = 0;

	m_upperbase = 0 * (0x400000);
	gcm394_game_state::machine_reset();

	//m_maincpu->set_paldisplaybank_high_hack(0);
	//m_maincpu->set_pal_sprites_hack(0x000);
	//m_maincpu->set_pal_back_hack(0x000);
	m_maincpu->set_alt_tile_addressing_hack(1);
	//m_maincpu->set_alt_extrasprite_hack(1);
	m_maincpu->set_legacy_video_mode();
}



void gameu_handheld_game_state::gameu(machine_config &config)
{
	gcm394_game_state::base(config);

	m_maincpu->porta_out().set(FUNC(gameu_handheld_game_state::gameu_porta_w));
	m_maincpu->portb_out().set(FUNC(gameu_handheld_game_state::gameu_portb_w));
	m_maincpu->portc_out().set(FUNC(gameu_handheld_game_state::gameu_portc_w));
	m_maincpu->portd_out().set(FUNC(gameu_handheld_game_state::gameu_portd_w));

	m_screen->set_refresh_hz(30); // too fast at 60, but maybe it's for other reasons?
	m_screen->set_visarea(0, (160)-1, 0, (128)-1); // appears to be the correct resolution for the LCD panel
}

void gormiti_game_state::machine_reset()
{
	gcm394_game_state::machine_reset();
	m_maincpu->set_alt_tile_addressing_hack(1);
}

uint16_t gameu_handheld_game_state::cs0_r(offs_t offset)
{
	return m_romregion[(offset & 0x00fffff) + m_upperbase];
}

void gameu_handheld_game_state::gameu_porta_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: porta write %04x\n", machine().describe_context(), data);
	m_porta_data = data;
}

void gameu_handheld_game_state::gameu_portb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portb write %04x\n", machine().describe_context(), data);
	m_portb_data = data;
}

void gameu_handheld_game_state::gameu_portc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: portc write %04x\n", machine().describe_context(), data);
	m_portc_data = data;
}

void gameu_handheld_game_state::gameu_portd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// hacky, maybe we need better direction/attribute handling on the ports in the core?
	m_portd_data = data;
	//int pc = m_maincpu->pc();
	//if ((pc != 0x2b49) && (pc != 0x2b34) && (pc != 0x2b8b) && (pc != 0x2bc0))
	{
		logerror("%s: portd write %04x %04x\n", machine().describe_context(), data, mem_mask);

		uint8_t bank = (data & 0xfc00) >> 10;
		m_upperbase = bank * 0x40000;
	}

}
void gameu_handheld_game_state::machine_start()
{
	m_upperbase = 0;
	m_porta_data = 0;
	m_portb_data = 0;
	m_portc_data = 0;
	m_portd_data = 0;

	save_item(NAME(m_upperbase));
	save_item(NAME(m_porta_data));
	save_item(NAME(m_portb_data));
	save_item(NAME(m_portc_data));
	save_item(NAME(m_portd_data));
}

void gameu_handheld_game_state::machine_reset()
{
	gcm394_game_state::machine_reset();
	m_maincpu->set_alt_tile_addressing_hack(1);
	m_upperbase = 0;
}

void gameu_handheld_game_state::init_gameu()
{
	uint16_t *ROM = (uint16_t*)memregion("maincpu")->base();
	int size = memregion("maincpu")->bytes();

	for (int i = 0; i < size/2; i++)
	{
		ROM[i] = ROM[i] ^ 0x3b90;

		ROM[i] = bitswap<16>(ROM[i], 3, 1, 11, 9,  6, 14, 0,  2,
									 8, 7, 13, 15, 4, 5,  12, 10);
	}

	m_maincpu->set_alt_tile_addressing_hack(0);
	m_maincpu->set_disallow_resolution_control();

	// why do we need these? it will jump to 0 after the menu selection (prior to fadeout and bank select) otherwise, which can't be correct

	ROM[0x19c9a / 2] = 0xf165;
	ROM[0x19c9c / 2] = 0xf165;
	ROM[0x19c9e / 2] = 0xf165;

	ROM[0x19cb8 / 2] = 0xf165;
	ROM[0x19cba / 2] = 0xf165;
	ROM[0x19cbc / 2] = 0xf165;

	ROM[0x19cd4 / 2] = 0xf165;
	ROM[0x19cd6 / 2] = 0xf165;
	ROM[0x19cd8 / 2] = 0xf165;
}



// the JAKKS ones of these seem to be known as 'Generalplus GPAC500' hardware?
CONS(2009, smartfp,   0,       0, base, smartfp,  gcm394_game_state, empty_init, "Fisher-Price", "Fun 2 Learn Smart Fit Park (UK)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, smartfps,  smartfp, 0, base, smartfp,  gcm394_game_state, empty_init, "Fisher-Price", "Fun 2 Learn Smart Fit Park (Spain)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
CONS(2009, smartfpf,  smartfp, 0, base, smartfp,  gcm394_game_state, empty_init, "Fisher-Price", "Fun 2 Learn Smart Fit Park (France)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND) // boxart simply has 'Smart Fit'

// These are ports of the 'Family Sport' games to GPL16250 type hardware, but they don't seem to use many unSP 2.0 instructions.
// The menu style is close to 'm505neo' but the game selection is closer to 'dnv200fs' (but without the Sports titles removed, and with a few other extras not found on that unit)
CONS(201?, tkmag220,  0,       0, tkmag220, tkmag220, tkmag220_game_state,  empty_init,      "TaiKee / Senca",         "Mini Arcade Games Console (Family Sport 220-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// DGUN-2891 or DGUN-2864 ? both look the same, no indication on unboxed unit?
CONS(201?, myac220,   0,       0, tkmag220, tkmag220, tkmag220_game_state,  empty_init,      "dreamGEAR / Senca",      "My Arcade Go Gamer Portable (Family Sport 220-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// 2012 date from manual
CONS(2012, imgame,    0,       0, tkmag220, tkmag220, tkmag220_game_state,  empty_init,      "I'm Game / Senca",      "I'm Game! GP120 (Family Sport 120-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
// a 180 game Family Sport I'm Game! also exists (and some Famiclones)

// Also sold as 'BornKid 220 in 1' There are lower capacity versions too, was sold with note that 'X-Racer III crashes in-game'
// Does the 'Helicopter' game work properly on real hardware? The function at 0x0D2BE7 uses RAM address 0x2372 for the upper bits of the tile base offset calculation
// but that RAM address doesn't appear to be written anywhere in the code, resulting in scrolling being entirely broken.
CONS(201?, beijuehh,    0,       0, beijuehh, beijuehh, beijuehh_game_state,  empty_init,      "Beijue",      "Beijue 16 Bit Handheld Games 220-in-1 (Game Boy style case)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// die on this one is 'GCM420'
CONS(2013, gormiti,   0, 0, base, gormiti,  gormiti_game_state, empty_init, "Giochi Preziosi", "Gormiti Game Arena (Spain)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)

// Fun 2 Learn 3-in-1 SMART SPORTS  ?

// unit looks a bit like a knock-off Wii-U tablet, but much smaller
CONS( 201?, gameu50,       0,              0,      gameu, gameu, gameu_handheld_game_state, init_gameu, "YSN", "Play Portable Color GameU+ (50-in-1) (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
