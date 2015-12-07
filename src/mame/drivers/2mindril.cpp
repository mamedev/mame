// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina, David Haywood
/*
 Two Minute Drill - Taito 1993
 -----------------------------
 Half Video, Half Mechanical?
(video hw + motion/acceleration sensor ?)

 preliminary driver by
  David Haywood
  Tomasz Slanina
  Angelo Salese

TODO:
 - understand the ball hit sensor
 - simulate the sensors (there are still some shutter errors/defender errors that pops up)
 - Hook-up timers for shutter/defender sensors (check service mode)
 - Dip-Switches

 Brief hardware overview:
 ------------------------

 Main processor   - 68000 16Mhz

 Sound            - Yamaha YM2610B

 Taito custom ICs - TC0400YSC (m68k -> ym2610 communication)
                  - TC0260DAR (palette chip)
                  - TC0630FDP (Taito F3 video chip)
                  - TC0510NIO (known input chip)

DAC               -26.6860Mhz
                  -32.0000Mhz

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/2610intf.h"
#include "includes/taito_f3.h"


class _2mindril_state : public taito_f3_state
{
public:
	_2mindril_state(const machine_config &mconfig, device_type type, const char *tag)
		: taito_f3_state(mconfig, type, tag),
		m_iodata(*this, "iodata") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_iodata;

	/* input-related */
	UINT16        m_defender_sensor;
	UINT16        m_shutter_sensor;
	UINT16        m_irq_reg;

	/* devices */
	DECLARE_READ16_MEMBER(drill_io_r);
	DECLARE_WRITE16_MEMBER(drill_io_w);
	DECLARE_WRITE16_MEMBER(sensors_w);
	DECLARE_READ16_MEMBER(drill_irq_r);
	DECLARE_WRITE16_MEMBER(drill_irq_w);
	DECLARE_DRIVER_INIT(drill);
	DECLARE_MACHINE_START(drill);
	DECLARE_MACHINE_RESET(drill);
	INTERRUPT_GEN_MEMBER(drill_vblank_irq);
	//INTERRUPT_GEN_MEMBER(drill_device_irq);
	void tile_decode();
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	#ifdef UNUSED_FUNCTION
	enum
	{
		TIMER_SHUTTER_REQ,
		TIMER_DEFENDER_REQ
	};

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	#endif
};


READ16_MEMBER(_2mindril_state::drill_io_r)
{
//  if (offset * 2 == 0x4)
	/*popmessage("PC=%08x %04x %04x %04x %04x %04x %04x %04x %04x", space.device().safe_pc(), m_iodata[0/2], m_iodata[2/2], m_iodata[4/2], m_iodata[6/2],
	                                    m_iodata[8/2], m_iodata[0xa/2], m_iodata[0xc/2], m_iodata[0xe/2]);*/

	switch(offset)
	{
		case 0x0/2: return ioport("DSW")->read();
		case 0x2/2:
		{
			int arm_pwr = ioport("IN0")->read();//throw
			//popmessage("PC=%08x %02x",space.device().safe_pc(),arm_pwr);

			if(arm_pwr > 0xe0) return ~0x1800;
			if(arm_pwr > 0xc0) return ~0x1400;
			if(arm_pwr > 0x80) return ~0x1200;
			if(arm_pwr > 0x40) return ~0x1000;
			else return ~0x0000;
		}
		case 0x4/2: return (m_defender_sensor) | (m_shutter_sensor);
		case 0xe/2: return ioport("IN2")->read();//coins
//      default:  printf("PC=%08x [%04x] -> %04x R\n", space.device().safe_pc(), offset * 2, m_iodata[offset]);
	}

	return 0xffff;
}

WRITE16_MEMBER(_2mindril_state::drill_io_w)
{
	COMBINE_DATA(&m_iodata[offset]);

	switch(offset)
	{
		case 0x8/2:
			coin_counter_w(machine(), 0, m_iodata[offset] & 0x0400);
			coin_counter_w(machine(), 1, m_iodata[offset] & 0x0800);
			coin_lockout_w(machine(), 0, ~m_iodata[offset] & 0x0100);
			coin_lockout_w(machine(), 1, ~m_iodata[offset] & 0x0200);
			break;
	}

//  if(data != 0 && offset != 8)
//  printf("PC=%08x [%04x] <- %04x W\n", space.device().safe_pc(), offset * 2, data);
}

