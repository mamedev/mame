// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Roberto Fresca
/* Notes: DJH 04 Jan 2008

  fixed gridiron079gre (shared access to spriteram was broken)

  The inputs seem to be a hacky mess (although there was reportedly a
  hardware joystick hack for tehkanwc via plugin logic subboard, is this
  attempting to simulate it?

  Also there is a hack to reset the sound CPU...

*/

/***************************************************************************

Tehkan World Cup - (c) Tehkan 1985


Ernesto Corvi
ernesto@imagina.com

Roberto Juan Fresca
robbiex@rocketmail.com

TODO:
- dip switches and input ports for Gridiron and Tee'd Off


Additional notes (Steph 2002.01.14)

Even if there is NO "screen flipping" for 'tehkanwc' and 'gridiron', there are writes
to 0xfe60 and 0xfe70 of the main CPU with 00 ...

About 'teedoff' :

The main problem with that game is that it should sometimes jumps into shared memory
(see 'init_teedoff' function below) depending on a value that is supposed to be
in the palette RAM !

Palette RAM is reset here (main CPU) :

5D15: ED 57       ld   a,i
5D17: CB FF       set  7,a
5D19: ED 47       ld   i,a
5D1B: AF          xor  a
5D1C: 21 00 D8    ld   hl,$D800
5D1F: 01 80 0C    ld   bc,$0C80
5D22: 77          ld   (hl),a
5D23: 23          inc  hl
5D24: 0D          dec  c
5D25: 20 FB       jr   nz,$5D22
5D27: 0E 80       ld   c,$80
5D29: 10 F7       djnz $5D22
....

Then it is filled here (main CPU) :

5D50: 21 C4 70    ld   hl,$70C4
5D53: 11 00 D8    ld   de,$D800
5D56: 01 00 02    ld   bc,$0200
5D59: ED B0       ldir
5D5B: 21 C4 72    ld   hl,$72C4
5D5E: 01 00 01    ld   bc,$0100
5D61: ED B0       ldir
5D63: C9          ret

0x72c4 is in ROM and it's ALWAYS 00 !

Another place where the palette is filled is here (sub CPU) :

16AC: 21 06 1D    ld   hl,$1D06
16AF: 11 00 DA    ld   de,$DA00
16B2: 01 C0 00    ld   bc,$00C0
16B5: ED B0       ldir

But here again, 0x1d06 is in ROM and it's ALWAYS 00 !

So the "jp z" instruction at 0x0238 of the main CPU will ALWAYS jump
in shared memory when NO code seems to be written !

TO DO :

  - Check MEMORY_* definitions (too many M?A_NOP areas)
  - Check sound in all games (too many messages like this in the .log file :
    'Warning: sound latch 2 written before being read')
  - Figure out the controls in 'tehkanwc' (they are told to be better in MAME 0.34)
  - Figure out the controls in 'teedoff'
  - Confirm "Difficulty" Dip Switch in 'teedoff'

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "gridiron.lh"
#include "includes/tehkanwc.h"


void tehkanwc_state::machine_start()
{
	save_item(NAME(m_track0));
	save_item(NAME(m_track1));
	save_item(NAME(m_msm_data_offs));
	save_item(NAME(m_toggle));
}

WRITE8_MEMBER(tehkanwc_state::sub_cpu_halt_w)
{
	if (data)
		m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}


READ8_MEMBER(tehkanwc_state::track_0_r)
{
	int joy;

	joy = ioport("FAKE")->read() >> (2 * offset);
	if (joy & 1) return -63;
	if (joy & 2) return 63;
	return ioport(offset ? "P1Y" : "P1X")->read() - m_track0[offset];
}

READ8_MEMBER(tehkanwc_state::track_1_r)
{
	int joy;

	joy = ioport("FAKE")->read() >> (4 + 2 * offset);
	if (joy & 1) return -63;
	if (joy & 2) return 63;
	return ioport(offset ? "P2Y" : "P2X")->read() - m_track1[offset];
}

WRITE8_MEMBER(tehkanwc_state::track_0_reset_w)
{
	/* reset the trackball counters */
	m_track0[offset] = ioport(offset ? "P1Y" : "P1X")->read() + data;
}

WRITE8_MEMBER(tehkanwc_state::track_1_reset_w)
{
	/* reset the trackball counters */
	m_track1[offset] = ioport(offset ? "P2Y" : "P2X")->read() + data;
}



