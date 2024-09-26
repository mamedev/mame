// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, hap
/***********************************************************************************

  KURU KURU PYON PYON
  Success / Taiyo Jidoki

  Driver by Roberto Fresca & hap.


  This hardware seems to be a derivative of MSX2 'on steroids'.
  It has many similarities with sothello.cpp and tonton.cpp

  Special thanks to Charles MacDonald, for all the hardware traces: clocks,
  ports, descriptions and a lot of things... :)


  Pyon Pyon series:

  ぴょんぴょん (Pyon Pyon),                1988 Success / Taiyo Jidoki.
  くるくるぴょんぴょん (Kuru Kuru Pyon Pyon), 1990 Success / Taiyo Jidoki.
  ぴょんぴょんジャンプ (Pyon Pyon Jump),     1991 Success / Taiyo Jidoki.
  スイスイぴょんぴょん (Sui Sui Pyon Pyon),  1992 Success / Taiyo Jidoki.


************************************************************************************

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
  | 7908-B                         hole                                                SUCCESS CORP. |
  |         .----------.           .--.                                         .----------.         |
  |         |          | | | | | | |  | | | | | | | | | | | | | | | | | | | | | |          |         |
  |         |          | | | | | | |  | | | | | | | | | | | | | | | | | | | | | |          |         |
  '---------'          '-----------'  '-----------------------------------------'          '---------'
                        <-------- 2x28 JAMMA compatible edge connector -------->


  AA =  Texas Instruments RC4558P T835AJ34.
  AB =  NEC C1663C 8926B.


  Pinout:

                   solder side |##| components
            -------------------|--|-----------------
                           gnd |01| gnd
                           gnd |02| gnd
                           +5v |03| +5v
                           +5v |04| +5v
                           -5v |05| -5v
                          +12v |06| +12v
                               |  |
           hole (no connector) |07| hole (no connector)
                               |  |
            Coin B (10Y) meter |08| Coin A (100Y) meter
                  hopper motor |09| unknown out (ULN2003, pin15)
                     speaker - |10| speaker +
  (ULN2003, pin13) unknown out |11| unknown out (ULN2003, pin12)
                         green |12| red
                          sync |13| blue
  (ULN2003, pin13) unknown out |14| video gnd
                               |  |
                      medal in |15| management
               coin B (10Y) in |16| reset
              coin A (100Y) in |17| unknown in (active)
             +5v |<|-- (diode) |18| (diode) --|>| +5v
             +5v |<|-- (diode) |19| (diode) --|>| +5v
                               |  |
                Botechin (5th) |20| (1st) Boketa
           (active) unknown in |21| (2nd) Kunio
           (active) unknown in |22| (3rd) Pyon-Pyon
                         start |23| (4th) Pyokorin
                        payout |24| hopper in (opto in)
                               |  |
                      not used |25| not used
                      not used |26| not used
                           gnd |27| gnd
                           gnd |28| gnd


  Pyon Pyon Jump DIP switches:

  .---------------------------------------------------------------------------------.
  | DIP Switches bank #1                     1    2    3    4    5    6    7    8   |
  +-------------------------------------+-------------------------------------------+
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 11 medal |   off  off  off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 10 medal |   off  off  on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 6 medal  |   off  on   off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 5 medal  |   off  on   on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 4 medal  |   on   off  off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 3 medal  |   on   off  on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 2 medal  |   on   on   off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage Coin A  | 1 coin / 1 medal  |   on   on   on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Coin A (100Y)   | Exchange          |                  off                      |
  +-----------------+-------------------+-------------------------------------------+
  | Coin A (100Y)   | Credit            |                  on                       |
  +-----------------+-------------------+-------------------------------------------+
  | Win             | Medal out         |                       off                 |
  +-----------------+-------------------+-------------------------------------------+
  | Win             | Credit            |                       on                  |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage config  | Coin A: Payout    |                            off            |
  +-----------------+-------------------+-------------------------------------------+
  | Coinage config  | Coin A: Normal    |                            on             |
  +-----------------+-------------------+-------------------------------------------+
  | Payout Mode     | Manual            |                                 off       |
  +-----------------+-------------------+-------------------------------------------+
  | Payout Mode     | Automatic         |                                 on        |
  +-----------------+-------------------+-------------------------------------------+
  | Repeat last bet | No                |                                      off  |
  +-----------------+-------------------+-------------------------------------------+
  | Repeat last bet | Yes               |                                      on   |
  '-----------------+-------------------+-------------------------------------------'

  .---------------------------------------------------------------------------------.
  | DIP Switches bank #2                     1    2    3    4    5    6    7    8   |
  +-------------------------------------+-------------------------------------------+
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 95%               |   on   on   on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 90%               |   on   on   off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 85%               |   on   off  on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 80%               |   on   off  off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 75%               |   off  on   on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 70%               |   off  on   off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 60%               |   off  off  on                            |
  +-----------------+-------------------+-------------------------------------------+
  | Percentage      | 50%               |   off  off  off                           |
  +-----------------+-------------------+-------------------------------------------+
  | Winwave         | Small             |                  off                      |
  +-----------------+-------------------+-------------------------------------------+
  | Winwave         | Big               |                  on                       |
  +-----------------+-------------------+-------------------------------------------+
  | M.Medal         | Off               |                       off                 |
  +-----------------+-------------------+-------------------------------------------+
  | M.Medal         | On                |                       on                  |
  +-----------------+-------------------+-------------------------------------------+
  | HG              | 20-1              |                            off  off       |
  +-----------------+-------------------+-------------------------------------------+
  | HG              | 50-1              |                            off  on        |
  +-----------------+-------------------+-------------------------------------------+
  | HG              | 100-1             |                            on   off       |
  +-----------------+-------------------+-------------------------------------------+
  | HG              | 200-1             |                            on   on        |
  +-----------------+-------------------+-------------------------------------------+
  | unknown                             |                                      off  |
  +-------------------------------------+-------------------------------------------+
  | unknown                             |                                      on   |
  '-------------------------------------+-------------------------------------------'


************************************************************************************

  General Notes....

  There are at least 4 games in the Pyon Pyon series...

  1) Pyon Pyon (a marathon game with froggy characters).
  2) Kuru Kuru Pyon Pyon (a kind of slots game with the same froggy characters).
  3) Pyon Pyon Jump (a contents where the same characters try to cross the river jumping on pads).
  4) Sui Sui Pyon Pyon (a swimming competition where the same characters swim with different styles, even walking).

  The 100 Yen coin input (key 7) can be set as "Exchange" through a DIP switch, in
  which case its value is not accepted as credits but immediately paid out in
  "medals."

  If you get a 'medal jam' error, and the game is not responding anymore, press
  RESET (key F1), and the game will reset to default values (even all counters
  will be cleared; the program does this by zeroing the magic byte preceding the
  game ID string copied with it into NVRAM and then jumping to the boot routine).

  The tables for the I/O routines have room for 7 coin inputs (address + mask) and
  3 output latches, but only 3 coin inputs and 1 output latch are defined and used.


************************************************************************************

  Games Notes:
  ------------

  * Kuru Kuru Pyon Pyon:

  The game name could be translated as "Croak Croak Hop Hop"
  Kuru is the frog sound, and Pyon is the sound of jumps.

  How to play...

  Insert tokens (medals).
  You can bet to any (or all) of the following 5 characters: Botechin, Oume,
  Pyokorin, Kunio, and Pyon-Pyon. Press start, and the reels start to roll. You'll
  win if you can get 3 of the chosen character(s) in a row, column or diagonal.

  The black tadpoles behave just like jokers... If you have 2 chosen characters
  in a row and the remaining one is a black tadpole, it will transform into another
  character to complete the 3 in a row, allowing you to win.

  Red tadpoles are a bonus. Once you get one, it will go to the right panel,
  revealing a number. This number represents the extra credits you won.

  Bookkeeping...

  Pressing Bookkeeping key (key 0), you enter the Bookkeeping Mode. There are
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

  ----------------------------------------------------------------------------------

  * Pyon Pyon Jump:

  The game name could be translated as "Hop Hop Jump"
  Pyon Pyon is an onomatopoeia for hopping or jumping lightly.

  How to play...

  Insert tokens (medals).
  You can bet to any (or all) of the following 5 characters: Boketa, Kunio, Pyon-Pyon,
  Pyokorin and Botechin. Press start, and the river's pads start to roll. You'll win
  if your character gets the three pads to jump to the other side of the river.

  There is also a bonus game with a black tadpole rounding some pads with extra credits.
  you'll get the extra credits marked in the pad where the tadpole stopped.

  Bookkeeping...

  Pressing Bookkeeping key (key 0), you enter the Bookkeeping Mode. There are
  2 screens with all the game information like DIP switches and statistics...

  1st screen...

  - [Left panel]:  All the DIP switches parameters.

  - [Right panel]: Bet and Win totals, 100Y/10Y/medal IN/OUT, total of games,
                   won, loss, won by paid range, and 'omake' (extra/bonus).

  2nd screen (press Bookkeeping key again)...

  - Win distribution by character
   (Boketa, Kunio, Pyon-Pyon, Pyokorin and Botechin).

  - Bet distribution (1, 2, 3, 4, 5~10)

  - Omake (bonus) distribution (games total, win games, loss games)

  Pressing the Bookkeeping key once more, you exit the mode and go back to the game.


************************************************************************************

  ADPCM Samples....

  There are 14 samples in Kuru Kuru Pyon Pyon.

  00: "Botechin"
  01:
  02: "Hakase" (professor)
  03: "Pyokorin"
  04: "Kunio"
  05: "Pyon-Pyon"
  06: "Boketa"
  07:
  08: "Oume"
  09: "Haipaa" (hyper)
  10: "Ichi ni tsuite" (on your marks)
  11: "Youi" (get ready)
  12: "Bang" (sound for the tadpoles landing in the right panel).
  13: Sound effect for reels when running.

  The fact that there are samples for "on your marks", "get ready", and "bang",
  make me think that these sounds could be shared with the other unemulated marathon
  game of the series called just "Pyon Pyon".

***********************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/rstbuf.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "video/v9938.h"
#include "speaker.h"


namespace {

class kurukuru_state : public driver_device
{
public:
	kurukuru_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_adpcm(*this, "adpcm"),
		m_soundirq(*this, "soundirq"),
		m_hopper(*this, "hopper"),
		m_bank1(*this, "bank1")
	{ }

	void ppj(machine_config &config);
	void kurukuru(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_adpcm;
	required_device<rst_neg_buffer_device> m_soundirq;
	required_device<ticket_dispenser_device> m_hopper;
	required_memory_bank m_bank1;

	uint8_t m_adpcm_data;

	void kurukuru_out_latch_w(uint8_t data);
	void kurukuru_bankswitch_w(uint8_t data);
	void kurukuru_adpcm_reset_w(uint8_t data);
	uint8_t kurukuru_adpcm_timer_irqack_r();
	void kurukuru_adpcm_data_w(uint8_t data);
	void ym2149_aout_w(uint8_t data);
	void ym2149_bout_w(uint8_t data);

	void kurukuru_msm5205_vck(int state);
	void kurukuru_audio_io(address_map &map) ATTR_COLD;
	void kurukuru_audio_map(address_map &map) ATTR_COLD;
	void kurukuru_io(address_map &map) ATTR_COLD;
	void kurukuru_map(address_map &map) ATTR_COLD;
	void ppj_audio_io(address_map &map) ATTR_COLD;
	void ppj_audio_map(address_map &map) ATTR_COLD;
	void ppj_io(address_map &map) ATTR_COLD;
	void ppj_map(address_map &map) ATTR_COLD;
};

#define MAIN_CLOCK      XTAL(21'477'272)
#define CPU_CLOCK       MAIN_CLOCK/6
#define YM2149_CLOCK    MAIN_CLOCK/6/2  // '/SEL' pin tied to GND, so internal divisor x2 is active
#define M5205_CLOCK     XTAL(384'000)


/*************************************************
*                  Interrupts                    *
*************************************************/


