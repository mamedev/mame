// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jonathan Gevaryahu
/***************************************************************************

    IAI Swyft Model P0001
    Copyright (C) 2009-2013 Miodrag Milanovic and Jonathan Gevaryahu AKA Lord Nightmare
    With information and help from John "Sandy" Bumgarner, Dwight Elvey,
    Charles Springer, Terry Holmes, Jonathan Sand, Aza Raskin and others.

    See cat.c for the machine which the swyft later became.

    This driver is dedicated in memory of Jef Raskin and Dave Boulton

IAI detailed credits:
Scott Kim - responsible for fonts on swyft and cat
Ralph Voorhees - Model construction and mockups (swyft 'flat cat')

Cat HLSL stuff:
*scanlines:
the cat has somewhat visible and fairly close scanlines with very little fuzziness
try hlsl options:
hlsl_prescale_x           4
hlsl_prescale_y           4
scanline_alpha            0.3
scanline_size             1.0
scanline_height           0.7
scanline_bright_scale     1.0
scanline_bright_offset    0.6
*phosphor persistence of the original cat CRT is VERY LONG and fades to a greenish-yellow color, though the main color itself is white
try hlsl option:
phosphor_life             0.93,0.95,0.87
which is fairly close but may actually be too SHORT compared to the real thing.


Swyft versions:
There are at least 4 variants of machines called 'swyft':
* The earliest desktop units which use plexi or rubber-tooled case and an
  angled monitor; about a dozen were made and at least two of clear plexi.
  These are sometimes called "wrinkled" swyfts. 5.25" drive, they may be able
  to read Apple2 Swyftware/Swyftdisk and Swyftcard-created disks.
  It is possible no prototypes of this type got beyond the 'runs forth console only' stage.
  http://archive.computerhistory.org/resources/access/physical-object/2011/09/102746929.01.01.lg.JPG
  http://www.digibarn.com/collections/systems/swyft/Swyft-No2-05-1271.jpg
  http://www.digibarn.com/friends/jef-raskin/slides/iai/A%20-687%20SWYFTPRO.JPG
* The early "flat cat" or "roadkill" portable LCD screen units with a white
  case and just a keyboard. Model SP0001
  http://www.digibarn.com/collections/systems/swyft/Image82.jpg
* The later "ur-cat" desktop units which use a machine tooled case and look
  more or less like the canon cat. Around 100-200 were made. 3.5" drive.
  These have a fully functional EDDE editor as the cat does, and can even compile
  forth programs.
  (the 'swyft' driver is based on one of these)
* The very late portable LCD units with a dark grey case and a row of hotkey
  buttons below the screen. Not dumped yet. At least one functional prototype exists.
  At least one plastic mockup exists with no innards.
  http://www.digibarn.com/collections/systems/swyft/swyft.jpg

IAI Swyft:
Board name: 950-0001C
"INFORMATION APPLIANCE INC. COPYRIGHT 1985"
 _________________|||||||||___________________________________________________________________________________
|                     J8               [=======J3=======]  ____                              Trans-           |
==                                                        /    \  (E2)                       former           |
==Jx                   74LS107   J5                       |PB1 |        uA339     MC3403                 -----|
|                                          7407           \____/                                         J7   =
| Y1       "TIMING B" 74LS132    74LS175                                                                 -----|
|                                                  ____________                            4N37  Relay   -----|
| TMS4256   74LS161  "DECODE E" "DISK 3.5C"       |            |                                         J6   =
|                                                 |  MC6850P   |                                         -----|
| TMS4256   74LS166   74HCT259   74LS299          '------------'        MC3403    MC3403                 _____|
|                      ___________________     ___________________             ||                       |     =
| TMS4256   74LS373   |                   |   |                   |            J2                       | J1  =
|                     |   MC68008P8       |   |       R6522P      |            ||              I   P R  |     =
| TMS4256   74F153    '-------------------'   '-------------------'     MN4053 || MN4053       N   R E  | B   =
|                                    (E1)                                                      D   O S  | R   =
| TMS4256   74F153    74HCT08     __________   ___________________      MC14412   DS1489       U E T I  | E   =
|                                |          | |                   | ||                         C S E S  | A   =
| TMS4256   74F153    74HC4040E  | AM27256  | |       R6522P      | ||                         T D C T  | K   =
|                                '----------' '-------------------' ||  INFORMATION            O   T O  | O   =
| TMS4256   74F153    "VIDEO 2B" .----------.                       J4  APPLIANCE INC.         R   I R  | U   =
|                                | AM27256  |   74HC02     74HC374  ||  Copyright 1985         S   O S  | T   =
| TMS4256   74F153    74LS393  B1|__________|                       ||  UM95089  Y2                N    |     =
|_____________________________[________J9___]__________________________________________________D13______|_____=

*Devices of interest:
J1: breakout of joystick, serial/rs232, hex-keypad, parallel port, and forth switch (and maybe cassette?) pins
    DIL 60 pin 2-row right-angle rectangular connector with metal shield contact;
    not all 60 pins are populated in the connector, only pins 1-6, 8, 13-15, 17-18, 23-30, 35-60 are present
    (traced partly by dwight)
J2: unpopulated 8-pin sip header, serial/rs232-related?
    (vcc ? ? ? ? ? ? gnd) (random guess: txd, rxd, rts, cts, dsr, dtr, and one pin could be cd/ri though the modem circuit may do that separately?)
J3: Floppy Connector
    (standard DIL 34 pin 2-row rectangular connector for mini-shugart/pc floppy cable; pin 2 IS connected somewhere and ?probably? is used for /DISKCHANGE like on an Amiga, with pin 34 being /TRUEREADY?)
    (as opposed to normal ibm pc 3.5" drives where pin 2 is unconnected or is /DENSITY *input to drive*, and pin 34 is /DISKCHANGE)
J4: 18-pin sip header for keyboard ribbon cable; bottom edge of board is pin 1
    Pins:
    1: GND through 220k resistor r78
    2: ? phone hook related? anode of diode d7; one of the pins of relay k2; topmost (boardwise) pin of transistor Q10
    3: 74HCT34 pin

J5: locking-tab-type "CONN HEADER VERT 4POS .100 TIN" connector for supplying power
    through a small cable with a berg connector at the other end, to the floppy drive
    (5v gnd gnd 12v)
J6: Phone connector, rj11 jack
J7: Line connector, rj11 jack
J8: 9-pin Video out/power in connector "CONN RECEPT 6POS .156 R/A PCB" plus "CONN RECEPT 3POS .156 R/A PCB" acting as one 9-pin connector
    (NC ? ? ? NC NC ? 12v 5v) (video, vsync, hsync and case/video-gnd are likely here)
    (the video pinout of the cat is: (Video Vsync Hsync SyncGnd PwrGnd PwrGnd +5v(VCC) +12v(VDD) -12v(VEE)) which is not the same as the swyft.
J9: unpopulated DIL 40-pin straight connector for a ROM debug/expansion/RAM-shadow daughterboard
    the pins after pin 12 connect to that of the ROM-LO 27256 pinout counting pins 1,28,2,27,3,26,etc
    the ROM-HI rom has a different /HICE pin which is not connected to this connector
    /LOCE is a15
    /HICE is !a15
    /ROM_OE comes from pin 14 of DECODE_E pal, and is shorted to /ROM_OE' by the cuttable jumper B1 which is not cut
    /ROM_OE' goes to the two EPROMS
    DECODE_18 is DECODE_E pal pin 18
    pin 1 (GND) is in the lower left and the pins count low-high then to the right
    (gnd N/C   E_CLK     R/W    /ROM_OE a17 vcc a14 a13 a8 a9 a11 /ROM_OE' a10 a15 d7 d6 d5 d4 d3 )
    (GND /IPL1 DECODE_18 /RESET gnd     a16 vcc a12 a7  a6 a5 a4  a3       a2  a1  a0 d0 d1 d2 gnd)
Jx: 4 pin on top side, 6 pin on bottom side edge ?debug? connector (doesn't have a Jx number)
    (trace me!)
B1: a cuttable trace on the pcb. Not cut, affects one of the pins on the unpopulated J9 connector only.
E1: jumper, unknown purpose, not set
E2: jumper, unknown purpose, not set
D13: LED
R6522P (upper): parallel port via
R6522P (lower): keyboard via
UM95089: DTMF Tone generator chip (if you read the datasheet this is technically ROM based!)
Y1: 15.8976Mhz, main clock?
Y2: 3.579545Mhz, used by the DTMF generator chip UM95089 (connects to pins 7 and 8 of it)
TMS4256-15NL - 262144 x 1 DRAM
PB1 - piezo speaker

*Pals:
"TIMING B" - AMPAL16R4APC (marked on silkscreen "TIMING PAL")
"DECODE E" - AMPAL16L8PC (marked on silkscreen "DECODE PAL")
"VIDEO 2B" - AMPAL16R4PC (marked on silkscreen "VIDEO PAL")
"DISK 3.5C" - AMPAL16R4PC (marked on silkscreen "DISK PAL")

*Deviations from silkscreen:
4N37 (marked on silkscreen "4N35")
74F153 (marked on silkscreen "74ALS153")
74HCT259 is socketed, possibly intended that the rom expansion daughterboard will have a ribbon cable fit in its socket?


ToDo:
* Swyft
- Figure out the keyboard (interrupts are involved? or maybe an NMI on a
  timer/vblank? It is possible this uses a similar 'keyboard read int'
  to what the cat does)
- get the keyboard scanning actually working; the VIAs are going nuts right now.
- Beeper (on one of the vias?)
- vblank/hblank stuff
- Get the 6850 ACIA working for communications
- Floppy (probably similar to the Cat)
- Centronics port (attached to one of the VIAs)
- Joystick port (also likely on a via)
- Keypad? (also likely on a via done as a grid scan?)
- Forth button (on the port on the back; keep in mind shift-usefront-space ALWAYS enables forth on a swyft)
- Multple undumped firmware revisions exist (330 and 331 are dumped)

// 74ls107 @ u18 pin 1 is 68008 /BERR pin

// mc6850 @ u33 pin 2 (RX_DATA) is
// mc6850 @ u33 pin 3 (RX_CLK) is 6522 @ u35 pin 17 (PB7)
// mc6850 @ u33 pin 4 (TX_CLK) is 6522 @ u35 pin 17 (PB7)
// mc6850 @ u33 pin 5 (/RTS) is
// mc6850 @ u33 pin 6 (TX_DATA) is
// mc6850 @ u33 pin 7 (/IRQ) is 68008 /IPL1 pin 41
// mc6850 @ u33 pin 8 (CS0) is 68008 A12 pin 10
// mc6850 @ u33 pin 9 (CS2) is DECODE E pin 18
// mc6850 @ u33 pin 10 (CS1) is 68008 /BERR pin 40
// mc6850 @ u33 pin 11 (RS) is 68008 A0 pin 46
// mc6850 @ u33 pin 13 (R/W) is 68008 R/W pin 30
// mc6850 @ u33 pin 14 (E) is 68008 E pin 38
// mc6850 @ u33 pin 15-22 (D7-D0) are 68008 D7 to D0 pins 20-27
// mc6850 @ u33 pin 23 (/DCD) is 74hc02 @ u35 pin 1
// mc6850 @ u33 pin 24 (/CTS) is N/C?

// 6522 @ u34:
// pin 2 (PA0) :
// pin 3 (PA1) :
// pin 4 (PA2) :
// pin 5 (PA3) :
// pin 6 (PA4) :
// pin 7 (PA5) :
// pin 8 (PA6) :
// pin 9 (PA7) :
// pin 10 (PB0) :
// pin 11 (PB1) :
// pin 12 (PB2) :
// pin 13 (PB3) :
// pin 14 (PB4) :
// pin 15 (PB5) :
// pin 16 (PB6) :
// pin 17 (PB7) :
// pin 18 (CB1) : ?from/to? Floppy connector j3 pin 8
// pin 19 (CB2) : ?from/to? 6522 @ u35 pin 16 (PB6)
// pin 21 (/IRQ) : out to 68008 /IPL1 pin 41
// pin 22 (R/W) : in from 68008 R/W pin 30
// pin 23 (/CS2) : in from DECODE E pin 18
// pin 24 (CS1) : in from 68008 A13 pin 11
// pin 25 (Phi2) : in from 68008 E pin 38
// pins 26-33 : in/out from/to 68008 D7 to D0 pins 20-27
// pin 34 (/RES) : in from 68008 /RESET pin 37 AND 68008 /HALT pin 36
// pins 35-38 (RS3-RS0) are 68008 A9-A6 pins 7-4
// pin 39 (CA2) is through inductor L11 and resistor r128 to peripheral connector pin 35 <end minus 26>
// pin 40 (CA1) is through inductor L30 and resistor r138 to peripheral connector pin 53 <end minus 8>

// 6522 @ u35
// pin 2 (PA0) :
// pin 3 (PA1) :
// pin 4 (PA2) :
// pin 5 (PA3) :
// pin 6 (PA4) :
// pin 7 (PA5) :
// pin 8 (PA6) :
// pin 9 (PA7) :
// pin 10 (PB0) :
// pin 11 (PB1) : in from 74hc02 @ u36 pin 4
// pin 12 (PB2) :
// pin 13 (PB3) :
// pin 14 (PB4) :
// pin 15 (PB5) :
// pin 16 (PB6) : 6522 @ u34 pin 19 (CB2)
// pin 17 (PB7) : mc6850 @ u33 pins 3 and 4 (RX_CLK, TX_CLK)
// pin 18 (CB1) : ds1489an @ u45 pin 11
// pin 19 (CB2) : mn4053b @ u40 pin 11 and mc14412 @ u41 pin 10
// pin 21 (/IRQ) : out to 68008 /IPL1 pin 41
// pin 22 (R/W) : in from 68008 R/W pin 30
// pin 23 (/CS2) : in from DECODE E pin 18
// pin 24 (CS1) : in from 68008 A14 pin 12
// pin 25 (Phi2) : in from 68008 E pin 38
// pins 26-33 : in/out from/to 68008 D7 to D0 pins 20-27
// pin 34 (/RES) : in from 68008 /RESET pin 37 AND 68008 /HALT pin 36
// pins 35-38 (RS3-RS0) : in from 68008 A9-A6 pins 7-4
// pin 39 (CA2) : out to 74HCT34 pin 11 (CLK) (keyboard column latch)
// pin 40 (CA1) : out? to? ds1489an @ u45 pin 8

// 74hc02 @ u36:
// pin 1 (Y1) : out to mc6850 @ u33 pin 23 /DCD
// pin 2 (A1) : in from (2 places: resistor R58 to ua339 @ u38 pin 4 (In1-)) <where does this actually come from? modem offhook?>
// pin 3 (B1) : in from mn4053b @ u40  pin 10 <probably from rs232>
// pin 4 (Y2) : out to 6522 @u35 pin 11
// pin 5 (A2) : in from 4N37 @ u48 pin 5 (output side emitter pin) (tied via R189 to gnd) <ring indicator?>
// pin 6 (B2) : in from 4N37 @ u48 pin 5 (output side emitter pin) (tied via R189 to gnd) <ring indicator?>
// pin 8 (B3) :
// pin 9 (A3) :
// pin 10 (Y3) :
// pin 11 (B4) : in from 68008 A15
// pin 12 (A4) : in from 68008 A15
// pin 13 (Y4) : out to EPROM @ U31 /CE

****************************************************************************/

