// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/*******************************************************************************

PK-01 Lviv driver by Krzysztof Strzecha

Big thanks go to:
Anton V. Ignatichev for informations about Lviv hardware.
Dr. Volodimir Mosorov for two Lviv machines.

What's new:
-----------
28.02.2003      Snapshot verifying function added.
07.01.2003  Support for .SAV snapshots. Joystick support (there are strange
        problems with "Doroga (1991)(-)(Ru).lvt".
21.12.2002  Cassette support rewritten, WAVs saving and loading are working now.
08.12.2002  Comments on emulation status updated. Changed 'lvive' to 'lvivp'.
        ADC r instruction in I8080 core fixed (Arkanoid works now).
        Original keyboard layout added.
20.07.2002  "Reset" key fixed. I8080 core fixed (all BASIC commands works).
        now). Unsupported .lvt files versions aren't now loaded.
xx.07.2002  Improved port and memory mapping (Raphael Nabet).
        Hardware description updated (Raphael Nabet).
27.03.2002  CPU clock changed to 2.5MHz.
        New Lviv driver added for different ROM revision.
24.03.2002  Palette emulation.
        Bit 7 of port 0xc1 emulated - speaker enabled/disabled.
        Some notes about hardware added.
        "Reset" key added.
23.03.2002  Hardware description and notes on emulation status added.
        Few changes in keyboard mapping.

Notes on emulation status and to do list:
-----------------------------------------
1. LIMITATION: Printer is not emulated.
2. LIMITATION: Timings are not implemented, due to it emulated machine runs
   twice fast as original.
3. LIMITATION: .RSS files are not supported.
4. LIMITATION: Some usage notes and trivia are needed in sysinfo.dat.

Lviv technical information
==========================

CPU:
----
    I8080 2.5MHz (2MHz in first machines)

Memory map:
-----------
    start-up map (cleared by the first I/O write operation done by the CPU):
    0000-3fff ROM mirror #1
    4000-7fff ROM mirror #2
    8000-bfff ROM mirror #3
    c000-ffff ROM

    normal map with video RAM off:
    0000-3fff RAM
    4000-7fff RAM
    8000-bfff RAM
    c000-ffff ROM

    normal map with video RAM on:
    0000-3fff mirrors 8000-bfff
    4000-7fff video RAM
    8000-bfff RAM
    c000-ffff ROM

Interrupts:
-----------
    No interrupts in Lviv.

Ports:
------
    Only A4-A5 are decoded.  A2-A3 is ignored in the console, but could be used by extension
    devices.

    C0-C3   8255 PPI
        Port A: extension slot output, printer data
            bits 0-4 joystick scanner output
        Port B: palette control, extension slot input or output
            sound on/off
            bit 7 sound on/off
            bits 0-6 palette select
        Port C: memory page changing, tape input and output,
            printer control, sound
            bits 0-3 extension slot input
            bits 4-7 extension slot output
            bit 7: joystick scanner input
            bit 6: printer control AC/busy
            bit 5: not used
            bit 4: tape in
            bit 3: not used
            bit 2: printer control SC/strobe
            bit 1: memory paging, 0 - video ram, 1 - ram
            bit 0: tape out, sound

    D0-D3   8255 PPI
        Port A:
            keyboard scanning
        Port B:
            keyboard reading
        Port C:
            keyboard scanning/reading

