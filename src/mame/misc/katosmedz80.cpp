// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo

/**************************************************************************************************

  ドラネコバンバン - Dora Neco BanBan
  (as transliterated on the cab)

  Medal game by Kato Seisakusho
  whack-a-mole style

  Driver by Roberto Fresca & Grull Osgo.


***************************************************************************************************

  Main components:

  Sharp LH0080B Z80B-CPU
  12.000 MHz XTAL
  HM6116LP-3 Static RAM
  2x NEC D71055C
  OKI M6295GS
  4-DIP switches bank


***************************************************************************************************

  Some videos about the machine...

  https://www.youtube.com/watch?v=NwB9iffssts
  https://www.youtube.com/watch?v=zRxTYqj9On4
  https://www.youtube.com/watch?v=dGlkhfn2Nbw
  https://www.youtube.com/watch?v=zRxTYqj9On4
  https://www.youtube.com/watch?v=tj4V5Iaom6I


***************************************************************************************************

  We have two characters...

  Doraneko (ドラネコ) (means Stray Cat)
  Koneko (コネコ) (means Kitten)

  The game is basically similar to other gator/alligator games, where you must hit 4 mechanical
  cat arms that make attempts to steal the food through a fence.

  There is a rank system based on the number of hits you can get in the game. After time out,
  if you have at least 50 hits, you can get extended time.

   Hits   Level      Japanese              Translation           Extended Game
  ------+-------+------------------+---------------------------+---------------
  90-99 |  High | にゃんともてごわいやつ!! | How tough you are nyaa!!  |      Yes
  50-89 |  Med  | なかなかやるにゃん     | Not bad nyaa              |      Yes
  00-49 |  Low  | まだまだあまいにゃ～    | It's not there yet. nyaa~ |      No


  You can see the high score for Doraneko or Koneko, pressing the corresponding start button
  whilst game is over.


  The game has a DIP switch that trigger the Test Mode. This Mode start to handle the stepper motors
  of all arms, and test a sequence of lamps while triggers all the game sounds.

  Test mode: Press START 1 to test the stepper motors / arms.


  Some cat's speeches:

  "Umasouda nyaa" ----> "Looks delicious nyaa" (at start of a game)
  "Gochisousama" -----> "Thank you for the food" (at end of the game)

  "maitta, kousan" ---> "We lost, give up"
  "nakanaka yarunya" -> "Not bad nyaa"
  "mou yurusan!" -----> "We won't forgive you!"


***************************************************************************************************

  Error codes:

  E1 = Arm 1 sensor error.
  E2 = Arm 2 sensor error.
  E3 = Arm 3 sensor error.
  E4 = Arm 4 sensor error.
  E5 = Coin error.
  E6 = Service Coin error.
  E7 = Door Open error.


***************************************************************************************************

  This driver was made reverse-engineering the game code with some educated guesses.
  The only element we had, was a PCB picture. All the rest was figured out, as the
  multiple output lines connections to stepper motors, lamps, 7seg LEDs, etc...

  Will check lot of things with the hardware once available.


***************************************************************************************************

  PCB Layout:

  .---------------------------------------------------------------------------------------------------------.
  |    ............    IC1        IC7                 IC9         IC16                                      |
  !      CN3(12)      .-------.  .---------------.   .-----.     .-------------------------.    .-----.     |
  |                   |  NEC  |  | G25 V         |   | OKI |     | SHARP LH0080B           |    |XTAL |     |
  | :CN4(2)           |UPC2002|  |    27C1000-15 |   |M6295|     |                Z80B-CPU |    |12MHz|     |
  |                   '-------'  '---------------'   '-----'     '-------------------------'    '-----'     |
  | :                             IC8        .-----------------------.  .---------------.     .----------.  |
  | :CN5                (VR1)    .---------. | NEC D71055C           |  | G25 A         | IC20| 74HCU04  |  |
  | :(7)                         | 74HC574 | |                  IC10 |  | 27C256   IC17 |     +----------+  |
  | :                            '---------' '-----------------------'  '---------------' IC21| 74HC4040 |  |
  | :          IC2      IC5                      IC11        IC13                             '----------'  |
  |           .---.    .---------------------.  .--------.  .---------.  .--------------.     .----------.  |
  | :       R | 2 |    | NEC D71055C         |  | 74HC04 |  | 74HC86  |  | HM6116LP-3   | IC22| 74HC139  |  |
  | :CN6    E | 0 |    |                     |  '--------'  '---------'  |         IC18 |     '----------'  |
  | :(8)    S | 0 |    '---------------------'                IC14       '--------------'     .----------.  |
  | :       N | 3 |     DSW                                  .---------.                  IC23| 74HC74   |  |
  | :       E |---|    .----.     .----------.     RESNET    | 74HC04  |         RESNET       '----------'  |
  |         T | 2 |    |1234|     | TLP521-4 |   .--------.  '---------'       .--------.     .----------.  |
  | :         | 0 |    '----'     '----------'   | LM339N |   IC15             | LM339N | IC24| 74HC32   |  |
  | :CN7      | 0 |    .IC6-----. .----------.   '--------'  .----------.      '--------'     '----------'  |
  | :(11)     | 3 |    |74HC125 | | TLP521-4 |     RESNET    | TD62083  |        RESNET                     |
  | :         '---'    '--------' '----------'               '----------'                                   |
  | :          IC3                                   H  E  A  T  S  I  N  K                                 |
  | :                  .---------------------------------------------------------------------------------.  |
  | : Q1 Q2            '---------------------------------------------------------------------------------'  |
  | : Q3 Q4 .-------.  .------------.   DA1   .------------.   DA2   .------------.   DA3   .------------.  |
  | :       | A1012 |  |  uPA1456H  | RES RES |  uPA1456H  | RES RES |  uPA1456H  | RES RES |  uPA1456H  |  |
  |         '-------'  '------------'         '------------'         '------------'         '------------'  |
  |                      RES    RES             RES    RES             RES    RES             RES    RES    |
  | :      C11           RES    RES             RES    RES             RES    RES             RES    RES    |
  | :CN1              .------.  .------.    .------.  .------.    .------.  .------.    .------.  .------.  |
  | :(8)              | D525 |  | D525 |    | D525 |  | D525 |    | D525 |  | D525 |    | D525 |  | D525 |  |
  | :      C12        '------'  '------'    '------'  '------'    '------'  '------'    '------'  '------'  |
  | :                          .-------.             .-------.             .-------.             .-------.  |
  | :                          | A1443 |             | A1443 |             | A1443 |             | A1443 |  |
  | :                 RES (x4) '-------'    RES (x4) '-------'    RES (x4) '-------'    RES (x4) '-------'  |
  |                            .-------.             .-------.             .-------.             .-------.  |
  |   C15    C16               | A1443 |             | A1443 |             | A1443 |             | A1443 |  |
  |                   RES (x4) '-------'    RES (x4) '-------'    RES (x4) '-------'    RES (x4) '-------'  |
  | :CN2 .-------.                                                                                          |
  | :(3) | RELAY | Q6                                                                                       |
  |      |  RL1  |              CN8 (13)             CN9 (13)             CN10 (13)            CN11 (13)    |
  |      '-------'            .............        .............        .............        .............  |
  '---------------------------------------------------------------------------------------------------------'

  Q1-Q4:    A1020
  Q6:       A1015
  RL1:      Omron G5C-1 12V.
  C11-C12:  2200uf 16V.
  C15-C16:  4700uf 35V.


  PCB Silkscreened: AKG3I/G25800IM or AKG31/G258001M
  (can't see if are I's or 1's...)


***************************************************************************************************

  Measurements...
   (partial)

                   .------v------.
           --(PA3)-|01         40|-(PA4)--
           --(PA2)-|02  IC10   39|-(PA5)--
           --(PA1)-|03         38|-(PA6)--
           --(PA0)-|04  PPI#0  37|-(PA7)--
           --(/RD)-|05         36|-(/WR)--
           --(/CS)-|06         35|-(RES)--
           --(GND)-|07         34|-(D0)--
           ---(A1)-|08         33|-(D1)--
           ---(A0)-|09   NEC   32|-(D2)--
           --(PC7)-|10         31|-(D3)--
           --(PC6)-|11 D71055C 30|-(D4)--
           --(PC5)-|12         29|-(D5)--
           --(PC4)-|13         28|-(D6)--
           --(PC0)-|14         27|-(D7)--
           --(PC1)-|15         26|-(Vcc)--
           --(PC2)-|16         25|-(PB7)--
           --(PC3)-|17         24|-(PB6)--
           --(PB0)-|18         23|-(PB5)--
           --(PB1)-|19         22|-(PB4)--
           --(PB2)-|20         21|-(PB3)--
                   '-------------'


                   .------v------.
     DSW#4 --(PA3)-|01         40|-(PA4)--
     DSW#3 --(PA2)-|02   IC5   39|-(PA5)--
     DSW#2 --(PA1)-|03         38|-(PA6)--
     DSW#1 --(PA0)-|04  PPI#1  37|-(PA7)--
           --(/RD)-|05         36|-(/WR)--
           --(/CS)-|06         35|-(RES)--
           --(GND)-|07         34|-(D0)--
           ---(A1)-|08         33|-(D1)--
           ---(A0)-|09   NEC   32|-(D2)--
           --(PC7)-|10         31|-(D3)--
           --(PC6)-|11 D71055C 30|-(D4)--
           --(PC5)-|12         29|-(D5)--
           --(PC4)-|13         28|-(D6)--
           --(PC0)-|14         27|-(D7)--
           --(PC1)-|15         26|-(Vcc)--
           --(PC2)-|16         25|-(PB7)--
           --(PC3)-|17         24|-(PB6)--
           --(PB0)-|18         23|-(PB5)--
           --(PB1)-|19         22|-(PB4)--
           --(PB2)-|20         21|-(PB3)--
                   '-------------'



  CPU Clock...
  Binary divisor 12 --> 6 MHz.

                           .-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-.
      .-------.            |                                         |
      | Xtal  |            |          CLK      LH0080B Z80B          |
      | 12MHz |            |           6                             |
      '--+-+--'            '-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-'
         | |                           |
         + +-. .---------------------. |
         | | | |                     | |
     .-+-+-+-+-+-+-+-. .-+-+-+-+-+-+-+-+-.
     |   1 1 1 1     | |             1 0 |
     |   3 2 1 0     | |             0 9 |
     |               | |                 |
     '-+-+-+-+-+-+-+-' '-+-+-+-+-+-+-+-+-'
          74HC04            74HC4040


***************************************************************************************************

  TODO:

  - Hook the cats eye effect when the arms are hit.
  - Figure out the IN1: 0x80 currently mapped in the key "I".
  - Find if there is some type of reward... tickets, medals, etc. (doesn't seems to be one)


**************************************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/okim6295.h"
#include "video/pwm.h"

#include "speaker.h"

#include "dnbanban.lh"


namespace {

class katosmedz80_state : public driver_device
{
public:
	katosmedz80_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi%u", 0),
		m_digits_pwm(*this, "digits_pwm"),
		m_ledsp2(*this, "p2led%u", 0U),
		m_ledsp3(*this, "p3led%u", 0U),
		m_ledsp5(*this, "p5led%u", 0U),
		m_ledsp6(*this, "p6led%u", 0U),
		m_ledsp8(*this, "p8led%u", 0U),
		m_pos(*this, "mpos%u", 0U)
	{ }

	ioport_value arm_sensors_r();
	void dnbanban(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<pwm_display_device> m_digits_pwm;
	output_finder<8> m_ledsp2;
	output_finder<8> m_ledsp3;
	output_finder<8> m_ledsp5;
	output_finder<8> m_ledsp6;
	output_finder<8> m_ledsp8;
	output_finder<4> m_pos;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void port8_w(uint8_t data);

	void ppi0_b_w(uint8_t data);
	void ppi0_c_w(uint8_t data);
	void ppi1_b_w(uint8_t data);
	void ppi1_c_w(uint8_t data);

	u16 m_var[4] = { };
	u8 m_pre[4] = { };
	u8 m_sensors = 0xff;
};


/******************************************
*          Machine Start & Reset          *
******************************************/