// Defines

#undef DEBUG_GA2OPR_W
#undef DEBUG_VIDEO_CONTROL_W

#undef DEBUG_FLOPPY_CONTROL_W
#undef DEBUG_FLOPPY_CONTROL_R
#undef DEBUG_FLOPPY_DATA_W
#undef DEBUG_FLOPPY_DATA_R
#undef DEBUG_FLOPPY_STATUS_R

#undef DEBUG_PRINTER_DATA_W
#undef DEBUG_PRINTER_CONTROL_W

#undef DEBUG_MODEM_R
#undef DEBUG_MODEM_W

#undef DEBUG_DUART_OUTPUT_LINES
// data sent to rs232 port
#undef DEBUG_DUART_TXA
// data sent to modem chip
#undef DEBUG_DUART_TXB
#undef DEBUG_DUART_IRQ_HANDLER
#undef DEBUG_PRN_FF

#undef DEBUG_TEST_W

#define DEBUG_SWYFT_VIA0 1
#define DEBUG_SWYFT_VIA1 1


// Includes
#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/clock.h"
#include "machine/6850acia.h"
#include "machine/6522via.h"
#include "sound/speaker.h"
#include "bus/centronics/ctronics.h"

class swyft_state : public driver_device
{
public:
	swyft_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ctx(*this, "ctx"),
		m_ctx_data_out(*this, "ctx_data_out"),
		m_acia6850(*this, "acia6850"),
		m_via0(*this, "via6522_0"),
		m_via1(*this, "via6522_1"),
		m_speaker(*this, "speaker"),
		m_p_swyft_videoram(*this, "p_swyft_vram")
		/*m_y0(*this, "Y0"),
		m_y1(*this, "Y1"),
		m_y2(*this, "Y2"),
		m_y3(*this, "Y3"),
		m_y4(*this, "Y4"),
		m_y5(*this, "Y5"),
		m_y6(*this, "Y6"),
		m_y7(*this, "Y7")*/
		{ }