Keyboard:
---------
    Reset - connected to CPU reset line

                     Port D0
    --------T-------T-------T-------T-------T-------T-------T-------??
    |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
    +-------+-------+-------+-------+-------+-------+-------+-------+---??
    | Shift |   ;   |       |  CLS  | Space |   R   |   G   |   6   | 0 |
    +-------+-------+-------+-------+-------+-------+-------+-------+---+
    |   Q   |Russian|       |  (G)  |   B   |   O   |   [   |   7   | 1 |
    +-------+-------+-------+-------+-------+-------+-------+-------+---+
    |   ^   |  Key  |   J   |  (B)  |   @   |   L   |   ]   |   8   | 2 |
    +-------+-------+-------+-------+-------+-------+-------+-------+---+
    |   X   |   P   |   N   |   5   |  Alt  |  Del  | Enter | Ready | 3 |
    +-------+-------+-------+-------+-------+-------+-------+-------+---+ Port D1
    |   T   |   A   |   E   |   4   |   _   |   .   |  Run  |  Tab  | 4 |
    +-------+-------+-------+-------+-------+-------+-------+-------+---+
    |   I   |   W   |   K   |   3   | Latin |   \   |   :   |   -   | 5 |
    +-------+-------+-------+-------+-------+-------+-------+-------+---+
    |   M   |   Y   |   U   |   2   |   /   |   V   |   H   |   0   | 6 |
    +-------+-------+-------+-------+-------+-------+-------+-------+---+
    |   S   |   F   |   C   |   1   |   ,   |   D   |   Z   |   9   | 7 |
    L-------+-------+-------+-------+-------+-------+-------+-------+----

             Port D2
    --------T-------T-------T-------??
    |   3   |   2   |   1   |   0   |
    +-------+-------+-------+-------+-----??
    | Right | Home  |ScrPrn |PrnLock|  4  |
    +-------+-------+-------+-------+-----+
    |  Up   |  F5   |  F0   |ScrLock|  5  |
    +-------+-------+-------+-------+-----+ Port D2
    | Left  |  F4   |  F1   | Sound |  6  |
    +-------+-------+-------+-------+-----+
    | Down  |  F3   |  F2   |  (R)  |  7  |
    L-------+-------+-------+-------+------

    Notes:
        CLS - clear screen
        (G) - clear screen with border and set COLOR 0,0,0
        (B) - clear screen with border and set COLOR 1,0,6
        (R) - clear screen with border and set COLOR 0,7,3
        Sound   - sound on/off
        ScrLock - screen lock
        PrnLock - printer on/off
        ScrPrn  - screen and printer output mode
        Russian - russian keyboard mode
        Latin   - latin keyboard mode
        Right   - cursor key
        Up  - cursor key
        Left    - cursor key
        Down    - cursor key
        Keyword - BASIC keyword


Video:
-----
    Screen resolution is 256x256 pixels. 4 colors at once are possible,
    but there is a possibility of palette change. Bits 0..6 of port 0xc1
    are used for palette setting.

    One byte of video-RAM sets 4 pixels. Colors of pixels are corrected
    by current palette. Each bits combination (2 bits sets one pixel on
    the display), corrected with palette register, sets REAL pixel color.

    PBx - bit of port 0xC1 numbered x
    R,G,B - output color components
    == - "is equal"
    ! - inversion

    00   R = PB3 == PB4; G = PB5; B = PB2 == PB6;
    01   R = PB4; G = !PB5; B = PB6;
    10   R = PB0 == PB4; G = PB5; B = !PB6;
    11   R = !PB4; G = PB1 == PB5; B = PB6;

    Bit combinations are result of concatenation of appropriate bits of
    high and low byte halfs.

    Example:
    ~~~~~~~~

    Some byte of video RAM:  1101 0001
    Value of port 0xC1:      x000 1110

    1101
    0001
    ----
    10 10 00 11

    1st pixel (10): R = 1; G = 0; B = 1;
    2nd pixel (10): R = 1; G = 0; B = 1;
    3rd pixel (00): R = 0; G = 0; B = 0;
    4th pixel (11): R = 1; G = 0; B = 0;


Sound:
------
    Buzzer connected to port 0xc2 (bit 0).
    Bit 7 of port 0xc1 - enable/disable speaker.