WRITE8_MEMBER(tehkanwc_state::sound_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

void tehkanwc_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESET:
		m_audiocpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in tehkanwc_state::device_timer");
	}
}

WRITE8_MEMBER(tehkanwc_state::sound_answer_w)
{
	soundlatch2_byte_w(space, 0, data);

	/* in Gridiron, the sound CPU goes in a tight loop after the self test, */
	/* probably waiting to be reset by a watchdog */
	if (space.device().safe_pc() == 0x08bc) timer_set(attotime::from_seconds(1), TIMER_RESET);
}


/* Emulate MSM sound samples with counters */


READ8_MEMBER(tehkanwc_state::portA_r)
{
	return m_msm_data_offs & 0xff;
}

READ8_MEMBER(tehkanwc_state::portB_r)
{
	return (m_msm_data_offs >> 8) & 0xff;
}

WRITE8_MEMBER(tehkanwc_state::portA_w)
{
	m_msm_data_offs = (m_msm_data_offs & 0xff00) | data;
}

WRITE8_MEMBER(tehkanwc_state::portB_w)
{
	m_msm_data_offs = (m_msm_data_offs & 0x00ff) | (data << 8);
}

WRITE8_MEMBER(tehkanwc_state::msm_reset_w)
{
	m_msm->reset_w(data ? 0 : 1);
}

WRITE_LINE_MEMBER(tehkanwc_state::adpcm_int)
{
	UINT8 *SAMPLES = memregion("adpcm")->base();
	int msm_data = SAMPLES[m_msm_data_offs & 0x7fff];

	if (m_toggle == 0)
		m_msm->data_w((msm_data >> 4) & 0x0f);
	else
	{
		m_msm->data_w(msm_data & 0x0f);
		m_msm_data_offs++;
	}

	m_toggle ^= 1;
}

/* End of MSM with counters emulation */



