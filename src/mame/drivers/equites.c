// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Nicola Salmoria
/*******************************************************************************

Equites           (c) 1984 Alpha Denshi Co./Sega   8303
Bull Fighter      (c) 1984 Alpha Denshi Co./Sega   8303
Gekisou           (c) 1985 Eastern Corp.           8304
Violent Run       (c) 1985 Eastern Corp.           8304? (probably on Equites HW)
The Koukouyakyuh  (c) 1985 Alpha Denshi Co.        8304
Splendor Blast    (c) 1985 Alpha Denshi Co.        8303
High Voltage      (c) 1985 Alpha Denshi Co.        8304 (POST says 8404)

Driver by Acho A. Tang, Nicola Salmoria
Many thanks to Corrado Tomaselli for precious hardware info.

Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - To enter sort of "test mode", bits 0 and 1 need to be ON when the game is reset.
    Acho said that it could be a switch (but I'm not sure of that), and that's why
    I've added a EASY_TEST_MODE compilation switch.


1) 'equites'

  - When in "test mode", press START1 to cycle through next sound, and press START2
    to directly test the inputs and the Dip Switches.
  - When the number of buttons is set to 2, you need to press BOTH BUTTON1 and
    BUTTON2 to have the same effect as BUTTON3.


2) 'bullfgtr'

  - When in "test mode", press START1 to cycle through next sound, and press START2
    to directly test the inputs and the Dip Switches.
  - I'm not sure I understand how the coinage is handled, and so it's hard to make
    a good description. Anyway, the values are correct.


3) 'kouyakyu'

  - When in "test mode", press START1 to cycle through next sound, and press START2
    to directly test the inputs and the Dip Switches.
  - Bit 1 of Dip Switch is only read in combinaison of bit 0 during P.O.S.T. to
    enter the "test mode", but it doesn't add any credit ! That's why I've patched
    the inputs, so you can enter the "test mode" by pressing COIN1 during P.O.S.T.


4) 'splndrbt'

  - When starting a 2 players game, when player 1 game is over, the game enters in
    an infinite loop on displaying the "GAME OVER" message.
  - You can test player 2 by putting 0xff instead of 0x00 at 0x040009 ($9,A6).
  - FYI, what should change the contents of $9,A6 is the routine at 0x000932,
    but I haven't found where this routine could be called 8( 8303 issue ?


5) 'hvoltage'

  - There is sort of "debug mode" that you can access if 0x000038.w returns 0x0000
    instead of 0xffff. To enable it, turn HVOLTAGE_DEBUG to 1 then enable the fake
    Dip Switch.
  - When you are in "debug mode", the Inputs and Dip Switches have special features.
    Here is IMO the full list :

      * pressing IPT_JOYSTICK_DOWN of player 2 freezes the game
      * pressing IPT_JOYSTICK_UP of player 2 unfreezes the game
      * pressing IPT_COIN1 gives invulnerability (the collision routine isn't called)
      * pressing IPT_COIN2 speeds up the game and you don't need to kill the bosses
      * when bit 2 is On, you are given invulnerability (same effect as IPT_COIN1)
      * when bit 3 is On, you don't need to kill the bosses (only the last one)
      * when bit 4 is On ("Lives" Dip Switch set to "5"), some coordonates are displayed
      * when bit 7 is On ("Coinage" Dip Switch set to "A 1/3C B 1/6C" or "A 2/1C B 3/1C"),
        a "band" is displayed at the left of the screen


Notes:
-----
- The sound board in all games is identical, labelled SOUND BOARD NO.59 MC 07.
  There are three pots, labelled MUSIC, VOICE and FRQ. MUSIC and VOICE control
  the volume, while FRQ changes (a unique feature of this hardware) the input
  clock to the MSM5232. This affects the music pitch ONLY--tempo is unaffected.
  Clock speed apparently ranges from 6.144MHz (OSC value) to about 150kHz.

- equites hardware: even if there are 0x200 bytes of sprite RAM, which would give
  a total of 128 possible sprites, since all games only write to a limited part of
  that RAM it looks like the hardware can only display 55 sprites. This is confirmed
  by the POST test which only shows those 55 sprites. The strange thing is that the
  sprites are not consecutive in RAM. The "good" parts are
  100000-10005f
  1000e0-1000ff
  1001a4-1001ff
  Possibly the remaining RAM is used by the sprite hardware as buffer? This doesn't
  really explain the weird layout in memory, however.
  Also, the priority order is counterintuitive. It seems that the above blocks are
  in increasing priority order, however in each block the higher priority sprites
  are at the lower addresses. This gives good priorities in gekisou (the car and
  helicopter shadows would be wrong otherwise).

- similarly, splndrbt hardware only appears to be capable of displaying 24 sprites.
  This time, they are consecutive in RAM.

- gekisou doesn't have dip switches, but battery backed RAM. To enter the Settings
  menu, press F1. The settings menu is VERY spartan, with no indication of what the
  settings do.

  Settings:                1   2   3   4   5   6   7   8
  COIN 1  1 Coin / 1 Play  ON  ON
          1 Coin / 2 Play  ON  OFF
          1 Coin / 3 Play  OFF OFF
          2 Coin / 1 Play  OFF ON
  COIN 2  1 Coin / 1 Play          ON  ON
          1 Coin / 2 Play          ON  OFF
          1 Coin / 3 Play          OFF OFF
          2 Coin / 1 Play          OFF ON
  DIFFICULTY  Easy                         ON
              Hard                         OFF
  CABINET     Cocktail                         ON
              Upright                          OFF


TODO:
----

- on startup, the CPU continuously writes 5555 to 100000 in a tight loop and expects
  it to change to exit the loop. The value should obviously be modified by the
  sprite hardware but it's difficult to guess what should happen. A kludge read
  handler is used as a work around.

- the second interrupt source for main CPU is unknown.

- gekisou has some unknown device mapped to 580000/5a0000. The bit read from bit 7
  of 180000 must match the last write, otherwise the game will report a BOARD ERROR
  during boot.

- gekisou: there's a white line crossing the explosion sprite. This doesn't look
  like a bad ROM since the line is crossing several, not concecutive, explosion
  sprites, and no other sprites.

- gekisou: wrong sprite when hitting helicopter with missile?

- gekisou: there is a small glitch during the text intro at the beginning of player
  2 game in cocktail mode: a white line spills out from the text box as characters
  in the last line are written. This might well be a bug in the original.

- splndrbt, hvoltage: the interpretation of the scaling PROMs might be wrong.
  The sprite x scaling is not used at all because I couldn't figure it out.
  Sprite y scaling is slightly wrong and leaves gaps in tall objects.
  Note that sprites are 30x30 instead of 32x32.

- The "road" background in splndrbt is slightly wrong. Apparently, the black lines
  visible in some parts of the background should never disappear in the distance.
  Currently, they may or may not disappear depending on the X position.

- Need to use different default volumes for various games, especially gekisou
  where the car noise is really unpleasant with the default settings.

- analog drums/cymbals missing.

- bassline imperfect. This is just the square wave output of the 5232 at the moment.
  It should go through analog stages.

- properly emulate the 8155 on the sound board.

- implement low-pass filters on the DAC output

- the purpose of the sound PROM is unclear. From the schematics, it seems it
  should influence the MSM5232 clock. However, even removing it from the board
  doesn't seem to affect the sound.

* Special Thanks to:

  Jarek Burczynski for a superb MSM5232 emulation
  The Equites WIP webmasters for the vital screenshots and sound clips
  Yasuhiro Ogawa for the correct Equites ROM information


Other unemulated Alpha Denshi and SNK games that may use similar hardware:
-----------------------------------------------------------
Maker        Year Genre Name             Japanese Name
-----------------------------------------------------------
Alpha Denshi 1984 (SPT) Champion Croquet ?`?????s?I???N???b?P?[
Alpha Denshi 1985 (???) Tune Pit(?)      ?`?F?[???s?b?g
Alpha Denshi 1985 (MAJ) Perfect Janputer ?p?[?t?F?N?g?W?????s???[?^?[


*******************************************************************************

Bull Fighter
PCB layout
2005-03-21
f205v

|----------------------|
| Fully boxed = socket |
|----------------------|


| separation = solder


-----------------------------------------------------------------------|
|ALPHA DENSHI CO, LTD. SOUND BOARD NO.59MC07                           |
|                                                                      |
|          |--------------------|                                      |
|   P | \/ |    JAPAN 84250C    | SN74LS232J | SN74LS74AN | DISSIPATOR |
|     | /\ |      M5l8085AP     | 8131BJ     | 8314A      |            |
|          |--------------------|                                      |
|                                                                      |
|   N                           | SN74LS74A                            |
|                               | 8122AG                               |
|                                                                      |
|     |------------|                                                   |
|   M | HV1VR OKI  | SN74LS138J | SN74LS74AJ | LM324N     | LM324N     |
|     | 2764-45    | 8044AG     | F8048      | J423A2     | 98509      |- (+12V)
|     |------------|                                                   |
|                                                                      |- (+5V)
|     |------------|                                                   |
|   L | HV2VR OKI  | T74LS14B1  | SN74LS08N                            |- (OUT)
|     | 2764-45    | 88442      | K8208                                |
|     |------------|                                                   |- (
|                                                                      |  (GND)
|     |------------|                                                   |- (
|   K | HV3VR OKI  | SN74LS08J  | SN74LS04N  | LM324N                  |
|     | 2764-45    | K8208      | I8313      | 98509                   |
|     |------------|                                                   |
|                                                                      |
|   J              | SN74LS74A  | SN74LS232J |            | LM324N     |
|                  | 8122AG     | 8131BG     |            | 436A       |
|                                                                      |
|     |------------|            |------------|                         |
|   H | HV4VR OKI  | SN74LS393N | TBP18S030N |            | LM324N     |
|     | 2764-45    | K8208      | J419X      |            | 436A       |
|     |------------|            |------------|                         |
|                                                                      |
|   F              | SN74LS138J | HCF40174BE |            | CD4066BCN  |
|                  | 8044AG     | MSM40174   |            | MM5666BN   |
|                                                                      |
|   E              | M74LS123P  | SN74LS138J | CD4066BCN  | CD4066BCN  |
|                  | 84A6C1     | 8044AG     | MM5666BN   | MM5666BN   |
|                                                           MUSIC      |
|                  |-------------------------|                         |
|   D | SN74LS373N |       JAPAN 841903      |            | CD4066BCN  |
|     | 2764-45    |         M5L8155P        |            | MM5666BN   |
|                  |-------------------------|              VOICE      |
|                                                                      |
|C                 |-------------------------|                         |
|O  C | 74LS173 PC |        MSM5232RS        |                         |
|N    | 8247       |        OKI 4342         |                         |
|N                 |-------------------------|                         |
|                                                                      |
|T  B                                                                  |
|O                                                                     |
|                  |-------------------------|                         |
|M  A | 74LS173 PC |     SOUND AY-3-8910     | CD4066BCN  | CD4066BCN  |
|A    | 8247       |        8046 CDA         | MM5666BN   | MM5666BN   |
|I                 |-------------------------|                         |
|N                                                                     |
|     | 1          | 2           | 3         | 4          | 5          |
-----------------------------------------------------------------------|




-----------------------------------------------------------------------------------|
|ALPHA DENSHI CO, LTD. MAIN CONNECTOR BOARD                                        |
|                                                                                  |
|                                                                                  |
|      | K     | J     | H     | F     | E     | D     | C     | B     | A         |
|                                                                              |   |
J    1 | LS08  | LS74  | LS109 | LS27  | LS32  |       | LS153 | LS153 | 24S10 |L  |
2                                                                              |S  |
     2 | LS74  | LS04  | LS21  | LS00  | LS174 | LS153 | LS153 | LS273 | 24S10 |2  |
T                                                                              |7  |
O                                      |-------|                               |3  |
     3 | LS32  | LS161 | LS161 | LS157 |H      | LS194 | LS194 | LS04  | 24S10 |   |
L                                      |TMS2732|                                   |
O                                      |-------|                                   |
W                                                              |---------------|   |
E    4 | LS00  | LS86  | LS86  | LS08          | LS157 | LS157 | ALPHA-8303    |   |
R                                                              | 44801B42-3M 3 |   |
                                                               |---------------|   |
B                                      |-------|                                   |
O    5 | LS393 | LS86  | LS86  | OSCLT |M58725P| LS157 | LS157                     |
A                                      |-------|                                   |
R                                                                      |-------|   |
D    6 | LS04  | LS161 | LS161         | LS273 | LS157 | LS157 | LS08  |M58725P|   |-C
|                                                                      |-------|   |-O
|                                              |-----------|-----------|           |-N
|    7 | LS74  | LS367 | LS74  | LS368 | LS373 |  M58725P  |  M58725P  | LS245     |-N
|                                              |-----------|-----------|           |-E
|                                                                                  |-C
|    8 | LS32  | LS74  | LS32  | LS138 | LS245                         | LS245     |-T
J                                              |-----------|-----------|           |-O
1    9 | LS08  | LS00  | LS74  | LS138         |   empty   |   empty   | LS368     |-R
                                               |-----------|-----------|           |-
T                                              |-----------|-----------|           |-N
O   10 | LS08  | LS32  | LS74  | LS138 | LS139 |HP3VR      |HP7VR      | LS368     |-O
                                               |M5L2764K   |HN482764G  |           |-N
L        |-                                    |-----------|-----------|           |-J
O   11   |C \  | LS02  | LS00  | LS138 | LS259                         | LS368     |-A
W        |O  \                                 |-----------|-----------|           |-M
E        |N S |                                |HP2VR      |HP6VR      | LS368     |-M
R   12   |N O |                                |HN482764G  |M5L2764K   |           |-A
         |  U |                                |-----------|-----------|           |
B        |F N |        |-----------------------|-----------|-----------|           |
O   13   |R D || LS74  |       HD68000-8       |HP1VR      |HP5VR      |           |
A        |O  /         |         4D3-R         |M5L2764K   |M5L2764K   |           |
R        |M /          |-----------------------|-----------|-----------|           |
D        |-                                                                        |
|   14         | LS05  | LS193 | LS214 | LS08                  | LS139 | DIP 6x    |
|                                                                                  |
|----------------------------------------------------------------------------------|




-------------------------------------------------------------------------------------|
|ALPHA DENSHI CO, LTD. LOWER BOARD                                                   |
|                                                                                    |
|                                                                                    |
J      | K     | J     | H        | F        | E     | D     | C     | B     | A     |
2                                                                                    |
     1 | LS245 | LS139 |----------|----------| LS174 | LS153 | 4416  | LS04  | LS194 |
T                      | HM6116-3 | HS4VR    |                                       |
O                      | M58725P  | M5L2764K |                                       |
     2         | LS245 |          |          | LS174 | LS153 | 4416  | LS373 | LS194 |
M                      |----------|----------|                                       |
A                      | HM6116-3 | HS3VR    |                                       |
I    3 | LS08  | LS04  | M58725P  | M5L2764K | LS283 | LS153 | 4416  | LS157 | LS194 |
N                      |          |          |                                       |
                       |----------|----------|                                       |
B    4 | LS86  | LS86  | HS6VR    | HS2VR    | LS283 | LS153 | 4416  | LS373 | LS194 |
O                      | M5L2764K | HN482764G|                                       |
A                      |          |          |-------|-------|                       |
R    5 | LS157 | LS157 |----------|----------| LS193 | LS193 | LS08  | LS32  | LS32  |
D                      | HS5VR    | HS1VR    |-------|-------|                       |
|                      | M5L2764K | M5L2764K |                       |-------|       |
|    6 | LS08  | LS86  |          |          | LS157 | LS157 | LS153 | 24S10 | LS257 |
|                      |----------|----------|                       |-------|       |
|                      | HP5VR    | HB1VR    |                       |-------|       |
|    7 | LS157 | LS157 | M5L2764K | M5L2764K | LS157 | LS273 | LS153 | 24S10 | LS257 |
|                      |          |          |                       |-------|       |
|                      |----------|----------|                                       |
|    8 | LS669 | LS669 | HP6VR    | HB2VR    | LS157 | LS273 | LS153 | LS153 | LS32  |
J                      | M5L2764K | M5L2764K |                                       |
1                      |          |          |                       |-------|       |
     9 | LS669 | LS669 |----------|----------| LS157 | LS273 | LS153 | 24S10 | LS257 |
T                      | HM6116-3 | HB3VR    |                       |-------|       |
O                      | M58725P  | M5L2764K |                       |-------|       |
    10 | LS273 | LS273 |          |          | LS194 | LS04  | LS153 | 24S10 | LS257 |
M                      |----------|----------|                       |-------|       |
A                      | HM6116-3 | HB4VR    |                                       |
I   11 | LS245 | LS245 | M58725P  | M5L2764K | LS157 | LS194 | LS86  | LS08  | LS02  |
N                      |          |          |                                       |
                       |----------|----------|                                       |
B   12 | LS373 | LS373                       | LS194 | LS157 | LS08  | LS74  | LS175 |
O                                                                                    |
A                                                                                    |
R   13 | LS373 | LS273 | LS174    | LS139    | LS08  | LS32  | LS74  | LS10  | LS00  |
D                                                                                    |
|------------------------------------------------------------------------------------|


*******************************************************************************/
// Directives

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/alph8201/alph8201.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "machine/nvram.h"
#include "includes/equites.h"