Timings:
--------

    The CPU timing is controlled by a KR580GF24 (Sovietic copy of i8224) connected to a 18MHz(?)
    oscillator. CPU frequency must be 18MHz/9 = 2MHz.

    Memory timing uses a 8-phase clock, derived from a 20MHz(?) video clock (called VCLK0 here:
    in the schematics, it comes from pin 6 of V8, and it is labelled "0'" in the video clock bus).
    This clock is divided by G7, G6 and D5 to generate the signals we call VCLK1-VCLK11.  The memory
    clock phases Phi0-Phi7 are generated in D7, whereas PHI'14 and PHI'15 are generated in D8.

    When the CPU accesses RAM, wait states are inserted until the RAM transfer is complete.

    CPU clock: 18MHz/9 = 2MHz
    memory cycle time: 20MHz/8 = 2.5MHz
    CPU memory access time: (min) approx. 9/20MHz = 450ns
                            (max) approx. 25/20MHz = 1250ns
    pixel clock: 20MHz/4 = 5MHz
    screen size: 256*256
    HBL: 64 pixel clock cycles
    VBL: 64 lines
    horizontal frequency: 5MHZ/(256+64) = 15.625kHz
    vertical frequency: 15.625kHz/(256+64) = 48.83Hz

             |<--------VIDEO WINDOW--------->|<----------CPU WINDOW--------->|<--
            _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _   _
    VCLK0    |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |
            _     ___     ___     ___     ___     ___     ___     ___     ___     ___
    VCLK1    |___|   |___|   |___|   |___|   |___|   |___|   |___|   |___|   |___|   |
            _         _______         _______         _______         _______
    VCLK2    |_______|       |_______|       |_______|       |_______|       |______|
            _                 _______________                 _______________
    VCLK3    |_______________|               |_______________|               |_______
            _                                 _______________________________
    VCLK4    |_______________________________|                               |_______

              _                               _                               _
    PHI0    _| |_____________________________| |_____________________________| |_____
                  _                               _                               _
    PHI1    _____| |_____________________________| |_____________________________| |_
                      _                               _
    PHI2    _________| |_____________________________| |_____________________________
                          _                               _
    PHI3    _____________| |_____________________________| |_________________________
                              _                               _
    PHI4    _________________| |_____________________________| |_____________________
                                  _                               _
    PHI5    _____________________| |_____________________________| |_________________
                                      _                               _
    PHI6    _________________________| |_____________________________| |_____________
                                          _                               _
    PHI7    _____________________________| |_____________________________| |_________
                                                                      _
    PHI'14  _________________________________________________________| |_____________
                                                                          _
    PHI'15  _____________________________________________________________| |_________
            __________             __________________________________________________
    RAS*              \___________/                   \_a_________/
            ______________                 __________________________________________
    CAS*                  \_______________/               \_a_____________/
            _________________________________________________________________________
    WR*                                                       \_b_________////////
            _________________________________________________________________________
    WRM*    \\\\\\\\\\\\\\\\\\\\\\\\\\_b__________________________________///////////
                        _________________________________________________________________________
        RDM*    \\\\\\\\\\\\\\\\\\\\\\\\\\_c __________________________________///////////
                        _________________________________________________________________________
    RA      \\\\\\\\\\\\\\\\\\\\\\\\\\_a__________________________________/

    DRAM
    ADDRESS video row /\ video column /XXX\CPU row (a)/\  CPU column (a)  /\ video row

    a: only if the CPU is requesting a RAM read/write
    b: only if the CPU is requesting a RAM write
    c: only if the CPU is requesting a RAM read

*******************************************************************************/

#include "emu.h"
#include "includes/lviv.h"

#include "cpu/i8085/i8085.h"
#include "sound/wave.h"

#include "softlist.h"
#include "speaker.h"

#include "formats/lviv_lvt.h"


/* I/O ports */

void lviv_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(lviv_state::io_r), FUNC(lviv_state::io_w));
}

/* memory w/r functions */

void lviv_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankrw(m_bank[0]);
	map(0x4000, 0x7fff).bankrw(m_bank[1]);
	map(0x8000, 0xbfff).bankrw(m_bank[2]);
	map(0xc000, 0xffff).bankrw(m_bank[3]);
}


