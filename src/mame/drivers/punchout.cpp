// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Punch Out / Super Punch Out / Arm Wrestling

Arm Wrestling runs on about the same hardware, but the video board is different.
The most significant changes are that Punchout has a larger bottom tilemap,
with scrolling, while Arm Wrestling has an additional FG tilemap displayed on
the bottom screen.

driver by Nicola Salmoria

TODO:
- add useless driver config to choose between pink and white color proms
- video raw params - pixel clock is derived from 20.16mhz xtal
- money bag placement might not be 100% correct in Arm Wrestling


main CPU:

0000-bfff ROM
c000-c3ff NVRAM
d000-d7ff RAM
d800-dfff Video RAM (info screen)
e000-e7ff Video RAM (opponent)
e800-efff Video RAM (player)
f000-f03f Background row scroll (low/high couples)
f000-ffff Video RAM (background)

memory mapped ports:
write:
dfe0-dfef ??

dff0      big sprite #1 zoom low 8 bits
dff1      big sprite #1 zoom high 4 bits
dff2      big sprite #1 x pos low 8 bits
dff3      big sprite #1 x pos high 4 bits
dff4      big sprite #1 y pos low 8 bits
dff5      big sprite #1 y pos high bit
dff6      big sprite #1 x flip (bit 0)
dff7      big sprite #1 bit 0: show on top monitor; bit 1: show on bottom monitor

dff8      big sprite #2 x pos low 8 bits
dff9      big sprite #2 x pos high bit
dffa      big sprite #2 y pos low 8 bits
dffb      big sprite #2 y pos high bit
dffc      big sprite #2 x flip (bit 0)
dffd      palette bank (bit 0 = bottom monitor bit 1 = top monitor)

I/O
read:
00        IN0
01        IN1
02        DSW0
03        DSW1 (bit 4: VLM5030 busy signal)

write:
00        to 2A03 #1 IN0 (unpopulated)
01        to 2A03 #1 IN1 (unpopulated)
02        to 2A03 #2 IN0
03        to 2A03 #2 IN1
04        to VLM5030
08        NMI enable + watchdog reset
09        watchdog reset
0a        ? latched into Z80 BUS RQ
0b        to 2A03 #1 and #2 RESET
0c        to VLM5030 RESET
0d        to VLM5030 START
0e        to VLM5030 VCU
0f        enable NVRAM ?

sound CPU:
the sound CPU is a 2A03, which is a modified 6502 with built-in input ports
and two (analog?) outputs. The input ports are memory mapped at 4016-4017;
the outputs are more complicated. The only thing I have found is that 4011
goes straight into a DAC and produces the crowd sounds, but several addresses
in the range 4000-4017 are written to. There are probably three tone generators.

0000-07ff RAM
e000-ffff ROM

read:
4016      IN0
4017      IN1

write:
4000      ? is usually ORed with 90 or 50
4001      ? usually 7f, could be associated with 4000
4002-4003 ? tone #1 freq? (bit 3 of 4003 is always 1, bits 4-7 always 0)
4004      ? is usually ORed with 90 or 50
4005      ? usually 7f, could be associated with 4004
4006-4007 ? tone #2 freq? (bit 3 of 4007 is always 1, bits 4-7 always 0)
4008      ? at one point the max value is cut at 38
400a-400b ? tone #3 freq? (bit 3 of 400b is always 1, bits 4-7 always 0)
400c      ?
400e-400f ?
4011      DAC crowd noise
4015      ?? 00 or 0f
4017      ?? always c0

proms:
If you take a look at the Super Punch-Out Manual, you will notice that it
references different color prom labels. So both boards could use white labels
or pink labels and this is because Nintendo populated the boards with different
parts ie 6J and 6K on the BAK board could be populated with 74ls157 or 74ls158
regardless of PCB revision which would change proms 6E, 6F, and 7F.

***************************************************************************

DIP locations verified for:
    -punchout (manual)
    -spnchout (manual)

***************************************************************************/

#include "emu.h"
#include "includes/punchout.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "rendlay.h"
#include "screen.h"
#include "speaker.h"


/***************************************************************************

  I/O, Memory Maps

***************************************************************************/

// Z80 (main)

WRITE_LINE_MEMBER(punchout_state::nmi_mask_w)
{
	m_nmi_mask = state;
	if (!m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void punchout_state::punchout_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram().share("nvram");
	map(0xd000, 0xd7ff).ram();
	map(0xd800, 0xdfef).ram().w(FUNC(punchout_state::punchout_bg_top_videoram_w)).share("bg_top_videoram");
	map(0xdff0, 0xdff7).ram().share("spr1_ctrlram");
	map(0xdff8, 0xdffc).ram().share("spr2_ctrlram");
	map(0xdffd, 0xdffd).ram().share("palettebank");
	map(0xdffe, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(punchout_state::punchout_spr1_videoram_w)).share("spr1_videoram");
	map(0xe800, 0xefff).ram().w(FUNC(punchout_state::punchout_spr2_videoram_w)).share("spr2_videoram");
	map(0xf000, 0xffff).ram().w(FUNC(punchout_state::punchout_bg_bot_videoram_w)).share("bg_bot_videoram");   // also contains scroll RAM
}


void punchout_state::armwrest_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc3ff).ram().share("nvram");
	map(0xd000, 0xd7ff).ram();
	map(0xd800, 0xdfef).ram().w(FUNC(punchout_state::armwrest_fg_videoram_w)).share("armwrest_fgram");
	map(0xdff0, 0xdff7).ram().share("spr1_ctrlram");
	map(0xdff8, 0xdffc).ram().share("spr2_ctrlram");
	map(0xdffd, 0xdffd).ram().share("palettebank");
	map(0xdffe, 0xdfff).ram();
	map(0xe000, 0xe7ff).ram().w(FUNC(punchout_state::punchout_spr1_videoram_w)).share("spr1_videoram");
	map(0xe800, 0xefff).ram().w(FUNC(punchout_state::punchout_spr2_videoram_w)).share("spr2_videoram");
	map(0xf000, 0xf7ff).ram().w(FUNC(punchout_state::punchout_bg_bot_videoram_w)).share("bg_bot_videoram");
	map(0xf800, 0xffff).ram().w(FUNC(punchout_state::punchout_bg_top_videoram_w)).share("bg_top_videoram");
}


void punchout_state::punchout_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x00, 0x01).nopw(); // the 2A03 #1 is not present
	map(0x02, 0x02).portr("DSW2").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x03, 0x03).portr("DSW1").w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0x04, 0x04).w(m_vlm, FUNC(vlm5030_device::data_w));
	map(0x05, 0x07).nopw(); // spunchout protection
	map(0x08, 0x0f).w("mainlatch", FUNC(ls259_device::write_d0));
}


void punchout_state::punchout_vlm_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rom();
}


// Super Punch-Out!! comes with an extra security PCB that plugs into the Z80 socket
// CHS1-01-EXP, has Z80 CPU, RP5C01, RP5H01

// The RP5C01 features don't seem to be used at all except for very basic protection
// e.g. relying on the masking done by the internal registers.
// The RP5H01 one-time PROM (OTP) is confirmed to be unprogrammed.

uint8_t punchout_state::spunchout_exp_r(offs_t offset)
{
	// d0-d3: D0-D3 from RP5C01
	// d4: N/C
	// d5: _ALARM from RP5C01
	// d6: COUNTER OUT from RP5H01
	// d7: DATA OUT from RP5H01 - always 0?
	uint8_t ret = m_rtc->read(offset >> 4 & 0xf) & 0xf;
	ret |= 0x10;
	ret |= m_rtc->alarm_r() ? 0x00 : 0x20;
	ret |= m_rp5h01->counter_r() ? 0x00 : 0x40;
	ret |= m_rp5h01->data_r() ? 0x00 : 0x80;

	return ret;
}

void punchout_state::spunchout_exp_w(offs_t offset, uint8_t data)
{
	// d0-d3: D0-D3 to RP5C01
	m_rtc->write(offset >> 4 & 0xf, data & 0xf);
}

void punchout_state::spunchout_rp5h01_reset_w(uint8_t data)
{
	// d0: 74LS74 2D
	// 74LS74 2Q -> RP5H01 RESET
	// 74LS74 _2Q -> 74LS74 _1 RESET
	m_rp5h01->reset_w(data & 1);
	if (data & 1)
		spunchout_rp5h01_clock_w(0);
}

void punchout_state::spunchout_rp5h01_clock_w(uint8_t data)
{
	// d0: 74LS74 1D
	// 74LS74 1Q -> RP5H01 DATA CLOCK + TEST
	m_rp5h01->clock_w(data & 1);
	m_rp5h01->test_w(data & 1);
}

void punchout_state::spnchout_io_map(address_map &map)
{
	punchout_io_map(map);
	map(0x05, 0x05).mirror(0xf0).w(FUNC(punchout_state::spunchout_rp5h01_reset_w));
	map(0x06, 0x06).mirror(0xf0).w(FUNC(punchout_state::spunchout_rp5h01_clock_w));
	map(0x07, 0x07).select(0xf0).rw(FUNC(punchout_state::spunchout_exp_r), FUNC(punchout_state::spunchout_exp_w)); // protection ports
}

// 2A03 (sound)

void punchout_state::punchout_sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x4016, 0x4016).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x4017, 0x4017).r("soundlatch2", FUNC(generic_latch_8_device::read));
	map(0xe000, 0xffff).rom();
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( punchout )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, "Time" )                  PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(    0x00, "Longest" )
	PORT_DIPSETTING(    0x04, "Long" )
	PORT_DIPSETTING(    0x08, "Short" )
	PORT_DIPSETTING(    0x0c, "Shortest" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Rematch At A Discount" ) PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:!7" )       /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_HIGH, "SW2:!8" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )        /* Not documented */
	PORT_DIPSETTING(    0x0b, DEF_STR( 4C_1C ) )        /* Not documented */
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )        /* dupe, Not documented */
	PORT_DIPSETTING(    0x08, "1 Coin/2 Credits (2 Credits/1 Play)" ) /* Not documented */
	PORT_DIPSETTING(    0x0d, "1 Coin/3 Credits (2 Credits/1 Play)" ) /* Not documented */
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )        /* dupe, Not documented */
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_2C ) )        /* dupe, Not documented */
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( Free_Play ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("vlm", vlm5030_device, bsy) /* VLM5030 busy signal */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "R18:!1" )       /* Not documented, R18 resistor */
	PORT_DIPNAME( 0x80, 0x00, "Copyright" )             PORT_DIPLOCATION("R19:!1") /* Not documented, R19 resistor */
	PORT_DIPSETTING(    0x00, "Nintendo" )
	PORT_DIPSETTING(    0x80, "Nintendo of America Inc." )
INPUT_PORTS_END

/* same as punchout with additional duck button */
static INPUT_PORTS_START( spnchout )
	PORT_INCLUDE( punchout )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_BUTTON4 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!1,!2,!3,!4")
	PORT_DIPSETTING(    0x08, DEF_STR( 6C_1C ) )        /* Not documented */
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )        /* Not documented */
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )        /* Not documented */
	PORT_DIPSETTING(    0x09, DEF_STR( 4C_1C ) )        /* dupe, Not documented */
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )        /* dupe, Not documented */
	PORT_DIPSETTING(    0x0d, "1 Coin/3 Credits (2 Credits/1 Play)" ) /* Not documented */
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0b, "1 Coin/2 Credits (3 Credits/1 Play)" ) /* Not documented */
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( Free_Play ) )
INPUT_PORTS_END