void katosmedz80_state::machine_start()
{
	// resolve handlers
	m_ledsp2.resolve();
	m_ledsp3.resolve();
	m_ledsp5.resolve();
	m_ledsp6.resolve();
	m_ledsp8.resolve();
	m_pos.resolve();

	// register for savestates
	save_item(NAME(m_var));
	save_item(NAME(m_pre));
	save_item(NAME(m_sensors));
}

void katosmedz80_state::machine_reset()
{
	for(u8 i = 0; i < 4; i++)
	{
		m_var[i] = 0;
		m_pre[i] = 0;
		m_pos[i] = 0;
	}

	m_sensors = 0xff;
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void katosmedz80_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
	map(0x8000, 0x8000).portr("IN2");
}

void katosmedz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x08).w(FUNC(katosmedz80_state::port8_w));
	map(0x0c, 0x0c).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


/***********************************************
*        PPI's and other Ports Handling        *
***********************************************/
/*
  .-----.-----------.---------.---------.---------.-------.-------.-------.
  | PPI | Map Range | control | PA mode | PB mode | PortA | PortB | PortC |
  '-----+-----------'---------'---------'---------'-------'-------'-------'
   PPI0    00h-03h     0x90       M0        M0       IN      OUT     OUT
   PPI1    04h-07h     0x90       M0        M0       IN      OUT     OUT

*/

