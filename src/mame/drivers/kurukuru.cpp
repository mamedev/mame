// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, hap
/******************************************************************************

  KURU KURU PYON PYON
  Taiyo Jidoki / Success


  Driver by Roberto Fresca & hap.


  This hardware seems to be a derivative of MSX2 'on steroids'.
  It has many similarites with sothello.c and tonton.c

  Special thanks to Charles MacDonald, for all the hardware traces: clocks,
  ports, descriptions and a lot of things... :)


*******************************************************************************

  Technical Notes....


  CPU   : 2x Sharp LH0080A Z80A

  MEM   : 1x Sharp LH5116H-10 (2KB SRAM)
          1x Fairchild 8464A-10L (8KB SRAM) + battery
          6x Sharp LH2464-15 (192KB Video DRAM total)

  SOUND : 1x Yamaha YM2149F
          1x OKI M5205

  VIDEO : 1x Unknown 64-legs VDP (seems to be from V9938/58 family).

  XTAL  : 1x 21477.27 kHz.
          1x 384 kHz. blue resonator (CSB 384 P) surely for the M5202.


  1x Texas Instruments RC4558P (Dual General-Purpose Operational Amplifier, DIP8)
  1x Fairchild MB3712 (5,7 Watt Audio Power Amplifier, SIP8).
  2x 8 DIP switches banks.

  3x PAL16L8A (IC12, IC26 & IC27)
  1x PAL12L6 (IC32)


  PCB Layout...

  .--------------------------------------------------------------------------------------------------.
  |                                   IC4                  IC5                                       |
  |    IC2            IC3           .------------------.  .----------------.            BATT         |
  |  .-----------.  .-----------.   | M5L27512K        |  | LH5116H-10     |          .---------.    |
  |  | LH2464-15 |  | SN74HC04N |   |                  |  | SHARP JAPAN    |          |  3.6 V  |    |
  |  '-----------'  '-----------'   | '4'              |  |                |         -+         +-   |
  |    IC6                          |                  |  '----------------'          | BATTERY |    |
  |  .-----------.    IC8           '------------------'                              '---------'    |
  |  | LH2464-15 |  .----------------------------.       IC9                      IC10               |
  |  '-----------'  | LH0080A Z80A-CPU-D         |     .-----------------.      .-----------------.  |
  |    IC7          | SHARP JAPAN                |     | 8464A-10L       |      | M5L27512K       |  |
  |  .-----------.  |                            |     | |F| JAPAN       |      |                 |  |
  |  | LH2464-15 |  |                            |     |                 |      | '10'            |  |
  |  '-----------'  '----------------------------'     |                 |      |                 |  |
  |    IC11           IC12            IC13             '-----------------'      '-----------------'  |
  |  .-----------.  .------------.  .------------.       IC17                     IC18               |
  |  | LH2464-15 |  | PAL16L8ACJ |  | SN74LS74AN |     .-----------------.      .-----------------.  |
  |  '-----------'  '------------'  '------------'     | MBM27256-25     |      | M5L27512K       |  |
  |    IC14           IC15            IC16             |                 |      |                 |  |
  |  .-----------.  .------------.  .-------------.    | 'KP 17L'        |      | '18'            |  |
  |  | LH2464-15 |  | SN74LS08N  |  | SN74LS125AN |    |                 |      |                 |  |
  |  '-----------'  '------------'  '-------------'    '-----------------'      '-----------------'  |
  |    IC19           IC20            IC21           IC22                         IC23               |
  |  .-----------.  .------------.  .------------. .--------------------------. .-----------------.  |
  |  | LH2464-15 |  | SN74LS174N |  | SN74LS174N | | LH0080A Z80A-CPU-D       | |                 |  |
  |  '-----------'  '------------'  '------------' | SHARP JAPAN              | |    E M P T Y    |  |
  |                                   X2           |                          | |   S O C K E T   |  |
  |                                 .--------.     |                          | |                 |  |
  |    X1             IC24          | | RES  |     '--------------------------' '-----------------'  |
  |  .-----.        .------------.  | |384kHz|                                                       |
  |  |XTAL |        |OKI MSM5205 |  '--------'                                                       |
  |  |21.47|        '------------'    IC25             IC26            IC27            IC28          |
  |  |727  |                        .-------------.  .------------.  .------------.  .------------.  |
  |  '-----'                        | SN74LS374N  |  | PAL16L8ACJ |  | PAL16L8ACJ |  | HD74LS153P |  |
  |                                 '-------------'  '------------'  '------------'  '------------'  |
  |    IC29                           IC30             DSW1            IC31            IC32          |
  |  .-----------------------------. .----.          .------------.  .------------.  .------------.  |
  |  | SANDED...                   | | AA |          |  11111111  |  | SN74LS174N |  | PAL12L6CN  |  |
  |  | (YAMAHA V9938 VDP)          | '----'          '------------'  '------------'  '------------'  |
  |  |                             |                   IC33                           '7908B-4'      |
  |  | '81500'                     |                 .------------.    IC34                          |
  |  |                             |                 | SN74LS245N |  .----------------------------.  |
  |  |                             |                 '------------'  | YAMAHA                     |  |
  |  '-----------------------------'               DSW2              | YM2149F                    |  |
  |                                              .------------.      |                            |  |
  |                                              |  01111110  |      |                            |  |
  |                                              '------------'      '----------------------------'  |
  |   IC35    IC36    IC37    IC38     IC39            IC40            IC41                          |
  |  .----.  .----.  .----.  .----.  .------------.  .------------.  .------------.                  |
  |  | AB |  | AB |  | AB |  | AB |  | HD74LS273P |  | SN74LS245N |  | SN74LS245N |  .------------.  |
  |  '----'  '----'  '----'  '----'  '------------'  '------------'  '------------'  |  IC43      |  |
  |                                      IC42                                        |.----------.|  |
  |                                    .----------.   ||||||||||||    ||||||||||||   ||MB3712 M92||  |
  |                                    | ULN2003A |   +++RESNET+++    +++RESNET+++   |'----------'|  |
  |                                    '----------'   ||||||||||||    ||||||||||||   '------------'  |
  |                                                                                                  |
  | 7908-B                                                                             SUCCESS CORP. |
  |         .----------.           .--.                                         .----------.         |
  |         |          | | | | | | |  | | | | | | | | | | | | | | | | | | | | | |          |         |
  |         |          | | | | | | |  | | | | | | | | | | | | | | | | | | | | | |          |         |
  '---------'          '-----------'  '-----------------------------------------'          '---------'
                       2x6 edge conn             2x21 edge connector


  AA =  Texas Instruments RC4558P T835AJ34.
  AB =  NEC C1663C 8926B.


*******************************************************************************

  General Notes....

  The game name could be translated as "Croak Croak Hop Hop"
  Kuru is the frog sound, and Pyon is the sound of jumps.

  Coin 1 (key 5) could be set either as Coin 1 or as Payout button, through
  a DIP switch.

  If you get a 'medal jam' error, and the game is not responding anymore, press
  RESET (key 0), and the game will reset to default values (even all counters
  will be cleared).


*******************************************************************************

  * How to play...

  Insert tokens (medals)...

  You can bet to any (or all) of the following 5 characters: Bote, Oume, Pyoko,
  Kunio, and Pyon Pyon. Press start, and the reels start to roll. You'll win if
  you can get 3 of the choosen character(s) in a row, column or diagonal.

  The black tadpoles behave just like jokers... If you have 2 choosen characters
  in a row and the remaining one is a black tadpole, it will transform into another
  character to complete the 3 in a row, allowing you to win.

  Red tadpoles are a bonus. Once you get one, it will go to the right panel,
  revealing a number. This number represents the extra credits you won.


  * Bookkeeping...

  Pressing Bookkeeping key (key 9), you enter the Bookkeeping Mode. There are
  2 screens with all the game information like DIP switches and statistics...

  1st screen...

  - [Left panel]:  All the DIP switches parameters.

  - [Right panel]: Bet and Win totals, 100Y/10Y/medal IN/OUT, total of games,
                   won, loss, won by paid range, and 'omake' (extra/bonus).

  2nd screen (press Bookkeeping key again)...

  - Tate (vertical), Yoko (horizontal) and Naname (diagonal),
    for each character (Bote, Oume, Pyoko, Kunio and Pyon Pyon).

    Also Aka (red) and Kuro (black).

  Pressing the Bookkeeping key once more, you exit the mode and go back to the game.


*******************************************************************************

  ADPCM Samples....

  There are 14 samples in the system.

  00: "Boterin"
  01:
  02: "Hakase" ("professor")
  03: "Pyokorin"
  04: "Kunio"
  05: "Pyon Pyon"
  06:
  07:
  08: "Oume"
  09: "Haipaa" ("hyper")
  10: "Ichi ni tsuite" ("on your marks")
  11: "Youi" ("get ready")
  12: Bang sound for the tadpoles landing in the right panel.
  13: Sound effect for reels when running.


******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "video/v9938.h"
#include "machine/ticket.h"
#include "machine/nvram.h"

class kurukuru_state : public driver_device
{
public:
	kurukuru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_v9938(*this, "v9938"),
		m_maincpu(*this, "maincpu"),
		m_adpcm(*this, "adpcm"),
		m_hopper(*this, "hopper"),
		m_bank1(*this, "bank1")
	{ }

	required_device<cpu_device> m_audiocpu;
	required_device<v9938_device> m_v9938;

	UINT8 m_sound_irq_cause;
	UINT8 m_adpcm_data;

	DECLARE_WRITE8_MEMBER(kurukuru_out_latch_w);
	DECLARE_WRITE8_MEMBER(kurukuru_bankswitch_w);
	DECLARE_WRITE8_MEMBER(kurukuru_soundlatch_w);
	DECLARE_READ8_MEMBER(kurukuru_soundlatch_r);
	DECLARE_WRITE8_MEMBER(kurukuru_adpcm_reset_w);
	DECLARE_READ8_MEMBER(kurukuru_adpcm_timer_irqack_r);
	DECLARE_WRITE8_MEMBER(kurukuru_adpcm_data_w);
	DECLARE_WRITE8_MEMBER(ym2149_aout_w);
	DECLARE_WRITE8_MEMBER(ym2149_bout_w);

	void update_sound_irq(UINT8 cause);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_WRITE_LINE_MEMBER(kurukuru_msm5205_vck);
	DECLARE_WRITE_LINE_MEMBER(kurukuru_vdp_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_adpcm;
	required_device<ticket_dispenser_device> m_hopper;
	required_memory_bank m_bank1;
};

#define MAIN_CLOCK      XTAL_21_4772MHz
#define CPU_CLOCK       MAIN_CLOCK/6
#define YM2149_CLOCK    MAIN_CLOCK/6/2  // '/SEL' pin tied to GND, so internal divisor x2 is active
#define M5205_CLOCK     XTAL_384kHz

#define HOPPER_PULSE    50          // time between hopper pulses in milliseconds
#define VDP_MEM         0x30000


/*************************************************
*                  Interrupts                    *
*************************************************/

