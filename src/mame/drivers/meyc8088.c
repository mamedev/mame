/****************************************************************

  Meyco 8088 based hardware

  i8088 CPU @ 5MHz (15MHz XTAL + i8284A clock generator),
  3 x 8KB EPROM (max 4), 3 x 8KB RAM (max 4), 2KB battery RAM,
  2 x i8155, optional i8251A + RS232 for factory debug

  driver by MAME team
  also thanks to Darrell Hal Smith, Kevin Mullins


  To initialize battery RAM, go into Meter Read mode (F1 -> 9),
  and then press the Meter Read + Reset buttons (9 + 0).

  If a game is not turned off properly, eg. exiting MAME
  in mid-game, it may run faulty on the next boot.
  Enable the Night Switch to prevent this.


  TODO:
  - coincounters/hopper

****************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/i8155.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/resnet.h"

#include "gldarrow.lh"


class meyc8088_state : public driver_device
{
public:
	meyc8088_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_vram(*this, "vram"),
		m_heartbeat(*this, "heartbeat")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_vram;
	required_device<timer_device> m_heartbeat;

	UINT8 m_status;
	UINT8 m_common;

	DECLARE_WRITE8_MEMBER(drive_w);
	DECLARE_WRITE8_MEMBER(video5_flip_w);
	DECLARE_READ8_MEMBER(video5_flip_r);
	DECLARE_WRITE8_MEMBER(screen_flip_w);
	DECLARE_READ8_MEMBER(screen_flip_r);
	DECLARE_READ8_MEMBER(meyc8088_input_r);
	DECLARE_READ8_MEMBER(meyc8088_status_r);
	DECLARE_WRITE8_MEMBER(meyc8088_lights1_w);
	DECLARE_WRITE8_MEMBER(meyc8088_lights2_w);
	DECLARE_WRITE8_MEMBER(meyc8088_common_w);

	DECLARE_WRITE_LINE_MEMBER(meyc8088_sound_out);
};


/***************************************************************************

  Video

***************************************************************************/

/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:
  (even and uneven pins were switched around in the schematics)

  bit 7 -- N/C
        -- 820 ohm resistor  -- GREEN
        --                   -- GREEN
        -- 820 ohm resistor  -- BLUE
        --                   -- BLUE
        -- 820 ohm resistor  -- RED
        --                   -- RED
  bit 0 -- N/C

  plus 330 ohm pullup resistors on all lines

***************************************************************************/

static const res_net_decode_info meyc8088_decode_info =
{
	1,		// there may be two proms needed to construct color
	0, 31,	// start/end
	//  R,   G,   B,
	{   0,   0,   0, },		// offsets
	{   1,   5,   3, },		// shifts
	{0x03,0x03,0x03, }	    // masks
};

static const res_net_info meyc8088_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_OPEN_COL,
	{
		{ RES_NET_AMP_NONE, 330, 0, 2, { 1, 820 } },
		{ RES_NET_AMP_NONE, 330, 0, 2, { 1, 820 } },
		{ RES_NET_AMP_NONE, 330, 0, 2, { 1, 820 } }
	}
};

PALETTE_INIT( meyc8088 )
{
	const UINT8 *color_prom = machine.root_device().memregion("proms")->base();
	rgb_t *rgb;

	rgb = compute_res_net_all(machine, color_prom, &meyc8088_decode_info, &meyc8088_net_info);
	palette_set_colors(machine, 0, rgb, 32);
	auto_free(machine, rgb);
}

static SCREEN_UPDATE_IND16( meyc8088 )
{
	meyc8088_state *state = screen.machine().driver_data<meyc8088_state>();
	UINT8 v[5];
	v[4] = state->m_status << 2 & 0x10; // video5: color prom d4

	if (~state->m_status & 2)
	{
		// screen off
		bitmap.fill(v[4]);
		return 0;
	}

	for (offs_t offs = 0x800; offs < 0x4000; offs+=2)
	{
		UINT8 y = (offs-0x800) >> 6;
		UINT8 x = (offs-0x800) << 2;

		v[0] = state->m_vram[offs|0x0000]; // video1: color prom d0
		v[1] = state->m_vram[offs|0x0001]; // video2: color prom d1
		v[2] = state->m_vram[offs|0x4000]; // video3: color prom d2
		v[3] = state->m_vram[offs|0x4001]; // video4: color prom d3

		for (int i = 0; i < 8; i++)
			bitmap.pix16(y, x | i) = ((v[0] << i) >> 7 & 1) | ((v[1] << i) >> 6 & 2) | ((v[2] << i) >> 5 & 4) | ((v[3] << i) >> 4 & 8) | v[4];
	}

	return 0;
}