ioport_value katosmedz80_state::arm_sensors_r()
{
	return m_sensors;
}


void katosmedz80_state::ppi0_b_w(uint8_t data)
{
/*
    ---- --xx    Stepper Motor 1
    ---- xx--    Stepper Motor 2
    --xx ----    Stepper Motor 3
    xx-- ----    Stepper Motor 4

*/

	u8 m[4], m_bit = 1;
	for(u8 i = 0; i < 4; i++)
	{
		m[i] = (data >> (i * 2)) & 0x03;

		if (m[i] == ((m_pre[i] + 1) % 4))
			m_var[i]++;                     // arm goes forward

		if (m[i] == ((m_pre[i] - 1) & 3))
			m_var[i]--;                     // arm goes back

		if (m[i] == m_pre[i])
			logerror("Motor_%i Stopped\n", i + 1);  // delete after debug

		if (m[i] != m_pre[i])
		{
			if (m_var[i] >= 0x20)
				m_sensors &= 0xff - m_bit;  // sensor on

			if (m_var[i] < 0x07)
				m_sensors |= m_bit;         // sensor off

			logerror("Motor_%i: %02X   -  m_sensors:%02X\n", i + 1, m_var[i], m_sensors);
			m_pre[i] = m[i];
		}

		m_bit = m_bit << 2;
		m_pos[i] = m_var[i];
	}

	// show layout (debug)
	for(u8 i = 0; i < 8; i++)
		m_ledsp2[i] = (data >> i) & 0x01;

}