WRITE_LINE_MEMBER(kurukuru_state::kurukuru_vdp_interrupt)
{
	m_maincpu->set_input_line(0, (state ? ASSERT_LINE : CLEAR_LINE));
}


void kurukuru_state::update_sound_irq(UINT8 cause)
{
	m_sound_irq_cause = cause & 3;
	if (m_sound_irq_cause)
	{
		// use bit 0 for latch irq, and bit 1 for timer irq
		// latch irq vector is $ef (rst $28)
		// timer irq vector is $f7 (rst $30)
		// if both are asserted, the vector becomes $f7 AND $ef = $e7 (rst $20)
		const UINT8 irq_vector[4] = { 0x00, 0xef, 0xf7, 0xe7 };
		m_audiocpu->set_input_line_and_vector(0, ASSERT_LINE, irq_vector[m_sound_irq_cause]);
	}
	else
	{
		m_audiocpu->set_input_line(0, CLEAR_LINE);
	}
}


WRITE_LINE_MEMBER(kurukuru_state::kurukuru_msm5205_vck)
{
	update_sound_irq(m_sound_irq_cause | 2);
	m_adpcm->data_w(m_adpcm_data);
}


/*************************************************
*               Memory Map / I/O                 *
*************************************************/

// Main CPU

WRITE8_MEMBER(kurukuru_state::kurukuru_out_latch_w)
{
/*
   00-0f is output latch (controls jamma output pins)

    BIT  EDGE CONNECTOR       JAMMA FUNCTION
   ----+--------------------+----------------
    00 | Pin 08 top side    | coin counter 1
    01 | Pin 09 top side    | coin lockout 1
    02 | Pin 14 bottom side | service switch
    03 | Pin 11 bottom side | unused
    04 | Pin 11 top side    | unused
    05 | Pin 08 bottom side | coin counter 2
    06 | Pin 09 bottom side | coin lockout 2
    07 | Not connected      | unused

*/
	machine().bookkeeping().coin_counter_w(0, data & 0x01);      /* Coin Counter 1 */
	machine().bookkeeping().coin_counter_w(1, data & 0x20);      /* Coin Counter 2 */
	machine().bookkeeping().coin_lockout_global_w(data & 0x40);  /* Coin Lock */
	m_hopper->write(space, 0, (data & 0x40));    /* Hopper Motor */

	if (data & 0x9e)
		logerror("kurukuru_out_latch_w %02X @ %04X\n", data, space.device().safe_pc());
}