/*
    PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )//up sensor <- shutter
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )//down sensor
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )//left sensor <-defender
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )//right sensor
    PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
    PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
*/
#ifdef UNUSED_FUNCTION
void _2mindril_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SHUTTER_REQ:
			m_shutter_sensor = param;
			break;
	case TIMER_DEFENDER_REQ:
			m_defender_sensor = param;
			break;
	default:
			assert_always(FALSE, "Unknown id in _2mindril_state::device_timer");
	}
}
#endif

WRITE16_MEMBER(_2mindril_state::sensors_w)
{
	/*---- xxxx ---- ---- select "lamps" (guess)*/
	/*---- ---- ---- -x-- lamp*/
	if (data & 1)
	{
		//timer_set( attotime::from_seconds(2), TIMER_SHUTTER_REQ, 0x100);
		m_shutter_sensor = 0x100;
	}
	else if (data & 2)
	{
		//timer_set( attotime::from_seconds(2), TIMER_SHUTTER_REQ, 0x200);
		m_shutter_sensor = 0x200;
	}

	if (data & 0x1000 || data & 0x4000)
	{
		//timer_set( attotime::from_seconds(2), TIMER_DEFENDER_REQ, 0x800);
		m_defender_sensor = 0x800;
	}
	else if (data & 0x2000 || data & 0x8000)
	{
		//timer_set( attotime::from_seconds(2), TIMER_DEFENDER_REQ, 0x400);
		m_defender_sensor = 0x400;
	}
}

READ16_MEMBER(_2mindril_state::drill_irq_r)
{
	return m_irq_reg;
}

WRITE16_MEMBER(_2mindril_state::drill_irq_w)
{
	/*
	(note: could rather be irq mask)
	---- ---- ---x ---- irq lv 5 ack, 0->1 latch
	---- ---- ---- x--- irq lv 4 ack, 0->1 latch
	---- ---- -??- -??? connected to the other levels?
	*/
	if(((m_irq_reg & 8) == 0) && data & 8)
		m_maincpu->set_input_line(4, CLEAR_LINE);

	if(((m_irq_reg & 0x10) == 0) && data & 0x10)
		m_maincpu->set_input_line(5, CLEAR_LINE);

	if(data & 0xffe7)
		printf("%04x\n",data);

	COMBINE_DATA(&m_irq_reg);
}