void katosmedz80_state::ppi0_c_w(uint8_t data)
{
/*
    ---- ---x    Lamp 1: Doraneko (ドラネコ) Start / Game
    ---- --x-    Lamp 2: Koneko (コネコ) Start / Game
    ---- -x--    * On Game: Blinks | Off Game and Test Mode: Turns ON when unknown key "I" is active
    ---- x---    * High when hits give points. It could be the "Eyes" effect actuator. Related to RAM 4047-4048
    ---x ----    Low: Arm 1 Action. Low when food Lamp 1 is On
    --x- ----    Low: Arm 2 Action. Low when food Lamp 2 is On
    -x-- ----    Low: Arm 3 Action. Low when food Lamp 3 is On
    x--- ----    Low: Arm 4 Action. Low when food Lamp 4 is On

*/

	// show layout (debug)
	for(u8 i = 0; i < 8; i++)
		m_ledsp3[i] = (data >> i) & 0x01;
}

void katosmedz80_state::ppi1_b_w(uint8_t data)
{
/*
    ---- ---x    Segment a
    ---- --x-    Segment b
    ---- -x--    Segment c
    ---- x---    Segment d
    ---x ----    Segment e
    --x- ----    Segment f
    -x-- ----    Segment g
    x--- ----    Coin Lock / Inhibit

*/

	// show layout (game score - max score)
	m_digits_pwm->write_mx(data & 0x7f);

	// show layout (debug)
	for(u8 i = 0; i < 8; i++)
		m_ledsp5[i] = (data >> i) & 0x01;

}

void katosmedz80_state::ppi1_c_w(uint8_t data)
{
/*
    ---- ---x    Digit 0
    ---- --x-    Digit 1
    ---- -x--    Digit 2
    ---- x---    Digit 3
    ---x ----    Coin Counter
    --x- ----    * XOR between digit selection
    -x-- ----    * Off while playing Sounds (RDY / BUSY / Digital Mute signal?
    x--- ----    * On = funcional; Off = error

*/

	// Digit Selector for multiplexed 7Seg display
	m_digits_pwm->write_my(data & 0xf);

	// show layout (debug)
	for(u8 i = 0; i < 8; i++)
		m_ledsp6[i] = (data >> i) & 0x01;

	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
}