void kurukuru_state::kurukuru_msm5205_vck(int state)
{
	m_soundirq->rst30_w(1);
	m_adpcm->data_w(m_adpcm_data);
}


/*************************************************
*               Memory Map / I/O                 *
*************************************************/

// Main CPU

void kurukuru_state::kurukuru_out_latch_w(uint8_t data)
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
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 5));
	//machine().bookkeeping().coin_lockout_global_w(BIT(data, 6));
	m_hopper->motor_w(BIT(data, 6));

	if (data & 0x9e)
		logerror("kurukuru_out_latch_w %02X @ %04X\n", data, m_maincpu->pc());
}

void kurukuru_state::kurukuru_bankswitch_w(uint8_t data)
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


void kurukuru_state::kurukuru_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0xdfff).bankr("bank1");
	map(0xe000, 0xffff).ram().share("nvram");
}

void kurukuru_state::kurukuru_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_out_latch_w));
	map(0x10, 0x10).mirror(0x0f).portr("DSW1");
	map(0x20, 0x20).mirror(0x0f).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x80, 0x83).mirror(0x0c).rw("v9938", FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x90, 0x90).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_bankswitch_w));
	map(0xa0, 0xa0).mirror(0x0f).portr("IN0");
	map(0xb0, 0xb0).mirror(0x0f).portr("IN1");
	map(0xc0, 0xc0).mirror(0x0f).rw("ym2149", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0xd0, 0xd0).mirror(0x0f).w("ym2149", FUNC(ay8910_device::data_w));
}