static INPUT_PORTS_START( armwrest )
	PORT_INCLUDE( punchout )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )

	/* Coinage:

	R18 (Coin Slots setting) determines which table is used.

	L = number of credits per coin for left coin slot
	R = number of credits per coin for right coin slot
	C = number of credits needed for one play

Table 1 (for 2 Coin Slots):

	DSW1 DSW2               DSW1 DSW2               DSW1 DSW2               DSW1 DSW2
bit 3210 5432  L  R  C      3210 5432  L  R  C      3210 5432  L  R  C      3210 5432  L  R  C

	0000 0000  1  1  1      0001 0000  1  2  1      0010 0000  1  3  1      0011 0000  1  4  1
	0000 0001  8  1  1      0001 0001  1  8  1      0010 0001  9  1  1      0011 0001  1  9  1
	0000 0010  2  1  1      0001 0010  2  2  1      0010 0010  2  3  1      0011 0010  2  4  1
	0000 0011  8  3  3      0001 0011  3  8  3      0010 0011  3  4  4      0011 0011  4  3  4
	0000 0100  3  1  2      0001 0100  3  2  1      0010 0100  3  3  1      0011 0100  3  4  1
	0000 0101  3  12 4      0001 0101  12 3  4      0010 0101  4  4  1      0011 0101  4  4  1
	0000 0110  1  2  2      0001 0110  1  4  2      0010 0110  1  6  2      0011 0110  1  8  2
	0000 0111  3  24 2      0001 0111  24 3  2      0010 0111  3  1  2      0011 0111  1  3  2
	0000 1000  1  3  3      0001 1000  1  6  3      0010 1000  1  9  3      0011 1000  1  12 3
	0000 1001  4  1  3      0001 1001  1  4  3      0010 1001  10 1  3      0011 1001  1  10 3
	0000 1010  1  4  4      0001 1010  1  8  4      0010 1010  1  12 4      0011 1010  1  16 4
	0000 1011  3  3  4      0001 1011  3  3  4      0010 1011  1  1  6      0011 1011  1  1  6
	0000 1100  1  5  5      0001 1100  1  10 5      0010 1100  1  15 5      0011 1100  1  20 5
	0000 1101  1  1  1      0001 1101  1  1  2      0010 1101  2  2  1      0011 1101  1  1  4
	0000 1110  2  3  3      0001 1110  2  6  3      0010 1110  2  9  3      0011 1110  2  12 3
	0000 1111  5  5  3      0001 1111  5  5  4      0010 1111  2  2  5      0011 1111  8  8  1

	0100 0000  1  5  1      0101 0000  1  6  1      0110 0000  2  1  2      0111 0000  3  1  3
	0100 0001  10 1  1      0101 0001  1  10 1      0110 0001  12 1  1      0111 0001  1  12 1
	0100 0010  2  5  1      0101 0010  2  6  1      0110 0010  4  1  2      0111 0010  6  1  3
	0100 0011  2  5  5      0101 0011  5  2  5      0110 0011  10 2  1      0111 0011  2  10 1
	0100 0100  3  5  1      0101 0100  3  6  1      0110 0100  6  1  2      0111 0100  9  1  3
	0100 0101  4  4  1      0101 0101  4  5  1      0110 0101  3  8  2      0111 0101  8  3  2
	0100 0110  1  10 2      0101 0110  1  12 2      0110 0110  1  1  2      0111 0110  3  2  6
	0100 0111  10 1  2      0101 0111  1  10 2      0110 0111  3  2  4      0111 0111  2  3  4
	0100 1000  1  15 3      0101 1000  1  18 3      0110 1000  2  3  6      0111 1000  1  1  3
	0100 1001  20 1  3      0101 1001  1  20 3      0110 1001  9  4  12     0111 1001  4  9  12
	0100 1010  1  20 4      0101 1010  1  24 4      0110 1010  1  2  4      0111 1010  3  4  12
	0100 1011  16 1  2      0101 1011  20 20 0*     0110 1011  1  1  1      0111 1011  1  1  1
	0100 1100  1  25 5      0101 1100  1  30 5      0110 1100  2  5  10     0111 1100  3  5  15
	0100 1101  1  1  5      0101 1101  3  3  3      0110 1101  4  4  1      0111 1101  6  6  1
	0100 1110  2  15 3      0101 1110  2  18 1      0110 1110  4  3  6      0111 1110  2  2  3
	0100 1111  8  8  3      0101 1111  9  9  1      0110 1111  9  9  2      0111 1111  10 10 1

	DSW1 DSW2               DSW1 DSW2               DSW1 DSW2               DSW1 DSW2
bit 3210 5432  L  R  C      3210 5432  L  R  C      3210 5432  L  R  C      3210 5432  L  R  C

	1000 0000  4  1  4      1001 0000  5  1  5      1010 0000  3  2  3      1011 0000  3  5  3
	1000 0001  3  2  2      1001 0001  2  3  2      1010 0001  5  2  2      1011 0001  2  5  2
	1000 0010  8  1  4      1001 0010  10 1  5      1010 0010  6  2  3      1011 0010  6  5  3
	1000 0011  12 2  1      1001 0011  2  12 1      1010 0011  3  4  2      1011 0011  4  3  2
	1000 0100  12 1  4      1001 0100  15 1  5      1010 0100  9  2  3      1011 0100  9  5  3
	1000 0101  5  5  1      1001 0101  5  5  1      1010 0101  6  6  1      1011 0101  6  6  1
	1000 0110  2  1  4      1001 0110  5  2  10     1010 0110  3  4  6      1011 0110  3  10 6
	1000 0111  3  3  2      1001 0111  3  3  2      1010 0111  4  9  6      1011 0111  9  4  6
	1000 1000  4  3  12     1001 1000  5  3  15     1010 1000  1  2  3      1011 1000  1  5  3
	1000 1001  4  2  3      1001 1001  2  4  3      1010 1001  10 2  3      1011 1001  2  10 3
	1000 1010  1  1  4      1001 1010  5  4  20     1010 1010  3  8  12     1011 1010  3  20 12
	1000 1011  1  1  1      1001 1011  1  1  1      1010 1011  1  1  1      1011 1011  1  1  1
	1000 1100  4  5  20     1001 1100  1  1  5      1010 1100  2  10 15     1011 1100  3  25 15
	1000 1101  1  1  6      1001 1101  2  2  3      1010 1101  5  5  1      1011 1101  3  3  2
	1000 1110  8  3  12     1001 1110  10 3  15     1010 1110  2  2  3      1011 1110  2  5  3
	1000 1111  10 10 3      1001 1111  11 11 1      1010 1111  11 11 3      1011 1111  12 12 1

	1100 0000  4  5  4      1101 0000  4  1  1      1110 0000  5  5  1      1111 0000  6  1  1
	1100 0001  9  2  2      1101 0001  2  9  2      1110 0001  4  4  3      1111 0001  3  4  3
	1100 0010  8  5  4      1101 0010  4  2  1      1110 0010  5  2  1      1111 0010  6  2  1
	1100 0011  3  8  4      1101 0011  8  3  4      1110 0011  11 2  1      1111 0011  2  11 1
	1100 0100  12 5  4      1101 0100  4  3  1      1110 0100  5  3  1      1111 0100  6  3  1
	1100 0101  3  12 2      1101 0101  12 3  2      1110 0101  3  24 4      1111 0101  24 3  1
	1100 0110  2  5  4      1101 0110  8  1  2      1110 0110  10 1  2      1111 0110  12 1  2
	1100 0111  8  9  6      1101 0111  9  8  6      1110 0111  1  6  4      1111 0111  6  1  4
	1100 1000  4  15 12     1101 1000  12 1  3      1110 1000  15 1  3      1111 1000  18 1  3
	1100 1001  11 2  3      1101 1001  2  11 3      1110 1001  9  8  12     1111 1001  8  9  12
	1100 1010  1  5  4      1101 1010  12 2  3      1110 1010  20 1  4      1111 1010  24 1  4
	1100 1011  1  1  1      1101 1011  1  1  1      1110 1011  1  1  1      1111 1011  1  1  1
	1100 1100  4  25 20     1101 1100  20 1  5      1110 1100  25 1  5      1111 1100  15 2  3
	1100 1101  1  1  3      1101 1101  5  5  2      1110 1101  4  4  3      1111 1101  3  3  4
	1100 1110  8  15 12     1101 1110  18 2  3      1110 1110  20 4  4      1111 1110  "Freeplay"
	1100 1111  20 20 3      1101 1111  3  3  4      1110 1111  20 20 0*     1111 1111  "Freeplay"

Table 2 (for 1 Coin Slot):

	DSW1 DSW2
bit 3210 5432  L  R  C

	0000 0xxx  1  1  1
	0000 1xxx  5  5  3

	0001 0xxx  1  1  2
	0001 1xxx  5  5  4

	0010 0xxx  2  2  1
	0010 1xxx  2  2  5

	0011 0xxx  1  1  4
	0011 1xxx  8  8  1

	0100 0xxx  1  1  5
	0100 1xxx  8  8  3

	0101 0xxx  3  3  1
	0101 1xxx  9  9  1

	0110 0xxx  4  4  1
	0110 1xxx  9  9  2

	0111 0xxx  6  6  1
	0111 1xxx  10 10 1

	1000 0xxx  1  1  6
	1000 1xxx  10 10 3

	1001 0xxx  5  1  5
	1001 1xxx  11 11 1

	1010 0xxx  5  5  1
	1010 1xxx  11 11 3

	1011 0xxx  3  3  2
	1011 1xxx  12 12 1

	1100 0xxx  1  1  3
	1100 1xxx  20 20 3

	1101 0xxx  5  5  2
	1101 1xxx  3  3  4

	1110 0xxx  4  4  3
	1110 1xxx  20 20 0*

	1111 xxxx  "Freeplay"

	0*: Not a "Freeplay": you MUST insert a coin!

	*/

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x3c, 0x00, "Coinage 2" )             PORT_DIPLOCATION("SW2:!3,!4,!5,!6") // K,L,M,N
	PORT_DIPSETTING(    0x00, "0000" )
	PORT_DIPSETTING(    0x04, "0001" )
	PORT_DIPSETTING(    0x08, "0010" )
	PORT_DIPSETTING(    0x0c, "0011" )
	PORT_DIPSETTING(    0x10, "0100" )
	PORT_DIPSETTING(    0x14, "0101" )
	PORT_DIPSETTING(    0x18, "0110" )
	PORT_DIPSETTING(    0x1c, "0111" )
	PORT_DIPSETTING(    0x20, "1000" )
	PORT_DIPSETTING(    0x24, "1001" )
	PORT_DIPSETTING(    0x28, "1010" )
	PORT_DIPSETTING(    0x2c, "1011" )
	PORT_DIPSETTING(    0x30, "1100" )
	PORT_DIPSETTING(    0x34, "1101" )
	PORT_DIPSETTING(    0x38, "1110" )
	PORT_DIPSETTING(    0x3c, "1111" )
	PORT_DIPNAME( 0x40, 0x00, "Rematches" )             PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "7" )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x00, "Coinage 1" )             PORT_DIPLOCATION("SW1:!1,!2,!3,!4") // A,B,C,D
	PORT_DIPSETTING(    0x00, "0000" )
	PORT_DIPSETTING(    0x01, "0001" )
	PORT_DIPSETTING(    0x02, "0010" )
	PORT_DIPSETTING(    0x03, "0011" )
	PORT_DIPSETTING(    0x04, "0100" )
	PORT_DIPSETTING(    0x05, "0101" )
	PORT_DIPSETTING(    0x06, "0110" )
	PORT_DIPSETTING(    0x07, "0111" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x09, "1001" )
	PORT_DIPSETTING(    0x0a, "1010" )
	PORT_DIPSETTING(    0x0b, "1011" )
	PORT_DIPSETTING(    0x0c, "1100" )
	PORT_DIPSETTING(    0x0d, "1101" )
	PORT_DIPSETTING(    0x0e, "1110" )
	PORT_DIPSETTING(    0x0f, "1111" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("vlm", vlm5030_device, bsy) /* VLM5030 busy signal */
	PORT_DIPNAME( 0x40, 0x00, "Coin Slots" )            PORT_DIPLOCATION("R18:!1") /* R18 resistor */
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "R19:!1" )       /* R19 resistor */
INPUT_PORTS_END