	required_device<cpu_device> m_maincpu;
	optional_device<centronics_device> m_ctx;
	optional_device<output_latch_device> m_ctx_data_out;
	required_device<acia6850_device> m_acia6850; // only swyft uses this
	required_device<via6522_device> m_via0; // only swyft uses this
	required_device<via6522_device> m_via1; // only swyft uses this
	optional_device<speaker_sound_device> m_speaker;
	required_shared_ptr<UINT8> m_p_swyft_videoram;
	/*optional_ioport m_y0;
	optional_ioport m_y1;
	optional_ioport m_y2;
	optional_ioport m_y3;
	optional_ioport m_y4;
	optional_ioport m_y5;
	optional_ioport m_y6;
	optional_ioport m_y7;*/

	DECLARE_MACHINE_START(swyft);
	DECLARE_MACHINE_RESET(swyft);
	DECLARE_VIDEO_START(swyft);

	UINT32 screen_update_swyft(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(swyft_d0000);

	DECLARE_READ8_MEMBER(swyft_via0_r);
	DECLARE_WRITE8_MEMBER(swyft_via0_w);
	DECLARE_READ8_MEMBER(via0_pa_r);
	DECLARE_WRITE8_MEMBER(via0_pa_w);
	DECLARE_WRITE_LINE_MEMBER(via0_ca2_w);
	DECLARE_READ8_MEMBER(via0_pb_r);
	DECLARE_WRITE8_MEMBER(via0_pb_w);
	DECLARE_WRITE_LINE_MEMBER(via0_cb1_w);
	DECLARE_WRITE_LINE_MEMBER(via0_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(via0_int_w);

	DECLARE_READ8_MEMBER(swyft_via1_r);
	DECLARE_WRITE8_MEMBER(swyft_via1_w);
	DECLARE_READ8_MEMBER(via1_pa_r);
	DECLARE_WRITE8_MEMBER(via1_pa_w);
	DECLARE_WRITE_LINE_MEMBER(via1_ca2_w);
	DECLARE_READ8_MEMBER(via1_pb_r);
	DECLARE_WRITE8_MEMBER(via1_pb_w);
	DECLARE_WRITE_LINE_MEMBER(via1_cb1_w);
	DECLARE_WRITE_LINE_MEMBER(via1_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(via1_int_w);

	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);

	/* gate array 2 has a 16-bit counter inside which counts at 10mhz and
	   rolls over at FFFF->0000; on roll-over (or likely at FFFF terminal count)
	   it triggers the KTOBF output. It does this every 6.5535ms, which causes
	   a 74LS74 d-latch at IC100 to invert the state of the DUART IP2 line;
	   this causes the DUART to fire an interrupt, which makes the 68000 read
	   the keyboard.
	   The watchdog counter and the 6ms counter are both incremented
	   every time the KTOBF pulses.
	 */
	UINT8 m_keyboard_line;
	UINT8 m_floppy_control;

//protected:
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};


static INPUT_PORTS_START( swyft )
// insert dwight and sandy's swyft keyboard map here once we figure out the byte line order
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('\xa2')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('n') PORT_CHAR('B')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left USE FRONT") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right USE FRONT") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_F2) // intl only: latin diaresis and latin !; norway, danish and finnish * and '; others
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('\xbd') PORT_CHAR('\xbc') //PORT_CHAR('}') PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_F1) // intl only: latin inv ? and inv !; norway and danish ! and |; finnish <>; others
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left LEAP") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('[')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Leap") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Page") PORT_CODE(KEYCODE_PGUP) PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Erase") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // totally unused
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UNDO") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('\xb1') PORT_CHAR('\xb0') // PORT_CHAR('\\') PORT_CHAR('~')
INPUT_PORTS_END