void kurukuru_state::ppj_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0xdfff).bankr(m_bank1);
	map(0xe000, 0xffff).ram().share("nvram");
}

void kurukuru_state::ppj_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_bankswitch_w));
	map(0x10, 0x13).mirror(0x0c).rw("v9938", FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x30, 0x30).mirror(0x0f).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x40, 0x40).mirror(0x0f).portr("DSW1");
	map(0x50, 0x50).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_out_latch_w));
	map(0x60, 0x60).mirror(0x0f).portr("IN1");
	map(0x70, 0x70).mirror(0x0f).portr("IN0");
	map(0xc0, 0xc0).mirror(0x0f).w("ym2149", FUNC(ay8910_device::address_w));
	map(0xc0, 0xc0).mirror(0x0f).r("ym2149", FUNC(ay8910_device::data_r));
	map(0xd0, 0xd0).mirror(0x0f).w("ym2149", FUNC(ay8910_device::data_w));
}
/*

 00h  W --> bankswitching reg...

 10h  W --> 00's           \
 11h  W --> 02 8f 20 91...  > V9938 OK
 13h  W -->                /

 30h  W --> soundlatch...
 40h R  --> (very beginning) seems DSW1
 50h  W --> Output port (counters)
 60h R  --> Input port
 70h R  --> Input port

 C0h  W --> YM2149 address W
 C8h R  --> YM2149 data R --> DSW2
 D0h  W --> YM2149 data W

*/