/***************************************************************************

  Machine Configs

***************************************************************************/


static GFXDECODE_START( gfx_punchout )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar, 0x000, 0x100/4 )   // bg chars (top monitor only)
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x2_planar, 0x100, 0x100/4 )   // bg chars (bottom monitor only)
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x3_planar, 0x000, 0x200/8 )   // big sprite #1 (top and bottom monitor)
	GFXDECODE_ENTRY( "gfx4", 0, gfx_8x8x2_planar, 0x100, 0x100/4 )   // big sprite #2 (bottom monitor only)
GFXDECODE_END

static GFXDECODE_START( gfx_armwrest )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar, 0x000, 0x200/4 )   // bg chars (top and bottom monitor)
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x3_planar, 0x100, 0x100/8 )   // fg chars (bottom monitor only)
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x3_planar, 0x000, 0x200/8 )   // big sprite #1 (top and bottom monitor)
	GFXDECODE_ENTRY( "gfx4", 0, gfx_8x8x2_planar, 0x100, 0x100/4 )   // big sprite #2 (bottom monitor only)
GFXDECODE_END


WRITE_LINE_MEMBER(punchout_state::vblank_irq)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


MACHINE_RESET_MEMBER(punchout_state, spnchout)
{
	m_rp5h01->enable_w(0); // _CE -> GND
}


void punchout_state::punchout(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &punchout_state::punchout_map);
	m_maincpu->set_addrmap(AS_IO, &punchout_state::punchout_io_map);

	N2A03(config, m_audiocpu, NTSC_APU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &punchout_state::punchout_sound_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 2B
	mainlatch.q_out_cb<0>().set(FUNC(punchout_state::nmi_mask_w));
	mainlatch.q_out_cb<1>().set_nop(); // watchdog reset, seldom used because 08 clears the watchdog as well
	mainlatch.q_out_cb<2>().set_nop(); // ?
	mainlatch.q_out_cb<3>().set_inputline("audiocpu", INPUT_LINE_RESET);
	mainlatch.q_out_cb<4>().set("vlm", FUNC(vlm5030_device::rst));
	mainlatch.q_out_cb<5>().set("vlm", FUNC(vlm5030_device::st));
	mainlatch.q_out_cb<6>().set("vlm", FUNC(vlm5030_device::vcu));
	mainlatch.q_out_cb<7>().set_nop(); // enable NVRAM?

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_punchout);
	PALETTE(config, m_palette).set_entries(0x200);
	config.set_default_layout(layout_dualhovu);

	screen_device &top(SCREEN(config, "top", SCREEN_TYPE_RASTER));
	top.set_refresh_hz(60);
	top.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	top.set_size(32*8, 32*8);
	top.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	top.set_screen_update(FUNC(punchout_state::screen_update_punchout_top));
	top.set_palette(m_palette);
	top.screen_vblank().set(FUNC(punchout_state::vblank_irq));
	top.screen_vblank().append_inputline(m_audiocpu, INPUT_LINE_NMI);

	screen_device &bottom(SCREEN(config, "bottom", SCREEN_TYPE_RASTER));
	bottom.set_refresh_hz(60);
	bottom.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	bottom.set_size(32*8, 32*8);
	bottom.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	bottom.set_screen_update(FUNC(punchout_state::screen_update_punchout_bottom));
	bottom.set_palette(m_palette);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2");

	VLM5030(config, m_vlm, N2A03_NTSC_XTAL/6);
	m_vlm->set_addrmap(0, &punchout_state::punchout_vlm_map);
	m_vlm->add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	m_audiocpu->add_route(ALL_OUTPUTS, "rspeaker", 0.50);
}


void punchout_state::spnchout(machine_config &config)
{
	punchout(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &punchout_state::spnchout_io_map);

	RP5C01(config, m_rtc, 0); // OSCIN -> Vcc
	m_rtc->remove_battery();
	RP5H01(config, m_rp5h01, 0);

	MCFG_MACHINE_RESET_OVERRIDE(punchout_state, spnchout)
}