static SCREEN_VBLANK( meyc8088 )
{
	meyc8088_state *state = screen.machine().driver_data<meyc8088_state>();

	// INTR on LC255 (pulses at start and end of vblank), INTA hardwired to $20
	generic_pulse_irq_line_and_vector(state->m_maincpu, 0, 0x20, 1);
}


/***************************************************************************

  I/O

***************************************************************************/

TIMER_DEVICE_CALLBACK( heartbeat_callback )
{
	meyc8088_state *state = timer.machine().driver_data<meyc8088_state>();
	state->m_status |= 0x20;
}

WRITE8_MEMBER(meyc8088_state::drive_w)
{
	// drivers go into high-impedance state ~100ms after write (LS374 /OC)
	m_status &= ~0x20;
	m_heartbeat->adjust(attotime::from_msec(100));

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
	AM_RANGE(0x00000, 0x007ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x70000, 0x77fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xb0000, 0xb00ff) AM_DEVREADWRITE("i8155_2", i8155_device, memory_r, memory_w)
	AM_RANGE(0xb0800, 0xb0807) AM_DEVREADWRITE("i8155_2", i8155_device, io_r, io_w)
	AM_RANGE(0xb1000, 0xb10ff) AM_DEVREADWRITE("i8155_1", i8155_device, memory_r, memory_w)
	AM_RANGE(0xb1800, 0xb1807) AM_DEVREADWRITE("i8155_1", i8155_device, io_r, io_w)
	AM_RANGE(0xb2000, 0xb2000) AM_WRITE(drive_w)
	AM_RANGE(0xb3000, 0xb3000) AM_NOP // i8251A data (debug related, unpopulated on sold boards)
	AM_RANGE(0xb3800, 0xb3800) AM_NOP // "
	AM_RANGE(0xb4000, 0xb4000) AM_READWRITE(screen_flip_r, screen_flip_w)
	AM_RANGE(0xb5000, 0xb5000) AM_READWRITE(video5_flip_r, video5_flip_w)
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END


READ8_MEMBER(meyc8088_state::meyc8088_input_r)
{
	UINT8 ret = 0xff;

	// multiplexed switch inputs
	if (~m_common & 1) ret &= ioport("C0")->read_safe(0); // bit switches
	if (~m_common & 2) ret &= ioport("C1")->read_safe(0); // control switches
	if (~m_common & 4) ret &= ioport("C2")->read_safe(0); // light switches
	if (~m_common & 8) ret &= ioport("C3")->read_safe(0); // light switches

	return ret;
}

READ8_MEMBER(meyc8088_state::meyc8088_status_r)
{
	// d0: /CR2
	// d1: screen on
	// d2: video5
	// d3: N/C
	// d4: battery ok
	// d5: /drive on
	return (m_status & 0x27) | 0x18;
}


WRITE8_MEMBER(meyc8088_state::meyc8088_lights1_w)
{
	// lite 1-8
	for (int i = 0; i < 8; i++)
		output_set_lamp_value(i, ~data >> i & 1);
}

WRITE8_MEMBER(meyc8088_state::meyc8088_lights2_w)
{
	// lite 9-16
	for (int i = 0; i < 8; i++)
		output_set_lamp_value(i + 8, ~data >> i & 1);
}

WRITE8_MEMBER(meyc8088_state::meyc8088_common_w)
{
	// d0: /CR2
	m_status = (m_status & ~1) | (data & 1);

	// d1: battery on
	m_status = (m_status & ~0x10) | (data << 3 & 0x10);

	// d2-d5: /common
	m_common = data >> 2 & 0xf;
}

WRITE_LINE_MEMBER(meyc8088_state::meyc8088_sound_out)
{
	machine().device<dac_device>("dac")->write_signed8(state ? 0x7f : 0);
}


static const i8155_interface i8155_intf[2] =
{
	{
		// all ports set to input
		DEVCB_DRIVER_MEMBER(meyc8088_state,meyc8088_input_r),
		DEVCB_NULL,
		DEVCB_INPUT_PORT("SW"), // filtered switch inputs
		DEVCB_NULL,
		DEVCB_DRIVER_MEMBER(meyc8088_state,meyc8088_status_r),
		DEVCB_NULL,
		DEVCB_NULL // i8251A trigger txc/rxc (debug related, unpopulated on sold boards)
	},
	{
		// all ports set to output
		DEVCB_NULL,
		DEVCB_DRIVER_MEMBER(meyc8088_state,meyc8088_lights2_w),
		DEVCB_NULL,
		DEVCB_DRIVER_MEMBER(meyc8088_state,meyc8088_lights1_w),
		DEVCB_NULL,
		DEVCB_DRIVER_MEMBER(meyc8088_state,meyc8088_common_w),
		DEVCB_DRIVER_LINE_MEMBER(meyc8088_state,meyc8088_sound_out)
	}
};


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( gldarrow )
	PORT_START("SW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) // coin4
	PORT_BIT( 0x78, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // hopper coin switch?

	PORT_START("C0")
	PORT_DIPNAME( 0x03, 0x00, "Payout Percentage" )		PORT_DIPLOCATION("BSW:1,2")
	PORT_DIPSETTING(    0x03, "85%")
	PORT_DIPSETTING(    0x02, "88%")
	PORT_DIPSETTING(    0x01, "90%")
	PORT_DIPSETTING(    0x00, "93%")
	PORT_DIPNAME( 0x04, 0x00, "Bit Switch 3" )			PORT_DIPLOCATION("BSW:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bonus Award" )			PORT_DIPLOCATION("BSW:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Coinage ) )		PORT_DIPLOCATION("BSW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x20, 0x00, "Bit Switch 6" )			PORT_DIPLOCATION("BSW:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Bit Switch 7" )			PORT_DIPLOCATION("BSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "BSW:8" )

	PORT_START("C1")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Meter Read")
	PORT_DIPNAME( 0x80, 0x80, "Night Switch" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("C2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("C3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( meyc8088, meyc8088_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, (XTAL_15MHz / 3) * 0.95) // NOTE: underclocked to prevent errors on diagnostics, MAME i8088 cycle timing is probably inaccurate
	MCFG_CPU_PROGRAM_MAP(meyc8088_map)

	MCFG_I8155_ADD("i8155_1", XTAL_15MHz / (3*1), i8155_intf[0])
	MCFG_I8155_ADD("i8155_2", XTAL_15MHz / (3*32), i8155_intf[1])

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_ADD("heartbeat", heartbeat_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_15MHz/3, 320, 0, 256, 261, 0, 224)
	MCFG_SCREEN_UPDATE_STATIC(meyc8088)
	MCFG_SCREEN_VBLANK_STATIC(meyc8088)

	MCFG_PALETTE_LENGTH(32)
	MCFG_PALETTE_INIT(meyc8088)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


ROM_START( gldarrow )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "3.14h",   0x0fa000, 0x002000, CRC(a4acc6df) SHA1(b25f2cf8154932834100615e2e9c44ef47a15fea) )
	ROM_LOAD( "2.13h",   0x0fc000, 0x002000, CRC(595e380d) SHA1(6f8e58f646106d33cb651d97ca6a1133f7b05373) )
	ROM_LOAD( "1.12h",   0x0fe000, 0x002000, CRC(71bd0e39) SHA1(15345f5726cd33ecb1b2da05f2852b6cc3ac7747) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "prom.2c", 0x00, 0x20, CRC(2839bb14) SHA1(c9acdb3ae00c2f9344aedaf77c0f4e860a3184fc) ) // M3-7602-5 color prom
ROM_END


GAMEL(1984, gldarrow, 0,        meyc8088, gldarrow, meyc8088_state, 0, ROT0,  "Meyco Games, Inc.", "Golden Arrow (Standard G8-03)", 0, layout_gldarrow )
