/****************************************************************

  Meyco 8088 based hardware

  i8088 CPU @ 5MHz (15MHz XTAL + i8284A clock generator),
  3 x 8KB EPROM (max 4), 3 x 8KB RAM (max 4), 2KB battery RAM,
  2 x i8155, optional i8251A + RS232 for factory debug

  driver by MAME team
  also thanks to Darrell Hal Smith, Kevin Mullins


****************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/i8155.h"
#include "sound/dac.h"

class meyc8088_state : public driver_device
{
public:
	meyc8088_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this,"maincpu"),
		m_vram(*this, "vram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_vram;

	UINT8 m_status;
	UINT8 m_common;

	DECLARE_WRITE8_MEMBER(drive_w);
	DECLARE_WRITE8_MEMBER(video5_flip_w);
	DECLARE_READ8_MEMBER(video5_flip_r);
	DECLARE_WRITE8_MEMBER(screen_flip_w);
	DECLARE_READ8_MEMBER(screen_flip_r);
};


/***************************************************************************

  Video

***************************************************************************/

static SCREEN_UPDATE_IND16( meyc8088 )
{
	meyc8088_state *state = screen.machine().driver_data<meyc8088_state>();

	for (int y = 0; y < 512; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			UINT8 data = state->m_vram[y*64+x];
			for (int i = 0; i < 8; i++)
			{
				bitmap.pix16(y, x << 3 | i) = (data << i & 0x80) ? 1 : 0;
			}

		}
	}

	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(meyc8088_state::drive_w)
{
	m_status &= ~0x20;

	// d0-d3: DC counter drivers
	// d4-d7: AC motor drivers
}

// switch screen on/off on $b4000 access
READ8_MEMBER(meyc8088_state::screen_flip_r)
{
	m_status ^= 2;
	return 0;
}

WRITE8_MEMBER(meyc8088_state::screen_flip_w)
{
	m_status ^= 2;
}

// switch video5 (color prom d4) on/off on $b5000 access
READ8_MEMBER(meyc8088_state::video5_flip_r)
{
	m_status ^= 4;
	return 0;
}

WRITE8_MEMBER(meyc8088_state::video5_flip_w)
{
	m_status ^= 4;
}


static ADDRESS_MAP_START( meyc8088_map, AS_PROGRAM, 8, meyc8088_state )
	AM_RANGE(0x00000, 0x007ff) AM_RAM
	AM_RANGE(0x70000, 0x77fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xb0000, 0xb00ff) AM_DEVREADWRITE("i8155_2", i8155_device, memory_r, memory_w)
	AM_RANGE(0xb0800, 0xb0807) AM_DEVREADWRITE("i8155_2", i8155_device, io_r, io_w)
	AM_RANGE(0xb1000, 0xb10ff) AM_DEVREADWRITE("i8155_1", i8155_device, memory_r, memory_w)
	AM_RANGE(0xb1800, 0xb1807) AM_DEVREADWRITE("i8155_1", i8155_device, io_r, io_w)
	AM_RANGE(0xb2000, 0xb2000) AM_WRITE(drive_w)
//  AM_RANGE(0xb3000, 0xb3000) AM_NOP // 8251A, debug related
//  AM_RANGE(0xb3800, 0xb3800) AM_NOP // "
	AM_RANGE(0xb4000, 0xb4000) AM_READWRITE(screen_flip_r, screen_flip_w)
	AM_RANGE(0xb5000, 0xb5000) AM_READWRITE(video5_flip_r, video5_flip_w)
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static WRITE8_DEVICE_HANDLER(meyc8088_lights1_w)
{
	// lite 1-8
	for (int i = 0; i < 8; i++)
		output_set_lamp_value(i, data >> i & 1);
}

static WRITE8_DEVICE_HANDLER(meyc8088_lights2_w)
{
	// lite 9-16
	for (int i = 0; i < 8; i++)
		output_set_lamp_value(i + 8, data >> i & 1);
}