static ADDRESS_MAP_START( main_mem, AS_PROGRAM, 8, tehkanwc_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd800, 0xddff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xde00, 0xdfff) AM_RAM AM_SHARE("share5") /* unused part of the palette RAM, I think? Gridiron uses it */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xe800, 0xebff) AM_RAM AM_SHARE("spriteram") /* sprites */
	AM_RANGE(0xec00, 0xec01) AM_RAM_WRITE(scroll_x_w)
	AM_RANGE(0xec02, 0xec02) AM_RAM_WRITE(scroll_y_w)
	AM_RANGE(0xf800, 0xf801) AM_READWRITE(track_0_r, track_0_reset_w) /* track 0 x/y */
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("SYSTEM") AM_WRITE(gridiron_led0_w)
	AM_RANGE(0xf803, 0xf803) AM_READ_PORT("P1BUT")
	AM_RANGE(0xf806, 0xf806) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf810, 0xf811) AM_READWRITE(track_1_r, track_1_reset_w) /* track 1 x/y */
	AM_RANGE(0xf812, 0xf812) AM_WRITE(gridiron_led1_w)
	AM_RANGE(0xf813, 0xf813) AM_READ_PORT("P2BUT")
	AM_RANGE(0xf820, 0xf820) AM_READ(soundlatch2_byte_r) AM_WRITE(sound_command_w)  /* answer from the sound CPU */
	AM_RANGE(0xf840, 0xf840) AM_READ_PORT("DSW1") AM_WRITE(sub_cpu_halt_w)
	AM_RANGE(0xf850, 0xf850) AM_READ_PORT("DSW2") AM_WRITENOP           /* ?? writes 0x00 or 0xff */
	AM_RANGE(0xf860, 0xf860) AM_READ(watchdog_reset_r) AM_WRITE(flipscreen_x_w)
	AM_RANGE(0xf870, 0xf870) AM_READ_PORT("DSW3") AM_WRITE(flipscreen_y_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_mem, AS_PROGRAM, 8, tehkanwc_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd800, 0xddff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xde00, 0xdfff) AM_RAM AM_SHARE("share5") /* unused part of the palette RAM, I think? Gridiron uses it */
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xe800, 0xebff) AM_RAM AM_SHARE("spriteram") /* sprites */
	AM_RANGE(0xec00, 0xec01) AM_RAM_WRITE(scroll_x_w)
	AM_RANGE(0xec02, 0xec02) AM_RAM_WRITE(scroll_y_w)
	AM_RANGE(0xf860, 0xf860) AM_READ(watchdog_reset_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_mem, AS_PROGRAM, 8, tehkanwc_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8001, 0x8001) AM_WRITE(msm_reset_w)/* MSM51xx reset */
	AM_RANGE(0x8002, 0x8002) AM_WRITENOP    /* ?? written in the IRQ handler */
	AM_RANGE(0x8003, 0x8003) AM_WRITENOP    /* ?? written in the NMI handler */
	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_byte_r) AM_WRITE(sound_answer_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_port, AS_IO, 8, tehkanwc_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x02, 0x02) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( tehkanwc )
	PORT_START("DSW1")  /* DSW1 - Active LOW */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING (   0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING (   0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (   0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (   0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (   0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING (   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING (   0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (   0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (   0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (   0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Start Credits (P1&P2)/Extra" )
	PORT_DIPSETTING (   0x80, "1&1/200%" )
	PORT_DIPSETTING (   0xc0, "1&2/100%" )
	PORT_DIPSETTING (   0x40, "2&2/100%" )
	PORT_DIPSETTING (   0x00, "2&3/67%" )

	PORT_START("DSW2")  /* DSW2 - Active LOW */
	PORT_DIPNAME( 0x03, 0x03, "1P Game Time" )
	PORT_DIPSETTING (   0x00, "2:30" )
	PORT_DIPSETTING (   0x01, "2:00" )
	PORT_DIPSETTING (   0x03, "1:30" )
	PORT_DIPSETTING (   0x02, "1:00" )
	PORT_DIPNAME( 0x7c, 0x7c, "2P Game Time" )
	PORT_DIPSETTING (   0x00, "5:00/3:00 Extra" )
	PORT_DIPSETTING (   0x60, "5:00/2:45 Extra" )
	PORT_DIPSETTING (   0x20, "5:00/2:35 Extra" )
	PORT_DIPSETTING (   0x40, "5:00/2:30 Extra" )
	PORT_DIPSETTING (   0x04, "4:00/2:30 Extra" )
	PORT_DIPSETTING (   0x64, "4:00/2:15 Extra" )
	PORT_DIPSETTING (   0x24, "4:00/2:05 Extra" )
	PORT_DIPSETTING (   0x44, "4:00/2:00 Extra" )
	PORT_DIPSETTING (   0x1c, "3:30/2:15 Extra" )
	PORT_DIPSETTING (   0x7c, "3:30/2:00 Extra" )
	PORT_DIPSETTING (   0x3c, "3:30/1:50 Extra" )
	PORT_DIPSETTING (   0x5c, "3:30/1:45 Extra" )
	PORT_DIPSETTING (   0x08, "3:00/2:00 Extra" )
	PORT_DIPSETTING (   0x68, "3:00/1:45 Extra" )
	PORT_DIPSETTING (   0x28, "3:00/1:35 Extra" )
	PORT_DIPSETTING (   0x48, "3:00/1:30 Extra" )
	PORT_DIPSETTING (   0x0c, "2:30/1:45 Extra" )
	PORT_DIPSETTING (   0x6c, "2:30/1:30 Extra" )
	PORT_DIPSETTING (   0x2c, "2:30/1:20 Extra" )
	PORT_DIPSETTING (   0x4c, "2:30/1:15 Extra" )
	PORT_DIPSETTING (   0x10, "2:00/1:30 Extra" )
	PORT_DIPSETTING (   0x70, "2:00/1:15 Extra" )
	PORT_DIPSETTING (   0x30, "2:00/1:05 Extra" )
	PORT_DIPSETTING (   0x50, "2:00/1:00 Extra" )
	PORT_DIPSETTING (   0x14, "1:30/1:15 Extra" )
	PORT_DIPSETTING (   0x74, "1:30/1:00 Extra" )
	PORT_DIPSETTING (   0x34, "1:30/0:50 Extra" )
	PORT_DIPSETTING (   0x54, "1:30/0:45 Extra" )
	PORT_DIPSETTING (   0x18, "1:00/1:00 Extra" )
	PORT_DIPSETTING (   0x78, "1:00/0:45 Extra" )
	PORT_DIPSETTING (   0x38, "1:00/0:35 Extra" )
	PORT_DIPSETTING (   0x58, "1:00/0:30 Extra" )
	PORT_DIPNAME( 0x80, 0x80, "Game Type" )
	PORT_DIPSETTING (   0x80, "Timer In" )
	PORT_DIPSETTING (   0x00, "Credit In" )

	PORT_START("DSW3")  /* DSW3 - Active LOW */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING (   0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING (   0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x04, "Timer Speed" )
	PORT_DIPSETTING (   0x04, "60/60" )
	PORT_DIPSETTING (   0x00, "55/60" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x08, DEF_STR( On ) )

	PORT_START("P1X")   /* IN0 - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("P1Y")   /* IN0 - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("P1BUT") /* IN0 - BUTTON */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2X")    /* IN1 - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2)

	PORT_START("P2Y")   /* IN1 - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(2)

	PORT_START("P2BUT") /* IN1 - BUTTON */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")    /* IN2 - Active LOW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("FAKE")  /* fake port to emulate trackballs with keyboard */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( gridiron )
	PORT_START("DSW1")  /* DSW1 - Active LOW */
	PORT_DIPNAME( 0x03, 0x03, "Start Credits (P1&P2)/Extra" )
	PORT_DIPSETTING (   0x01, "1&1/200%" )
	PORT_DIPSETTING (   0x03, "1&2/100%" )
//  PORT_DIPSETTING (   0x00, "2&1/200%" )              // Is this setting possible ?
	PORT_DIPSETTING (   0x02, "2&2/100%" )
	/* This Dip Switch only has an effect in a 2 players game.
	   If offense player selects his formation before defense player,
	   defense formation time will be set to 3, 5 or 7 seconds.
	   Check code at 0x3ed9 and table at 0x3f89. */
	PORT_DIPNAME( 0x0c, 0x0c, "Formation Time (Defense)" )
	PORT_DIPSETTING (   0x0c, "Same as Offense" )
	PORT_DIPSETTING (   0x00, "7" )
	PORT_DIPSETTING (   0x08, "5" )
	PORT_DIPSETTING (   0x04, "3" )
	PORT_DIPNAME( 0x30, 0x30, "Timer Speed" )
	PORT_DIPSETTING (   0x30, "60/60" )
	PORT_DIPSETTING (   0x00, "57/60" )
	PORT_DIPSETTING (   0x10, "54/60" )
	PORT_DIPSETTING (   0x20, "50/60" )
	PORT_DIPNAME( 0xc0, 0xc0, "Formation Time (Offense)" )
	PORT_DIPSETTING (   0x00, "25" )
	PORT_DIPSETTING (   0x40, "20" )
	PORT_DIPSETTING (   0xc0, "15" )
	PORT_DIPSETTING (   0x80, "10" )

	PORT_START("DSW2")  /* DSW2 - Active LOW */
	PORT_DIPNAME( 0x03, 0x03, "1P Game Time" )
	PORT_DIPSETTING (   0x00, "2:30" )
	PORT_DIPSETTING (   0x01, "2:00" )
	PORT_DIPSETTING (   0x03, "1:30" )
	PORT_DIPSETTING (   0x02, "1:00" )
	PORT_DIPNAME( 0x7c, 0x7c, "2P Game Time" )
	PORT_DIPSETTING (   0x60, "5:00/3:00 Extra" )
	PORT_DIPSETTING (   0x00, "5:00/2:45 Extra" )
	PORT_DIPSETTING (   0x20, "5:00/2:35 Extra" )
	PORT_DIPSETTING (   0x40, "5:00/2:30 Extra" )
	PORT_DIPSETTING (   0x64, "4:00/2:30 Extra" )
	PORT_DIPSETTING (   0x04, "4:00/2:15 Extra" )
	PORT_DIPSETTING (   0x24, "4:00/2:05 Extra" )
	PORT_DIPSETTING (   0x44, "4:00/2:00 Extra" )
	PORT_DIPSETTING (   0x68, "3:30/2:15 Extra" )
	PORT_DIPSETTING (   0x08, "3:30/2:00 Extra" )
	PORT_DIPSETTING (   0x28, "3:30/1:50 Extra" )
	PORT_DIPSETTING (   0x48, "3:30/1:45 Extra" )
	PORT_DIPSETTING (   0x6c, "3:00/2:00 Extra" )
	PORT_DIPSETTING (   0x0c, "3:00/1:45 Extra" )
	PORT_DIPSETTING (   0x2c, "3:00/1:35 Extra" )
	PORT_DIPSETTING (   0x4c, "3:00/1:30 Extra" )
	PORT_DIPSETTING (   0x7c, "2:30/1:45 Extra" )
	PORT_DIPSETTING (   0x1c, "2:30/1:30 Extra" )
	PORT_DIPSETTING (   0x3c, "2:30/1:20 Extra" )
	PORT_DIPSETTING (   0x5c, "2:30/1:15 Extra" )
	PORT_DIPSETTING (   0x70, "2:00/1:30 Extra" )
	PORT_DIPSETTING (   0x10, "2:00/1:15 Extra" )
	PORT_DIPSETTING (   0x30, "2:00/1:05 Extra" )
	PORT_DIPSETTING (   0x50, "2:00/1:00 Extra" )
	PORT_DIPSETTING (   0x74, "1:30/1:15 Extra" )
	PORT_DIPSETTING (   0x14, "1:30/1:00 Extra" )
	PORT_DIPSETTING (   0x34, "1:30/0:50 Extra" )
	PORT_DIPSETTING (   0x54, "1:30/0:45 Extra" )
	PORT_DIPSETTING (   0x78, "1:00/1:00 Extra" )
	PORT_DIPSETTING (   0x18, "1:00/0:45 Extra" )
	PORT_DIPSETTING (   0x38, "1:00/0:35 Extra" )
	PORT_DIPSETTING (   0x58, "1:00/0:30 Extra" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )      // Check code at 0x14b4
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x80, DEF_STR( On ) )

	PORT_START("DSW3")  /* no DSW3 */
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1X")   /* IN0 - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("P1Y")   /* IN0 - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("P1BUT") /* IN0 - BUTTON */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2X")   /* IN1 - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_START("P2Y")   /* IN1 - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_START("P2BUT") /* IN1 - BUTTON */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")    /* IN2 - Active LOW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("FAKE")  /* no fake port here */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( teedoff )
	PORT_START("DSW1")  /* DSW1 - Active LOW */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING (   0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING (   0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (   0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (   0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (   0x00, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x30, "Balls" )
	PORT_DIPSETTING (   0x30, "5" )
	PORT_DIPSETTING (   0x20, "6" )
	PORT_DIPSETTING (   0x10, "7" )
	PORT_DIPSETTING (   0x00, "8" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )          // Check code at 0x0c5c
	PORT_DIPSETTING (   0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING (   0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )      // Check code at 0x5dd0
	PORT_DIPSETTING (   0x00, DEF_STR( Off ) )
	PORT_DIPSETTING (   0x80, DEF_STR( On ) )

	PORT_START("DSW2")  /* DSW2 - Active LOW */
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x18, 0x18, "Penalty (Over Par)" )        // Check table at 0x2d67
	PORT_DIPSETTING (   0x10, "1/1/2/3/4" )             // +1 / +2 / +3 / +4 / +5 or +6
	PORT_DIPSETTING (   0x18, "1/2/3/3/4" )
	PORT_DIPSETTING (   0x08, "1/2/3/4/4" )
	PORT_DIPSETTING (   0x00, "2/3/3/4/4" )
	PORT_DIPNAME( 0x20, 0x20, "Bonus Balls (Multiple coins)" )
	PORT_DIPSETTING (   0x20, DEF_STR( None ) )
	PORT_DIPSETTING (   0x00, "+1" )
	PORT_DIPNAME( 0xc0, 0xc0, "Difficulty?" )               // Check table at 0x5df9
	PORT_DIPSETTING (   0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING (   0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING (   0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING (   0x00, DEF_STR( Hardest ) )

	PORT_START("DSW3")  /* no DSW3 */
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1X")   /* IN0 - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("P1Y")   /* IN0 - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("P1BUT") /* IN0 - BUTTON */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2X")   /* IN1 - X AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_START("P2Y")   /* IN1 - Y AXIS */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_START("P2BUT") /* IN1 - BUTTON */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")    /* IN2 - Active LOW */
	/* "Coin"  buttons are read from address 0xf802 */
	/* "Start" buttons are read from address 0xf806 */
	/* coin input must be active between 2 and 15 frames to be consistently recognized */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("FAKE")  /* no fake port here */
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END




static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 32 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	512,    /* 512 sprites */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			8*32+1*4, 8*32+0*4, 8*32+3*4, 8*32+2*4, 8*32+5*4, 8*32+4*4, 8*32+7*4, 8*32+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	16,8,   /* 16*8 characters */
	1024,   /* 1024 characters */
	4,  /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the bitplanes are packed in one nibble */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
		32*8+1*4, 32*8+0*4, 32*8+3*4, 32*8+2*4, 32*8+5*4, 32*8+4*4, 32*8+7*4, 32*8+6*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	64*8    /* every char takes 64 consecutive bytes */
};

static GFXDECODE_START( tehkanwc )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 16 ) /* Colors 0 - 255 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 256,  8 ) /* Colors 256 - 383 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   512, 16 ) /* Colors 512 - 767 */
GFXDECODE_END


static MACHINE_CONFIG_START( tehkanwc, tehkanwc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 18432000/4)    /* 18.432000 / 4 */
	MCFG_CPU_PROGRAM_MAP(main_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tehkanwc_state,  irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80, 18432000/4)
	MCFG_CPU_PROGRAM_MAP(sub_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tehkanwc_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 18432000/4)
	MCFG_CPU_PROGRAM_MAP(sound_mem)
	MCFG_CPU_IO_MAP(sound_port)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tehkanwc_state,  irq0_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))   /* 10 CPU slices per frame - seems enough to keep the CPUs in sync */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(tehkanwc_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tehkanwc)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1536000)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(tehkanwc_state, portA_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(tehkanwc_state, portB_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1536000)
	MCFG_AY8910_PORT_A_READ_CB(READ8(tehkanwc_state, portA_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(tehkanwc_state, portB_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(tehkanwc_state, adpcm_int)) /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8KHz               */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_CONFIG_END


DRIVER_INIT_MEMBER(tehkanwc_state,teedoff)
{
	/* Patch to avoid the game jumping in shared memory */

	/* Code at 0x0233 (main CPU) :

	    0233: 3A 00 DA    ld   a,($DA00)
	    0236: CB 7F       bit  7,a
	    0238: CA 00 C8    jp   z,$C800

	   changed to :

	    0233: 3A 00 DA    ld   a,($DA00)
	    0236: CB 7F       bit  7,a
	    0238: 00          nop
	    0239: 00          nop
	    023A: 00          nop
	*/

	UINT8 *ROM = memregion("maincpu")->base();

	ROM[0x0238] = 0x00;
	ROM[0x0239] = 0x00;
	ROM[0x023a] = 0x00;
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tehkanwc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "twc-1.bin",    0x0000, 0x4000, CRC(34d6d5ff) SHA1(72f4d408b8a7766d348f6a229d395e0c98215c40) )
	ROM_LOAD( "twc-2.bin",    0x4000, 0x4000, CRC(7017a221) SHA1(4b4700af0a6ff64f976db369ba4b9d97cee1fd5f) )
	ROM_LOAD( "twc-3.bin",    0x8000, 0x4000, CRC(8b662902) SHA1(13bcd4bf23e34dd7193545561e05bb2cb2c95f9b) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "twc-4.bin",    0x0000, 0x8000, CRC(70a9f883) SHA1(ace04359265271eb37512a89eb0217eb013aecb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "twc-6.bin",    0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "twc-12.bin",   0x00000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "twc-8.bin",    0x00000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) )   /* sprites */
	ROM_LOAD( "twc-7.bin",    0x08000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "twc-11.bin",   0x00000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) )   /* bg tiles */
	ROM_LOAD( "twc-9.bin",    0x08000, 0x8000, CRC(347ef108) SHA1(bb9c2f51d65f28655404e10c3be44d7ade98711b) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "twc-5.bin",    0x0000, 0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) )
ROM_END

/* from a bootleg board, but clearly a different revision of the game code too,
   it still displays the Tehkan copyright etc. so might actually be a legitimate alt revision */

/*
CPUs

    on main board:
        3x Z8400A-Z80ACPU (main, sound)
        2x YM-3-8910 (sound)
        1x OKI M5205 (sound)

    on roms board:
        1x oscillator 18.000MHz
        1x oscillator 4.00000MHz

ROMs:

    on main board:
        5x TMM27128D (1,2,3,5,6)
        1x HN27256G (4)
        1x PAL16L8A (on a small piggyback, not dumped)

    on roms board:
        1x TMM27128D (12)
        4x HN27256G (7,8,9,11)

Notes:

    on main board:
        2x 18x2 edge connectors
        2x 50 pins flat cable connectors to roms board
        1x trimmer (volume)
        2x red LEDs
        2x 8x2 switches dip

    on roms board:
        2x 50 pins flat cable connectors to roms board

*/

ROM_START( tehkanwcb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e-1.3-18.ic32",    0x0000, 0x4000, CRC(ac9d851b) SHA1(38a799cec4f29a88ed22c7a1e35fd2287cee869a) )
	ROM_LOAD( "e-2.3-17.ic31",    0x4000, 0x4000, CRC(65b53d99) SHA1(ea172b2540763d64dc4a238700421cea27138fae) )
	ROM_LOAD( "e-3.3-15.ic30",    0x8000, 0x4000, CRC(12064bfc) SHA1(954b56a548c697927d58b9cb2ecfe32b4db8d769) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "e-4.9-17.ic100",    0x0000, 0x8000, CRC(70a9f883) SHA1(ace04359265271eb37512a89eb0217eb013aecb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e-6.8-3.ic83",    0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "e-12.8c.ic233",   0x00000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) )    /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "e-8.5n.ic191",    0x00000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) )    /* sprites */
	ROM_LOAD( "e-7.5r.ic193",    0x08000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "e-11.8k.ic238",   0x00000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) )    /* bg tiles */
	ROM_LOAD( "e-9.8n.ic240",    0x08000, 0x8000, CRC(347ef108) SHA1(bb9c2f51d65f28655404e10c3be44d7ade98711b) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "e-5.4-3.ic35",    0x0000, 0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) )
ROM_END

/* only rom e1 is changed from above bootleg */
ROM_START( tehkanwcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e1.bin",    0x0000, 0x4000, CRC(7aaaddef) SHA1(10f1f8c86504e5b13a6358b633789f9a27be85e3) )
	ROM_LOAD( "e2.bin",    0x4000, 0x4000, CRC(65b53d99) SHA1(ea172b2540763d64dc4a238700421cea27138fae) )
	ROM_LOAD( "e3.bin",    0x8000, 0x4000, CRC(12064bfc) SHA1(954b56a548c697927d58b9cb2ecfe32b4db8d769) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "e4.bin",    0x0000, 0x8000, CRC(70a9f883) SHA1(ace04359265271eb37512a89eb0217eb013aecb7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "e6.bin",    0x0000, 0x4000, CRC(e3112be2) SHA1(7859e51b4312dc5df01c88e1d97cf608abc7ca72) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "e12.bin",   0x00000, 0x4000, CRC(a9e274f8) SHA1(02b46e1b149a856f0be74a23faaeb792935b66c7) )  /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "e8.bin",    0x00000, 0x8000, CRC(055a5264) SHA1(fe294ba57c2c858952e2fab0be1b8859730846cb) )  /* sprites */
	ROM_LOAD( "e7.bin",    0x08000, 0x8000, CRC(59faebe7) SHA1(85dad90928369601e039467d575750539410fcf6) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "e11.bin",   0x00000, 0x8000, CRC(669389fc) SHA1(a93e8455060ce5242cb65f78e47b4840aa13ab13) )  /* bg tiles */
	ROM_LOAD( "e9.bin",    0x08000, 0x8000, CRC(347ef108) SHA1(bb9c2f51d65f28655404e10c3be44d7ade98711b) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "e5.bin",    0x0000, 0x4000, CRC(444b5544) SHA1(0786d6d9ada7fe49c8ab9751b049095474d2e598) )
ROM_END


ROM_START( gridiron )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gfight1.bin",  0x0000, 0x4000, CRC(51612741) SHA1(a0417a35f0ce51ba7fc81f27b356852a97f52a58) )
	ROM_LOAD( "gfight2.bin",  0x4000, 0x4000, CRC(a678db48) SHA1(5ddcb93b3ed52cec6ba04bb19832ae239b7d2287) )
	ROM_LOAD( "gfight3.bin",  0x8000, 0x4000, CRC(8c227c33) SHA1(c0b58dbebc159ee681aed33c858f5e0172edd75a) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "gfight4.bin",  0x0000, 0x4000, CRC(8821415f) SHA1(772ce0770ed869ebf625d210bc2b9c381b14b7ea) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "gfight5.bin",  0x0000, 0x4000, CRC(92ca3c07) SHA1(580077ca8cf01996b29497187e41a54242de7f50) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "gfight7.bin",  0x00000, 0x4000, CRC(04390cca) SHA1(ff010c0c18ddd1f793b581f0a70bc1b98ef7d21d) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "gfight8.bin",  0x00000, 0x4000, CRC(5de6a70f) SHA1(416aba9de59d46861671c49f8ca33489db1b8634) )   /* sprites */
	ROM_LOAD( "gfight9.bin",  0x04000, 0x4000, CRC(eac9dc16) SHA1(8b3cf87ede8aba45752cc2651a471a5942570037) )
	ROM_LOAD( "gfight10.bin", 0x08000, 0x4000, CRC(61d0690f) SHA1(cd7c81b0e5356bc865380cae5582d6c6b017dfa1) )
	/* 0c000-0ffff empty */

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "gfight11.bin", 0x00000, 0x4000, CRC(80b09c03) SHA1(41627bb6d0f163430c1709a449a42f0f216da852) )   /* bg tiles */
	ROM_LOAD( "gfight12.bin", 0x04000, 0x4000, CRC(1b615eae) SHA1(edfdb4311c5cc314806c8f017f190f7b94f8cd98) )
	/* 08000-0ffff empty */

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "gfight6.bin",  0x0000, 0x4000, CRC(d05d463d) SHA1(30f2bce0ad75c4a7d8344cff16bce27f5e3a3f5d) )
ROM_END

ROM_START( teedoff )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "to-1.bin",     0x0000, 0x4000, CRC(cc2aebc5) SHA1(358e77e53b35dd89fcfdb3b2484b8c4fbc34c1be) )
	ROM_LOAD( "to-2.bin",     0x4000, 0x4000, CRC(f7c9f138) SHA1(2fe56059ef67387b5938bb4751aa2f74a58b04fb) )
	ROM_LOAD( "to-3.bin",     0x8000, 0x4000, CRC(a0f0a6da) SHA1(72390c8dc5519d90e39a660e6ec18861fdbadcc8) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "to-4.bin",     0x0000, 0x8000, CRC(e922cbd2) SHA1(922c030be70150efb760fa81bda0bc54f2ec681a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "to-6.bin",     0x0000, 0x4000, CRC(d8dfe1c8) SHA1(d00a71ad89b530339990780334588f5738c60f25) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "to-12.bin",    0x00000, 0x4000, CRC(4f44622c) SHA1(161c3646a3ec2274bffc957240d47d55a35a8416) )   /* fg tiles */

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "to-8.bin",     0x00000, 0x8000, CRC(363bd1ba) SHA1(c5b7d56b0595712b18351403a9e3325a03de1676) )   /* sprites */
	ROM_LOAD( "to-7.bin",     0x08000, 0x8000, CRC(6583fa5b) SHA1(1041181887350d860c517c0a031ab064a20f5cee) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "to-11.bin",    0x00000, 0x8000, CRC(1ec00cb5) SHA1(0e61eed3d6fc44ff89d8b9e4f558f0989eb8094f) )   /* bg tiles */
	ROM_LOAD( "to-9.bin",     0x08000, 0x8000, CRC(a14347f0) SHA1(00a34ed56ec32336bb524424fcb007d8160163ec) )

	ROM_REGION( 0x8000, "adpcm", 0 )    /* ADPCM samples */
	ROM_LOAD( "to-5.bin",     0x0000, 0x8000, CRC(e5e4246b) SHA1(b2fe2e68fa86163ebe1ef00ecce73fb62cef6b19) )
ROM_END



GAME( 1985, tehkanwc,  0,        tehkanwc, tehkanwc, driver_device, 0,        ROT0,  "Tehkan", "Tehkan World Cup (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tehkanwcb, tehkanwc, tehkanwc, tehkanwc, driver_device, 0,        ROT0,  "Tehkan", "Tehkan World Cup (set 2, bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, tehkanwcc, tehkanwc, tehkanwc, tehkanwc, driver_device, 0,        ROT0,  "bootleg", "Tehkan World Cup (set 3, bootleg)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // aka 'World Cup 85', different inputs?
GAMEL(1985, gridiron,  0,        tehkanwc, gridiron, driver_device, 0,        ROT0,  "Tehkan", "Gridiron Fight", MACHINE_SUPPORTS_SAVE, layout_gridiron )
GAME( 1986, teedoff,   0,        tehkanwc, teedoff, tehkanwc_state,  teedoff,  ROT90, "Tecmo", "Tee'd Off (Japan)", MACHINE_SUPPORTS_SAVE )