#define HVOLTAGE_DEBUG  0
#define EASY_TEST_MODE  0

#define FRQ_ADJUSTER_TAG    "FRQ"

// MSM5232 clock is generated by a transistor oscillator circuit, not by the pcb xtal
// The circuit is controlled by a pot to allow adjusting the frequency.  All games
// have been hand-tuned to a recording claiming to be an original recording from the
// boards.  You can adjust this in the Slider Controls while using -cheat if needed.

#define MSM5232_MAX_CLOCK 6144000
#define MSM5232_MIN_CLOCK  214000   // unstable




/******************************************************************************/
// Sound

/******************************************************************************/

TIMER_CALLBACK_MEMBER(equites_state::equites_nmi_callback)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

TIMER_CALLBACK_MEMBER(equites_state::equites_frq_adjuster_callback)
{
	UINT8 frq = ioport(FRQ_ADJUSTER_TAG)->read();

	m_msm->set_clock(MSM5232_MIN_CLOCK + frq * (MSM5232_MAX_CLOCK - MSM5232_MIN_CLOCK) / 100);
//popmessage("8155: C %02x A %02x  AY: A %02x B %02x Unk:%x", m_eq8155_port_c, m_eq8155_port_a, m_ay_port_a, m_ay_port_b, m_eq_cymbal_ctrl & 15);

	m_cymvol *= 0.94f;
	m_hihatvol *= 0.94f;

	m_msm->set_output_gain(10, m_hihatvol + m_cymvol * (m_ay_port_b & 3) * 0.33f);   /* NO from msm5232 */
}

WRITE8_MEMBER(equites_state::equites_c0f8_w)
{
	switch (offset)
	{
		case 0: // c0f8: NMI ack (written by NMI handler)
			m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
			break;

		case 1: // c0f9: RST75 trigger (written by NMI handler)
			// Note: solder pad CP3 on the pcb would allow to disable this
			generic_pulse_irq_line(m_audiocpu, I8085_RST75_LINE, 1);
			break;

		case 2: // c0fa: INTR trigger (written by NMI handler)
			// verified on PCB:
			m_audiocpu->set_input_line(I8085_INTR_LINE, HOLD_LINE);
			break;

		case 3: // c0fb: n.c.
			// Note: this is written at end of the INTR handler, however LS138@E3 pin 12 is definitely unconnected
			break;

		case 4: // c0fc: increment PROM address (written by NMI handler)
			m_sound_prom_address = (m_sound_prom_address + 1) & 0x1f;
//       at this point, the 5-bit value
//       goes to an op-amp and to the base of a transistor. The transistor is part
//       of a resonator that is used to generate the M5232 clock. The PROM doesn't
//       actually seem to be important, since even removing it the M5232 clock
//       continues to come out normally.
			break;

		case 5: // c0fd: n.c.
			// Note: this is written at end of the RST75 handler, however LS138@E3 pin 10 is definitely unconnected
			break;

		case 6: // c0fe: 4-bit answer for main CPU (unused)
			// Note: this would go to the LS173@B1, which is however unpopulated on the pcb
			break;

		case 7: // c0ff: sound command latch clear
			// Note: solder pad CP1 on the pcb would allow to disable this
			soundlatch_clear_byte_w(space, 0, 0);
			break;
	}
}


WRITE8_MEMBER(equites_state::equites_8910porta_w)
{
	// bongo 1
	m_samples->set_volume(0, ((data & 0x30) >> 4) * 0.33);
	if (data & ~m_ay_port_a & 0x80)
		m_samples->start(0, 0);

	// bongo 2
	m_samples->set_volume(1, (data & 0x03) * 0.33);
	if (data & ~m_ay_port_a & 0x08)
		m_samples->start(1, 1);

	m_ay_port_a = data;

#if POPDRUMKIT
popmessage("HH %d(%d) CYM %d(%d)", m_hihat, BIT(m_ay_port_b, 6), m_cymbal, m_ay_port_b & 3);
#endif
}

WRITE8_MEMBER(equites_state::equites_8910portb_w)
{
#if POPDRUMKIT
if (data & ~m_ay_port_b & 0x08) m_cymbal++;
if (data & ~m_ay_port_b & 0x04) m_hihat++;
#endif

	// bongo 3
	m_samples->set_volume(2, ((data & 0x30)>>4) * 0.33);
	if (data & ~m_ay_port_b & 0x80)
		m_samples->start(2, 2);

	// FIXME I'm just enabling the MSM5232 Noise Output for now. Proper emulation
	// of the analog circuitry should be done instead.
//  if (data & ~m_ay_port_b & 0x08)   cymbal hit trigger
//  if (data & ~m_ay_port_b & 0x04)   hi-hat hit trigger
//  data & 3   cymbal volume
//  data & 0x40  hi-hat enable

	if (data & ~m_ay_port_b & 0x08)
		m_cymvol = 1.0f;

	if (data & ~m_ay_port_b & 0x04)
		m_hihatvol = 0.8f;

	if (~data & 0x40)
		m_hihatvol = 0.0f;

	m_ay_port_b = data;

#if POPDRUMKIT
popmessage("HH %d(%d) CYM %d(%d)",m_hihat,BIT(m_ay_port_b,6),m_cymbal,m_ay_port_b & 3);
#endif
}

WRITE8_MEMBER(equites_state::equites_cymbal_ctrl_w)
{
	m_eq_cymbal_ctrl++;
}


void equites_state::equites_update_dac(  )
{
	// there is only one latch, which is used to drive two DAC channels.
	// When the channel is enabled in the 4066, it goes to a series of
	// low-pass filters. The channel is kept enabled only for a short time,
	// then it's disabled again.
	// Note that PB0 goes through three filters while PB1 only goes through one.

	if (m_eq8155_port_b & 1)
		m_dac_1->write_signed8(m_dac_latch);

	if (m_eq8155_port_b & 2)
		m_dac_2->write_signed8(m_dac_latch);
}

WRITE8_MEMBER(equites_state::equites_dac_latch_w)
{
	m_dac_latch = data << 2;
	equites_update_dac();
}

WRITE8_MEMBER(equites_state::equites_8155_portb_w)
{
	m_eq8155_port_b = data;
	equites_update_dac();
}

WRITE_LINE_MEMBER(equites_state::equites_msm5232_gate)
{
}

/******************************************************************************/
// Local Functions

/******************************************************************************/
// Interrupt Handlers