WRITE8_MEMBER(kurukuru_state::kurukuru_bankswitch_w)
{
	m_bank1->set_entry(7); // remove banked rom
/*
    if bits 5,4 are 00,10,01 then IC10 is enabled
    if bits 3,2 are 00,10,01 then IC18 is enabled
    if bits 1,0 are 00,10,01 then IC23 is enabled
    Then in addition, A15 (ROM half) is determined by the low bit.
    Note that in theory, it can cause a conflict by enabling more than one chip,
    but the game never does this.
*/
	for (int chip = 0; chip < 3; chip++)
	{
		if ((data & 3) != 3)
			m_bank1->set_entry((chip << 1) | (~data & 1));
		data >>= 2;
	}
}

WRITE8_MEMBER(kurukuru_state::kurukuru_soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	update_sound_irq(m_sound_irq_cause | 1);
}


static ADDRESS_MAP_START( kurukuru_map, AS_PROGRAM, 8, kurukuru_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kurukuru_io, AS_IO, 8, kurukuru_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x0f) AM_WRITE(kurukuru_out_latch_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0f) AM_READ_PORT("DSW1")
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x0f) AM_WRITE(kurukuru_soundlatch_w)
	AM_RANGE(0x80, 0x83) AM_MIRROR(0x0c) AM_DEVREADWRITE( "v9938", v9938_device, read, write )
	AM_RANGE(0x90, 0x90) AM_MIRROR(0x0f) AM_WRITE(kurukuru_bankswitch_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_READ_PORT("IN0")
	AM_RANGE(0xb0, 0xb0) AM_MIRROR(0x0f) AM_READ_PORT("IN1")
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x0f) AM_DEVREADWRITE("ym2149", ay8910_device, data_r, address_w)
	AM_RANGE(0xd0, 0xd0) AM_MIRROR(0x0f) AM_DEVWRITE("ym2149", ay8910_device, data_w)