// Audio CPU

void kurukuru_state::kurukuru_adpcm_data_w(uint8_t data)
{
/*
     6-bit latch. only 4 connected...
       bit 0-3 = MSM5205 data.
*/
	m_adpcm_data = data & 0xf;
}

void kurukuru_state::kurukuru_adpcm_reset_w(uint8_t data)
{
/*
     6-bit latch. only 4 connected...
       bit 0 = RESET
       bit 1 = 4B/3B
       bit 2 = S2
       bit 3 = S1
*/
	m_adpcm->playmode_w(bitswap<8>((data>>1), 7,6,5,4,3,0,1,2));
	m_adpcm->reset_w(data & 1);
}

uint8_t kurukuru_state::kurukuru_adpcm_timer_irqack_r()
{
	if (!machine().side_effects_disabled())
		m_soundirq->rst30_w(0);
	return 0;
}


void kurukuru_state::kurukuru_audio_map(address_map &map)
{
	map(0x0000, 0xf7ff).rom();
	map(0xf800, 0xffff).ram();
}

void kurukuru_state::kurukuru_audio_io(address_map &map)
{
	map.global_mask(0x7f);
	map(0x40, 0x40).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_adpcm_data_w));
	map(0x50, 0x50).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_adpcm_reset_w));
	map(0x60, 0x60).mirror(0x0f).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x70, 0x70).mirror(0x0f).r(FUNC(kurukuru_state::kurukuru_adpcm_timer_irqack_r));
}

void kurukuru_state::ppj_audio_map(address_map &map)
{
	map(0x0000, 0xf7ff).rom();
	map(0xf800, 0xffff).ram();
}

void kurukuru_state::ppj_audio_io(address_map &map)
{
	map.global_mask(0x7f);
	map(0x20, 0x20).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_adpcm_data_w));
	map(0x30, 0x30).mirror(0x0f).w(FUNC(kurukuru_state::kurukuru_adpcm_reset_w));
	map(0x40, 0x40).mirror(0x0f).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x50, 0x50).mirror(0x0f).r(FUNC(kurukuru_state::kurukuru_adpcm_timer_irqack_r));
}
/*
  30h -W  --> 0x0b
  40h R-  --> soundlatch...
  50h R-  --> adpcm irq ack
*/