/* Swyft rom and ram notes:
rom:
**Vectors:
0x0000-0x0003: SP boot vector
0x0004-0x0007: PC boot vector
**unknown:
0x0009-0x00BF: ? table
0x00C0-0x01DF: ? table
0x01E0-0x02DF: ? table (may be part of next table)
0x02E0-0x03DF: ? table
0x03E0-0x0B3F: int16-packed jump table (expanded to int32s at ram at 0x46000-0x46EC0 on boot)
0x0B40-0x0E83: ? function index tables?
0x0E84-0x1544: binary code (purpose?)
0x1545-0x24CF: ?
**Fonts:
0x24D0-0x254F: ? (likely font 1 width lookup table)
0x2550-0x2BCF: Font 1 data
0x2BD0-0x2C4F: ? (likely font 2 width lookup table)
0x2C50-0x32CF: Font 2 data
**unknown?:
0x32D0-0x360F: String data (and control codes?)
0x3610-0x364F: ? fill (0x03 0xe8)
0x3650-0x369F: ? fill (0x03 0x20)
0x36A0-0x384d: ? forth code?
0x384e-0x385d: Lookup table for phone keypad
0x385e-...: ?
...-0xC951: ?
0xC952: boot vector
0xC952-0xCAAE: binary code (purpose?)
    0xCD26-0xCD3B: ?init forth bytecode?
0xCD3C-0xCEBA: 0xFF fill (unused?)
0xCEEB-0xFFFE: Forth dictionaries for compiling, with <word> then <3 bytes> afterward? (or before it? most likely afterward)

ram: (system dram ranges from 0x40000-0x7FFFF)
0x40000-0x425CF - the screen display ram
(?0x425D0-0x44BA0 - ?unknown (maybe screen ram page 2?))
0x44DC6 - SP vector
0x46000-0x46EC0 - jump tables to instructions for ? (each forth word?)


on boot:
copy/expand packed rom short words 0x3E0-0xB3F to long words at 0x46000-0x46EC0
copy 0x24f longwords of zero beyond that up to 0x47800
CD26->A5 <?pointer to init stream function?>
44DC6->A7 <reset SP... why it does this twice, once by the vector and once here, i'm gonna guess has to do with running the code in a debugger or on a development daughterboard like the cat had, where the 68008 wouldn't get explicitly reset>
44F2A->A6 <?pointer to work ram space?>
EA2->A4 <?function>
E94->A3 <?function>
EAE->A2 <?function>
41800->D7 <?forth? opcode index base; the '1800' portion gets the opcode type added to it then is multiplied by 4 to produce the jump table offset within the 0x46000-0x46EC0 range>
46e3c->D4 <?pointer to more work ram space?>
CD22->D5 <?pointer to another function?>
write 0xFFFF to d0004.l
jump to A4(EA2)

read first stream byte (which is 0x03) from address pointed to by A5 (which is CD26), inc A5, OR the opcode (0x03) to D7
 (Note: if the forth opcodes are in order in the dictionary, then 0x03 is "!char" which is used to read a char from an arbitrary address)
copy D7 to A0
Add A0 low word to itself
Add A0 low word to itself again
move the long word from address pointed to by A0 (i.e. the specific opcode's area at the 46xxx part of ram) to A1
Jump to A1(11A4)

11A4: move 41b00 to D0 (select an opcode "page" 1bxx)
jump to 118E

118E: read next stream byte (in this case, 0x8E) from address pointed to by A5 (which is CD27), inc A5, OR the opcode (0x8e) to D7
add to 41b00 in d0, for 41b8E
Add A0 low word to itself
Add A0 low word to itself again
move the long word from address pointed to by A0 (i.e. the specific opcode's area at the 46xxx part of ram) to A1
Jump to A1(CD06)

CD06: jump to A3 (E94)

E94: subtract D5 from A5 (cd28 - cd22 = 0x0006)
write 6 to address @A5(44f28) and decrement A5
write D4(46e3c) to address @a6(44f26) and decrement a5
lea ($2, A1), A5 - i.e. increment A1 by 2, and write that to A5, so write CD06+2=CD08 to A5
A1->D5
A0->D4
read next stream byte (in this case, 0x03) from address pointed to by A5 (which is CD08), inc A5, OR the opcode (0x03) to D7

*/