ADDRESS_MAP_END


// Audio CPU

WRITE8_MEMBER(kurukuru_state::kurukuru_adpcm_data_w)
{
/*
     6-bit latch. only 4 connected...
       bit 0-3 = MSM5205 data.
*/
	m_adpcm_data = data & 0xf;
}

WRITE8_MEMBER(kurukuru_state::kurukuru_adpcm_reset_w)
{
/*
     6-bit latch. only 4 connected...
       bit 0 = RESET
       bit 1 = 4B/3B
       bit 2 = S2
       bit 3 = S1
*/
	m_adpcm->playmode_w(BITSWAP8((data>>1), 7,6,5,4,3,0,1,2));
	m_adpcm->reset_w(data & 1);
}

READ8_MEMBER(kurukuru_state::kurukuru_soundlatch_r)
{
	update_sound_irq(m_sound_irq_cause & ~1);
	return soundlatch_byte_r(space, 0);
}

READ8_MEMBER(kurukuru_state::kurukuru_adpcm_timer_irqack_r)
{
	update_sound_irq(m_sound_irq_cause & ~2);
	return 0;
}


static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, kurukuru_state )
	AM_RANGE(0x0000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( audio_io, AS_IO, 8, kurukuru_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7f)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x0f) AM_WRITE(kurukuru_adpcm_data_w)
	AM_RANGE(0x50, 0x50) AM_MIRROR(0x0f) AM_WRITE(kurukuru_adpcm_reset_w)
	AM_RANGE(0x60, 0x60) AM_MIRROR(0x0f) AM_READ(kurukuru_soundlatch_r)
	AM_RANGE(0x70, 0x70) AM_MIRROR(0x0f) AM_READ(kurukuru_adpcm_timer_irqack_r)