// Equites Hardware
TIMER_DEVICE_CALLBACK_MEMBER(equites_state::equites_scanline)
{
	int scanline = param;

	if(scanline == 232) // vblank-out irq
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 24) // vblank-in irq
		m_maincpu->set_input_line(2, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(equites_state::splndrbt_scanline)
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		m_maincpu->set_input_line(1, HOLD_LINE);

	if(scanline == 32) // vblank-in irq
		m_maincpu->set_input_line(2, HOLD_LINE);
}

WRITE8_MEMBER(equites_state::equites_8155_w)
{
	// FIXME proper 8155 emulation must be implemented
	switch( offset )
	{
		case 0: //logerror( "8155 Command register write %x, timer command = %x, interrupt enable = %x, ports = %x\n", data, (data >> 6) & 3, (data >> 4) & 3, data & 0xf );
			if (((data >> 6) & 3) == 3)
				m_nmi_timer->adjust(attotime::from_hz(XTAL_6_144MHz/2 / m_timer_count), 0, attotime::from_hz(XTAL_6_144MHz/2 / m_timer_count));
			break;
		case 1: //logerror( "8155 I/O Port A write %x\n", data );
			m_eq8155_port_a = data;
			m_msm->set_output_gain(0, (data >> 4) / 15.0);  /* group1 from msm5232 */
			m_msm->set_output_gain(1, (data >> 4) / 15.0);  /* group1 from msm5232 */
			m_msm->set_output_gain(2, (data >> 4) / 15.0);  /* group1 from msm5232 */
			m_msm->set_output_gain(3, (data >> 4) / 15.0);  /* group1 from msm5232 */
			m_msm->set_output_gain(4, (data & 0x0f) / 15.0);    /* group2 from msm5232 */
			m_msm->set_output_gain(5, (data & 0x0f) / 15.0);    /* group2 from msm5232 */
			m_msm->set_output_gain(6, (data & 0x0f) / 15.0);    /* group2 from msm5232 */
			m_msm->set_output_gain(7, (data & 0x0f) / 15.0);    /* group2 from msm5232 */
			break;
		case 2: //logerror( "8155 I/O Port B write %x\n", data );
			equites_8155_portb_w(space, 0, data);
			break;
		case 3: //logerror( "8155 I/O Port C (or control) write %x\n", data );
			m_eq8155_port_c = data;
			m_msm->set_output_gain(8, (data & 0x0f) / 15.0);    /* SOLO  8' from msm5232 */
			if (data & 0x20)
				m_msm->set_output_gain(9, (data & 0x0f) / 15.0);    /* SOLO 16' from msm5232 */
			else
				m_msm->set_output_gain(9, 0);   /* SOLO 16' from msm5232 */

			break;
		case 4: //logerror( "8155 Timer low 8 bits write %x\n", data );
			m_timer_count = (m_timer_count & 0xff00) | data;
			break;
		case 5: //logerror( "8155 Timer high 6 bits write %x, timer mode %x\n", data & 0x3f, (data >> 6) & 3);
			m_timer_count = (m_timer_count & 0x00ff) | ((data & 0x3f) << 8);
			break;
	}
}

/******************************************************************************/
// Main CPU Handlers

#if HVOLTAGE_DEBUG
READ16_MEMBER(equites_state::hvoltage_debug_r)
{
	return(ioport("FAKE")->read());
}
#endif


CUSTOM_INPUT_MEMBER(equites_state::gekisou_unknown_status)
{
	return m_unknown_bit;
}

WRITE16_MEMBER(equites_state::gekisou_unknown_0_w)
{
	m_unknown_bit = 0;
}

WRITE16_MEMBER(equites_state::gekisou_unknown_1_w)
{
	m_unknown_bit = 1;
}

/******************************************************************************/
// Main CPU Memory Map


READ16_MEMBER(equites_state::equites_spriteram_kludge_r)
{
	if (m_spriteram[0] == 0x5555)
		return 0;
	else
		return m_spriteram[0];
}

READ16_MEMBER(equites_state::mcu_r)
{
	return 0xff00 | m_mcu_ram[offset];
}

WRITE16_MEMBER(equites_state::mcu_w)
{
	if (ACCESSING_BITS_0_7)
		m_mcu_ram[offset] = data & 0xff;
}

WRITE16_MEMBER(equites_state::mcu_halt_assert_w)
{
	m_mcu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
}

WRITE16_MEMBER(equites_state::mcu_halt_clear_w)
{
	m_mcu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}



static ADDRESS_MAP_START( equites_map, AS_PROGRAM, 16, equites_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM // ROM area is written several times (dev system?)
	AM_RANGE(0x040000, 0x040fff) AM_RAM AM_SHARE("nvram")   // nvram is for gekisou only
	AM_RANGE(0x080000, 0x080fff) AM_READWRITE(equites_fg_videoram_r, equites_fg_videoram_w) // 8-bit
	AM_RANGE(0x0c0000, 0x0c01ff) AM_RAM_WRITE(equites_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x0c0200, 0x0c0fff) AM_RAM
	AM_RANGE(0x100000, 0x100001) AM_READ(equites_spriteram_kludge_r)
	AM_RANGE(0x100000, 0x1001ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x140000, 0x1407ff) AM_READWRITE(mcu_r, mcu_w) // 8-bit
	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("IN1") AM_WRITE(soundlatch_word_w) // LSB: sound latch
	AM_RANGE(0x184000, 0x184001) AM_WRITE(equites_flip0_w)
	AM_RANGE(0x188000, 0x188001) AM_WRITE(mcu_halt_clear_w) // 8404 control port1
	AM_RANGE(0x18c000, 0x18c001) AM_WRITENOP // 8404 control port2
	AM_RANGE(0x1a4000, 0x1a4001) AM_WRITE(equites_flip1_w)
	AM_RANGE(0x1a8000, 0x1a8001) AM_WRITE(mcu_halt_assert_w) // 8404 control port3
	AM_RANGE(0x1ac000, 0x1ac001) AM_WRITENOP // 8404 control port4
	AM_RANGE(0x1c0000, 0x1c0001) AM_READ_PORT("IN0") AM_WRITE(equites_scrollreg_w) // scroll register[XXYY]
	AM_RANGE(0x380000, 0x380001) AM_WRITE(equites_bgcolor_w) // bg color register[CC--]
	// 580000 unknown (protection?) (gekisou only, installed by DRIVER_INIT)
	// 5a0000 unknown (protection?) (gekisou only, installed by DRIVER_INIT)
	AM_RANGE(0x780000, 0x780001) AM_WRITE(watchdog_reset16_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( splndrbt_map, AS_PROGRAM, 16, equites_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x040fff) AM_RAM AM_SHARE("workram") // work RAM
	AM_RANGE(0x080000, 0x080001) AM_READ_PORT("IN0") // joyport [2211]
	AM_RANGE(0x0c0000, 0x0c0001) AM_READ_PORT("IN1") AM_WRITE(splndrbt_flip0_w) // [MMLL] MM: bg color register, LL: normal screen
	AM_RANGE(0x0c4000, 0x0c4001) AM_WRITE(mcu_halt_clear_w) // 8404 control port1
	AM_RANGE(0x0c8000, 0x0c8001) AM_WRITENOP // 8404 control port2
	AM_RANGE(0x0cc000, 0x0cc001) AM_WRITE(splndrbt_selchar0_w) // select active char map
	AM_RANGE(0x0e0000, 0x0e0001) AM_WRITE(splndrbt_flip1_w) // [MMLL] MM: not used, LL: flip screen
	AM_RANGE(0x0e4000, 0x0e4001) AM_WRITE(mcu_halt_assert_w) // 8404 control port3
	AM_RANGE(0x0e8000, 0x0e8001) AM_WRITENOP // 8404 control port4
	AM_RANGE(0x0ec000, 0x0ec001) AM_WRITE(splndrbt_selchar1_w) // select active char map
	AM_RANGE(0x100000, 0x100001) AM_WRITE(splndrbt_bg_scrollx_w)
	AM_RANGE(0x140000, 0x140001) AM_WRITE(soundlatch_word_w) // LSB: sound command
	AM_RANGE(0x1c0000, 0x1c0001) AM_WRITE(splndrbt_bg_scrolly_w)
	AM_RANGE(0x180000, 0x1807ff) AM_READWRITE(mcu_r, mcu_w) // 8-bit
	AM_RANGE(0x200000, 0x200fff) AM_MIRROR(0x1000) AM_READWRITE(equites_fg_videoram_r, equites_fg_videoram_w)   // 8-bit
	AM_RANGE(0x400000, 0x4007ff) AM_RAM_WRITE(equites_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x400800, 0x400fff) AM_RAM
	AM_RANGE(0x600000, 0x6000ff) AM_RAM AM_SHARE("spriteram")   // sprite RAM 0,1
	AM_RANGE(0x600100, 0x6001ff) AM_RAM AM_SHARE("spriteram_2") // sprite RAM 2 (8-bit)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, equites_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xc080, 0xc08d) AM_DEVWRITE("msm", msm5232_device, write)
	AM_RANGE(0xc0a0, 0xc0a1) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
	AM_RANGE(0xc0b0, 0xc0b0) AM_WRITENOP // n.c.
	AM_RANGE(0xc0c0, 0xc0c0) AM_WRITE(equites_cymbal_ctrl_w)
	AM_RANGE(0xc0d0, 0xc0d0) AM_WRITE(equites_dac_latch_w)  // followed by 1 (and usually 0) on 8155 port B
	AM_RANGE(0xc0e0, 0xc0e0) AM_WRITE(equites_dac_latch_w)  // followed by 2 (and usually 0) on 8155 port B
	AM_RANGE(0xc0f8, 0xc0ff) AM_WRITE(equites_c0f8_w)
	AM_RANGE(0xe000, 0xe0ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, equites_state )
	AM_RANGE(0x00e0, 0x00e5) AM_WRITE(equites_8155_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mcu_map, AS_PROGRAM, 8, equites_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("mcu_ram") /* main CPU shared RAM */
ADDRESS_MAP_END

/******************************************************************************/
// Common Port Map

#define EQUITES_PLAYER_INPUT_LSB( button1, button2, button3, start ) \
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY \
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY \
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY \
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY \
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, button1 ) \
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, button2 ) \
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, button3 ) \
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, start )

#define EQUITES_PLAYER_INPUT_MSB( button1, button2, button3, start ) \
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL \
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, button1 ) PORT_COCKTAIL \
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, button2 ) PORT_COCKTAIL \
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, button3 ) PORT_COCKTAIL \
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, start )

/******************************************************************************/
// Equites Port Map

static INPUT_PORTS_START( equites )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
#if EASY_TEST_MODE
	PORT_SERVICE( 0x0300, IP_ACTIVE_HIGH )