/* Swyft Memory map, based on watching the infoapp roms do their thing:
68k address map:
(a23,a22,a21,a20 lines don't exist on the 68008 so are considered unconnected)
a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  a0
x   x   x   x   0   0   ?   ?   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *        R   ROM-LO (/LOCE is 0, /HICE is 1)
x   x   x   x   0   0   ?   ?   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *        R   ROM-HI (/LOCE is 1, /HICE is 0)
x   x   x   x   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   a        RW  RAM
x   x   x   x   1   1  ?0? ?1?  ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   *   *   *   *        R   ? status of something? floppy?
x   x   x   x   1   1  ?1? ?0?  ?   0   0   1   x   x   x   x   x   x   x   x   x   x   x   *        RW  6850 acia @U33, gets 0x55 steadystate and 0x57 written to it to reset it
x   x   x   x   1   1  ?1? ?0?  ?   0   1   0   x   x   *   *   *   *   x   x   x   x   x   x        RW  Parallel VIA 0 @ U34
x   x   x   x   1   1  ?1? ?0?  ?   1   0   0   x   x   *   *   *   *   x   x   x   x   x   x        RW  Keyboard VIA 1 @ U35
              ^               ^               ^               ^               ^

*/

static ADDRESS_MAP_START(swyft_mem, AS_PROGRAM, 8, swyft_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x00ffff) AM_ROM AM_MIRROR(0xF00000) // 64 KB ROM
	AM_RANGE(0x040000, 0x07ffff) AM_RAM AM_MIRROR(0xF00000) AM_SHARE("p_swyft_vram") // 256 KB RAM
	AM_RANGE(0x0d0000, 0x0d000f) AM_READ(swyft_d0000) AM_MIRROR(0xF00000) // status of something? reads from d0000, d0004, d0008, d000a, d000e
	AM_RANGE(0x0e1000, 0x0e1000) AM_DEVWRITE("acia6850", acia6850_device, control_w) AM_MIRROR(0xF00000) // 6850 ACIA lives here
	AM_RANGE(0x0e2000, 0x0e2fff) AM_READWRITE(swyft_via0_r, swyft_via0_w) AM_MIRROR(0xF00000)// io area with selector on a9 a8 a7 a6?
	AM_RANGE(0x0e4000, 0x0e4fff) AM_READWRITE(swyft_via1_r, swyft_via1_w) AM_MIRROR(0xF00000)