ADDRESS_MAP_END


/* YM2149 ports */
WRITE8_MEMBER(kurukuru_state::ym2149_aout_w)
{
	logerror("YM2149: Port A out: %02X\n", data);
}

WRITE8_MEMBER(kurukuru_state::ym2149_bout_w)
{
	logerror("YM2149: Port B out: %02X\n", data);
}


/*************************************************
*            Input Ports Definitions             *
*************************************************/

static INPUT_PORTS_START( kurukuru )
	PORT_START("IN0")
/*  bits d0-d3 are JAMMA top side pins 20,21,22,23, bits d4-d7 are JAMMA bottom side pins 20,21,22,23
    so that's player 1 left/right/button1/button2 then player 2 left/right/button1/button2
*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("1st (Bote)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("2nd (Oume)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("3rd (Pyoko)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("4th (Kunio)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("5th (Pyon Pyon)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Unknown A0h - bit5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_M) PORT_NAME("Unknown A0h - bit6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
/*  routed to JAMMA top side 15, bottom 15, top 16, bottom 16, top 17, bottom 17, top 24, bottom 24
    so that's test, tilt/slam, coin 1, coin 2, p1 start, p2 start, p1 button 3, p2 button 3
*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Bookkeeping")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("Medal In")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Reset Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_CODE(KEYCODE_A) PORT_NAME("Unknown B0h - bit4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_IMPULSE (2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)    // hopper feedback
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("DSW1")  // found in the PCB: 11111111
	PORT_DIPNAME( 0x07, 0x00, "Coinage A (100 Y)" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x02, "1 Coin / 3 Medal" )
	PORT_DIPSETTING(    0x06, "1 Coin / 4 Medal" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Medal" )
	PORT_DIPSETTING(    0x05, "1 Coin / 6 Medal" )
	PORT_DIPSETTING(    0x03, "1 Coin / 10 Medal" )
	PORT_DIPSETTING(    0x07, "1 Coin / 11 Medal" )
	PORT_DIPSETTING(    0x04, "1 Coin / 20 Medal" )
	PORT_DIPSETTING(    0x00, "1 Coin / 50 Medal" )
	PORT_DIPNAME( 0x18, 0x00, "Coinage B (10 Y)" )  PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, "3 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x10, "2 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x18, "1 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x08, "1 Coin / 2 Medal" )
	PORT_DIPNAME( 0x20, 0x00, "Coinage Config" )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, "Coin 1 = Normal; Medal In = 2 Credits by Medal" )
	PORT_DIPSETTING(    0x20, "Coin 1 = Payout; Medal In = 1 Credit by Medal" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x80, 0x00, "Repeat Last Bet")    PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("DSW2")  // found in the PCB: 01111110
	PORT_DIPNAME( 0x07, 0x01, "Percentage" )    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x08, 0x08, "Winwave" )       PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPSETTING(    0x00, "Big" )
	PORT_DIPNAME( 0x10, 0x10, "M.Medal" )       PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "HG" )            PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(    0x60, "10-1" )
	PORT_DIPSETTING(    0x20, "20-1" )
	PORT_DIPSETTING(    0x40, "50-1" )
	PORT_DIPSETTING(    0x00, "100-1" )
	PORT_DIPNAME( 0x80, 0x80, "Bet Max" )       PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "10" )

INPUT_PORTS_END


/*************************************************
*        Machine Start & Reset Routines          *
*************************************************/