// YM2149 ports
void kurukuru_state::ym2149_aout_w(uint8_t data)
{
	logerror("YM2149: Port A out: %02X\n", data);
}

void kurukuru_state::ym2149_bout_w(uint8_t data)
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("1st (Botechin)")                       // edge connector pin 20 top
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("2nd (Oume)")                           // edge connector pin 21 top
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("3rd (Pyokorin)")                       // edge connector pin 22 top
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("4th (Kunio)")                          // edge connector pin 23 top
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("5th (Pyon-Pyon)")                      // edge connector pin 20 bottom
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                                                        // edge connector pin 21 bottom
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                                                        // edge connector pin 22 bottom
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )                                                        // edge connector pin 23 bottom

	PORT_START("IN1")
/*  routed to JAMMA top side 15, bottom 15, top 16, bottom 16, top 17, bottom 17, top 24, bottom 24
    so that's test, tilt/slam, coin 1, coin 2, p1 start, p2 start, p1 button 3, p2 button 3
*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                                   // edge connector pin 15 top
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("Medal In")                                 // edge connector pin 15 bottom
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                                                  // edge connector pin 16 top
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME(u8"¥10 In")                                 // edge connector pin 16 bottom
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                                                        // edge connector pin 17 top (active)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME(u8"¥100 In") PORT_IMPULSE(2)                // edge connector pin 17 bottom
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)    // hopper feedback, edge connector pin 24 top
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                                                 // edge connector pin 24 bottom

	PORT_START("DSW1")  // found in the PCB: 11111111
	PORT_DIPNAME( 0x07, 0x00, u8"Coinage A (¥100)" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x02, "1 Coin / 3 Medal" )
	PORT_DIPSETTING(    0x06, "1 Coin / 4 Medal" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Medal" )
	PORT_DIPSETTING(    0x05, "1 Coin / 6 Medal" )
	PORT_DIPSETTING(    0x03, "1 Coin / 10 Medal" )
	PORT_DIPSETTING(    0x07, "1 Coin / 11 Medal" )
	PORT_DIPSETTING(    0x04, "1 Coin / 20 Medal" )
	PORT_DIPSETTING(    0x00, "1 Coin / 50 Medal" )
	PORT_DIPNAME( 0x18, 0x00, u8"Coinage B (¥10)" )  PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, "3 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x10, "2 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x18, "1 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x08, "1 Coin / 2 Medal" )
	PORT_DIPNAME( 0x20, 0x00, "Coinage Config" )    PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, u8"¥100 = Credits; Medal In = 2 Credits by Medal" )
	PORT_DIPSETTING(    0x20, u8"¥100 = Exchange; Medal In = 1 Credit by Medal" )
	PORT_DIPNAME( 0x40, 0x00, "Payout Mode" )       PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, "Automatic" )
	PORT_DIPSETTING(    0x00, "Manual" )
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


static INPUT_PORTS_START( ppj )
	PORT_START("IN0")
/*  bits d0-d3 are JAMMA top side pins 20,21,22,23, bits d4-d7 are JAMMA bottom side pins 20,21,22,23
    so that's player 1 left/right/button1/button2 then player 2 left/right/button1/button2
*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("1st (Boketa)")                         // edge connector pin 20 top
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("2nd (Kunio)")                          // edge connector pin 21 top
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("3rd (Pyon-Pyon)")                      // edge connector pin 22 top
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("4th (Pyokorin)")                       // edge connector pin 23 top
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("5th (Botechin)")                       // edge connector pin 20 bottom
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )                                                        // edge connector pin 21 bottom (active)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )                                                        // edge connector pin 22 bottom (active)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )                                                        // edge connector pin 23 bottom

	PORT_START("IN1")
/*  routed to JAMMA top side 15, bottom 15, top 16, bottom 16, top 17, bottom 17, top 24, bottom 24
    so that's test, tilt/slam, coin 1, coin 2, p1 start, p2 start, p1 button 3, p2 button 3
*/
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                                   // edge connector pin 15 top
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("Medal In")                                 // edge connector pin 15 bottom
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                                                  // edge connector pin 16 top
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME(u8"¥10 In")                                 // edge connector pin 16 bottom
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )                                                        // edge connector pin 17 top (active)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME(u8"¥100 In") PORT_IMPULSE(2)                // edge connector pin 17 bottom
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)  // hopper feedback, edge connector pin 24 top
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                                                 // edge connector pin 24 bottom

	PORT_START("DSW1")  // found in the PCB: 00000000 (arranged for sale since they are uncommon settings)
	PORT_DIPNAME( 0x07, 0x03, u8"Coinage A (¥100)" ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x00, "1 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x04, "1 Coin / 2 Medal" )
	PORT_DIPSETTING(    0x02, "1 Coin / 3 Medal" )
	PORT_DIPSETTING(    0x06, "1 Coin / 4 Medal" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Medal" )
	PORT_DIPSETTING(    0x05, "1 Coin / 6 Medal" )
	PORT_DIPSETTING(    0x03, "1 Coin / 10 Medal" )
	PORT_DIPSETTING(    0x07, "1 Coin / 11 Medal" )
	// Coinage B is always 1 Coin / 1 Medal
	PORT_DIPNAME( 0x08, 0x00, "Coinage Config" )    PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, u8"¥100 = Credits" )
	PORT_DIPSETTING(    0x08, u8"¥100 = Exchange" )
	PORT_DIPNAME( 0x10, 0x00, "Payout Mode" )       PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, "Automatic" )
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")  // found in the PCB: 00000000 (arranged for sale since they are uncommon settings)
	PORT_DIPNAME( 0x07, 0x01, "Percentage" )    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x08, 0x00, "Winwave" )       PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, "Small" )
	PORT_DIPSETTING(    0x00, "Big" )
	PORT_DIPNAME( 0x10, 0x00, "M.Medal" )       PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "HG" )            PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(    0x60, "20-1" )
	PORT_DIPSETTING(    0x20, "50-1" )
	PORT_DIPSETTING(    0x40, "100-1" )
	PORT_DIPSETTING(    0x00, "200-1" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/*************************************************
*        Machine Start & Reset Routines          *
*************************************************/

void kurukuru_state::machine_start()
{
	m_bank1->configure_entries(0, 8, memregion("user1")->base(), 0x8000);

	save_item(NAME(m_adpcm_data));
}

void kurukuru_state::machine_reset()
{
}

/*************************************************
*                 Machine Driver                 *
*************************************************/

void kurukuru_state::kurukuru(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &kurukuru_state::kurukuru_map);
	m_maincpu->set_addrmap(AS_IO, &kurukuru_state::kurukuru_io);

	Z80(config, m_audiocpu, CPU_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &kurukuru_state::kurukuru_audio_map);
	m_audiocpu->set_addrmap(AS_IO, &kurukuru_state::kurukuru_audio_io);
	m_audiocpu->set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	v9938_device &v9938(V9938(config, "v9938", MAIN_CLOCK));
	v9938.set_screen_ntsc("screen");
	v9938.set_vram_size(0x30000);
	v9938.int_cb().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	TICKET_DISPENSER(config, "hopper", attotime::from_msec(50));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set(m_soundirq, FUNC(rst_neg_buffer_device::rst28_w));

	// latch irq vector is $ef (rst $28)
	// timer irq vector is $f7 (rst $30)
	// if both are asserted, the vector becomes $f7 AND $ef = $e7 (rst $20)
	RST_NEG_BUFFER(config, m_soundirq, 0).int_callback().set_inputline(m_audiocpu, 0);

	ym2149_device &ym2149(YM2149(config, "ym2149", YM2149_CLOCK));
	ym2149.port_b_read_callback().set_ioport("DSW2");
	ym2149.port_a_write_callback().set(FUNC(kurukuru_state::ym2149_aout_w));
	ym2149.port_b_write_callback().set(FUNC(kurukuru_state::ym2149_bout_w));
	ym2149.add_route(ALL_OUTPUTS, "mono", 0.80);

	MSM5205(config, m_adpcm, M5205_CLOCK);
	m_adpcm->vck_legacy_callback().set(FUNC(kurukuru_state::kurukuru_msm5205_vck));
	m_adpcm->set_prescaler_selector(msm5205_device::S48_4B);    // changed on the fly
	m_adpcm->add_route(ALL_OUTPUTS, "mono", 0.80);
}