#endif
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:!6")
	PORT_DIPSETTING(      0x0400, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Buttons" ) PORT_DIPLOCATION("SW:!5")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0800, "3" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR ( Lives ) ) PORT_DIPLOCATION("SW:!4")
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!2,!1")
	PORT_DIPSETTING(      0xc000, "A 2C/1C B 3C/1C" )
	PORT_DIPSETTING(      0x0000, "A 1C/1C B 2C/1C" )
	PORT_DIPSETTING(      0x8000, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(      0x4000, "A 1C/3C B 1C/6C" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(25, "MSM5232 Clock")
INPUT_PORTS_END

/******************************************************************************/
// Gekisou Port Map

static INPUT_PORTS_START( gekisou )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
#if EASY_TEST_MODE
	PORT_SERVICE( 0x0300, IP_ACTIVE_HIGH )
#endif
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Settings") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, equites_state,gekisou_unknown_status, NULL)

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(24, "MSM5232 Clock")
INPUT_PORTS_END

/******************************************************************************/
// Bull Fighter Port Map

static INPUT_PORTS_START( bullfgtr )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
#if EASY_TEST_MODE
	PORT_SERVICE( 0x0300, IP_ACTIVE_HIGH )
#endif
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0c00, "3:00" )
	PORT_DIPSETTING(      0x0800, "2:00" )
	PORT_DIPSETTING(      0x0000, "1:30" )
	PORT_DIPSETTING(      0x0400, "1:00" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x9000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!4,!1")
	PORT_DIPSETTING(      0x9000, "1C/1C (3C per player)" )
	PORT_DIPSETTING(      0x0000, "1C/1C (1C per player)" )
	PORT_DIPSETTING(      0x8000, "A 1C/1C B 1C/4C" )
	PORT_DIPSETTING(      0x1000, "A 1C/2C B 1C/3C" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(33, "MSM5232 Clock")
INPUT_PORTS_END

/******************************************************************************/
// Koukouyakyuh Port Map

static INPUT_PORTS_START( kouyakyu )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_BUTTON3, IPT_START2 )

	PORT_START("IN1")
//  PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
//  PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0300, IP_ACTIVE_HIGH, IPT_COIN1 )
#if EASY_TEST_MODE
	PORT_SERVICE( 0x0300, IP_ACTIVE_HIGH )
#endif
	PORT_DIPNAME( 0x0c00, 0x0000, "Game Points" ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0800, "3000" )
	PORT_DIPSETTING(      0x0400, "4000" )
	PORT_DIPSETTING(      0x0000, "5000" )
	PORT_DIPSETTING(      0x0c00, "7000" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:!2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x9000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!4,!1")
	PORT_DIPSETTING(      0x9000, "1C/1C (2C per player)" )
	PORT_DIPSETTING(      0x0000, "1C/1C (1C per player)" )
	PORT_DIPSETTING(      0x8000, "1C/1C (1C for 2 players)" )
	PORT_DIPSETTING(      0x1000, "1C/3C (1C per player)" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(33, "MSM5232 Clock")
INPUT_PORTS_END

/******************************************************************************/
// Splendor Blast Port Map

static INPUT_PORTS_START( splndrbt )
	PORT_START("IN0")
	EQUITES_PLAYER_INPUT_LSB( IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN, IPT_START1 )
	EQUITES_PLAYER_INPUT_MSB( IPT_BUTTON1, IPT_BUTTON2, IPT_UNKNOWN, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
#if EASY_TEST_MODE
	PORT_SERVICE( 0x0300, IP_ACTIVE_HIGH )
#endif
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:!4")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!2,!1")
	PORT_DIPSETTING(      0xc000, "A 2C/1C B 3C/1C" )
	PORT_DIPSETTING(      0x0000, "A 1C/1C B 2C/1C" )
	PORT_DIPSETTING(      0x4000, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(      0x8000, "A 1C/3C B 1C/6C" )

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(28, "MSM5232 Clock")
INPUT_PORTS_END

/******************************************************************************/
// High Voltage Port Map

static INPUT_PORTS_START( hvoltage )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Upright ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Cocktail ) )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) PORT_COCKTAIL
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_COIN2 )
#if EASY_TEST_MODE
	PORT_SERVICE( 0x0300, IP_ACTIVE_HIGH )
#endif
#if HVOLTAGE_DEBUG
	PORT_DIPNAME( 0x0400, 0x0000, "Invulnerability" ) PORT_DIPLOCATION("SW:!6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, "Need to kill Bosses" ) PORT_DIPLOCATION("SW:!5")
	PORT_DIPSETTING(      0x0800, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
#else
	PORT_DIPNAME( 0x0c00, 0x0000, DEF_STR ( Difficulty ) ) PORT_DIPLOCATION("SW:!6,!5")
	PORT_DIPSETTING(      0x0400, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Hardest ) )
#endif
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR ( Lives ) ) PORT_DIPLOCATION("SW:!4") // See notes
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR ( Bonus_Life ) ) PORT_DIPLOCATION("SW:!3")
	PORT_DIPSETTING(      0x0000, "50k, 100k then every 100k" )
	PORT_DIPSETTING(      0x2000, "50k, 200k then every 100k" )
	PORT_DIPNAME( 0xc000, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:!2,!1") // See notes
	PORT_DIPSETTING(      0xc000, "A 2C/1C B 3C/1C" )
	PORT_DIPSETTING(      0x0000, "A 1C/1C B 2C/1C" )
	PORT_DIPSETTING(      0x4000, "A 1C/2C B 1C/4C" )
	PORT_DIPSETTING(      0x8000, "A 1C/3C B 1C/6C" )

#if HVOLTAGE_DEBUG
	/* Fake port to handle debug mode */
	PORT_START("FAKE")
	PORT_DIPNAME( 0xffff, 0xffff, "Debug Mode" )
	PORT_DIPSETTING(      0xffff, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#endif

	/* this is actually a variable resistor */
	PORT_START(FRQ_ADJUSTER_TAG)
	PORT_ADJUSTER(27, "MSM5232 Clock")
INPUT_PORTS_END

/******************************************************************************/
// Graphics Layouts

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(8*8+3,-1), STEP4(0*8+3,-1) },
	{ STEP8(0*8,8) },
	16*8
};

static const gfx_layout tilelayout_3bpp =
{
	16, 16,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(16*8+3,-1), STEP4(32*8+3,-1), STEP4(48*8+3,-1), STEP4(0*8+3,-1) },
	{ STEP16(0,8) },
	64*8
};

static const gfx_layout tilelayout_2bpp =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(16*8+3,-1), STEP4(32*8+3,-1), STEP4(48*8+3,-1), STEP4(0*8+3,-1) },
	{ STEP16(0*8,8) },
	64*8
};

static const gfx_layout spritelayout_16x14 =
{
	16, 14, // 16x14, very unusual
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(128*0+3,-1), STEP4(128*1+3,-1), STEP4(128*2+3,-1), STEP4(128*3+3,-1) },
	{ 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8 },
	64*8
};

static const gfx_layout spritelayout_32x32 =
{
	32,32,
	RGN_FRAC(1,2),
	3,
	{ 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(0*8+3,-1), STEP4(1*8+3,-1), STEP4(2*8+3,-1), STEP4(3*8+3,-1), STEP4(4*8+3,-1), STEP4(5*8+3,-1), STEP4(6*8+3,-1), STEP4(7*8+3,-1) },
	{ STEP16(0*8*8,8*8), STEP16(31*8*8,-8*8) },
	256*8
};


static GFXDECODE_START( equites )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0x000, 0x80/4 ) // chars
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_3bpp,    0x080, 0x80/8 ) // tiles
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_16x14, 0x100, 0x80/8 ) // sprites
GFXDECODE_END

static GFXDECODE_START( splndrbt )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0x000, 0x100/4 ) // chars
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_2bpp,    0x100, 0x080/4 ) // tiles
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_32x32, 0x180, 0x100/8 ) // sprites
GFXDECODE_END



/******************************************************************************/

static const char *const alphamc07_sample_names[] =
{
	"*equites",
	"bongo1",
	"bongo2",
	"bongo3",
	0
};


#define MSM5232_BASE_VOLUME 1.0

// the sound board is the same in all games
static MACHINE_CONFIG_FRAGMENT( common_sound )

	MCFG_CPU_ADD("audiocpu", I8085A, XTAL_6_144MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("msm", MSM5232, MSM5232_MAX_CLOCK)   // will be adjusted at runtime through PORT_ADJUSTER
	MCFG_MSM5232_SET_CAPACITORS(0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6, 0.47e-6) // verified
	MCFG_MSM5232_GATE_HANDLER_CB(WRITELINE(equites_state, equites_msm5232_gate))
	MCFG_SOUND_ROUTE(0, "mono", MSM5232_BASE_VOLUME/2.2)    // pin 28  2'-1 : 22k resistor
	MCFG_SOUND_ROUTE(1, "mono", MSM5232_BASE_VOLUME/1.5)    // pin 29  4'-1 : 15k resistor
	MCFG_SOUND_ROUTE(2, "mono", MSM5232_BASE_VOLUME)        // pin 30  8'-1 : 10k resistor
	MCFG_SOUND_ROUTE(3, "mono", MSM5232_BASE_VOLUME)        // pin 31 16'-1 : 10k resistor
	MCFG_SOUND_ROUTE(4, "mono", MSM5232_BASE_VOLUME/2.2)    // pin 36  2'-2 : 22k resistor
	MCFG_SOUND_ROUTE(5, "mono", MSM5232_BASE_VOLUME/1.5)    // pin 35  4'-2 : 15k resistor
	MCFG_SOUND_ROUTE(6, "mono", MSM5232_BASE_VOLUME)        // pin 34  8'-2 : 10k resistor
	MCFG_SOUND_ROUTE(7, "mono", MSM5232_BASE_VOLUME)        // pin 33 16'-2 : 10k resistor
	MCFG_SOUND_ROUTE(8, "mono", 1.0)        // pin 1 SOLO  8' (this actually feeds an analog section)
	MCFG_SOUND_ROUTE(9, "mono", 1.0)        // pin 2 SOLO 16' (this actually feeds an analog section)
	MCFG_SOUND_ROUTE(10,"mono", 0.12)       // pin 22 Noise Output (this actually feeds an analog section)

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_6_144MHz/4) /* verified on pcb */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(equites_state, equites_8910porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(equites_state, equites_8910portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(3)
	MCFG_SAMPLES_NAMES(alphamc07_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/******************************************************************************/

MACHINE_START_MEMBER(equites_state,equites)
{
	save_item(NAME(m_fg_char_bank));
	save_item(NAME(m_bgcolor));
	save_item(NAME(m_splndrbt_bg_scrollx));
	save_item(NAME(m_splndrbt_bg_scrolly));
	save_item(NAME(m_sound_prom_address));
	save_item(NAME(m_dac_latch));
	save_item(NAME(m_eq8155_port_b));
	save_item(NAME(m_eq8155_port_a));
	save_item(NAME(m_eq8155_port_c));
	save_item(NAME(m_ay_port_a));
	save_item(NAME(m_ay_port_b));
	save_item(NAME(m_eq_cymbal_ctrl));
	save_item(NAME(m_cymvol));
	save_item(NAME(m_hihatvol));
	save_item(NAME(m_timer_count));
	save_item(NAME(m_unknown_bit));
#if POPDRUMKIT
	save_item(NAME(m_hihat));
	save_item(NAME(m_cymbal));
#endif

	m_nmi_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(equites_state::equites_nmi_callback), this));

	m_adjuster_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(equites_state::equites_frq_adjuster_callback), this));
	m_adjuster_timer->adjust(attotime::from_hz(60), 0, attotime::from_hz(60));
}

MACHINE_RESET_MEMBER(equites_state,equites)
{
	flip_screen_set(0);

	m_fg_char_bank = 0;
	m_bgcolor = 0;
	m_splndrbt_bg_scrollx = 0;
	m_splndrbt_bg_scrolly = 0;
	m_sound_prom_address = 0;
	m_dac_latch = 0;
	m_eq8155_port_b = 0;
	m_eq8155_port_a = 0;
	m_eq8155_port_c = 0;
	m_ay_port_a = 0;
	m_ay_port_b = 0;
	m_eq_cymbal_ctrl = 0;
	m_cymvol = 0.0;
	m_hihatvol = 0.0;
	m_timer_count = 0;
	m_unknown_bit = 0;
#if POPDRUMKIT
	m_hihat = m_cymbal = 0;
#endif
}