void kurukuru_state::machine_start()
{
	m_bank1->configure_entries(0, 8, memregion("user1")->base(), 0x8000);
}

void kurukuru_state::machine_reset()
{
	update_sound_irq(0);
}

/*************************************************
*                 Machine Driver                 *
*************************************************/

static MACHINE_CONFIG_START( kurukuru, kurukuru_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(kurukuru_map)
	MCFG_CPU_IO_MAP(kurukuru_io)

	MCFG_CPU_ADD("audiocpu", Z80, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_io)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_V9938_ADD("v9938", "screen", VDP_MEM, MAIN_CLOCK)
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(kurukuru_state,kurukuru_vdp_interrupt))
	MCFG_V99X8_SCREEN_ADD_NTSC("screen", "v9938", MAIN_CLOCK)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(HOPPER_PULSE), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym2149", YM2149, YM2149_CLOCK)
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW2"))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(kurukuru_state, ym2149_aout_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(kurukuru_state, ym2149_bout_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("adpcm", MSM5205, M5205_CLOCK)
	MCFG_MSM5205_VCLK_CB(WRITELINE(kurukuru_state, kurukuru_msm5205_vck))
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* changed on the fly */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kurukuru )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "kp_17l.ic17",  0x00000, 0x08000, CRC(9b552ebc) SHA1(07d0e62b7fdad381963a345376b72ad31eb7b96d) ) // program code

	ROM_REGION( 0x40000, "user1", 0 ) // maincpu banked roms
	ROM_FILL(                 0x00000, 0x10000, 0xff )                                                         // ic23: unpopulated
	ROM_LOAD( "18.ic18",      0x10000, 0x10000, CRC(afb13c6a) SHA1(ac3cd40fad081f7a2b3d1fc72ea96282b9d1f4a3) ) // ic18: big frog gfx
	ROM_LOAD( "10.ic10",      0x20000, 0x10000, CRC(3d6012bc) SHA1(2764f70e0e0bef3f2f71dd6c78e0a4189057beca) ) // ic10: title + text + ingame gfx
	ROM_FILL(                 0x30000, 0x10000, 0xff )                                                         // dummy entry for when no romchip is selected

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "4.ic4",        0x00000, 0x10000, CRC(85d86f32) SHA1(f2aa93d702e6577f8f2204c74c44ac26d05be699) ) // code & adpcm samples

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "51.ic26",      0x0000, 0x0104, CRC(ce4a601b) SHA1(07f5bbb327b220e5846927cbb91149174dd07b36) )
	ROM_LOAD( "52.ic27",      0x0200, 0x0104, CRC(e23296a5) SHA1(4747923d201fcc5e0e752acbf50b41f0414e4ca8) )
	ROM_LOAD( "53.ic12",      0x0400, 0x0104, CRC(2ac654f2) SHA1(18668c73781a55dcffc4bf4c107026b0e72a75d1) )
	ROM_LOAD( "7908b-4.ic32", 0x0600, 0x0034, CRC(bddf925e) SHA1(861cf5966444d0c0392241e5cfa08db475fb439a) )
ROM_END


/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT  ROT    COMPANY                   FULLNAME                       FLAGS  */
GAME( 199?, kurukuru, 0,      kurukuru, kurukuru, driver_device, 0,    ROT0, "Success / Taiyo Jidoki", "Kuru Kuru Pyon Pyon (Japan)",  0 )