void kurukuru_state::ppj(machine_config &config)
{
	kurukuru(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &kurukuru_state::ppj_map);
	m_maincpu->set_addrmap(AS_IO, &kurukuru_state::ppj_io);

	m_audiocpu->set_addrmap(AS_PROGRAM, &kurukuru_state::ppj_audio_map);
	m_audiocpu->set_addrmap(AS_IO, &kurukuru_state::ppj_audio_io);
}


/*************************************************
*                 ROMs Loading                   *
*************************************************/

/*  Kuru Kuru Pyon Pyon.
    くるくるぴょんぴょん
    1990, Success / Taiyo Jidoki.
*/
ROM_START( kurukuru )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "kp_17l.ic17",  0x00000, 0x08000, CRC(9b552ebc) SHA1(07d0e62b7fdad381963a345376b72ad31eb7b96d) ) // program code
	// Game ID string: "carp carp carp hiroshima ---"

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

/*  Pyon Pyon Jump.
    ぴょんぴょんジャンプ
    Ver 1.40.
    1991, Success / Taiyo Jidoki.
*/
ROM_START( ppj )
	ROM_REGION( 0x08000, "maincpu", 0 )
	ROM_LOAD( "ppj17.ic17",  0x00000, 0x08000, CRC(5d9c9ceb) SHA1(0f52c8a0aaaf978afeb07e56493399133b4ce781) ) // program code
	// Game ID string: "PYON PYON JUMP V1.40"

	ROM_REGION( 0x40000, "user1", 0 ) // maincpu banked roms
	ROM_FILL(                 0x00000, 0x10000, 0xff )                                                         // ic23: unpopulated
	ROM_LOAD( "ppj18.ic18",   0x10000, 0x10000, CRC(69612fc6) SHA1(c6de2ec0db8ad2ace91c3a557a03ed73d0e7336d) ) // ic18: gfx set 1
	ROM_LOAD( "ppj10.ic10",   0x20000, 0x10000, CRC(95314d84) SHA1(1a8cf50e9a1e9e8a8f5702cc735ec993ddd2fdce) ) // ic10: gfx set 2
	ROM_FILL(                 0x30000, 0x10000, 0xff )                                                         // dummy entry for when no romchip is selected

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ppj4.ic4",     0x00000, 0x10000, CRC(6573a0a0) SHA1(6bb99e153a22fce01a71efb3bba6c6cc04bbf8b1) ) // code & adpcm samples

	ROM_REGION( 0x800, "plds", 0 )
	ROM_LOAD( "pal16l8a_no21.ic26",    0x0000, 0x0104, CRC(414c8b50) SHA1(17f562c50a2bca41aeb7a1a7cb3916853cea0d24) )
	ROM_LOAD( "pal16l8a_no22.ic27",    0x0200, 0x0104, CRC(ee2b9257) SHA1(15c79b143eafc7915e0f376a87c01afad8fad2b9) )
	ROM_LOAD( "pal16l8a_no23.ic12",    0x0400, 0x0104, CRC(8a7fbbe0) SHA1(aab8d6b77d46cf2d8620861af1f7c039b6dcda99) )
	ROM_LOAD( "pal12l6a_7908b-4.ic32", 0x0600, 0x0034, CRC(bddf925e) SHA1(861cf5966444d0c0392241e5cfa08db475fb439a) )  // identical to kurukuru...
ROM_END

} // anonymous namespace


/***************************************************************************
*                              Game Drivers                                *
***************************************************************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT        ROT   COMPANY                   FULLNAME                         FLAGS
GAME( 1990, kurukuru, 0,      kurukuru, kurukuru, kurukuru_state, empty_init, ROT0, "Success / Taiyo Jidoki", "Kuru Kuru Pyon Pyon (Japan)",   MACHINE_SUPPORTS_SAVE )
GAME( 1991, ppj,      0,      ppj,      ppj,      kurukuru_state, empty_init, ROT0, "Success / Taiyo Jidoki", "Pyon Pyon Jump (V1.40, Japan)", MACHINE_SUPPORTS_SAVE )

// unemulated....

//  ぴょんぴょん (Pyon Pyon),                1988 Success / Taiyo Jidoki.
//  スイスイぴょんぴょん (Sui Sui Pyon Pyon),  1992 Success / Taiyo Jidoki.