ADDRESS_MAP_END

MACHINE_START_MEMBER(swyft_state,swyft)
{
	m_via0->write_ca1(1);
	m_via0->write_ca2(1);
	m_via0->write_cb1(1);
	m_via0->write_cb2(1);

	m_via1->write_ca1(1);
	m_via1->write_ca2(1);
	m_via1->write_cb1(1);
	m_via1->write_cb2(1);
}

MACHINE_RESET_MEMBER(swyft_state,swyft)
{
}

VIDEO_START_MEMBER(swyft_state,swyft)
{
}

UINT32 swyft_state::screen_update_swyft(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 code;
	int y, x, b;

	int addr = 0;
	for (y = 0; y < 242; y++)
	{
		int horpos = 0;
		for (x = 0; x < 40; x++)
		{
			code = m_p_swyft_videoram[addr++];
			for (b = 7; b >= 0; b--)
			{
				bitmap.pix16(y, horpos++) = (code >> b) & 0x01;
			}
		}
	}
	return 0;
}

READ8_MEMBER( swyft_state::swyft_d0000 )
{
	// wtf is this supposed to be?
	UINT8 byte = 0xFF; // ?
	logerror("mystery device: read from 0x%5X, returning %02X\n", offset+0xD0000, byte);
	return byte;
}


// if bit is 1 enable: (obviously don't set more than one bit or you get bus contention!)
//                                           acia
//                                       via0
//                                    via1
// x   x   x   x   1   1  ?1? ?0?  ?   ^   ^   ^   ?   ?   *   *   *   *  ?*?  ?   ?   ?   ?   ?
//                                                         ^   ^   ^   ^  <- these four bits address the VIA registers? is this correct?
static const char *const swyft_via_regnames[] = { "0: ORB/IRB", "1: ORA/IRA", "2: DDRB", "3: DDRA", "4: T1C-L", "5: T1C-H", "6: T1L-L", "7: T1L-H", "8: T2C-L", "9: T2C-H", "A: SR", "B: ACR", "C: PCR", "D: IFR", "E: IER", "F: ORA/IRA*" };