static WRITE8_DEVICE_HANDLER(meyc8088_common_w)
{
	meyc8088_state *state = device->machine().driver_data<meyc8088_state>();

	// d0: /CR2
	state->m_status = (state->m_status & ~1) | (data & 1);

	// d1: battery on
	state->m_status = (state->m_status & ~0x10) | (data << 3 & 0x10);

	// d2-d5: /common
	state->m_common = data >> 2 & 0xf;

	logerror("i8155 Port C: %02X\n", data);
}

static WRITE_LINE_DEVICE_HANDLER(meyc8088_i8155_1_timer_out)
{
	// clock 8251A
	//logerror("Timer 1 out %d\n", state);
}

static WRITE_LINE_DEVICE_HANDLER(meyc8088_i8155_2_timer_out)
{
	// sound dac
	//logerror("Timer 2 out %d\n", state);
}


static READ8_DEVICE_HANDLER(meyc8088_status_r)
{
	meyc8088_state *state = device->machine().driver_data<meyc8088_state>();

	// d0: /CR2
	// d1: screen on
	// d2: video5
	// d3: N/C
	// d4: battery ok
	// d5: /drive on
	return (state->m_status & 7) | 0x18;
}

static const i8155_interface i8155_intf[2] =
{
	{
		// all ports set to input
		DEVCB_INPUT_PORT("IN0"),
		DEVCB_NULL,
		DEVCB_INPUT_PORT("IN1"),
		DEVCB_NULL,
		DEVCB_HANDLER(meyc8088_status_r),
		DEVCB_NULL,
		DEVCB_LINE(meyc8088_i8155_1_timer_out)
	},
	{
		// all ports set to output
		DEVCB_NULL,
		DEVCB_HANDLER(meyc8088_lights2_w),
		DEVCB_NULL,
		DEVCB_HANDLER(meyc8088_lights1_w),
		DEVCB_NULL,
		DEVCB_HANDLER(meyc8088_common_w),
		DEVCB_LINE(meyc8088_i8155_2_timer_out)
	}
};


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( gldarrow )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN0 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN0 3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN0 4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN0 5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN0 6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN0 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN0 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_DIPNAME( 0x02, 0x02, "IN1 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN1 3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN1 4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN1 5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN1 6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN1 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN1 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "IN2 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "IN2 3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "IN2 4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "IN2 5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "IN2 6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "IN2 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "IN2 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static INTERRUPT_GEN( meyc8088_irq )
{
	// INTR on LC255, INTA hardwired to $20
	device_set_input_line_and_vector(device, 0, HOLD_LINE, 0x20);
}

static MACHINE_CONFIG_START( meyc8088, meyc8088_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, XTAL_15MHz / 3)
	MCFG_CPU_PROGRAM_MAP(meyc8088_map)
	MCFG_CPU_VBLANK_INT("screen", meyc8088_irq)

	MCFG_I8155_ADD("i8155_1", XTAL_15MHz / 3, i8155_intf[0])
	MCFG_I8155_ADD("i8155_2", XTAL_15MHz / 3, i8155_intf[1])

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(512,512)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 512-1)
	MCFG_SCREEN_UPDATE_STATIC(meyc8088)

	MCFG_PALETTE_LENGTH(8)

MACHINE_CONFIG_END


ROM_START( gldarrow )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "3.14h",   0x0fa000, 0x002000, CRC(a4acc6df) SHA1(b25f2cf8154932834100615e2e9c44ef47a15fea) )
	ROM_LOAD( "2.13h",   0x0fc000, 0x002000, CRC(595e380d) SHA1(6f8e58f646106d33cb651d97ca6a1133f7b05373) )
	ROM_LOAD( "1.12h",   0x0fe000, 0x002000, CRC(71bd0e39) SHA1(15345f5726cd33ecb1b2da05f2852b6cc3ac7747) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom.2c", 0x00, 0x20, NO_DUMP ) // M3-7602-5 color prom
ROM_END


GAME( 1984, gldarrow, 0,        meyc8088, gldarrow, 0, ROT0,  "Meyco Games, Inc.", "Golden Arrow", GAME_NOT_WORKING | GAME_NO_SOUND )