void punchout_state::armwrest(machine_config &config)
{
	punchout(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &punchout_state::armwrest_map);

	/* video hardware */
	m_gfxdecode->set_info(gfx_armwrest);

	MCFG_VIDEO_START_OVERRIDE(punchout_state, armwrest)
	subdevice<screen_device>("top")->set_screen_update(FUNC(punchout_state::screen_update_armwrest_top));
	subdevice<screen_device>("bottom")->set_screen_update(FUNC(punchout_state::screen_update_armwrest_bottom));
}



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( punchout )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chp1-c.8l",    0x0000, 0x2000, CRC(a4003adc) SHA1(a8026eb39aa883993a0c9cb4400bf1a7e5898a2b) )    /* Revision e-1 */
	ROM_LOAD( "chp1-c.8k",    0x2000, 0x2000, CRC(745ecf40) SHA1(430f80b688a515953fab177a3ec2eb31c886df22) )    /* Revision e-1 */
	ROM_LOAD( "chp1-c.8j",    0x4000, 0x2000, CRC(7a7f870e) SHA1(76bb9f3ef0a2fd514db63fb77f35bde12c15c29c) )    /* Revision e */
	ROM_LOAD( "chp1-c.8h",    0x6000, 0x2000, CRC(5d8123d7) SHA1(04ddfcde969db93ff31e9c8a2af4dde285b82e2e) )    /* Revision e */
	ROM_LOAD( "chp1-c.8f",    0x8000, 0x4000, CRC(c8a55ddb) SHA1(f91fb368542c50969a086f01a2e70ecce7f2697b) )    /* Revision e-1 */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c.4k",    0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-b.4c",    0x00000, 0x0800, CRC(49b763bc) SHA1(064739bf4f5eb18567fd4df9c37280dc84101715) )   /* chars #1 */ /* Revision B */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-b.4d",    0x02000, 0x0800, CRC(08bc6d67) SHA1(d229a7c9152bb43fe12c313c8d3b681226b847e0) )   /* Revision B */
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )

	ROM_REGION( 0x04000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-b.4a",    0x00000, 0x0800, CRC(c075f831) SHA1(f22d9e415637599420c443ce08e7e70d1eb1c6f5) )   /* chars #2 */ /* Revision B */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-b.4b",    0x02000, 0x0800, CRC(c4cc2b5a) SHA1(7b9d4dcecc67271980c3c44561fc25a6f6c93ee3) )   /* Revision B */
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v.2r",    0x00000, 0x4000, CRC(bd1d4b2e) SHA1(492ae301a9890c2603d564c9048b1b67895052dd) )   /* chars #3 */ /* Labeled Rev B, but same as Rev A */
	ROM_LOAD( "chp1-v.2t",    0x04000, 0x4000, CRC(dd9a688a) SHA1(fbb98eebfbaab445928da939846a2d07a8046afb) )
	ROM_LOAD( "chp1-v.2u",    0x08000, 0x2000, CRC(da6a3c4b) SHA1(e03469fb6f552f41a9b7f4b3e51c15a52b61cf84) )
	/* 0a000-0bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.2v",    0x0c000, 0x2000, CRC(8c734a67) SHA1(d59b5a2517e4890e7ca7da52ca2813a6abc484a3) )
	/* 0e000-0ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.3r",    0x10000, 0x4000, CRC(2e74ad1d) SHA1(538b3f9273699106a50887c927f0251537bf0f42) )
	ROM_LOAD( "chp1-v.3t",    0x14000, 0x4000, CRC(630ba9fb) SHA1(36cec8658597239385cada3bc947b940ab66954b) )
	ROM_LOAD( "chp1-v.3u",    0x18000, 0x2000, CRC(6440321d) SHA1(c8c084ad408cb6bf65959ed4db03c4b4cf9b1c1a) )
	/* 1a000-1bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.3v",    0x1c000, 0x2000, CRC(bb7b7198) SHA1(64572668d30e008daf4ccaa5689518ecc41f1091) )
	/* 1e000-1ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.4r",    0x20000, 0x4000, CRC(4e5b0fe9) SHA1(c5c4fb735cc232b43c49442e62af0ebe99eaab0c) )
	ROM_LOAD( "chp1-v.4t",    0x24000, 0x4000, CRC(37ffc940) SHA1(d555807a6a1025c81637c5db0184b48306aa01ac) )
	ROM_LOAD( "chp1-v.4u",    0x28000, 0x2000, CRC(1a7521d4) SHA1(4e8a8298f2ff8257d2058e5133ad295f92c7deb8) )
	/* 2a000-2bfff empty (space for 16k ROM) */
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v.6p",    0x00000, 0x0800, CRC(75be7aae) SHA1(396bc1d301b99e064de4dad699882618b1b9c958) )   /* chars #4 */ /* Revision B */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-v.6n",    0x02000, 0x0800, CRC(daf74de0) SHA1(9373d4527b675b3128a5a830f42e1dc5dcb85307) )   /* Revision B */
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chp1-v.8p",    0x08000, 0x0800, CRC(4cb7ea82) SHA1(213b7c1431f4c92e5519a8771035bda28b3bab8a) )   /* Revision B */
	ROM_CONTINUE(             0x09000, 0x0800 )
	ROM_CONTINUE(             0x08800, 0x0800 )
	ROM_CONTINUE(             0x09800, 0x0800 )
	ROM_LOAD( "chp1-v.8n",    0x0a000, 0x0800, CRC(1c0d09aa) SHA1(3276bae7400453f3612f53d7b47fb199cbe53e6d) )   /* Revision B */
	ROM_CONTINUE(             0x0b000, 0x0800 )
	ROM_CONTINUE(             0x0a800, 0x0800 )
	ROM_CONTINUE(             0x0b800, 0x0800 )
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x2100, "proms", ROMREGION_ERASEFF ) // see driver notes
	// pink labeled color proms
	ROM_LOAD( "chp1-b-6e_pink.6e",  0x0000, 0x0200, CRC(e9ca3ac6) SHA1(68d9739d8a0dadc6fe3b3767437526096ca5db98) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_pink.6f",  0x0200, 0x0200, CRC(02be56ab) SHA1(a88f332cb26928350ed20ab5f4c04d5324bb516a) )  /* G */
	ROM_LOAD( "chp1-b-7f_pink.7f",  0x0400, 0x0200, CRC(11de55f1) SHA1(269b82f4bc73fac197e0bb6d9a90a220e77ce478) )  /* B */
	ROM_LOAD( "chp1-b-7e_pink.7e",  0x0600, 0x0200, CRC(fddaa777) SHA1(ee6bff5ffeefc82374868a268f885d0bc3a76da2) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_pink.8e",  0x0800, 0x0200, CRC(c3d5d71f) SHA1(2b02479614a801f539fead17860b84e9a180e761) )  /* G */
	ROM_LOAD( "chp1-b-8f_pink.8f",  0x0a00, 0x0200, CRC(a3037155) SHA1(8b6c0c80278ca859a08a1a2429190d51be4f9401) )  /* B */
	// white labeled color proms (indices are reversed)
	ROM_LOAD( "chp1-b-6e_white.6e", 0x1000, 0x0200, CRC(ddac5f0e) SHA1(25dabe415757ccea057609a3f3f79a56b613032a) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_white.6f", 0x1200, 0x0200, CRC(846c6261) SHA1(f012a02bbdf0166b9bfd3dc9749db18759a22421) )  /* G */
	ROM_LOAD( "chp1-b-7f_white.7f", 0x1400, 0x0200, CRC(1682dd30) SHA1(84b92d1b292210bda0c25537c8ee3e274aaac75c) )  /* B */
	ROM_LOAD( "chp1-b-7e_white.7e", 0x1600, 0x0200, CRC(47adf7a2) SHA1(1d37d5207cd37a9c122251c60cc8f43dd680f484) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_white.8e", 0x1800, 0x0200, CRC(b0fc15a8) SHA1(a1af09cfea81231240bd94f3b98de1be8235ebe7) )  /* G */
	ROM_LOAD( "chp1-b-8f_white.8f", 0x1a00, 0x0200, CRC(1ffd894a) SHA1(9e8c1c28b4c12acf42f814bc109d353729a25652) )  /* B */
	ROM_LOAD( "chp1-v-2d.2d",       0x2000, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )  /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chp1-c.6p",    0x0000, 0x4000, CRC(ea0bbb31) SHA1(b1da024cb688341d39791a78d1144fe09acb00cf) )
ROM_END

ROM_START( punchouta )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chp1-c.8l",    0x0000, 0x2000, CRC(a4003adc) SHA1(a8026eb39aa883993a0c9cb4400bf1a7e5898a2b) )   /* Revision e-1 */
	ROM_LOAD( "chp1-c.8k",    0x2000, 0x2000, CRC(745ecf40) SHA1(430f80b688a515953fab177a3ec2eb31c886df22) )   /* Revision e-1 */
	ROM_LOAD( "chp1-c.8j",    0x4000, 0x2000, CRC(7a7f870e) SHA1(76bb9f3ef0a2fd514db63fb77f35bde12c15c29c) )   /* Revision e */
	ROM_LOAD( "chp1-c.8h",    0x6000, 0x2000, CRC(5d8123d7) SHA1(04ddfcde969db93ff31e9c8a2af4dde285b82e2e) )   /* Revision e */
	ROM_LOAD( "chp1-c.8f",    0x8000, 0x4000, CRC(c8a55ddb) SHA1(f91fb368542c50969a086f01a2e70ecce7f2697b) )   /* Revision e-1 */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c.4k",    0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-b.4c",    0x00000, 0x2000, CRC(e26dc8b3) SHA1(a704d39ef6f5cbad64a478e5c109b18aae427cbc) )   /* chars #1 */ /* Revision A */
	ROM_LOAD( "chp1-b.4d",    0x02000, 0x2000, CRC(dd1310ca) SHA1(918d2eda000244b692f1da7ac57d7a0edaef95fb) )

	ROM_REGION( 0x04000, "gfx2", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-b.4a",    0x00000, 0x2000, CRC(20fb4829) SHA1(9f0ce9379eb31c19bfacdc514ac6a28aa4217cbb) )   /* chars #2 */ /* Revision A */
	ROM_LOAD( "chp1-b.4b",    0x02000, 0x2000, CRC(edc34594) SHA1(fbb4a8b979d60b183dc23bdbb7425100b9325287) )

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v.2r",    0x00000, 0x4000, CRC(bd1d4b2e) SHA1(492ae301a9890c2603d564c9048b1b67895052dd) )   /* chars #3 */ /* Same as Rev B */
	ROM_LOAD( "chp1-v.2t",    0x04000, 0x4000, CRC(dd9a688a) SHA1(fbb98eebfbaab445928da939846a2d07a8046afb) )
	ROM_LOAD( "chp1-v.2u",    0x08000, 0x2000, CRC(da6a3c4b) SHA1(e03469fb6f552f41a9b7f4b3e51c15a52b61cf84) )
	/* 0a000-0bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.2v",    0x0c000, 0x2000, CRC(8c734a67) SHA1(d59b5a2517e4890e7ca7da52ca2813a6abc484a3) )
	/* 0e000-0ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.3r",    0x10000, 0x4000, CRC(2e74ad1d) SHA1(538b3f9273699106a50887c927f0251537bf0f42) )
	ROM_LOAD( "chp1-v.3t",    0x14000, 0x4000, CRC(630ba9fb) SHA1(36cec8658597239385cada3bc947b940ab66954b) )
	ROM_LOAD( "chp1-v.3u",    0x18000, 0x2000, CRC(6440321d) SHA1(c8c084ad408cb6bf65959ed4db03c4b4cf9b1c1a) )
	/* 1a000-1bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.3v",    0x1c000, 0x2000, CRC(bb7b7198) SHA1(64572668d30e008daf4ccaa5689518ecc41f1091) )
	/* 1e000-1ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.4r",    0x20000, 0x4000, CRC(4e5b0fe9) SHA1(c5c4fb735cc232b43c49442e62af0ebe99eaab0c) )
	ROM_LOAD( "chp1-v.4t",    0x24000, 0x4000, CRC(37ffc940) SHA1(d555807a6a1025c81637c5db0184b48306aa01ac) )
	ROM_LOAD( "chp1-v.4u",    0x28000, 0x2000, CRC(1a7521d4) SHA1(4e8a8298f2ff8257d2058e5133ad295f92c7deb8) )
	/* 2a000-2bfff empty (space for 16k ROM) */
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-v.6p",    0x00000, 0x2000, CRC(16588f7a) SHA1(1aeaaa5cc2477c3aa4bf80df7d9474cc9ded9f15) )   /* chars #4 */ /* Revision A */
	ROM_LOAD( "chp1-v.6n",    0x02000, 0x2000, CRC(dc743674) SHA1(660582c76ee68a7267d5686a2f8ea0fd6c2b25fc) )
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chp1-v.8p",    0x08000, 0x2000, CRC(c2db5b4e) SHA1(39d009af597fa28d34af31aec111aa6fe09fea39) )
	ROM_LOAD( "chp1-v.8n",    0x0a000, 0x2000, CRC(e6af390e) SHA1(73984cbdc8fbf667126ada63ab9500609eb25c61) )
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x2100, "proms", ROMREGION_ERASEFF ) // see driver notes
	// pink labeled color proms
	ROM_LOAD( "chp1-b-6e_pink.6e",  0x0000, 0x0200, CRC(e9ca3ac6) SHA1(68d9739d8a0dadc6fe3b3767437526096ca5db98) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_pink.6f",  0x0200, 0x0200, CRC(02be56ab) SHA1(a88f332cb26928350ed20ab5f4c04d5324bb516a) )  /* G */
	ROM_LOAD( "chp1-b-7f_pink.7f",  0x0400, 0x0200, CRC(11de55f1) SHA1(269b82f4bc73fac197e0bb6d9a90a220e77ce478) )  /* B */
	ROM_LOAD( "chp1-b-7e_pink.7e",  0x0600, 0x0200, CRC(fddaa777) SHA1(ee6bff5ffeefc82374868a268f885d0bc3a76da2) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_pink.8e",  0x0800, 0x0200, CRC(c3d5d71f) SHA1(2b02479614a801f539fead17860b84e9a180e761) )  /* G */
	ROM_LOAD( "chp1-b-8f_pink.8f",  0x0a00, 0x0200, CRC(a3037155) SHA1(8b6c0c80278ca859a08a1a2429190d51be4f9401) )  /* B */
	// white labeled color proms (indices are reversed)
	ROM_LOAD( "chp1-b-6e_white.6e", 0x1000, 0x0200, CRC(ddac5f0e) SHA1(25dabe415757ccea057609a3f3f79a56b613032a) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_white.6f", 0x1200, 0x0200, CRC(846c6261) SHA1(f012a02bbdf0166b9bfd3dc9749db18759a22421) )  /* G */
	ROM_LOAD( "chp1-b-7f_white.7f", 0x1400, 0x0200, CRC(1682dd30) SHA1(84b92d1b292210bda0c25537c8ee3e274aaac75c) )  /* B */
	ROM_LOAD( "chp1-b-7e_white.7e", 0x1600, 0x0200, CRC(47adf7a2) SHA1(1d37d5207cd37a9c122251c60cc8f43dd680f484) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_white.8e", 0x1800, 0x0200, CRC(b0fc15a8) SHA1(a1af09cfea81231240bd94f3b98de1be8235ebe7) )  /* G */
	ROM_LOAD( "chp1-b-8f_white.8f", 0x1a00, 0x0200, CRC(1ffd894a) SHA1(9e8c1c28b4c12acf42f814bc109d353729a25652) )  /* B */
	ROM_LOAD( "chp1-v-2d.2d",       0x2000, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )  /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chp1-c.6p",    0x0000, 0x4000, CRC(ea0bbb31) SHA1(b1da024cb688341d39791a78d1144fe09acb00cf) )
ROM_END

ROM_START( punchoutj )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chp1-c_8l_a.8l", 0x0000, 0x2000, CRC(9735eb5a) SHA1(0c68e91568845ae3cda5eb6f62c2e271f66c79b4) )
	ROM_LOAD( "chp1-c_8k_a.8k", 0x2000, 0x2000, CRC(98baba41) SHA1(87d6ab86cf593e0098edbee62727b253489bdb47) )
	ROM_LOAD( "chp1-c_8j_a.8j", 0x4000, 0x2000, CRC(7a7f870e) SHA1(76bb9f3ef0a2fd514db63fb77f35bde12c15c29c) )
	ROM_LOAD( "chp1-c_8h_a.8h", 0x6000, 0x2000, CRC(5d8123d7) SHA1(04ddfcde969db93ff31e9c8a2af4dde285b82e2e) )
	ROM_LOAD( "chp1-c_8f_a.8f", 0x8000, 0x4000, CRC(ea52cda1) SHA1(76e50ab9e09ca4cad6e4030f8372121396898b93) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c_4k_a.4k", 0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-b_4c_a.4c", 0x00000, 0x2000, CRC(e26dc8b3) SHA1(a704d39ef6f5cbad64a478e5c109b18aae427cbc) )   /* chars #1 */
	ROM_LOAD( "chp1-b_4d_a.4d", 0x02000, 0x2000, CRC(dd1310ca) SHA1(918d2eda000244b692f1da7ac57d7a0edaef95fb) )

	ROM_REGION( 0x04000, "gfx2", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-b_4a_a.4a", 0x00000, 0x2000, CRC(20fb4829) SHA1(9f0ce9379eb31c19bfacdc514ac6a28aa4217cbb) )   /* chars #2 */
	ROM_LOAD( "chp1-b_4b_a.4b", 0x02000, 0x2000, CRC(edc34594) SHA1(fbb4a8b979d60b183dc23bdbb7425100b9325287) )

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v_2r_a.2r", 0x00000, 0x4000, CRC(bd1d4b2e) SHA1(492ae301a9890c2603d564c9048b1b67895052dd) )   /* chars #3 */
	ROM_LOAD( "chp1-v_2t_a.2t", 0x04000, 0x4000, CRC(dd9a688a) SHA1(fbb98eebfbaab445928da939846a2d07a8046afb) )
	ROM_LOAD( "chp1-v_2u_a.2u", 0x08000, 0x2000, CRC(da6a3c4b) SHA1(e03469fb6f552f41a9b7f4b3e51c15a52b61cf84) )
	/* 0a000-0bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v_2v_a.2v", 0x0c000, 0x2000, CRC(8c734a67) SHA1(d59b5a2517e4890e7ca7da52ca2813a6abc484a3) )
	/* 0e000-0ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v_3r_a.3r", 0x10000, 0x4000, CRC(2e74ad1d) SHA1(538b3f9273699106a50887c927f0251537bf0f42) )
	ROM_LOAD( "chp1-v_3t_a.3t", 0x14000, 0x4000, CRC(630ba9fb) SHA1(36cec8658597239385cada3bc947b940ab66954b) )
	ROM_LOAD( "chp1-v_3u_a.3u", 0x18000, 0x2000, CRC(6440321d) SHA1(c8c084ad408cb6bf65959ed4db03c4b4cf9b1c1a) )
	/* 1a000-1bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v_3v_a.3v", 0x1c000, 0x2000, CRC(bb7b7198) SHA1(64572668d30e008daf4ccaa5689518ecc41f1091) )
	/* 1e000-1ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v_4r_a.4r", 0x20000, 0x4000, CRC(4e5b0fe9) SHA1(c5c4fb735cc232b43c49442e62af0ebe99eaab0c) )
	ROM_LOAD( "chp1-v_4t_a.4t", 0x24000, 0x4000, CRC(37ffc940) SHA1(d555807a6a1025c81637c5db0184b48306aa01ac) )
	ROM_LOAD( "chp1-v_4u_a.4u", 0x28000, 0x2000, CRC(1a7521d4) SHA1(4e8a8298f2ff8257d2058e5133ad295f92c7deb8) )
	/* 2a000-2bfff empty (space for 16k ROM) */
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-v_6p_a.6p", 0x00000, 0x2000, CRC(16588f7a) SHA1(1aeaaa5cc2477c3aa4bf80df7d9474cc9ded9f15) )   /* chars #4 */
	ROM_LOAD( "chp1-v_6n_a.6n", 0x02000, 0x2000, CRC(dc743674) SHA1(660582c76ee68a7267d5686a2f8ea0fd6c2b25fc) )
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chp1-v_8p_a.8p", 0x08000, 0x2000, CRC(c2db5b4e) SHA1(39d009af597fa28d34af31aec111aa6fe09fea39) )
	ROM_LOAD( "chp1-v_8n_a.8n", 0x0a000, 0x2000, CRC(e6af390e) SHA1(73984cbdc8fbf667126ada63ab9500609eb25c61) )
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x2100, "proms", ROMREGION_ERASEFF ) // see driver notes
	// pink labeled color proms
	ROM_LOAD( "chp1-b-6e_pink.6e",  0x0000, 0x0200, CRC(e9ca3ac6) SHA1(68d9739d8a0dadc6fe3b3767437526096ca5db98) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_pink.6f",  0x0200, 0x0200, CRC(02be56ab) SHA1(a88f332cb26928350ed20ab5f4c04d5324bb516a) )  /* G */
	ROM_LOAD( "chp1-b-7f_pink.7f",  0x0400, 0x0200, CRC(11de55f1) SHA1(269b82f4bc73fac197e0bb6d9a90a220e77ce478) )  /* B */
	ROM_LOAD( "chp1-b-7e_pink.7e",  0x0600, 0x0200, CRC(fddaa777) SHA1(ee6bff5ffeefc82374868a268f885d0bc3a76da2) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_pink.8e",  0x0800, 0x0200, CRC(c3d5d71f) SHA1(2b02479614a801f539fead17860b84e9a180e761) )  /* G */
	ROM_LOAD( "chp1-b-8f_pink.8f",  0x0a00, 0x0200, CRC(a3037155) SHA1(8b6c0c80278ca859a08a1a2429190d51be4f9401) )  /* B */
	// white labeled color proms (indices are reversed)
	ROM_LOAD( "chp1-b-6e_white.6e", 0x1000, 0x0200, CRC(ddac5f0e) SHA1(25dabe415757ccea057609a3f3f79a56b613032a) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_white.6f", 0x1200, 0x0200, CRC(846c6261) SHA1(f012a02bbdf0166b9bfd3dc9749db18759a22421) )  /* G */
	ROM_LOAD( "chp1-b-7f_white.7f", 0x1400, 0x0200, CRC(1682dd30) SHA1(84b92d1b292210bda0c25537c8ee3e274aaac75c) )  /* B */
	ROM_LOAD( "chp1-b-7e_white.7e", 0x1600, 0x0200, CRC(47adf7a2) SHA1(1d37d5207cd37a9c122251c60cc8f43dd680f484) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_white.8e", 0x1800, 0x0200, CRC(b0fc15a8) SHA1(a1af09cfea81231240bd94f3b98de1be8235ebe7) )  /* G */
	ROM_LOAD( "chp1-b-8f_white.8f", 0x1a00, 0x0200, CRC(1ffd894a) SHA1(9e8c1c28b4c12acf42f814bc109d353729a25652) )  /* B */
	ROM_LOAD( "chp1-v-2d.2d",       0x2000, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )  /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chp1-c_6p_a.6p",  0x0000, 0x4000, CRC(597955ca) SHA1(e03b1ff1b506d38515d74710ced741d4e50e04b2) )
ROM_END

/* Italian bootleg set from an original board found in Italy,
   uses new program roms, 2 new gfx roms, and a mix of PunchOut and Super PunchOut graphic roms
   Service mode is diaabled
*/

ROM_START( punchita )
	/* Unique to this set */
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chp1-c.8l",    0x0000, 0x2000, CRC(1d595ce2) SHA1(affd43bef96c68f953e66cfa14ad4e9c304dc022) ) // sldh
	ROM_LOAD( "chp1-c.8k",    0x2000, 0x2000, CRC(c062fa5c) SHA1(8ebd6fd76f1fd1b85216a4e21d8a13be8317b9e2) ) // sldh
	ROM_LOAD( "chp1-c.8j",    0x4000, 0x2000, CRC(48d453ef) SHA1(145f3ace8bec87e83b64c6472e2b71f1ebea13ea) ) // sldh
	ROM_LOAD( "chp1-c.8h",    0x6000, 0x2000, CRC(67f5aedc) SHA1(c63a8b0696eec87bb147d435c18ee7e26d19e2a4) ) // sldh
	ROM_LOAD( "chp1-c.8f",    0x8000, 0x4000, CRC(761de4f3) SHA1(66754bc762c14fea620fabf408f85e6e3acb89ad) ) // sldh

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c.4k",    0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )

	/* Unique to this set */
	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-b.4c",    0x00000, 0x0800, CRC(9a9ff1d3) SHA1(d91adf69acb717f238cd5954909701a8748f2185) ) /* chars #1 */ // sldh
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-b.4d",    0x02000, 0x0800, CRC(4c23350f) SHA1(70a76002db9209699cdf1f092b2b5ef32d0b7b75) )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )

	/* These match SUPER PunchOut */
	ROM_REGION( 0x04000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-b.4a",    0x00000, 0x0800, CRC(c075f831) SHA1(f22d9e415637599420c443ce08e7e70d1eb1c6f5) ) /* chars #2 */ // sldh
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-b.4b",    0x02000, 0x0800, CRC(c4cc2b5a) SHA1(7b9d4dcecc67271980c3c44561fc25a6f6c93ee3) ) // sldh
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v.2r",    0x00000, 0x4000, CRC(bd1d4b2e) SHA1(492ae301a9890c2603d564c9048b1b67895052dd) ) /* chars #3 */
	ROM_LOAD( "chp1-v.2t",    0x04000, 0x4000, CRC(dd9a688a) SHA1(fbb98eebfbaab445928da939846a2d07a8046afb) )
	ROM_LOAD( "chp1-v.2u",    0x08000, 0x2000, CRC(da6a3c4b) SHA1(e03469fb6f552f41a9b7f4b3e51c15a52b61cf84) )
	/* 0a000-0bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.2v",    0x0c000, 0x2000, CRC(8c734a67) SHA1(d59b5a2517e4890e7ca7da52ca2813a6abc484a3) )
	/* 0e000-0ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.3r",    0x10000, 0x4000, CRC(2e74ad1d) SHA1(538b3f9273699106a50887c927f0251537bf0f42) )
	ROM_LOAD( "chp1-v.3t",    0x14000, 0x4000, CRC(630ba9fb) SHA1(36cec8658597239385cada3bc947b940ab66954b) )
	ROM_LOAD( "chp1-v.3u",    0x18000, 0x2000, CRC(6440321d) SHA1(c8c084ad408cb6bf65959ed4db03c4b4cf9b1c1a) )
	/* 1a000-1bfff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.3v",    0x1c000, 0x2000, CRC(bb7b7198) SHA1(64572668d30e008daf4ccaa5689518ecc41f1091) )
	/* 1e000-1ffff empty (space for 16k ROM) */
	ROM_LOAD( "chp1-v.4r",    0x20000, 0x4000, CRC(4e5b0fe9) SHA1(c5c4fb735cc232b43c49442e62af0ebe99eaab0c) )
	ROM_LOAD( "chp1-v.4t",    0x24000, 0x4000, CRC(37ffc940) SHA1(d555807a6a1025c81637c5db0184b48306aa01ac) )
	ROM_LOAD( "chp1-v.4u",    0x28000, 0x2000, CRC(1a7521d4) SHA1(4e8a8298f2ff8257d2058e5133ad295f92c7deb8) )
	/* 2a000-2bfff empty (space for 16k ROM) */
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	/* These match SUPER PunchOut */
	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v.6p",    0x00000, 0x0800, CRC(75be7aae) SHA1(396bc1d301b99e064de4dad699882618b1b9c958) ) /* chars #4 */ // sldh
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-v.6n",    0x02000, 0x0800, CRC(daf74de0) SHA1(9373d4527b675b3128a5a830f42e1dc5dcb85307) ) // sldh
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chp1-v.8p",    0x08000, 0x0800, CRC(4cb7ea82) SHA1(213b7c1431f4c92e5519a8771035bda28b3bab8a) ) // sldh
	ROM_CONTINUE(             0x09000, 0x0800 )
	ROM_CONTINUE(             0x08800, 0x0800 )
	ROM_CONTINUE(             0x09800, 0x0800 )
	ROM_LOAD( "chp1-v.8n",    0x0a000, 0x0800, CRC(1c0d09aa) SHA1(3276bae7400453f3612f53d7b47fb199cbe53e6d) ) // sldh
	ROM_CONTINUE(             0x0b000, 0x0800 )
	ROM_CONTINUE(             0x0a800, 0x0800 )
	ROM_CONTINUE(             0x0b800, 0x0800 )
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x2100, "proms", ROMREGION_ERASEFF ) // see driver notes
	// pink labeled color proms
	ROM_LOAD( "chp1-b-6e_pink.6e",  0x0000, 0x0200, CRC(e9ca3ac6) SHA1(68d9739d8a0dadc6fe3b3767437526096ca5db98) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_pink.6f",  0x0200, 0x0200, CRC(02be56ab) SHA1(a88f332cb26928350ed20ab5f4c04d5324bb516a) )  /* G */
	ROM_LOAD( "chp1-b-7f_pink.7f",  0x0400, 0x0200, CRC(11de55f1) SHA1(269b82f4bc73fac197e0bb6d9a90a220e77ce478) )  /* B */
	ROM_LOAD( "chp1-b-7e_pink.7e",  0x0600, 0x0200, CRC(fddaa777) SHA1(ee6bff5ffeefc82374868a268f885d0bc3a76da2) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_pink.8e",  0x0800, 0x0200, CRC(c3d5d71f) SHA1(2b02479614a801f539fead17860b84e9a180e761) )  /* G */
	ROM_LOAD( "chp1-b-8f_pink.8f",  0x0a00, 0x0200, CRC(a3037155) SHA1(8b6c0c80278ca859a08a1a2429190d51be4f9401) )  /* B */
	// white labeled color proms (indices are reversed)
	ROM_LOAD( "chp1-b-6e_white.6e", 0x1000, 0x0200, CRC(ddac5f0e) SHA1(25dabe415757ccea057609a3f3f79a56b613032a) )  /* R (top monitor) */
	ROM_LOAD( "chp1-b-6f_white.6f", 0x1200, 0x0200, CRC(846c6261) SHA1(f012a02bbdf0166b9bfd3dc9749db18759a22421) )  /* G */
	ROM_LOAD( "chp1-b-7f_white.7f", 0x1400, 0x0200, CRC(1682dd30) SHA1(84b92d1b292210bda0c25537c8ee3e274aaac75c) )  /* B */
	ROM_LOAD( "chp1-b-7e_white.7e", 0x1600, 0x0200, CRC(47adf7a2) SHA1(1d37d5207cd37a9c122251c60cc8f43dd680f484) )  /* R (bottom monitor) */
	ROM_LOAD( "chp1-b-8e_white.8e", 0x1800, 0x0200, CRC(b0fc15a8) SHA1(a1af09cfea81231240bd94f3b98de1be8235ebe7) )  /* G */
	ROM_LOAD( "chp1-b-8f_white.8f", 0x1a00, 0x0200, CRC(1ffd894a) SHA1(9e8c1c28b4c12acf42f814bc109d353729a25652) )  /* B */
	ROM_LOAD( "chp1-v-2d.2d",       0x2000, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )  /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chp1-c.6p",    0x0000, 0x4000, CRC(ea0bbb31) SHA1(b1da024cb688341d39791a78d1144fe09acb00cf) )
ROM_END

ROM_START( spnchout )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chs1-c.8l",    0x0000, 0x2000, CRC(703b9780) SHA1(93b2fd8392ef094413330cd2474ac406c3db426e) )
	ROM_LOAD( "chs1-c.8k",    0x2000, 0x2000, CRC(e13719f6) SHA1(d0f08a0999801dd5d55f2f4ae3e76f25b765b8d6) )
	ROM_LOAD( "chs1-c.8j",    0x4000, 0x2000, CRC(1fa629e8) SHA1(e0c37883e65c77e9f25e323fb4dc05f7dcdc6347) )
	ROM_LOAD( "chs1-c.8h",    0x6000, 0x2000, CRC(15a6c068) SHA1(3f42697a6d79c6fd4b638feb366c80e98a7f02e2) )
	ROM_LOAD( "chs1-c.8f",    0x8000, 0x4000, CRC(4ff3cdd9) SHA1(282edf9a3fa085bc82523249a519f2a3fe04e87e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c.4k",    0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "chs1-b.4c",    0x00000, 0x0800, CRC(9f2ede2d) SHA1(58a0f8c34ff9ec425c846c1eb6c6ccd99c2d0132) )   /* chars #1 */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chs1-b.4d",    0x02000, 0x0800, CRC(143ae5c6) SHA1(4c8426ba336941ac3341b1dd65c0d68b9aae56de) )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )

	ROM_REGION( 0x04000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-b.4a",    0x00000, 0x0800, CRC(c075f831) SHA1(f22d9e415637599420c443ce08e7e70d1eb1c6f5) )   /* chars #2 */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-b.4b",    0x02000, 0x0800, CRC(c4cc2b5a) SHA1(7b9d4dcecc67271980c3c44561fc25a6f6c93ee3) )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chs1-v.2r",    0x00000, 0x4000, CRC(ff33405d) SHA1(31b892d184d24a0ec05fd6facec61a532ce8535b) )   /* chars #3 */
	ROM_LOAD( "chs1-v.2t",    0x04000, 0x4000, CRC(f507818b) SHA1(fb99c5c88e829d7e81c53ead21554a614b6fdcf9) )
	ROM_LOAD( "chs1-v.2u",    0x08000, 0x4000, CRC(0995fc95) SHA1(d056fc61ad2409525622b4db69796668c3145460) )
	ROM_LOAD( "chs1-v.2v",    0x0c000, 0x2000, CRC(f44d9878) SHA1(327a8bbc8f1a33fcf95ebc75db97406feb6435d9) )
	/* 0e000-0ffff empty (space for 16k ROM) */
	ROM_LOAD( "chs1-v.3r",    0x10000, 0x4000, CRC(09570945) SHA1(c3e2a8f76eebacc9042d087db2dfdc8ea267d46a) )
	ROM_LOAD( "chs1-v.3t",    0x14000, 0x4000, CRC(42c6861c) SHA1(2b160cde3cc3ee7adb276fe719f7919c9295ba38) )
	ROM_LOAD( "chs1-v.3u",    0x18000, 0x4000, CRC(bf5d02dd) SHA1(f1f4932fc258c087783450e7c964902fa45c4568) )
	ROM_LOAD( "chs1-v.3v",    0x1c000, 0x2000, CRC(5673f4fc) SHA1(682a81b60494b2c77d1da312c97bc807021eac67) )
	/* 1e000-1ffff empty (space for 16k ROM) */
	ROM_LOAD( "chs1-v.4r",    0x20000, 0x4000, CRC(8e155758) SHA1(d21ce2d81b2d47e5ff091e48cf46d41d01ea6314) )
	ROM_LOAD( "chs1-v.4t",    0x24000, 0x4000, CRC(b4e43448) SHA1(1ed6bf913c15851cf86554713c122b55c18c5d67) )
	ROM_LOAD( "chs1-v.4u",    0x28000, 0x4000, CRC(74e0d956) SHA1(b172cdcc5d26f3be06a7f0f9e19879957e87f992) )
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v.6p",    0x00000, 0x0800, CRC(75be7aae) SHA1(396bc1d301b99e064de4dad699882618b1b9c958) )   /* chars #4 */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-v.6n",    0x02000, 0x0800, CRC(daf74de0) SHA1(9373d4527b675b3128a5a830f42e1dc5dcb85307) )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chp1-v.8p",    0x08000, 0x0800, CRC(4cb7ea82) SHA1(213b7c1431f4c92e5519a8771035bda28b3bab8a) )
	ROM_CONTINUE(             0x09000, 0x0800 )
	ROM_CONTINUE(             0x08800, 0x0800 )
	ROM_CONTINUE(             0x09800, 0x0800 )
	ROM_LOAD( "chp1-v.8n",    0x0a000, 0x0800, CRC(1c0d09aa) SHA1(3276bae7400453f3612f53d7b47fb199cbe53e6d) )
	ROM_CONTINUE(             0x0b000, 0x0800 )
	ROM_CONTINUE(             0x0a800, 0x0800 )
	ROM_CONTINUE(             0x0b800, 0x0800 )
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x2100, "proms", ROMREGION_ERASEFF ) // see driver notes
	// pink labeled color proms
	ROM_LOAD( "chs1-b-6e_pink.6e",  0x0000, 0x0200, CRC(0ad4d727) SHA1(5fa4247d58d10b4644f0a7492efb22b7a9ce7b62) )  /* R (top monitor) */
	ROM_LOAD( "chs1-b-6f_pink.6f",  0x0200, 0x0200, CRC(86f5cfdb) SHA1(a2a3a4e9ca15826fe8c86650d50c8ce203d57eae) )  /* G */
	ROM_LOAD( "chs1-b-7f_pink.7f",  0x0400, 0x0200, CRC(8bd406f8) SHA1(eaf0b62eccf1f47452bf983b3ffc6cacc25d4585) )  /* B */
	ROM_LOAD( "chs1-b-7e_pink.7e",  0x0600, 0x0200, CRC(4c7e3a67) SHA1(a6b3436673ba31e04a7614b71bc4039ad4ce56f1) )  /* R (bottom monitor) */
	ROM_LOAD( "chs1-b-8e_pink.8e",  0x0800, 0x0200, CRC(ec659313) SHA1(a1209650187fce92ef63d77b3ef190e49fdb2e2b) )  /* G */
	ROM_LOAD( "chs1-b-8f_pink.8f",  0x0a00, 0x0200, CRC(8b493c09) SHA1(607d4237ebf41009db567009949d685bceee1f23) )  /* B */
	// white labeled color proms (indices are reversed)
	ROM_LOAD( "chs1-b-6e_white.6e", 0x1000, 0x0200, CRC(8efd867f) SHA1(d5f2bfe750bb5d472922bdb7e915ee28a3eec9bd) )  /* R (top monitor) */
	ROM_LOAD( "chs1-b-6f_white.6f", 0x1200, 0x0200, CRC(279d6cbc) SHA1(aea56970801908b4d51be0c15043c7b315d2637f) )  /* G */
	ROM_LOAD( "chs1-b-7f_white.7f", 0x1400, 0x0200, CRC(cad6b7ad) SHA1(62b61d5fa47ca6e2dd15295674dff62e4e69471a) )  /* B */
	ROM_LOAD( "chs1-b-7e_white.7e", 0x1600, 0x0200, CRC(9e170f64) SHA1(9548bfec2f5b7d222e91562b5459aef8c107b3ec) )  /* R (bottom monitor) */
	ROM_LOAD( "chs1-b-8e_white.8e", 0x1800, 0x0200, CRC(3a2e333b) SHA1(5cf0324cc07ac4af63598c5c6acc61d24215b233) )  /* G */
	ROM_LOAD( "chs1-b-8f_white.8f", 0x1a00, 0x0200, CRC(1663eed7) SHA1(90ff876a6b885f8a80c17531cde8b91864f1a6a5) )  /* B */
	ROM_LOAD( "chs1-v.2d",          0x2000, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )  /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chs1-c.6p",    0x0000, 0x4000, CRC(ad8b64b8) SHA1(0f1232a10faf71b782f9f6653cca8570243c17e0) )
ROM_END

ROM_START( spnchouta )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chs1-c.8l",    0x0000, 0x2000, CRC(703b9780) SHA1(93b2fd8392ef094413330cd2474ac406c3db426e) )    /* Revision e-1 */
	ROM_LOAD( "chs1-c.8k",    0x2000, 0x2000, CRC(e13719f6) SHA1(d0f08a0999801dd5d55f2f4ae3e76f25b765b8d6) )    /* Revision e-1 */
	ROM_LOAD( "chs1-c.8j",    0x4000, 0x2000, CRC(1fa629e8) SHA1(e0c37883e65c77e9f25e323fb4dc05f7dcdc6347) )    /* Revision e */
	ROM_LOAD( "chs1-c.8h",    0x6000, 0x2000, CRC(15a6c068) SHA1(3f42697a6d79c6fd4b638feb366c80e98a7f02e2) )    /* Revision e */
	ROM_LOAD( "chs1-c.8f",    0x8000, 0x4000, CRC(4ff3cdd9) SHA1(282edf9a3fa085bc82523249a519f2a3fe04e87e) )    /* Revision e-1 */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c.4k",    0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chs1-b.4c",    0x00000, 0x2000, CRC(b017e1e9) SHA1(39e98f48bff762a674a2506efa39b3619337a1e0) )   /* chars #1 */ /* Revision A */
	ROM_LOAD( "chs1-b.4d",    0x02000, 0x2000, CRC(e3de9d18) SHA1(f55b6f522e127e6239197dd7eb1564e6f275df74) )   /* Revision A */

	ROM_REGION( 0x04000, "gfx2", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-b.4a",    0x00000, 0x2000, CRC(20fb4829) SHA1(9f0ce9379eb31c19bfacdc514ac6a28aa4217cbb) )   /* chars #2 */ /* Revision A */
	ROM_LOAD( "chp1-b.4b",    0x02000, 0x2000, CRC(edc34594) SHA1(fbb4a8b979d60b183dc23bdbb7425100b9325287) )   /* Revision A */

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chs1-v.2r",    0x00000, 0x4000, CRC(ff33405d) SHA1(31b892d184d24a0ec05fd6facec61a532ce8535b) )   /* chars #3 */
	ROM_LOAD( "chs1-v.2t",    0x04000, 0x4000, CRC(f507818b) SHA1(fb99c5c88e829d7e81c53ead21554a614b6fdcf9) )
	ROM_LOAD( "chs1-v.2u",    0x08000, 0x4000, CRC(0995fc95) SHA1(d056fc61ad2409525622b4db69796668c3145460) )
	ROM_LOAD( "chs1-v.2v",    0x0c000, 0x2000, CRC(f44d9878) SHA1(327a8bbc8f1a33fcf95ebc75db97406feb6435d9) )
	/* 0e000-0ffff empty (space for 16k ROM) */
	ROM_LOAD( "chs1-v.3r",    0x10000, 0x4000, CRC(09570945) SHA1(c3e2a8f76eebacc9042d087db2dfdc8ea267d46a) )
	ROM_LOAD( "chs1-v.3t",    0x14000, 0x4000, CRC(42c6861c) SHA1(2b160cde3cc3ee7adb276fe719f7919c9295ba38) )
	ROM_LOAD( "chs1-v.3u",    0x18000, 0x4000, CRC(bf5d02dd) SHA1(f1f4932fc258c087783450e7c964902fa45c4568) )
	ROM_LOAD( "chs1-v.3v",    0x1c000, 0x2000, CRC(5673f4fc) SHA1(682a81b60494b2c77d1da312c97bc807021eac67) )
	/* 1e000-1ffff empty (space for 16k ROM) */
	ROM_LOAD( "chs1-v.4r",    0x20000, 0x4000, CRC(8e155758) SHA1(d21ce2d81b2d47e5ff091e48cf46d41d01ea6314) )
	ROM_LOAD( "chs1-v.4t",    0x24000, 0x4000, CRC(b4e43448) SHA1(1ed6bf913c15851cf86554713c122b55c18c5d67) )
	ROM_LOAD( "chs1-v.4u",    0x28000, 0x4000, CRC(74e0d956) SHA1(b172cdcc5d26f3be06a7f0f9e19879957e87f992) )
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chp1-v.6p",    0x00000, 0x2000, CRC(16588f7a) SHA1(1aeaaa5cc2477c3aa4bf80df7d9474cc9ded9f15) )   /* chars #4 */ /* Revision A */
	ROM_LOAD( "chp1-v.6n",    0x02000, 0x2000, CRC(dc743674) SHA1(660582c76ee68a7267d5686a2f8ea0fd6c2b25fc) )   /* Revision A */
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chp1-v.8p",    0x08000, 0x2000, CRC(c2db5b4e) SHA1(39d009af597fa28d34af31aec111aa6fe09fea39) )   /* Revision A */
	ROM_LOAD( "chp1-v.8n",    0x0a000, 0x2000, CRC(e6af390e) SHA1(73984cbdc8fbf667126ada63ab9500609eb25c61) )   /* Revision A */
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x2100, "proms", ROMREGION_ERASEFF ) // see driver notes
	// pink labeled color proms
	ROM_LOAD( "chs1-b-6e_pink.6e",  0x0000, 0x0200, CRC(0ad4d727) SHA1(5fa4247d58d10b4644f0a7492efb22b7a9ce7b62) )  /* R (top monitor) */
	ROM_LOAD( "chs1-b-6f_pink.6f",  0x0200, 0x0200, CRC(86f5cfdb) SHA1(a2a3a4e9ca15826fe8c86650d50c8ce203d57eae) )  /* G */
	ROM_LOAD( "chs1-b-7f_pink.7f",  0x0400, 0x0200, CRC(8bd406f8) SHA1(eaf0b62eccf1f47452bf983b3ffc6cacc25d4585) )  /* B */
	ROM_LOAD( "chs1-b-7e_pink.7e",  0x0600, 0x0200, CRC(4c7e3a67) SHA1(a6b3436673ba31e04a7614b71bc4039ad4ce56f1) )  /* R (bottom monitor) */
	ROM_LOAD( "chs1-b-8e_pink.8e",  0x0800, 0x0200, CRC(ec659313) SHA1(a1209650187fce92ef63d77b3ef190e49fdb2e2b) )  /* G */
	ROM_LOAD( "chs1-b-8f_pink.8f",  0x0a00, 0x0200, CRC(8b493c09) SHA1(607d4237ebf41009db567009949d685bceee1f23) )  /* B */
	// white labeled color proms (indices are reversed)
	ROM_LOAD( "chs1-b-6e_white.6e", 0x1000, 0x0200, CRC(8efd867f) SHA1(d5f2bfe750bb5d472922bdb7e915ee28a3eec9bd) )  /* R (top monitor) */
	ROM_LOAD( "chs1-b-6f_white.6f", 0x1200, 0x0200, CRC(279d6cbc) SHA1(aea56970801908b4d51be0c15043c7b315d2637f) )  /* G */
	ROM_LOAD( "chs1-b-7f_white.7f", 0x1400, 0x0200, CRC(cad6b7ad) SHA1(62b61d5fa47ca6e2dd15295674dff62e4e69471a) )  /* B */
	ROM_LOAD( "chs1-b-7e_white.7e", 0x1600, 0x0200, CRC(9e170f64) SHA1(9548bfec2f5b7d222e91562b5459aef8c107b3ec) )  /* R (bottom monitor) */
	ROM_LOAD( "chs1-b-8e_white.8e", 0x1800, 0x0200, CRC(3a2e333b) SHA1(5cf0324cc07ac4af63598c5c6acc61d24215b233) )  /* G */
	ROM_LOAD( "chs1-b-8f_white.8f", 0x1a00, 0x0200, CRC(1663eed7) SHA1(90ff876a6b885f8a80c17531cde8b91864f1a6a5) )  /* B */
	ROM_LOAD( "chs1-v.2d",          0x2000, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )  /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chs1-c.6p",    0x0000, 0x4000, CRC(ad8b64b8) SHA1(0f1232a10faf71b782f9f6653cca8570243c17e0) )
ROM_END

ROM_START( spnchoutj )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chs1c8la.bin", 0x0000, 0x2000, CRC(dc2a592b) SHA1(a8a7fc5c836e2723ba6abcb1137f4c4f79e21c87) )
	ROM_LOAD( "chs1c8ka.bin", 0x2000, 0x2000, CRC(ce687182) SHA1(f07d930d90eda199b089f9023b51fd4456c87bdf) )
	ROM_LOAD( "chs1-c.8j",    0x4000, 0x2000, CRC(1fa629e8) SHA1(e0c37883e65c77e9f25e323fb4dc05f7dcdc6347) )
	ROM_LOAD( "chs1-c.8h",    0x6000, 0x2000, CRC(15a6c068) SHA1(3f42697a6d79c6fd4b638feb366c80e98a7f02e2) )
	ROM_LOAD( "chs1c8fa.bin", 0x8000, 0x4000, CRC(f745b5d5) SHA1(8130b5be011848625ebe6691fbb76dc338979b60) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c.4k",    0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )

	ROM_REGION( 0x04000, "gfx1", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "b_4c_01a.bin", 0x00000, 0x2000, CRC(b017e1e9) SHA1(39e98f48bff762a674a2506efa39b3619337a1e0) )   /* chars #1 */
	ROM_LOAD( "b_4d_01a.bin", 0x02000, 0x2000, CRC(e3de9d18) SHA1(f55b6f522e127e6239197dd7eb1564e6f275df74) )

	ROM_REGION( 0x04000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-b.4a",    0x00000, 0x0800, CRC(c075f831) SHA1(f22d9e415637599420c443ce08e7e70d1eb1c6f5) )   /* chars #2 */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-b.4b",    0x02000, 0x0800, CRC(c4cc2b5a) SHA1(7b9d4dcecc67271980c3c44561fc25a6f6c93ee3) )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chs1-v.2r",    0x00000, 0x4000, CRC(ff33405d) SHA1(31b892d184d24a0ec05fd6facec61a532ce8535b) )   /* chars #3 */
	ROM_LOAD( "chs1-v.2t",    0x04000, 0x4000, CRC(f507818b) SHA1(fb99c5c88e829d7e81c53ead21554a614b6fdcf9) )
	ROM_LOAD( "chs1-v.2u",    0x08000, 0x4000, CRC(0995fc95) SHA1(d056fc61ad2409525622b4db69796668c3145460) )
	ROM_LOAD( "chs1-v.2v",    0x0c000, 0x2000, CRC(f44d9878) SHA1(327a8bbc8f1a33fcf95ebc75db97406feb6435d9) )
	/* 0e000-0ffff empty (space for 16k ROM) */
	ROM_LOAD( "chs1-v.3r",    0x10000, 0x4000, CRC(09570945) SHA1(c3e2a8f76eebacc9042d087db2dfdc8ea267d46a) )
	ROM_LOAD( "chs1-v.3t",    0x14000, 0x4000, CRC(42c6861c) SHA1(2b160cde3cc3ee7adb276fe719f7919c9295ba38) )
	ROM_LOAD( "chs1-v.3u",    0x18000, 0x4000, CRC(bf5d02dd) SHA1(f1f4932fc258c087783450e7c964902fa45c4568) )
	ROM_LOAD( "chs1-v.3v",    0x1c000, 0x2000, CRC(5673f4fc) SHA1(682a81b60494b2c77d1da312c97bc807021eac67) )
	/* 1e000-1ffff empty (space for 16k ROM) */
	ROM_LOAD( "chs1-v.4r",    0x20000, 0x4000, CRC(8e155758) SHA1(d21ce2d81b2d47e5ff091e48cf46d41d01ea6314) )
	ROM_LOAD( "chs1-v.4t",    0x24000, 0x4000, CRC(b4e43448) SHA1(1ed6bf913c15851cf86554713c122b55c18c5d67) )
	ROM_LOAD( "chs1-v.4u",    0x28000, 0x4000, CRC(74e0d956) SHA1(b172cdcc5d26f3be06a7f0f9e19879957e87f992) )
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF )
	ROM_LOAD( "chp1-v.6p",    0x00000, 0x0800, CRC(75be7aae) SHA1(396bc1d301b99e064de4dad699882618b1b9c958) )   /* chars #4 */
	ROM_CONTINUE(             0x01000, 0x0800 )
	ROM_CONTINUE(             0x00800, 0x0800 )
	ROM_CONTINUE(             0x01800, 0x0800 )
	ROM_LOAD( "chp1-v.6n",    0x02000, 0x0800, CRC(daf74de0) SHA1(9373d4527b675b3128a5a830f42e1dc5dcb85307) )
	ROM_CONTINUE(             0x03000, 0x0800 )
	ROM_CONTINUE(             0x02800, 0x0800 )
	ROM_CONTINUE(             0x03800, 0x0800 )
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chp1-v.8p",    0x08000, 0x0800, CRC(4cb7ea82) SHA1(213b7c1431f4c92e5519a8771035bda28b3bab8a) )
	ROM_CONTINUE(             0x09000, 0x0800 )
	ROM_CONTINUE(             0x08800, 0x0800 )
	ROM_CONTINUE(             0x09800, 0x0800 )
	ROM_LOAD( "chp1-v.8n",    0x0a000, 0x0800, CRC(1c0d09aa) SHA1(3276bae7400453f3612f53d7b47fb199cbe53e6d) )
	ROM_CONTINUE(             0x0b000, 0x0800 )
	ROM_CONTINUE(             0x0a800, 0x0800 )
	ROM_CONTINUE(             0x0b800, 0x0800 )
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x2100, "proms", ROMREGION_ERASEFF ) // see driver notes
	// pink labeled color proms
	ROM_LOAD( "chs1-b-6e_pink.6e",  0x0000, 0x0200, CRC(0ad4d727) SHA1(5fa4247d58d10b4644f0a7492efb22b7a9ce7b62) )  /* R (top monitor) */
	ROM_LOAD( "chs1-b-6f_pink.6f",  0x0200, 0x0200, CRC(86f5cfdb) SHA1(a2a3a4e9ca15826fe8c86650d50c8ce203d57eae) )  /* G */
	ROM_LOAD( "chs1-b-7f_pink.7f",  0x0400, 0x0200, CRC(8bd406f8) SHA1(eaf0b62eccf1f47452bf983b3ffc6cacc25d4585) )  /* B */
	ROM_LOAD( "chs1-b-7e_pink.7e",  0x0600, 0x0200, CRC(4c7e3a67) SHA1(a6b3436673ba31e04a7614b71bc4039ad4ce56f1) )  /* R (bottom monitor) */
	ROM_LOAD( "chs1-b-8e_pink.8e",  0x0800, 0x0200, CRC(ec659313) SHA1(a1209650187fce92ef63d77b3ef190e49fdb2e2b) )  /* G */
	ROM_LOAD( "chs1-b-8f_pink.8f",  0x0a00, 0x0200, CRC(8b493c09) SHA1(607d4237ebf41009db567009949d685bceee1f23) )  /* B */
	// white labeled color proms (indices are reversed)
	ROM_LOAD( "chs1-b-6e_white.6e", 0x1000, 0x0200, CRC(8efd867f) SHA1(d5f2bfe750bb5d472922bdb7e915ee28a3eec9bd) )  /* R (top monitor) */
	ROM_LOAD( "chs1-b-6f_white.6f", 0x1200, 0x0200, CRC(279d6cbc) SHA1(aea56970801908b4d51be0c15043c7b315d2637f) )  /* G */
	ROM_LOAD( "chs1-b-7f_white.7f", 0x1400, 0x0200, CRC(cad6b7ad) SHA1(62b61d5fa47ca6e2dd15295674dff62e4e69471a) )  /* B */
	ROM_LOAD( "chs1-b-7e_white.7e", 0x1600, 0x0200, CRC(9e170f64) SHA1(9548bfec2f5b7d222e91562b5459aef8c107b3ec) )  /* R (bottom monitor) */
	ROM_LOAD( "chs1-b-8e_white.8e", 0x1800, 0x0200, CRC(3a2e333b) SHA1(5cf0324cc07ac4af63598c5c6acc61d24215b233) )  /* G */
	ROM_LOAD( "chs1-b-8f_white.8f", 0x1a00, 0x0200, CRC(1663eed7) SHA1(90ff876a6b885f8a80c17531cde8b91864f1a6a5) )  /* B */
	ROM_LOAD( "chs1-v.2d",          0x2000, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )  /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chs1c6pa.bin", 0x0000, 0x4000, CRC(d05fb730) SHA1(9f4c4c7e5113739312558eff4d3d3e42d513aa31) )
ROM_END

ROM_START( armwrest )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for code */
	ROM_LOAD( "chv1-c.8l",    0x0000, 0x2000, CRC(b09764c1) SHA1(2f32acd689ef70ec81fe958c7a604855ae39cf5e) )
	ROM_LOAD( "chv1-c.8k",    0x2000, 0x2000, CRC(0e147ff7) SHA1(7ea8b7b5562d9432c6cace2ee13377f91543975d) )
	ROM_LOAD( "chv1-c.8j",    0x4000, 0x2000, CRC(e7365289) SHA1(9d4ed5ce73b93c3917b1411ed902974e2a4f3d35) )
	ROM_LOAD( "chv1-c.8h",    0x6000, 0x2000, CRC(a2118eec) SHA1(93e1b19819352f88888b3caf67ed27cd50f866a9) )
	ROM_LOAD( "chpv-c.8f",    0x8000, 0x4000, CRC(664a07c4) SHA1(a8a049be5beeab3940079465fb0c80382f3860f0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the sound CPU */
	ROM_LOAD( "chp1-c.4k",    0xe000, 0x2000, CRC(cb6ef376) SHA1(503dbcc1b18a497311bf129689d5650860bf96c7) )    /* same as Punch Out */

	ROM_REGION( 0x08000, "gfx1", ROMREGION_ERASEFF )
	ROM_LOAD( "chpv-b.2e",    0x00000, 0x4000, CRC(8b45f365) SHA1(15fadccc9afe26672fbbb8eaeaa7d3ee70bcb056) )   /* chars #1 */
	ROM_LOAD( "chpv-b.2d",    0x04000, 0x4000, CRC(b1a2850c) SHA1(e3aec428bb52443921fb7ceb5eb21b5f9ee9edcb) )

	ROM_REGION( 0x0c000, "gfx2", ROMREGION_ERASEFF )
	ROM_LOAD( "chpv-b.2m",    0x00000, 0x4000, CRC(19245b37) SHA1(711e263d487661afca09f731e9333a84eb8d1541) )   /* chars #2 */
	ROM_LOAD( "chpv-b.2l",    0x04000, 0x4000, CRC(46797941) SHA1(e21fcec8e19702f9765205a4dc89105b4e98dcdd) )
	ROM_LOAD( "chpv-b.2k",    0x0a000, 0x2000, CRC(de189b00) SHA1(62b38d5f95bb4f0a0d04947c7c2031e07f95cbe4) )

	ROM_REGION( 0x30000, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "chv1-v.2r",    0x00000, 0x4000, CRC(d86056d9) SHA1(decedf6b54e5990ff14d8049791b2d06c33ae71b) )   /* chars #3 */
	ROM_LOAD( "chv1-v.2t",    0x04000, 0x4000, CRC(5ad77059) SHA1(05a1c7957982fa695bca62a05dc593c7913ccd7f) )
	/* 08000-0bfff empty */
	ROM_LOAD( "chv1-v.2v",    0x0c000, 0x4000, CRC(a0fd7338) SHA1(afd8d78661c3b7149f4c491ba930a8ce66d29977) )
	ROM_LOAD( "chv1-v.3r",    0x10000, 0x4000, CRC(690e26fb) SHA1(6c20daabf5db633482b288c8020130a80cc939fc) )
	ROM_LOAD( "chv1-v.3t",    0x14000, 0x4000, CRC(ea5d7759) SHA1(4d72d7b602455349be4a9cbf34127952aa2a99ea) )
	/* 18000-1bfff empty */
	ROM_LOAD( "chv1-v.3v",    0x1c000, 0x4000, CRC(ceb37c05) SHA1(9d0e3d52e018901c2f26a9de7aa9858b106487d3) )
	ROM_LOAD( "chv1-v.4r",    0x20000, 0x4000, CRC(e291cba0) SHA1(a03ff7eea3a7a841000b67a8baeca6e82e8496ef) )
	ROM_LOAD( "chv1-v.4t",    0x24000, 0x4000, CRC(e01f3b59) SHA1(9f47507094e03735adaf033f3b99e17dd9dfd5d0) )
	/* 28000-2bfff empty */
	/* 2c000-2ffff empty (4v doesn't exist, it is seen as a 0xff fill) */

	ROM_REGION( 0x10000, "gfx4", ROMREGION_ERASEFF | ROMREGION_INVERT )
	ROM_LOAD( "chv1-v.6p",    0x00000, 0x2000, CRC(d834e142) SHA1(e7d654145b695147b744af2284173f90749fbf0e) )   /* chars #4 */
	/* 02000-03fff empty (space for 16k ROM) */
	/* 04000-07fff empty (space for 6l and 6k) */
	ROM_LOAD( "chv1-v.8p",    0x08000, 0x2000, CRC(a2f531db) SHA1(c9be180fbc608135c892e8ee396b138f058edf24) )
	/* 0a000-0bfff empty (space for 16k ROM) */
	/* 0c000-0ffff empty (space for 8l and 8k) */

	ROM_REGION( 0x0e00, "proms", 0 )
	ROM_LOAD( "chpv-b.7b",    0x0000, 0x0200, CRC(df6fdeb3) SHA1(7766d420cb95377104e26d96afddc83b67553c2f) )    /* R (top monitor) */
	ROM_LOAD( "chpv-b.7c",    0x0200, 0x0200, CRC(b1da5f42) SHA1(55e744da70bbaa855cb1403eef028771a97578a1) )    /* G */
	ROM_LOAD( "chpv-b.7d",    0x0400, 0x0200, CRC(4ede813e) SHA1(6603465dae7d869c483d66768fab16f282caaa8b) )    /* B */
	ROM_LOAD( "chpv-b.4b",    0x0600, 0x0200, CRC(9d51416e) SHA1(ae933786c5fc19311144b2094305b4253dc8b75b) )    /* R (bottom monitor) */
	ROM_LOAD( "chpv-b.4c",    0x0800, 0x0200, CRC(b8a25795) SHA1(8e41baa796fd8f00739a95b2e07066d68193bd76) )    /* G */
	ROM_LOAD( "chpv-b.4d",    0x0a00, 0x0200, CRC(474fc3b1) SHA1(9cda1d1626285310524d048b60b1cf89e197a26d) )    /* B */
	ROM_LOAD( "chv1-b.3c",    0x0c00, 0x0100, CRC(c3f92ea2) SHA1(1a82cca1b9a8d9bd4a1d121d8c131a7d0be554bc) )    /* priority encoder - not used */
	ROM_LOAD( "chpv-v.2d",    0x0d00, 0x0100, CRC(71dc0d48) SHA1(dd6609f547d74887f520d7e71a1a00317ff181d0) )    /* timing - not used */

	ROM_REGION( 0x4000, "vlm", 0 )  /* 16k for the VLM5030 data */
	ROM_LOAD( "chv1-c.6p",    0x0000, 0x4000, CRC(31b52896) SHA1(395f59ac38b46042f79e9224ac6bc7d3dc299906) )
ROM_END



GAME( 1984, punchout,  0,        punchout, punchout, punchout_state, empty_init, ROT0, "Nintendo", "Punch-Out!! (Rev B)", 0 ) /* CHP1-02 boards */
GAME( 1984, punchouta, punchout, punchout, punchout, punchout_state, empty_init, ROT0, "Nintendo", "Punch-Out!! (Rev A)", 0 ) /* CHP1-01 boards */
GAME( 1984, punchoutj, punchout, punchout, punchout, punchout_state, empty_init, ROT0, "Nintendo", "Punch-Out!! (Japan)", 0 )
GAME( 1984, punchita,  punchout, punchout, punchout, punchout_state, empty_init, ROT0, "bootleg",  "Punch-Out!! (Italian bootleg)", 0 )
GAME( 1984, spnchout,  0,        spnchout, spnchout, punchout_state, empty_init, ROT0, "Nintendo", "Super Punch-Out!! (Rev B)", 0 ) /* CHP1-02 boards */
GAME( 1984, spnchouta, spnchout, spnchout, spnchout, punchout_state, empty_init, ROT0, "Nintendo", "Super Punch-Out!! (Rev A)", 0 ) /* CHP1-01 boards */
GAME( 1984, spnchoutj, spnchout, spnchout, spnchout, punchout_state, empty_init, ROT0, "Nintendo", "Super Punch-Out!! (Japan)", 0 )
GAME( 1985, armwrest,  0,        armwrest, armwrest, punchout_state, empty_init, ROT0, "Nintendo", "Arm Wrestling", 0 )