static ADDRESS_MAP_START( drill_map, AS_PROGRAM, 16, _2mindril_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x300000, 0x3000ff) AM_RAM
	AM_RANGE(0x400000, 0x40ffff) AM_READWRITE(f3_spriteram_r,f3_spriteram_w)
	AM_RANGE(0x410000, 0x41bfff) AM_READWRITE(f3_pf_data_r,f3_pf_data_w)
	AM_RANGE(0x41c000, 0x41dfff) AM_READWRITE(f3_videoram_r,f3_videoram_w)
	AM_RANGE(0x41e000, 0x41ffff) AM_READWRITE(f3_vram_r,f3_vram_w)
	AM_RANGE(0x420000, 0x42ffff) AM_READWRITE(f3_lineram_r,f3_lineram_w)
	AM_RANGE(0x430000, 0x43ffff) AM_READWRITE(f3_pivot_r,f3_pivot_w)
	AM_RANGE(0x460000, 0x46000f) AM_WRITE(f3_control_0_w)
	AM_RANGE(0x460010, 0x46001f) AM_WRITE(f3_control_1_w)
	AM_RANGE(0x500000, 0x501fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x502022, 0x502023) AM_WRITENOP //countinously switches between 0 and 2
	AM_RANGE(0x600000, 0x600007) AM_DEVREADWRITE8("ymsnd", ym2610_device, read, write, 0x00ff)
	AM_RANGE(0x60000c, 0x60000d) AM_READWRITE(drill_irq_r,drill_irq_w)
	AM_RANGE(0x60000e, 0x60000f) AM_RAM // unknown purpose, zeroed at start-up and nothing else
	AM_RANGE(0x700000, 0x70000f) AM_READWRITE(drill_io_r,drill_io_w) AM_SHARE("iodata") // i/o
	AM_RANGE(0x800000, 0x800001) AM_WRITE(sensors_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( drill )
	PORT_START("DSW")//Dip-Switches
	PORT_DIPNAME( 0x0001, 0x0001, "DSW" )
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

	PORT_START("IN0")//sensors
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("IN1")
	PORT_DIPNAME( 0x0001, 0x0000, "IN1" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )//up sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )//down sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )//left sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )//right sensor
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("IN2")//coins
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select SW-1")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select SW-2")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Select SW-3")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Select SW-4")
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	256,
	4,
	{ 0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout pivotlayout =
{
	8,8,
	2048,
	4,
	{ 0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spriteram_layout =
{
	16,16,
	RGN_FRAC(1,2),
	6,  /* Palettes have 4 bpp indexes despite up to 6 bpp data */
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, 0, 1, 2, 3 },
	{
	4, 0, 12, 8,
	16+4, 16+0, 16+12, 16+8,
	32+4, 32+0, 32+12, 32+8,
	48+4, 48+0, 48+12, 48+8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,2),
	6,  /* Palettes have 4 bpp indexes despite up to 6 bpp data */
	{ RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3, 0, 1, 2, 3 },
	{
	4, 0, 16+4, 16+0,
	8+4, 8+0, 24+4, 24+0,
	32+4, 32+0, 48+4, 48+0,
	40+4, 40+0, 56+4, 56+0,
	},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( 2mindril )
	GFXDECODE_ENTRY( nullptr,   0x000000, charlayout,       0x0000, 0x0400>>4 ) /* Dynamically modified */
	GFXDECODE_ENTRY( "gfx2", 0x000000, tile_layout,      0x0000, 0x2000>>4 ) /* Tiles area */
	GFXDECODE_ENTRY( "gfx1", 0x000000, spriteram_layout, 0x1000, 0x1000>>4 ) /* Sprites area */
	GFXDECODE_ENTRY( nullptr,   0x000000, pivotlayout,      0x0000,  0x400>>4 ) /* Dynamically modified */
GFXDECODE_END


INTERRUPT_GEN_MEMBER(_2mindril_state::drill_vblank_irq)
{
	device.execute().set_input_line(4, ASSERT_LINE);
}

#if 0
INTERRUPT_GEN_MEMBER(_2mindril_state::drill_device_irq)
{
	device.execute().set_input_line(5, ASSERT_LINE);
}
#endif

/* WRONG,it does something with 60000c & 700002,likely to be called when the player throws the ball.*/
WRITE_LINE_MEMBER(_2mindril_state::irqhandler)
{
//  m_maincpu->set_input_line(5, state ? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_START_MEMBER(_2mindril_state,drill)
{
	save_item(NAME(m_defender_sensor));
	save_item(NAME(m_shutter_sensor));
	save_item(NAME(m_irq_reg));
}

MACHINE_RESET_MEMBER(_2mindril_state,drill)
{
	m_defender_sensor = 0;
	m_shutter_sensor = 0;
	m_irq_reg = 0;
}

static MACHINE_CONFIG_START( drill, _2mindril_state )

	MCFG_CPU_ADD("maincpu", M68000, 16000000 )
	MCFG_CPU_PROGRAM_MAP(drill_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", _2mindril_state,  drill_vblank_irq)
	//MCFG_CPU_PERIODIC_INT_DRIVER(_2mindril_state, drill_device_irq, 60)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 2mindril)

	MCFG_MACHINE_START_OVERRIDE(_2mindril_state,drill)
	MCFG_MACHINE_RESET_OVERRIDE(_2mindril_state,drill)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* inaccurate, same as Taito F3? (needs screen raw params anyway) */
	MCFG_SCREEN_SIZE(40*8+48*2, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(46, 40*8-1 + 46, 24, 24+224-1)
	MCFG_SCREEN_UPDATE_DRIVER(_2mindril_state, screen_update_f3)
	MCFG_SCREEN_VBLANK_DRIVER(_2mindril_state, screen_eof_f3)

	MCFG_PALETTE_ADD("palette", 0x2000)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(_2mindril_state,f3)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610B, 16000000/2)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(_2mindril_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "lspeaker",  0.25)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.25)
	MCFG_SOUND_ROUTE(1, "lspeaker",  1.0)
	MCFG_SOUND_ROUTE(2, "rspeaker", 1.0)
MACHINE_CONFIG_END


ROM_START( 2mindril )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "d58-38.ic11", 0x00000, 0x40000, CRC(c58e8e4f) SHA1(648db679c3bfb5de1cd6c1b1217773a2fe56f11b) )
	ROM_LOAD16_BYTE( "d58-37.ic9",  0x00001, 0x40000, CRC(19e5cc3c) SHA1(04ac0eef893c579fe90d91d7fd55c5741a2b7460) )

	ROM_REGION( 0x200000, "ymsnd", 0 ) /* Samples */
	ROM_LOAD( "d58-11.ic31", 0x000000, 0x200000,  CRC(dc26d58d) SHA1(cffb18667da18f5367b02af85a2f7674dd61ae97) )

	ROM_REGION( 0x800000, "gfx1", ROMREGION_ERASE00 )

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "d58-09.ic28", 0x000001, 0x200000, CRC(d8f6a86a) SHA1(d6b2ec309e21064574ee63e025ae4716b1982a98) )
	ROM_LOAD16_BYTE( "d58-08.ic27", 0x000000, 0x200000, CRC(9f5a3f52) SHA1(7b696bd823819965b974c853cebc1660750db61e) )
	ROM_LOAD( "d58-10.ic29", 0x400000, 0x200000, CRC(74c87e08) SHA1(f39b3a64f8338ccf5ca6eb76cee92a10fe0aad8f) )
	ROM_RELOAD(              0x600000, 0x200000 )
ROM_END

void _2mindril_state::tile_decode()
{
	UINT8 lsb,msb;
	UINT32 offset,i;
	UINT8 *gfx = memregion("gfx2")->base();
	int size=memregion("gfx2")->bytes();
	int data;

	/* Setup ROM formats:

	    Some games will only use 4 or 5 bpp sprites, and some only use 4 bpp tiles,
	    I don't believe this is software or prom controlled but simply the unused data lines
	    are tied low on the game board if unused.  This is backed up by the fact the palette
	    indices are always related to 4 bpp data, even in 6 bpp games.

	    Most (all?) games with 5bpp tiles have the sixth bit set. Also, in Arabian Magic
	    sprites 1200-120f contain 6bpp data which is probably bogus.
	    video_start clears the fifth and sixth bit of the decoded graphics according
	    to the bit depth specified in f3_config_table.

	*/

	offset = size/2;
	for (i = size/2+size/4; i<size; i+=2)
	{
		/* Expand 2bits into 4bits format */
		lsb = gfx[i+1];
		msb = gfx[i];

		gfx[offset+0]=((msb&0x02)<<3) | ((msb&0x01)>>0) | ((lsb&0x02)<<4) | ((lsb&0x01)<<1);
		gfx[offset+2]=((msb&0x08)<<1) | ((msb&0x04)>>2) | ((lsb&0x08)<<2) | ((lsb&0x04)>>1);
		gfx[offset+1]=((msb&0x20)>>1) | ((msb&0x10)>>4) | ((lsb&0x20)<<0) | ((lsb&0x10)>>3);
		gfx[offset+3]=((msb&0x80)>>3) | ((msb&0x40)>>6) | ((lsb&0x80)>>2) | ((lsb&0x40)>>5);

		offset+=4;
	}

	gfx = memregion("gfx1")->base();
	size=memregion("gfx1")->bytes();

	offset = size/2;
	for (i = size/2+size/4; i<size; i++)
	{
		int d1,d2,d3,d4;

		/* Expand 2bits into 4bits format */
		data = gfx[i];
		d1 = (data>>0) & 3;
		d2 = (data>>2) & 3;
		d3 = (data>>4) & 3;
		d4 = (data>>6) & 3;

		gfx[offset] = (d1<<2) | (d2<<6);
		offset++;

		gfx[offset] = (d3<<2) | (d4<<6);
		offset++;
	}
}

DRIVER_INIT_MEMBER(_2mindril_state,drill)
{
	m_f3_game=TMDRILL;
	tile_decode();
}

GAME( 1993, 2mindril,    0,        drill,    drill, _2mindril_state,    drill, ROT0,  "Taito", "Two Minute Drill", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_MECHANICAL)