/* keyboard input */
static INPUT_PORTS_START (lviv)
	PORT_START("KEY0") /* 2nd PPI port A bit 0 low */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('\'')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('(')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ready") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('=')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR(')')
	PORT_START("KEY1") /* 2nd PPI port A bit 1 low */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('G')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('[')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR(']')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)        PORT_CHAR(13)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Run") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR('*') PORT_CHAR(':')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('H')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('Z')
	PORT_START("KEY2") /* 2nd PPI port A bit 2 low */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('R')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('O')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('L')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)          PORT_CHAR('.') PORT_CHAR('>')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)    PORT_CHAR('\\')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR('V')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('D')
	PORT_START("KEY3") /* 2nd PPI port A bit 3 low */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR('B')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('@')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('_')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Latin") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_START("KEY4") /* 2nd PPI port A bit 4 low */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cls") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(G)") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(B)") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('"')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')
	PORT_START("KEY5") /* 2nd PPI port A bit 5 low */
		PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('J')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('N')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('E')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('K')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('U')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('C')
	PORT_START("KEY6") /* 2nd PPI port A bit 6 low */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR(';') PORT_CHAR('+')
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Russian") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Key") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('P')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('A')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('W')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('Y')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('F')
	PORT_START("KEY7") /* 2nd PPI port A bit 7 low */
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('Q')
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('^')
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('X')
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('T')
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('I')
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('M')
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('S')
	PORT_START("KEY8") /* 2nd PPI port C bit 0 low */
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PrnLck") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ScrLck") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sound") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(R)") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_START("KEY9") /* 2nd PPI port C bit 1 low */
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ScrPrn") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_START("KEY10") /* 2nd PPI port C bit 2 low */
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_SCRLOCK) PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_START("KEY11") /* 2nd PPI port C bit 3 low */
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_START("RESET") /* CPU */
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_CHANGED_MEMBER(DEVICE_SELF, lviv_state, reset_button, 0)
	PORT_START("JOY") /* Joystick */
		PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
		PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
		PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)
		PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)
		PORT_BIT(0x10,  IP_ACTIVE_HIGH, IPT_BUTTON1)
		PORT_BIT(0x20,  IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40,  IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80,  IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


/* machine definition */
MACHINE_CONFIG_START(lviv_state::lviv)
	/* basic machine hardware */
	I8080(config, m_maincpu, 2500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &lviv_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &lviv_state::io_map);
	config.m_minimum_quantum = attotime::from_hz(60);

	I8255(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set(FUNC(lviv_state::ppi_0_porta_r));
	m_ppi[0]->out_pa_callback().set(FUNC(lviv_state::ppi_0_porta_w));
	m_ppi[0]->in_pb_callback().set(FUNC(lviv_state::ppi_0_portb_r));
	m_ppi[0]->out_pb_callback().set(FUNC(lviv_state::ppi_0_portb_w));
	m_ppi[0]->in_pc_callback().set(FUNC(lviv_state::ppi_0_portc_r));
	m_ppi[0]->out_pc_callback().set(FUNC(lviv_state::ppi_0_portc_w));

	I8255(config, m_ppi[1]);
	m_ppi[1]->in_pa_callback().set(FUNC(lviv_state::ppi_1_porta_r));
	m_ppi[1]->out_pa_callback().set(FUNC(lviv_state::ppi_1_porta_w));
	m_ppi[1]->in_pb_callback().set(FUNC(lviv_state::ppi_1_portb_r));
	m_ppi[1]->out_pb_callback().set(FUNC(lviv_state::ppi_1_portb_w));
	m_ppi[1]->in_pc_callback().set(FUNC(lviv_state::ppi_1_portc_r));
	m_ppi[1]->out_pc_callback().set(FUNC(lviv_state::ppi_1_portc_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(0);
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(lviv_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(lviv_state::lviv_palette), ARRAY_LENGTH(s_palette));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", m_cassette).add_route(ALL_OUTPUTS, "mono", 0.25);
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* snapshot */
	MCFG_SNAPSHOT_ADD("snapshot", lviv_state, lviv, "sav")

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(lviv_lvt_format);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("lviv_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("lviv");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
MACHINE_CONFIG_END


ROM_START(lviv)
	ROM_REGION(0x14000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "lviv", "Lviv/L'vov" )
	ROMX_LOAD("lviv.bin", 0x10000, 0x4000, CRC(44a347d9) SHA1(74e067493b2b7d9ab17333202009a1a4f5e460fd), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "lviva", "Lviv/L'vov (alternate)" )
	ROMX_LOAD("lviva.bin", 0x10000, 0x4000, CRC(551622f5) SHA1(b225f3542b029d767b7db9dce562e8a3f77f92a2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "lvivp", "Lviv/L'vov (prototype)" )
	ROMX_LOAD("lvivp.bin", 0x10000, 0x4000, CRC(f171c282) SHA1(c7dc2bdb02400e6b5cdcc50040eb06f506a7ed84), ROM_BIOS(2))
ROM_END

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY        FULLNAME      FLAGS */
COMP( 1989, lviv, 0,      0,      lviv,    lviv,  lviv_state, empty_init, "V. I. Lenin", "PK-01 Lviv", 0 )