static MACHINE_CONFIG_START( equites, equites_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz/4) /* 68000P8 running at 3mhz! verified on pcb */
	MCFG_CPU_PROGRAM_MAP(equites_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", equites_state, equites_scanline, "screen", 0, 1)

	MCFG_FRAGMENT_ADD(common_sound)

	MCFG_CPU_ADD("mcu", ALPHA8301L, 4000000/8)
	MCFG_CPU_PROGRAM_MAP(mcu_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 3*8, 29*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(equites_state, screen_update_equites)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", equites)
	MCFG_PALETTE_ADD("palette", 0x180)
	MCFG_PALETTE_INDIRECT_ENTRIES(0x100)
	MCFG_PALETTE_INIT_OWNER(equites_state,equites)

	MCFG_VIDEO_START_OVERRIDE(equites_state,equites)

	MCFG_MACHINE_START_OVERRIDE(equites_state,equites)
	MCFG_MACHINE_RESET_OVERRIDE(equites_state,equites)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( gekisou, equites )

	// gekisou has battery-backed RAM to store settings
	MCFG_NVRAM_ADD_0FILL("nvram")

MACHINE_CONFIG_END


static MACHINE_CONFIG_START( splndrbt, equites_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/4) /* 68000P8 running at 6mhz, verified on pcb */
	MCFG_CPU_PROGRAM_MAP(splndrbt_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", equites_state, splndrbt_scanline, "screen", 0, 1)

	MCFG_FRAGMENT_ADD(common_sound)

	MCFG_CPU_ADD("mcu", ALPHA8301L, 4000000/8)
	MCFG_CPU_PROGRAM_MAP(mcu_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(equites_state, screen_update_splndrbt)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", splndrbt)
	MCFG_PALETTE_ADD("palette", 0x280)
	MCFG_PALETTE_INDIRECT_ENTRIES(0x100)
	MCFG_PALETTE_INIT_OWNER(equites_state,splndrbt)

	MCFG_VIDEO_START_OVERRIDE(equites_state,splndrbt)

	MCFG_MACHINE_START_OVERRIDE(equites_state,equites)
	MCFG_MACHINE_RESET_OVERRIDE(equites_state,equites)
MACHINE_CONFIG_END


/******************************************************************************/
// Equites ROM Map

/*
Equites by ALPHA DENSHI CO. (1984)

Note: CPU - Main PCB
      SND - Sound PCB    NO.59 MC07
      VID - Video PCB

Main processor   - 68000 2.988MHz

Protection processor  - ALPHA-8303 custom

Sound processor  - 8085 3.073MHz
                 - TMP8155 RIOTs    (RAM & I/O Timers)
                 - MSM5232R3
                 - AY-3-8910
*/
ROM_START( equites )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "ep1",         0x00001, 0x2000, CRC(6a4fe5f7) SHA1(5ff1594a2cee28cc7d59448eb57473088ac6f14b) )
	ROM_LOAD16_BYTE( "ep5",         0x00000, 0x2000, CRC(00faa3eb) SHA1(6b31d041ad4ca81eda36487f659997cc4030f23c) )
	ROM_LOAD16_BYTE( "epr-ep2.12d", 0x04001, 0x2000, CRC(0c1bc2e7) SHA1(4c3510dfeee2fb2f295a32e2fe2021c4c7f08e8a) )
	ROM_LOAD16_BYTE( "epr-ep6.12b", 0x04000, 0x2000, CRC(bbed3dcc) SHA1(46ef2c60ccfa76a187b19dc0b7e6c594050b183f) )
	ROM_LOAD16_BYTE( "epr-ep3.10d", 0x08001, 0x2000, CRC(5f2d059a) SHA1(03fe904a445cce89462788fecfd61ac53f4dd17f) )
	ROM_LOAD16_BYTE( "epr-ep7.10b", 0x08000, 0x2000, CRC(a8f6b8aa) SHA1(ee4edb54c147a95944482e998616b025642a268a) )
	ROM_LOAD16_BYTE( "ep4",         0x0c001, 0x2000, CRC(b636e086) SHA1(5fc23a86b6051ecf6ff3f95f810f0eb471a203b0) )
	ROM_LOAD16_BYTE( "ep8",         0x0c000, 0x2000, CRC(d7ee48b0) SHA1(d0398704d8e89f2b0a9ed05e18f7c644d1e047c0) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "ev1.1m", 0x00000, 0x2000, CRC(43faaf2e) SHA1(c9aaf298d673eb70399366776474f1b242549eb4) )
	ROM_LOAD( "ev2.1l", 0x02000, 0x2000, CRC(09e6702d) SHA1(896771f73a486e5035909eeed9ef48103d81d4ae) )
	ROM_LOAD( "ev3.1k", 0x04000, 0x2000, CRC(10ff140b) SHA1(7c28f988a9c8b2a702d007096199e67b447a183c) )
	ROM_LOAD( "ev4.1h", 0x06000, 0x2000, CRC(b7917264) SHA1(e58345fda088b171fd348959de15082f3cb42514) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "ep9",  0x00000, 0x1000, CRC(0325be11) SHA1(d95667b439e3d97b08efeaf08022348546a4f385) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "eb5.7h",  0x00000, 0x2000, CRC(cbef7da5) SHA1(c5fcd2341ce5b039a15116fbd85796bb5ddc4701) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb6.8h",  0x04000, 0x2000, CRC(1e5e5475) SHA1(80ebe9326c628685faafb259f956a98ac435c809) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb1.7f",  0x08000, 0x2000, CRC(9a236583) SHA1(fcc4da2efe904f0178bd83fdee25d4752b9cc5ce) )
	ROM_LOAD( "eb2.8f",  0x0a000, 0x2000, CRC(f0fb6355) SHA1(3c4c009f80e648d02767b29bb8d18f4de7b26d4e) )
	ROM_LOAD( "eb3.10f", 0x0c000, 0x2000, CRC(dbd0044b) SHA1(5611517bb0f54bfb0585eeca8af21fbfc2f65b2c) )
	ROM_LOAD( "eb4.11f", 0x0e000, 0x2000, CRC(f8f8e600) SHA1(c7c97e4dc1f7a73694c98b2b1a3d7fa9f3317a2a) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "es5.5h",  0x00000, 0x2000, CRC(d5b82e6a) SHA1(956a1413426e53f8a735260e660805b04016ca8d) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es6.4h",  0x04000, 0x2000, CRC(cb4f5da9) SHA1(5af8f6aca0a3bb6417430e0179ec97c33d4014e3) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es1.5f",  0x08000, 0x2000, CRC(cf81a2cd) SHA1(a1b45451cafeaceabe3dfe24eb073098a33ab22b) )
	ROM_LOAD( "es2.4f",  0x0a000, 0x2000, CRC(ae3111d8) SHA1(d63633b531339fa04af757f42e956b8eb1debc4e) )
	ROM_LOAD( "es3.2f",  0x0c000, 0x2000, CRC(3d44f815) SHA1(1835aef280a6915acbf7cad771d65bf1074f0f98) )
	ROM_LOAD( "es4.1f",  0x0e000, 0x2000, CRC(16e6d18a) SHA1(44f9045ad034808070cd6497a3b94c3d8cc93262) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "bprom.3a",  0x0000, 0x100, CRC(2fcdf217) SHA1(4acf67d37e844c2773028ecffe72a66754ed5bca) ) // R
	ROM_LOAD( "bprom.1a",  0x0100, 0x100, CRC(d7e6cd1f) SHA1(ce330e43ba8a97ab79040c053a25e46e8fe60bdb) ) // G
	ROM_LOAD( "bprom.2a",  0x0200, 0x100, CRC(e3d106e8) SHA1(6b153eb8140d36b4d194e26106a5ba5bffd1a851) ) // B
	ROM_LOAD( "bprom.6b",  0x0300, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) ) // CLUT(same PROM x 4)
	ROM_LOAD( "bprom.7b",  0x0400, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.9b",  0x0500, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.10b", 0x0600, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "bprom.3h",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/*
Equites
(c)1984 SEGA/ALPHA

CPU   : 68000 Z-80x2
SOUND : AY-3-8910 MSM5232RS
OSC.  : 12.000MHz 14.31818MHz ?MHz
*/
ROM_START( equitess )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "epr-ep1.13d", 0x00001, 0x2000, CRC(c6edf1cd) SHA1(21dba62e692f4fdc79155ce169a48ae827bd5994) )
	ROM_LOAD16_BYTE( "epr-ep5.13b", 0x00000, 0x2000, CRC(c11f0759) SHA1(5caf2b6b777b2fdabc26ea232225be2d789e87f3) )
	ROM_LOAD16_BYTE( "epr-ep2.12d", 0x04001, 0x2000, CRC(0c1bc2e7) SHA1(4c3510dfeee2fb2f295a32e2fe2021c4c7f08e8a) )
	ROM_LOAD16_BYTE( "epr-ep6.12b", 0x04000, 0x2000, CRC(bbed3dcc) SHA1(46ef2c60ccfa76a187b19dc0b7e6c594050b183f) )
	ROM_LOAD16_BYTE( "epr-ep3.10d", 0x08001, 0x2000, CRC(5f2d059a) SHA1(03fe904a445cce89462788fecfd61ac53f4dd17f) )
	ROM_LOAD16_BYTE( "epr-ep7.10b", 0x08000, 0x2000, CRC(a8f6b8aa) SHA1(ee4edb54c147a95944482e998616b025642a268a) )
	ROM_LOAD16_BYTE( "epr-ep4.9d",  0x0c001, 0x2000, CRC(956a06bd) SHA1(a716f9aaf0c32c522968f4ff13de904d6e8c7f98) )
	ROM_LOAD16_BYTE( "epr-ep8.9b",  0x0c000, 0x2000, CRC(4c78d60d) SHA1(207a82779e2fe3e9082f4fa09b87c713a51167e6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "ev1.1m", 0x00000, 0x2000, CRC(43faaf2e) SHA1(c9aaf298d673eb70399366776474f1b242549eb4) )
	ROM_LOAD( "ev2.1l", 0x02000, 0x2000, CRC(09e6702d) SHA1(896771f73a486e5035909eeed9ef48103d81d4ae) )
	ROM_LOAD( "ev3.1k", 0x04000, 0x2000, CRC(10ff140b) SHA1(7c28f988a9c8b2a702d007096199e67b447a183c) )
	ROM_LOAD( "ev4.1h", 0x06000, 0x2000, CRC(b7917264) SHA1(e58345fda088b171fd348959de15082f3cb42514) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "epr-ep0.3e",  0x00000, 0x1000, CRC(3f5a81c3) SHA1(8fd5bc621f483bfa46be7e40e6480b25243bdf70) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "eb5.7h",  0x00000, 0x2000, CRC(cbef7da5) SHA1(c5fcd2341ce5b039a15116fbd85796bb5ddc4701) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb6.8h",  0x04000, 0x2000, CRC(1e5e5475) SHA1(80ebe9326c628685faafb259f956a98ac435c809) )
	// empty space to unpack previous ROM
	ROM_LOAD( "eb1.7f",  0x08000, 0x2000, CRC(9a236583) SHA1(fcc4da2efe904f0178bd83fdee25d4752b9cc5ce) )
	ROM_LOAD( "eb2.8f",  0x0a000, 0x2000, CRC(f0fb6355) SHA1(3c4c009f80e648d02767b29bb8d18f4de7b26d4e) )
	ROM_LOAD( "eb3.10f", 0x0c000, 0x2000, CRC(dbd0044b) SHA1(5611517bb0f54bfb0585eeca8af21fbfc2f65b2c) )
	ROM_LOAD( "eb4.11f", 0x0e000, 0x2000, CRC(f8f8e600) SHA1(c7c97e4dc1f7a73694c98b2b1a3d7fa9f3317a2a) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "es5.5h",  0x00000, 0x2000, CRC(d5b82e6a) SHA1(956a1413426e53f8a735260e660805b04016ca8d) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es6.4h",  0x04000, 0x2000, CRC(cb4f5da9) SHA1(5af8f6aca0a3bb6417430e0179ec97c33d4014e3) )
	// empty space to unpack previous ROM
	ROM_LOAD( "es1.5f",  0x08000, 0x2000, CRC(cf81a2cd) SHA1(a1b45451cafeaceabe3dfe24eb073098a33ab22b) )
	ROM_LOAD( "es2.4f",  0x0a000, 0x2000, CRC(ae3111d8) SHA1(d63633b531339fa04af757f42e956b8eb1debc4e) )
	ROM_LOAD( "es3.2f",  0x0c000, 0x2000, CRC(3d44f815) SHA1(1835aef280a6915acbf7cad771d65bf1074f0f98) )
	ROM_LOAD( "es4.1f",  0x0e000, 0x2000, CRC(16e6d18a) SHA1(44f9045ad034808070cd6497a3b94c3d8cc93262) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "bprom.3a",  0x0000, 0x100, CRC(2fcdf217) SHA1(4acf67d37e844c2773028ecffe72a66754ed5bca) ) // R
	ROM_LOAD( "bprom.1a",  0x0100, 0x100, CRC(d7e6cd1f) SHA1(ce330e43ba8a97ab79040c053a25e46e8fe60bdb) ) // G
	ROM_LOAD( "bprom.2a",  0x0200, 0x100, CRC(e3d106e8) SHA1(6b153eb8140d36b4d194e26106a5ba5bffd1a851) ) // B
	ROM_LOAD( "bprom.6b",  0x0300, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) ) // CLUT(same PROM x 4)
	ROM_LOAD( "bprom.7b",  0x0400, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.9b",  0x0500, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )
	ROM_LOAD( "bprom.10b", 0x0600, 0x100, CRC(6294cddf) SHA1(c7a2854f62e31032df2b07fae3fb3b51ac6daac2) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "bprom.3h",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
// Bull Fighter ROM Map

ROM_START( bullfgtr )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "hp1vr.bin",  0x00001, 0x2000, CRC(e5887586) SHA1(c06883f5c4a2e777b011199787bd4d52f48ceb41) )
	ROM_LOAD16_BYTE( "hp5vr.bin",  0x00000, 0x2000, CRC(b49fa09f) SHA1(ee947f7b4fa96f887f9b14e7503f98b4d117d1c8) )
	ROM_LOAD16_BYTE( "hp2vr.bin",  0x04001, 0x2000, CRC(845bdf28) SHA1(4eac9ca034aaa6a7db4061ad11587189fc843ca0) )
	ROM_LOAD16_BYTE( "hp6vr.bin",  0x04000, 0x2000, CRC(3dfadcf4) SHA1(724d45df0be7073bbe2767f3c0d050c8b45c9d27) )
	ROM_LOAD16_BYTE( "hp3vr.bin",  0x08001, 0x2000, CRC(d3a21f8a) SHA1(2b3135aaae798eeee5850e616ed6ad8987fbc01b) )
	ROM_LOAD16_BYTE( "hp7vr.bin",  0x08000, 0x2000, CRC(665cc015) SHA1(17fe18c8f22808a102f48bc4cbc8e4a1f6f9eaf1) )
	ROM_FILL(                      0x0c000, 0x4000, 0 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "hv1vr.bin", 0x00000, 0x2000, CRC(2a8e6fcf) SHA1(866903408e05938a982ffef4c9b849203c6cc060) )
	ROM_LOAD( "hv2vr.bin", 0x02000, 0x2000, CRC(026e1533) SHA1(6271869a3faaafacfac35262746e87a83c158b93) )
	ROM_LOAD( "hv3vr.bin", 0x04000, 0x2000, CRC(51ee751c) SHA1(60bf848dfdfe313ab05df5a5c05819b0fa87ca50) )
	ROM_LOAD( "hv4vr.bin", 0x06000, 0x2000, CRC(62c7a25b) SHA1(237d3cbdfbf45b33c2f65d30faba151380866a93) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "h.bin", 0x000000, 0x1000, CRC(c6894c9a) SHA1(0d5a55cded4fd833211bdc733a78c6c8423897de) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "hb5vr.bin",  0x00000, 0x2000, CRC(6d05e9f2) SHA1(4b5c92b72bf73a08a2359fe889a327a696a45e8a) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb6vr.bin",  0x04000, 0x2000, CRC(016340ae) SHA1(f980d39337c711a15520388967ca4503e7970e18) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb1vr.bin",  0x08000, 0x2000, CRC(4352d069) SHA1(bac687f050837b023da00cb53bb524b2a76310d4) )
	ROM_LOAD( "hb2vr.bin",  0x0a000, 0x2000, CRC(24edfd7d) SHA1(be8a40d8d5ccff06f37c1ab67341f56e41a5ea88) )
	ROM_LOAD( "hb3vr.bin",  0x0c000, 0x2000, CRC(4947114e) SHA1(822dc3f14b71dc9e5b69078aefbed6b438aa0690) )
	ROM_LOAD( "hb4vr.bin",  0x0e000, 0x2000, CRC(fa296cb3) SHA1(2ba864766655cb3dd2999a6cdf96dcefd6818135) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "hs5vr.bin",  0x00000, 0x2000, CRC(48394389) SHA1(a5c6021b60226a775b2052909e8d21b5f79d9ec5) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs6vr.bin",  0x04000, 0x2000, CRC(141409ec) SHA1(3cc542fa34fdbd71e392c7c22da5d5120642be86) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs1vr.bin",  0x08000, 0x2000, CRC(7c69b473) SHA1(abc181b4e5b3f48c667a0bb4814c3818dfc6e9e2) )
	ROM_LOAD( "hs2vr.bin",  0x0a000, 0x2000, CRC(c3dc713f) SHA1(c2072cc71ea61e0c718c339bda1460d93343469e) )
	ROM_LOAD( "hs3vr.bin",  0x0c000, 0x2000, CRC(883f93fd) SHA1(a96df701f82e62582522953830049d29bcb3d458) )
	ROM_LOAD( "hs4vr.bin",  0x0e000, 0x2000, CRC(578ace7b) SHA1(933e85d49db7b27fd85e4713f0984612bc29e134) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "24s10n.a3",  0x0000, 0x100, CRC(e8a9d159) SHA1(d015149a39adcb5fc6d12d9afe7820ecef480039) ) // R
	ROM_LOAD( "24s10n.a1",  0x0100, 0x100, CRC(3956af86) SHA1(ccbb69535ece5e228622907d17c959b195b97a0a) ) // G
	ROM_LOAD( "24s10n.a2",  0x0200, 0x100, CRC(f50f8ec5) SHA1(ec2d934618e25e3471153f9fe7b34f978b113a47) ) // B
	ROM_LOAD( "24s10n.b6",  0x0300, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) ) // CLUT(same PROM x 4)
	ROM_LOAD( "24s10n.b7",  0x0400, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b9",  0x0500, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b10", 0x0600, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "18s030.h3",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/*
Bull Fighter

Upper board (Sound)
-------------------
All roms 2764
1 prom TBP18S030
CPU 8085, AY-3-8910, 8155, 5232

Midle board
-----------
Chips named M_xxx

e3 = 2732
All other roms 2764
A1- A3 TBP24S030 PROM

CPU 68000
Special chip:
Alpha 8303 44801B42

Lower board
-----------
Chips named L_xxx
All roms 2764
All Proms TBP24S10
*/
ROM_START( bullfgtrs )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "m_d13.bin",  0x00001, 0x2000, CRC(7c35dd4b) SHA1(6bd604ee32c0c5db17f90e24aa254ec7072d27dd) )
	ROM_LOAD16_BYTE( "m_b13.bin",  0x00000, 0x2000, CRC(c4adddce) SHA1(48b6ddbad52a3941d3e651642b26d9adf70f71f5) )
	ROM_LOAD16_BYTE( "m_d12.bin",  0x04001, 0x2000, CRC(5d51be2b) SHA1(55d2718479cb71ceefefbaf40c14285e5603e526) )
	ROM_LOAD16_BYTE( "m_b12.bin",  0x04000, 0x2000, CRC(d98390ef) SHA1(17006503325627055c8b22052d7ed94e474f4ef7) )
	ROM_LOAD16_BYTE( "m_d10.bin",  0x08001, 0x2000, CRC(21875752) SHA1(016db4125b1a4584ae277af427370780d96a17c5) )
	ROM_LOAD16_BYTE( "m_b10.bin",  0x08000, 0x2000, CRC(9d84f678) SHA1(32584d54788cb570bd5210992836f28ba9c87aac) )
	ROM_FILL(                      0x0c000, 0x4000, 0 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "hv1vr.bin", 0x00000, 0x2000, CRC(2a8e6fcf) SHA1(866903408e05938a982ffef4c9b849203c6cc060) )
	ROM_LOAD( "hv2vr.bin", 0x02000, 0x2000, CRC(026e1533) SHA1(6271869a3faaafacfac35262746e87a83c158b93) )
	ROM_LOAD( "hv3vr.bin", 0x04000, 0x2000, CRC(51ee751c) SHA1(60bf848dfdfe313ab05df5a5c05819b0fa87ca50) )
	ROM_LOAD( "hv4vr.bin", 0x06000, 0x2000, CRC(62c7a25b) SHA1(237d3cbdfbf45b33c2f65d30faba151380866a93) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "h.bin", 0x000000, 0x1000, CRC(c6894c9a) SHA1(0d5a55cded4fd833211bdc733a78c6c8423897de) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "hb5vr.bin",  0x00000, 0x2000, CRC(6d05e9f2) SHA1(4b5c92b72bf73a08a2359fe889a327a696a45e8a) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb6vr.bin",  0x04000, 0x2000, CRC(016340ae) SHA1(f980d39337c711a15520388967ca4503e7970e18) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hb1vr.bin",  0x08000, 0x2000, CRC(4352d069) SHA1(bac687f050837b023da00cb53bb524b2a76310d4) )
	ROM_LOAD( "hb2vr.bin",  0x0a000, 0x2000, CRC(24edfd7d) SHA1(be8a40d8d5ccff06f37c1ab67341f56e41a5ea88) )
	ROM_LOAD( "hb3vr.bin",  0x0c000, 0x2000, CRC(4947114e) SHA1(822dc3f14b71dc9e5b69078aefbed6b438aa0690) )
	ROM_LOAD( "hb4vr.bin",  0x0e000, 0x2000, CRC(fa296cb3) SHA1(2ba864766655cb3dd2999a6cdf96dcefd6818135) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "hs5vr.bin",  0x00000, 0x2000, CRC(48394389) SHA1(a5c6021b60226a775b2052909e8d21b5f79d9ec5) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs6vr.bin",  0x04000, 0x2000, CRC(141409ec) SHA1(3cc542fa34fdbd71e392c7c22da5d5120642be86) )
	// empty space to unpack previous ROM
	ROM_LOAD( "hs1vr.bin",  0x08000, 0x2000, CRC(7c69b473) SHA1(abc181b4e5b3f48c667a0bb4814c3818dfc6e9e2) )
	ROM_LOAD( "hs2vr.bin",  0x0a000, 0x2000, CRC(c3dc713f) SHA1(c2072cc71ea61e0c718c339bda1460d93343469e) )
	ROM_LOAD( "hs3vr.bin",  0x0c000, 0x2000, CRC(883f93fd) SHA1(a96df701f82e62582522953830049d29bcb3d458) )
	ROM_LOAD( "hs4vr.bin",  0x0e000, 0x2000, CRC(578ace7b) SHA1(933e85d49db7b27fd85e4713f0984612bc29e134) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "24s10n.a3",  0x0000, 0x100, CRC(e8a9d159) SHA1(d015149a39adcb5fc6d12d9afe7820ecef480039) ) // R
	ROM_LOAD( "24s10n.a1",  0x0100, 0x100, CRC(3956af86) SHA1(ccbb69535ece5e228622907d17c959b195b97a0a) ) // G
	ROM_LOAD( "24s10n.a2",  0x0200, 0x100, CRC(f50f8ec5) SHA1(ec2d934618e25e3471153f9fe7b34f978b113a47) ) // B
	ROM_LOAD( "24s10n.b6",  0x0300, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) ) // CLUT(same PROM x 4)
	ROM_LOAD( "24s10n.b7",  0x0400, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b9",  0x0500, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )
	ROM_LOAD( "24s10n.b10", 0x0600, 0x100, CRC(8835a069) SHA1(bc8d4130d4fa0f16bb2511ac66b84d53218042a3) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "18s030.h3",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
// Koukouyakyuh ROM Map

/*
The Koukouyakyuh (JPN Ver.)

(c)1985 Alpha denshi

CPU   :MAIN  68000
       SOUND 8085
Sound :AY-3-8910 M3M5232RS
OSC   :12.000MHz 6.??MHz 14.31818MHz
*/
ROM_START( kouyakyu )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 68000 ROMs
	ROM_LOAD16_BYTE( "epr-6704.bin", 0x00001, 0x2000, CRC(c7ac2292) SHA1(614bfb0949620d4c260768f14a116b076dd38438) )
	ROM_LOAD16_BYTE( "epr-6707.bin", 0x00000, 0x2000, CRC(9cb2962e) SHA1(bd1bcbc53a3346e22789f24a35ab3aa681317d02) )
	ROM_LOAD16_BYTE( "epr-6705.bin", 0x04001, 0x2000, CRC(985327cb) SHA1(86969fe763cbaa527d64de35844773b5ab1d7f83) )
	ROM_LOAD16_BYTE( "epr-6708.bin", 0x04000, 0x2000, CRC(f8863dc5) SHA1(bfdd294d51420dd70aa97942909a9b8a95ffc05c) )
	ROM_LOAD16_BYTE( "epr-6706.bin", 0x08001, 0x2000, BAD_DUMP CRC(79e94cd2) SHA1(f44c2292614b46116818fad9a7eb48cceeb3b819)  )  // was bad, manually patched
	ROM_LOAD16_BYTE( "epr-6709.bin", 0x08000, 0x2000, CRC(f41cb58c) SHA1(f0d1048e949d51432739755f985e4df65b8e918b) )
	ROM_FILL(                        0x0c000, 0x4000, 0 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "epr-6703.bin", 0x00000, 0x2000, CRC(fbff3a86) SHA1(4ed2887b1e4509ded853a230f735d4d2aa475886) )
	ROM_LOAD( "epr-6702.bin", 0x02000, 0x2000, CRC(27ddf031) SHA1(2f11d3b693e46852762669ed1e35a667990edec7) )
	ROM_LOAD( "epr-6701.bin", 0x04000, 0x2000, CRC(3c83588a) SHA1(a84c813ba9d464cffc855397aaacbb9177c86fb4) )
	ROM_LOAD( "epr-6700.bin", 0x06000, 0x2000, CRC(ee579266) SHA1(94dfcf506049fc78db00084ff7031d19520d9a85) )
	ROM_LOAD( "epr-6699.bin", 0x08000, 0x2000, CRC(9bfa4a72) SHA1(8ac4d308dab0d67a26b4e3550c2e8064aaf36a74) )
	ROM_LOAD( "epr-6698.bin", 0x0a000, 0x2000, CRC(7adfd1ff) SHA1(b543dd6734a681a187dabf602bea390de663039c) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "epr-6710.bin", 0x00000, 0x1000, CRC(accda190) SHA1(265d2fd92574d65e7890e48d5f305bf903a67bc8) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "epr-6695.bin", 0x00000, 0x2000, CRC(22bea465) SHA1(4860d7ee3c386cdacc9c608ffe74ec8bfa58edcb) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6694.bin", 0x04000, 0x2000, CRC(51a7345e) SHA1(184c890559ed633e23cb459c313e6179cc3eb542) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6689.bin", 0x08000, 0x2000, CRC(53bf7587) SHA1(0046cd04d11ce789ff69e0807700a624af96eb36) )
	ROM_LOAD( "epr-6688.bin", 0x0a000, 0x2000, CRC(ceb76c5b) SHA1(81fa236871f10c77eb201e1c9771bd57406df15b) )
	ROM_LOAD( "epr-6687.bin", 0x0c000, 0x2000, CRC(9c1f49df) SHA1(1a5cf5278777f829d3654e838bd2bb9f4dbb57ba) )
	ROM_LOAD( "epr-6686.bin", 0x0e000, 0x2000, CRC(3d9e516f) SHA1(498614821f87dbcc39edb1756e1af6b536044e6a) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "epr-6696.bin", 0x00000, 0x2000, CRC(0625f48e) SHA1(bea09ccf37f38678fb53c55bd0a79557d6c81b3f) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6697.bin", 0x04000, 0x2000, CRC(f18afabe) SHA1(abd7f6c0bd0de145c423166a2f4e86ccdb12b1ce) )
	// empty space to unpack previous ROM
	ROM_LOAD( "epr-6690.bin", 0x08000, 0x2000, CRC(a142a11d) SHA1(209c7e0591622434ada4445f3f8789059c5f4f77) )
	ROM_LOAD( "epr-6691.bin", 0x0a000, 0x2000, CRC(b640568c) SHA1(8cef1387c469abec8b488621a94cc9575d6c5fcc) )
	ROM_LOAD( "epr-6692.bin", 0x0c000, 0x2000, CRC(b91d8172) SHA1(8d8f6ea78ebf652f295ce96abf19e628fe777d07) )
	ROM_LOAD( "epr-6693.bin", 0x0e000, 0x2000, CRC(874e3acc) SHA1(29438f196811fc2c8f54b6c47f1c175e4797dd4c) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "pr6627.bpr",  0x0000, 0x100, CRC(5ec5480d) SHA1(f966a277539a5d257f32692cdd92ce44b08599e8) ) // R
	ROM_LOAD( "pr6629.bpr",  0x0100, 0x100, CRC(29c7a393) SHA1(67cced39c0a80655c420aad668dfe836c1d7c643) ) // G
	ROM_LOAD( "pr6628.bpr",  0x0200, 0x100, CRC(8af247a4) SHA1(01702fbce53dd4875e4825f0487e7aed9cf212fa) ) // B
	ROM_LOAD( "pr6630a.bpr", 0x0300, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) ) // CLUT(same PROM x 4)
	ROM_LOAD( "pr6630b.bpr", 0x0400, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) )
	ROM_LOAD( "pr6630c.bpr", 0x0500, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) )
	ROM_LOAD( "pr6630d.bpr", 0x0600, 0x100, CRC(d6e202da) SHA1(500ebd5c95d2d2c33535d25cf7f8f649897dc224) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "pr.bpr",      0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
/*
Gekisou (JPN Ver.)
(c)1985 Eastern

68K55-2
CPU:MC68000P8
OSC:12.000MHz

SOUND BOARD NO.59 MC 07
CPU  :TMP8085AP
Sound:AY-3-8910A,OKI M5232,D8155HC
OSC  :6.144MHz
*/
ROM_START( gekisou )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs
	ROM_LOAD16_BYTE( "1.15b", 0x00001, 0x4000, CRC(945fd546) SHA1(6045dbf11272fcec8320aacb2852d4223d0943a0) )
	ROM_LOAD16_BYTE( "2.15d", 0x00000, 0x4000, CRC(3c057150) SHA1(2b1ad7993addfd1c0eee99dfe5bb3476cd387f6a) )
	ROM_LOAD16_BYTE( "3.14b", 0x08001, 0x4000, CRC(7c1cf4d0) SHA1(a122d3a51d205123e04c694912809e0bb31155d5) )
	ROM_LOAD16_BYTE( "4.14d", 0x08000, 0x4000, CRC(c7282391) SHA1(144a34d74bb1e71e2f799913ab04927d00faec87) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "v1.1l", 0x00000, 0x4000, CRC(dc6af437) SHA1(77112fec51343d8e73765b2a342a888612813c3b) )
	ROM_LOAD( "v2.1h", 0x04000, 0x4000, CRC(cb12582e) SHA1(ef378232e2744540cc4c9187cfb36d780dadc962) )
	ROM_LOAD( "v3.1e", 0x08000, 0x4000, CRC(0ab5e777) SHA1(9177c42418f022a65d73c3302873b894c5a137a4) )

	ROM_REGION( 0x1000, "gfx1", 0 ) // chars
	ROM_LOAD( "0.5c",  0x00000, 0x1000, CRC(7e8bf4d1) SHA1(8abb82be006e8d1df449a5f83d59637314405119) )

	ROM_REGION( 0x10000, "gfx2", 0 ) // tiles
	ROM_LOAD( "7.18r",   0x00000, 0x2000, CRC(a1918b6c) SHA1(6ffa4c845d23d311b59cc19411a68a782618b3fd) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(        0x04000, 0x2000)
	// empty space to unpack previous ROM
	ROM_LOAD( "5.16r",   0x08000, 0x2000, CRC(88ef550a) SHA1(b50e7b8257d1bb6923d289e7af885c14d089b394) )
	ROM_CONTINUE(        0x0c000, 0x2000)
	ROM_LOAD( "6.15r",   0x0a000, 0x2000, CRC(473e3fbf) SHA1(5039387b3627c19f592d630ba7bd010a3881adc5) )
	ROM_CONTINUE(        0x0e000, 0x2000)

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "10.9r",   0x00000, 0x2000, CRC(11d89c73) SHA1(8753f635d321c8e9b93b0ab767cf44aca1db7a0a) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(        0x04000, 0x2000)
	// empty space to unpack previous ROM
	ROM_LOAD( "8.8r",    0x08000, 0x2000, CRC(2e0c392c) SHA1(48542a24a34e3d5d00af418b29a2ee15557efc99) )
	ROM_CONTINUE(        0x0c000, 0x2000)
	ROM_LOAD( "9.6r",    0x0a000, 0x2000, CRC(56a03b08) SHA1(d90b246890fedfc437de85be8bcc6b60ff068be1) )
	ROM_CONTINUE(        0x0e000, 0x2000)

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "1b.bpr",  0x0000, 0x100, CRC(11a1c0aa) SHA1(d007d31f68bf802c89422ea2393747ac8de94d70) ) // R
	ROM_LOAD( "4b.bpr",  0x0100, 0x100, CRC(c7ebe52c) SHA1(19d2ee70d67fd5e1c57f66d030ec9a5b6af5a49e) ) // G
	ROM_LOAD( "2b.bpr",  0x0200, 0x100, CRC(4f5d4141) SHA1(965221c6af4a868760e6d168b55e037fc5f9fa52) ) // B
	ROM_LOAD( "2n.bpr",  0x0300, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) ) // CLUT(same PROM x 4)
	ROM_LOAD( "3n.bpr",  0x0400, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) )
	ROM_LOAD( "4n.bpr",  0x0500, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) )
	ROM_LOAD( "5n.bpr",  0x0600, 0x100, CRC(c7333120) SHA1(ad590e8ece3dcf56b285c4a080f4ee8bb4c9d77c) )

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr",  0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )
ROM_END

/******************************************************************************/
// Splendor Blast ROM Map

/*
Splendor Blast (JPN Ver.)
(c)1985 Alpha denshi

ALPHA 68K24
CPU  :HD68000-8
OSC  :24.000MHz
Other:ALPHA-8303

SOUND BOARD NO.59 MC 07
CPU  :TMP8085AP
Sound:AY-3-8910A,OKI M5232,M5L8155P
OSC  :6.144MHz
*/
ROM_START( splndrbt )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs(16k x 4)
	ROM_LOAD16_BYTE( "1.16a", 0x00001, 0x4000, CRC(4bf4b047) SHA1(ef0efffa2f49905e17e4ed3a03cac419793b26d1) )
	ROM_LOAD16_BYTE( "2.16c", 0x00000, 0x4000, CRC(27acb656) SHA1(5f2f8d05f2f1c6c92c8364e9e6831ca525cbacd0) )
	ROM_LOAD16_BYTE( "3.15a", 0x08001, 0x4000, CRC(5b182189) SHA1(50ebb1fddcb6838442e8a20261f200f3386ce8a8) )
	ROM_LOAD16_BYTE( "4.15c", 0x08000, 0x4000, CRC(cde99613) SHA1(250b59f75eee84442da3cc7c599d1e16f0294df9) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "1_v.1m", 0x00000, 0x2000, CRC(1b3a6e42) SHA1(41a4f0503c939ec0a739c8bc6bf3c8fc354912ee) )
	ROM_LOAD( "2_v.1l", 0x02000, 0x2000, CRC(2a618c72) SHA1(6ad459d94352c317150ae6344d4db9bb613938dd) )
	ROM_LOAD( "3_v.1k", 0x04000, 0x2000, CRC(bbee5346) SHA1(753cb784b04f081fa1f8590dc28056d9918f313b) )
	ROM_LOAD( "4_v.1h", 0x06000, 0x2000, CRC(10f45af4) SHA1(00fa599bad8bf3ba6deee54165f381403096e8f9) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8303_44801b42.bin", 0x0000, 0x2000, CRC(66adcb37) SHA1(e1c72ecb161129dcbddc0b16dd90e716d0c79311) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars
	ROM_LOAD( "10.8c",  0x00000, 0x2000, CRC(501887d4) SHA1(3cf4401d6fddff1500066219a71ac3b30ecbdd28) )

	ROM_REGION( 0x8000, "gfx2", 0 ) // tiles
	ROM_LOAD( "8.14m",  0x00000, 0x4000, CRC(c2c86621) SHA1(a715c70ace98502f2c0d4a81539cd79d19e9b6c4) )
	ROM_LOAD( "9.12m",  0x04000, 0x4000, CRC(4f7da6ff) SHA1(0516271df4a36d6ea38d1b8a5e471e1d2a79e8c1) )

	ROM_REGION( 0x10000, "gfx3", 0 )    // sprites
	ROM_LOAD( "6.18n", 0x00000, 0x2000, CRC(aa72237f) SHA1(0a26746a6c448a7fb853ef708e2bdeb76edd99cf) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(      0x04000, 0x2000 )
	// empty space to unpack previous ROM
	ROM_LOAD( "5.18m", 0x08000, 0x4000, CRC(5f618b39) SHA1(2891067e71b8e1183ee5741487faa1561316cade) )
	ROM_LOAD( "7.17m", 0x0c000, 0x4000, CRC(abdd8483) SHA1(df8c8338c24fa487c49b01ce26db7eb28c8c6b85) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "r.3a",   0x0000, 0x100, CRC(ca1f08ce) SHA1(e46e2850d3ee3c8cbb23c10645f07d406c7ff50b) ) // R
	ROM_LOAD( "g.1a",   0x0100, 0x100, CRC(66f89177) SHA1(caa51c1bf071764d5089487342794cbf023136c0) ) // G
	ROM_LOAD( "b.2a",   0x0200, 0x100, CRC(d14318bc) SHA1(e219963b3e40eb246e608fbe10daa85dbb4c1226) ) // B
	ROM_LOAD( "2.8k",   0x0300, 0x100, CRC(e1770ad3) SHA1(e408b175b8fff934e07b0ded1ee21d7f91a9523d) ) // CLUT bg
	ROM_LOAD( "s5.15p", 0x0400, 0x100, CRC(7f6cf709) SHA1(5938faf937b682dcc83e53444cbf5e0bd7741363) ) // CLUT sprites

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr", 0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )

	ROM_REGION( 0x2100, "user1", 0 ) // bg scaling
	ROM_LOAD( "0.8h",   0x0000, 0x2000, CRC(12681fb5) SHA1(7a0930819d4cd00475d1897128daa6ac865e07d0) ) // x
	ROM_LOAD( "1.9j",   0x2000, 0x0100, CRC(f5b9b777) SHA1(a4ec731be77306db6baf319391c4fe78517fe43e) ) // y

	ROM_REGION( 0x0200, "user2", 0 ) // sprite scaling
	ROM_LOAD( "4.7m",   0x0000, 0x0100, CRC(12cbcd2c) SHA1(a7946820bbf3f7e110a328b673123988af97ce7e) ) // x
	ROM_LOAD( "s3.8l",  0x0100, 0x0100, CRC(1314b0b5) SHA1(31ef4b916110581390afc1ba90c5dca7c08c619f) ) // y
ROM_END

/******************************************************************************/
// High Voltage ROM Map

/*
High Voltage (JPN Ver.)
(c)1985 Alpha denshi

ALPHA 68K24
CPU  :HD68000-8
OSC  :24.000MHz
Other:ALPHA-8304

SOUND BOARD NO.59 MC 07
CPU  :TMP8085AP
Sound:AY-3-8910A,OKI M5232,D8155HC
OSC  :6.144MHz
*/
ROM_START( hvoltage )
	ROM_REGION( 0x10000, "maincpu", 0 ) // 68000 ROMs(16k x 4)
	ROM_LOAD16_BYTE( "1.16a", 0x00001, 0x4000, CRC(82606e3b) SHA1(25c3172928d8f1eda2c4c757d505fdfd91f21ea1) )
	ROM_LOAD16_BYTE( "2.16c", 0x00000, 0x4000, CRC(1d74fef2) SHA1(3df3dc98a78a137da8c5cddf6a8519b477824fb9) )
	ROM_LOAD16_BYTE( "3.15a", 0x08001, 0x4000, CRC(677abe14) SHA1(78b343122f9ad187c823bf49e8f001288c762586) )
	ROM_LOAD16_BYTE( "4.15c", 0x08000, 0x4000, CRC(8aab5a20) SHA1(fb90817173ad69c0e00d03814b4e10b18955c07e) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // 8085A ROMs
	ROM_LOAD( "5_v.1l", 0x00000, 0x4000, CRC(ed9bb6ea) SHA1(73b0251b86835368ec2a4e98a5f61e28e58fd234) )
	ROM_LOAD( "6_v.1h", 0x04000, 0x4000, CRC(e9542211) SHA1(482f2c90e842fe5cc31cc6a39025adf65ba47ce9) )
	ROM_LOAD( "7_v.1e", 0x08000, 0x4000, CRC(44d38554) SHA1(6765971376eafa218fda1accb1e173a7c1850cc8) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "alpha-8304_44801bxx.bin", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x2000, "gfx1", 0 ) // chars
	ROM_LOAD( "5.8c",   0x00000, 0x2000, CRC(656d53cd) SHA1(9971ed7e7da0e8bf46e97e8f75a2c2201b33fc2f) )

	ROM_REGION( 0x8000, "gfx2", 0 ) // tiles
	ROM_LOAD( "9.14m",  0x00000, 0x4000, CRC(506a0989) SHA1(0e7f2c9bab5e83f06a8148f69d8d0cbfe7d55c5e) )
	ROM_LOAD( "10.12m", 0x04000, 0x4000, CRC(98f87d4f) SHA1(94a7a14b0905597993595b347102436d97fc1dc9) )

	ROM_REGION( 0x10000, "gfx3", 0 ) // sprites
	ROM_LOAD( "8.18n", 0x00000, 0x2000, CRC(725acae5) SHA1(ba54598a087f8bb5fa7182b0e85d0e038003e622) )
	// empty space to unpack previous ROM
	ROM_CONTINUE(      0x04000, 0x2000 )
	// empty space to unpack previous ROM
	ROM_LOAD( "6.18m", 0x08000, 0x4000, CRC(9baf2c68) SHA1(208e5ac8eb157d4bf949ab4330827da032a04235) )
	ROM_LOAD( "7.17m", 0x0c000, 0x4000, CRC(12d25fb1) SHA1(99f5d68bd6d6ee5f2acb7685aceacfb0894c4961) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "r.3a",   0x0000, 0x100, CRC(98eccbf6) SHA1(a55755e8388d3edf3020b1129a638fe1e99362b6) ) // R
	ROM_LOAD( "g.1a",   0x0100, 0x100, CRC(fab2ed23) SHA1(6f63b6a3196dda76eb9a885b17d886a14365f922) ) // G
	ROM_LOAD( "b.2a",   0x0200, 0x100, CRC(7274961b) SHA1(d13070060e216d633675a528cf0dc3de94c95ffb) ) // B
	ROM_LOAD( "2.8k",   0x0300, 0x100, CRC(685f4e44) SHA1(110cb8f5a37f22ce9d391bd0cd46dcbb8fcf66b8) ) // CLUT bg
	ROM_LOAD( "s5.15p", 0x0400, 0x100, CRC(b09bcc73) SHA1(f8139feaa9563324b69aeac5c17beccfdbfa0864) ) // CLUT sprites

	ROM_REGION( 0x0020, "prom", 0 )
	ROM_LOAD( "3h.bpr", 0x00000, 0x020, CRC(33b98466) SHA1(017c73cf8c17dc5047c89316ae5b45f8d22092e8) )

	ROM_REGION( 0x2100, "user1", 0 ) // bg scaling
	ROM_LOAD( "0.8h",   0x0000, 0x2000, CRC(12681fb5) SHA1(7a0930819d4cd00475d1897128daa6ac865e07d0) ) // x
	ROM_LOAD( "1.9j",   0x2000, 0x0100, CRC(f5b9b777) SHA1(a4ec731be77306db6baf319391c4fe78517fe43e) ) // y

	ROM_REGION( 0x0200, "user2", 0 ) // sprite scaling
	ROM_LOAD( "4.7m",   0x0000, 0x0100, CRC(12cbcd2c) SHA1(a7946820bbf3f7e110a328b673123988af97ce7e) ) // x
	ROM_LOAD( "3.8l",   0x0100, 0x0100, CRC(1314b0b5) SHA1(31ef4b916110581390afc1ba90c5dca7c08c619f) ) // y
ROM_END

/******************************************************************************/
// Initializations

void equites_state::unpack_block( const char *region, int offset, int size )
{
	UINT8 *rom = memregion(region)->base();
	int i;

	for (i = 0; i < size; ++i)
	{
		rom[(offset + i + size)] = (rom[(offset + i)] >> 4);
		rom[(offset + i)] &= 0x0f;
	}
}

void equites_state::unpack_region( const char *region )
{
	unpack_block(region, 0x0000, 0x2000);
	unpack_block(region, 0x4000, 0x2000);
}


DRIVER_INIT_MEMBER(equites_state,equites)
{
	unpack_region("gfx2");
	unpack_region("gfx3");
}

DRIVER_INIT_MEMBER(equites_state,bullfgtr)
{
	unpack_region("gfx2");
	unpack_region("gfx3");
}

DRIVER_INIT_MEMBER(equites_state,kouyakyu)
{
	unpack_region("gfx2");
	unpack_region("gfx3");
}

DRIVER_INIT_MEMBER(equites_state,gekisou)
{
	unpack_region("gfx2");
	unpack_region("gfx3");

	// install special handlers for unknown device (protection?)
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x580000, 0x580001, write16_delegate(FUNC(equites_state::gekisou_unknown_0_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x5a0000, 0x5a0001, write16_delegate(FUNC(equites_state::gekisou_unknown_1_w),this));
}

DRIVER_INIT_MEMBER(equites_state,splndrbt)
{
	unpack_region("gfx3");
}

DRIVER_INIT_MEMBER(equites_state,hvoltage)
{
	unpack_region("gfx3");

#if HVOLTAGE_DEBUG
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x000038, 0x000039, read16_delegate(FUNC(equites_state::hvoltage_debug_r),this));
#endif
}

/******************************************************************************/

// Game Entries

// Equites Hardware
GAME( 1984, equites,  0,        equites,  equites, equites_state,  equites,  ROT90, "Alpha Denshi Co.",                "Equites", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, equitess, equites,  equites,  equites, equites_state,  equites,  ROT90, "Alpha Denshi Co. (Sega license)", "Equites (Sega)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, bullfgtr, 0,        equites,  bullfgtr, equites_state, bullfgtr, ROT90, "Alpha Denshi Co.",                "Bull Fighter", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, bullfgtrs,bullfgtr, equites,  bullfgtr, equites_state, bullfgtr, ROT90, "Alpha Denshi Co. (Sega license)", "Bull Fighter (Sega)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, kouyakyu, 0,        equites,  kouyakyu, equites_state, kouyakyu, ROT0,  "Alpha Denshi Co.",                "The Koukouyakyuh", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, gekisou,  0,        gekisou,  gekisou, equites_state,  gekisou,  ROT90, "Eastern Corp.",                   "Gekisou (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

// Splendor Blast Hardware
GAME( 1985, splndrbt, 0,        splndrbt, splndrbt, equites_state, splndrbt, ROT0,  "Alpha Denshi Co.", "Splendor Blast", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1985, hvoltage, 0,        splndrbt, hvoltage, equites_state, hvoltage, ROT0,  "Alpha Denshi Co.", "High Voltage", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