void katosmedz80_state::port8_w(u8 data)
{
/*
    ---- ---x    Lamp 1: Text Line 3 - Low Level (00-49 hits)
    ---- --x-    Lamp 2: Text Line 2 - Medium Level (50-89 hits)
    ---- -x--    Lamp 3: Text Line 1 - High Level (90-99 hits)
    ---- x---    Lamp 4: Game Over
    ---x ----    Lamp 5: Arm 1 food
    --x- ----    Lamp 6: Arm 2 food
    -x-- ----    Lamp 7: Arm 3 food
    x--- ----    Lamp 8: Arm 4 food

*/

	// show layout (debug)
	for (u8 i = 0; i < 8; i++)
		m_ledsp8[i] = (data >> i) & 0x01;
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( dnbanban )
	PORT_START("IN0")
/*
    ---- ---x    Arm 1 Sensor
    ---- --x-    Arm 1 Hit microswitch
    ---- -x--    Arm 2 Sensor
    ---- x---    Arm 2 Hit microswitch
    ---x ----    Arm 3 Sensor
    --x- ----    Arm 3 Hit microswitch
    -x-- ----    Arm 4 Sensor
    x--- ----    Arm 4 Hit microswitch

*/
	PORT_BIT( 0x55, IP_ACTIVE_HIGH, IPT_CUSTOM )PORT_CUSTOM_MEMBER(katosmedz80_state, arm_sensors_r)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1) PORT_NAME("Hit Arm 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2) PORT_NAME("Hit Arm 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3) PORT_NAME("Hit Arm 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4) PORT_NAME("Hit Arm 4")

	PORT_START("IN1")
	// Attract/demo sounds are triggered after 40d0-40d1 16bit counter decrements to 0.
	// Reg at 40d4 stores the triggered sample counter (4 bits).
	// Turning the switch ON (sounds off), both counters dissappear.

	PORT_DIPNAME( 0x01, 0x01, "Attract / Demo Sounds" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Test Mode" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )  // not accessed in game code
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1)     PORT_NAME("Coin In")          // COIN IN (related error E5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service Coin")     // Service COIN (related error E6)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_DOOR ) PORT_NAME("Door Switch")  // DOOR (related error E7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")  // to figure out...

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )  // Doraneko (ドラネコ) Start
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )  // Koneko (コネコ) Start
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )  // not accessed in game code

INPUT_PORTS_END


/*********************************************
*              Machine Drivers               *
*********************************************/

void katosmedz80_state::dnbanban(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 12_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &katosmedz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &katosmedz80_state::io_map);
	m_maincpu->set_periodic_int(FUNC(katosmedz80_state::irq0_line_hold), attotime::from_hz(12_MHz_XTAL / 0x2000));  // to verify

	I8255(config, m_ppi[0]);  // D71055C IC10
	// (00-03) Mode 0 - Ports A set as input, Ports B, high C & low C as output.
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->out_pb_callback().set(FUNC(katosmedz80_state::ppi0_b_w));
	m_ppi[0]->out_pc_callback().set(FUNC(katosmedz80_state::ppi0_c_w));

	I8255(config, m_ppi[1]);  // D71055C IC5
	// (04-07) Mode 0 - Ports A set as input, Ports B, high C & low C as output.
	m_ppi[1]->in_pa_callback().set_ioport("IN1");
	m_ppi[1]->out_pb_callback().set(FUNC(katosmedz80_state::ppi1_b_w));
	m_ppi[1]->out_pc_callback().set(FUNC(katosmedz80_state::ppi1_c_w));

	// video
	PWM_DISPLAY(config, m_digits_pwm).set_size(4, 7);
	m_digits_pwm->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_dnbanban);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.65);  // pin 7 and resonator verified
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( dnbanban )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "g25_a.ic17", 0x0000, 0x8000, CRC(ef441127) SHA1(69fea4992abb2c4905d3831b6f18e464088f0ec7) )  // MBM27C256A, 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "g25_v.ic7", 0x00000, 0x20000, CRC(87c7d45d) SHA1(3f035d5e62fe62111cee978ed1708e902c98526a) )  // MBM27C1000
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME      PARENT   MACHINE   INPUT     STATE              INIT        ROT    COMPANY            FULLNAME           FLAGS
GAME( 1993, dnbanban, 0,       dnbanban, dnbanban, katosmedz80_state, empty_init, ROT0, "Kato Seisakusho", "Dora Neco BanBan", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