READ8_MEMBER( swyft_state::swyft_via0_r )
{
	if (offset&0x000C3F) fprintf(stderr,"VIA0: read from invalid offset in 68k space: %06X!\n", offset);
	UINT8 data = m_via0->read(space, (offset>>6)&0xF);
#ifdef DEBUG_SWYFT_VIA0
	logerror("VIA0 register %s read by cpu: returning %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	return data;
}

WRITE8_MEMBER( swyft_state::swyft_via0_w )
{
#ifdef DEBUG_SWYFT_VIA0
	logerror("VIA0 register %s written by cpu with data %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	if (offset&0x000C3F) fprintf(stderr,"VIA0: write to invalid offset in 68k space: %06X, data: %02X!\n", offset, data);
	m_via1->write(space, (offset>>6)&0xF, data);
}

READ8_MEMBER( swyft_state::swyft_via1_r )
{
	if (offset&0x000C3F) fprintf(stderr," VIA1: read from invalid offset in 68k space: %06X!\n", offset);
	UINT8 data = m_via1->read(space, (offset>>6)&0xF);
#ifdef DEBUG_SWYFT_VIA1
	logerror(" VIA1 register %s read by cpu: returning %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	return data;
}

WRITE8_MEMBER( swyft_state::swyft_via1_w )
{
#ifdef DEBUG_SWYFT_VIA1
	logerror(" VIA1 register %s written by cpu with data %02x\n", swyft_via_regnames[(offset>>5)&0xF], data);
#endif
	if (offset&0x000C3F) fprintf(stderr," VIA1: write to invalid offset in 68k space: %06X, data: %02X!\n", offset, data);
	m_via0->write(space, (offset>>6)&0xF, data);
}

// first via
READ8_MEMBER( swyft_state::via0_pa_r )
{
	logerror("VIA0: Port A read!\n");
	return 0xFF;
}

WRITE8_MEMBER( swyft_state::via0_pa_w )
{
	logerror("VIA0: Port A written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( swyft_state::via0_ca2_w )
{
	logerror("VIA0: CA2 written with %d!\n", state);
}

READ8_MEMBER( swyft_state::via0_pb_r )
{
	logerror("VIA0: Port B read!\n");
	return 0xFF;
}

WRITE8_MEMBER( swyft_state::via0_pb_w )
{
	logerror("VIA0: Port B written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( swyft_state::via0_cb1_w )
{
	logerror("VIA0: CB1 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( swyft_state::via0_cb2_w )
{
	logerror("VIA0: CB2 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( swyft_state::via0_int_w )
{
	logerror("VIA0: INT output set to %d!\n", state);
}

// second via
READ8_MEMBER( swyft_state::via1_pa_r )
{
	logerror(" VIA1: Port A read!\n");
	return 0xFF;
}

WRITE8_MEMBER( swyft_state::via1_pa_w )
{
	logerror(" VIA1: Port A written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( swyft_state::via1_ca2_w )
{
	logerror(" VIA1: CA2 written with %d!\n", state);
}

READ8_MEMBER( swyft_state::via1_pb_r )
{
	logerror(" VIA1: Port B read!\n");
	return 0xFF;
}

WRITE8_MEMBER( swyft_state::via1_pb_w )
{
	logerror(" VIA1: Port B written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER ( swyft_state::via1_cb1_w )
{
	logerror(" VIA1: CB1 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( swyft_state::via1_cb2_w )
{
	logerror(" VIA1: CB2 written with %d!\n", state);
}

WRITE_LINE_MEMBER ( swyft_state::via1_int_w )
{
	logerror(" VIA1: INT output set to %d!\n", state);
}

WRITE_LINE_MEMBER( swyft_state::write_acia_clock )
{
	m_acia6850->write_txc(state);
	m_acia6850->write_rxc(state);
}

static MACHINE_CONFIG_START( swyft, swyft_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68008, XTAL_15_8976MHz/2) //MC68008P8, Y1=15.8976Mhz, clock GUESSED at Y1 / 2
	MCFG_CPU_PROGRAM_MAP(swyft_mem)

	MCFG_MACHINE_START_OVERRIDE(swyft_state,swyft)
	MCFG_MACHINE_RESET_OVERRIDE(swyft_state,swyft)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 242)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 242-1)
	MCFG_SCREEN_UPDATE_DRIVER(swyft_state, screen_update_swyft)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_VIDEO_START_OVERRIDE(swyft_state,swyft)

	MCFG_DEVICE_ADD("acia6850", ACIA6850, 0)
	// acia rx and tx clocks come from one of the VIA pins and are tied together, fix this below? acia e clock comes from 68008
	MCFG_DEVICE_ADD("acia_clock", CLOCK, (XTAL_15_8976MHz/2)/5) // out e clock from 68008, ~ 10in clocks per out clock
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(swyft_state, write_acia_clock))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, (XTAL_15_8976MHz/2)/5) // out e clock from 68008
	MCFG_VIA6522_READPA_HANDLER(READ8(swyft_state, via0_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(swyft_state, via0_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(swyft_state, via0_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(swyft_state, via0_pb_w))
	MCFG_VIA6522_CB1_HANDLER(WRITELINE(swyft_state, via0_cb1_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(swyft_state, via0_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(swyft_state, via0_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(swyft_state, via0_int_w))

	MCFG_DEVICE_ADD("via6522_1", VIA6522, (XTAL_15_8976MHz/2)/5) // out e clock from 68008
	MCFG_VIA6522_READPA_HANDLER(READ8(swyft_state, via1_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(swyft_state, via1_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(swyft_state, via1_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(swyft_state, via1_pb_w))
	MCFG_VIA6522_CB1_HANDLER(WRITELINE(swyft_state, via1_cb1_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(swyft_state, via1_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(swyft_state, via1_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(swyft_state, via1_int_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( swyft )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v331", "IAI Swyft Version 331 Firmware")
	ROMX_LOAD( "331-lo.u30", 0x0000, 0x8000, CRC(d6cc2e2f) SHA1(39ff26c18b1cf589fc48793263f280ef3780cc61), ROM_BIOS(1))
	ROMX_LOAD( "331-hi.u31", 0x8000, 0x8000, CRC(4677630a) SHA1(8845d702fa8b8e1a08352f4c59d3076cc2e1307e), ROM_BIOS(1))
	/* this version of the swyft code identifies itself at 0x3FCB as version 330 */
	ROM_SYSTEM_BIOS( 1, "v330", "IAI Swyft Version 330 Firmware")
	ROMX_LOAD( "infoapp.lo.u30", 0x0000, 0x8000, CRC(52c1bd66) SHA1(b3266d72970f9d64d94d405965b694f5dcb23bca), ROM_BIOS(2))
	ROMX_LOAD( "infoapp.hi.u31", 0x8000, 0x8000, CRC(83505015) SHA1(693c914819dd171114a8c408f399b56b470f6be0), ROM_BIOS(2))
	ROM_REGION( 0x4000, "pals", ROMREGION_ERASEFF )
	/* Swyft PALs:
	 * The Swyft has four PALs, whose rough function can be derived from their names:
	 * TIMING - state machine for DRAM refresh/access; handles ras/cas and choosing whether the video out shifter or the 68k is accessing ram. also divides clock
	 * DECODE - address decoder for the 68008
	 * VIDEO - state machine for the video shifter (and vblank/hblank?)
	 * DISK 3.5 - state machine for the floppy drive interface
	 */
	/* U9: Timing AMPAL16R4
	 *
	 * pins:
	 * 111111111000000000
	 * 987654321987654321
	 * ??QQQQ??EIIIIIIIIC
	 * |||||||||||||||||\-< /CK input - 15.8976mhz crystal and transistor oscillator
	 * ||||||||||||||||\--< ?
	 * |||||||||||||||\---< ?
	 * ||||||||||||||\----< ?
	 * |||||||||||||\-----< ?<also input to decode pal pin 1, video pal pin 1, source is ?>
	 * ||||||||||||\------< ?
	 * |||||||||||\-------< ?
	 * ||||||||||\--------< ?
	 * |||||||||\---------< ?
	 * ||||||||\----------< /OE input - shorted to GND
	 * |||||||\-----------? ?
	 * ||||||\------------? ?
	 * |||||\------------Q> /ROM_OE (to both eproms through jumper b1 and optionally j9 connector)
	 * ||||\-------------Q? ?
	 * |||\--------------Q? ?
	 * ||\---------------Q> output to decode pal pin 2
	 * |\----------------->? output? to ram multiplexer 'A' pins
	 * \------------------< ?
	 */
	ROM_LOAD( "timing_b.ampal16r4a.u9.jed", 0x0000, 0xb08, CRC(643e6e83) SHA1(7db167883f9d6cf385ce496d08976dc16fc3e2c3))
	/* U20: Decode AMPAL16L8
	 *
	 * pins:
	 * 111111111000000000
	 * 987654321987654321
	 * O??????OIIIIIIIIII
	 * |||||||||||||||||\-< TIMING PAL pin 5
	 * ||||||||||||||||\--< TIMING PAL pin 17
	 * |||||||||||||||\---< 68008 R/W (pin 30)
	 * ||||||||||||||\----< 68008 /DS (pin 29)
	 * |||||||||||||\-----< 68008 E (pin 38)
	 * ||||||||||||\------< 68008 A19
	 * |||||||||||\-------< 68008 A18
	 * ||||||||||\--------< 68008 A17
	 * |||||||||\---------< 68008 A16
	 * ||||||||\----------< ?
	 * |||||||\-----------> ?
	 * ||||||\------------? 68008 /VPA (pin 39)
	 * |||||\-------------> /ROM_OE (to both eproms through jumper b1 and optionally j9 connector)
	 * ||||\--------------? ?
	 * |||\---------------? ?
	 * ||\----------------? ?
	 * |\-----------------? goes to j9 connector pin 5
	 * \------------------< ?
	 */
	ROM_LOAD( "decode_e.ampal16l8.u20.jed", 0x1000, 0xb08, CRC(0b1dbd76) SHA1(08c144ad7a7bbdd53eefd271b2f6813f8b3b1594))
	ROM_LOAD( "video_2b.ampal16r4.u25.jed", 0x2000, 0xb08, CRC(caf91148) SHA1(3f8ddcb512a1c05395c74ad9a6ba7b87027ce4ec))
	ROM_LOAD( "disk_3.5c.ampal16r4.u28.jed", 0x3000, 0xb08, CRC(fd994d02) SHA1(f910ab16587dd248d63017da1e5b37855e4c1a0c))
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT   MACHINE    INPUT    DEVICE         INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1985, swyft,0,      0,       swyft,     swyft,   driver_device, 0,       "Information Applicance Inc", "Swyft", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
