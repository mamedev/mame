// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*************************************************************************************************

(Hologram) Time Traveler (c) 1991 Virtual Image Productions / Sega

preliminary driver by Angelo Salese

TODO:
- unemulated Pioneer LDV-4200 and Sony LDP-1450 players, needs a dump of the BIOSes and proper
  hook-up;

==================================================================================================

Time Traveler ROM image

warren@dragons-lair-project.com
6/25/01


ROM is a 27C020 (256kbit x 8 = 256 KB)
ROM sticker says 6/18/91

CPU is an Intel 80188

*************************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"


class timetrv_state : public driver_device
{
public:
	timetrv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_led_vram_lo(*this, "led_vralo"),
		m_led_vram_hi(*this, "led_vrahi"),
		m_maincpu(*this, "maincpu") { }

	required_shared_ptr<UINT8> m_led_vram_lo;
	required_shared_ptr<UINT8> m_led_vram_hi;
	DECLARE_READ8_MEMBER(test1_r);
	DECLARE_READ8_MEMBER(test2_r);
	DECLARE_READ8_MEMBER(in_r);
	DECLARE_READ8_MEMBER(ld_r);
	virtual void video_start() override;
	UINT32 screen_update_timetrv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(ld_irq);
	required_device<cpu_device> m_maincpu;
};



void timetrv_state::video_start()
{
}

UINT32 timetrv_state::screen_update_timetrv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	popmessage("%s%s",reinterpret_cast<char *>(m_led_vram_lo.target()),reinterpret_cast<char *>(m_led_vram_hi.target()));
	return 0;
}

READ8_MEMBER(timetrv_state::test1_r)
{
	return ioport("IN0")->read();//machine().rand();
}

READ8_MEMBER(timetrv_state::test2_r)
{
	/*bit 7,eeprom read bit*/
	return (ioport("IN1")->read() & 0x7f);//machine().rand();
}


READ8_MEMBER(timetrv_state::in_r)
{
	return 0xff;
}

READ8_MEMBER(timetrv_state::ld_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START( timetrv_map, AS_PROGRAM, 8, timetrv_state )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM //irq vectors + work ram
	AM_RANGE(0x10000, 0x107ff) AM_RAM
	AM_RANGE(0xc0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( timetrv_io, AS_IO, 8, timetrv_state )
	AM_RANGE(0x0122, 0x0123) AM_WRITENOP //eeprom write bits
	AM_RANGE(0x1000, 0x1000) AM_READ(test1_r) //inputs
	AM_RANGE(0x1001, 0x1001) AM_READ(test2_r) //eeprom read bit + inputs

	AM_RANGE(0x1080, 0x1082) AM_READ(in_r) //dsw
	AM_RANGE(0x1100, 0x1105) AM_WRITENOP //laserdisc write area
	AM_RANGE(0x1100, 0x1105) AM_READ(ld_r) //5 -> laserdisc read status
	AM_RANGE(0x1180, 0x1187) AM_RAM AM_SHARE("led_vralo")//led string,part 1
	AM_RANGE(0x1200, 0x1207) AM_RAM AM_SHARE("led_vrahi")//led string,part 2
	AM_RANGE(0xff80, 0xffff) AM_RAM //am80188-em-like cpu internal regs?
ADDRESS_MAP_END


static INPUT_PORTS_START( timetrv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// 0x80 eeprom read bit
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(timetrv_state::vblank_irq)
{
	device.execute().set_input_line_and_vector(0,HOLD_LINE,0x20/4); //vblank bit flag clear
}

INTERRUPT_GEN_MEMBER(timetrv_state::ld_irq)
{
	device.execute().set_input_line_and_vector(0,HOLD_LINE,0x48/4); //ld irq
}

static MACHINE_CONFIG_START( timetrv, timetrv_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I80188,20000000) //???
	MCFG_CPU_PROGRAM_MAP(timetrv_map)
	MCFG_CPU_IO_MAP(timetrv_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", timetrv_state, vblank_irq)
	MCFG_CPU_PERIODIC_INT_DRIVER(timetrv_state, ld_irq, 60) //remove from here

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 512)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 512-1, 0*8, 512-1)
	MCFG_SCREEN_UPDATE_DRIVER(timetrv_state, screen_update_timetrv)

	MCFG_PALETTE_ADD("palette", 512)

	/* sound hardware */
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( timetrv )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "tt061891.bin",   0xc0000, 0x40000, CRC(a3d44219) SHA1(7c5003b6d3df1e472db45abd725e7d3d43f0dfb4) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "timetrv", 0, NO_DUMP )
ROM_END

GAME( 1991, timetrv,  0,       timetrv,  timetrv, driver_device,  0, ROT0, "Virtual Image Productions (Sega license)", "Time Traveler", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
