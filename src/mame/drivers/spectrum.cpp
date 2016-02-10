// license:GPL-2.0+
// copyright-holders:Kevin Thacker
/***************************************************************************

    NOTE: ****** Specbusy: press N, R, or E to boot *************


        Spectrum/Inves/TK90X etc. memory map:

    CPU:
        0000-3fff ROM
        4000-ffff RAM

        Spectrum 128/+2/+2a/+3 memory map:

        CPU:
                0000-3fff Banked ROM/RAM (banked rom only on 128/+2)
                4000-7fff Banked RAM
                8000-bfff Banked RAM
                c000-ffff Banked RAM

        TS2068 memory map: (Can't have both EXROM and DOCK active)
        The 8K EXROM can be loaded into multiple pages.

    CPU:
                0000-1fff     ROM / EXROM / DOCK (Cartridge)
                2000-3fff     ROM / EXROM / DOCK
                4000-5fff \
                6000-7fff  \
                8000-9fff  |- RAM / EXROM / DOCK
                a000-bfff  |
                c000-dfff  /
                e000-ffff /


Interrupts:

Changes:

29/1/2000   KT -    Implemented initial +3 emulation.
30/1/2000   KT -    Improved input port decoding for reading and therefore
            correct keyboard handling for Spectrum and +3.
31/1/2000   KT -    Implemented buzzer sound for Spectrum and +3.
            Implementation copied from Paul Daniel's Jupiter driver.
            Fixed screen display problems with dirty chars.
            Added support to load .Z80 snapshots. 48k support so far.
13/2/2000   KT -    Added Interface II, Kempston, Fuller and Mikrogen
            joystick support.
17/2/2000   DJR -   Added full key descriptions and Spectrum+ keys.
            Fixed Spectrum +3 keyboard problems.
17/2/2000   KT -    Added tape loading from WAV/Changed from DAC to generic
            speaker code.
18/2/2000   KT -    Added tape saving to WAV.
27/2/2000   KT -    Took DJR's changes and added my changes.
27/2/2000   KT -    Added disk image support to Spectrum +3 driver.
27/2/2000   KT -    Added joystick I/O code to the Spectrum +3 I/O handler.
14/3/2000   DJR -   Tape handling dipswitch.
26/3/2000   DJR -   Snapshot files are now classifed as snapshots not
            cartridges.
04/4/2000   DJR -   Spectrum 128 / +2 Support.
13/4/2000   DJR -   +4 Support (unofficial 48K hack).
13/4/2000   DJR -   +2a Support (rom also used in +3 models).
13/4/2000   DJR -   TK90X, TK95 and Inves support (48K clones).
21/4/2000   DJR -   TS2068 and TC2048 support (TC2048 Supports extra video
            modes but doesn't have bank switching or sound chip).
09/5/2000   DJR -   Spectrum +2 (France, Spain), +3 (Spain).
17/5/2000   DJR -   Dipswitch to enable/disable disk drives on +3 and clones.
27/6/2000   DJR -   Changed 128K/+3 port decoding (sound now works in Zub 128K).
06/8/2000   DJR -   Fixed +3 Floppy support
10/2/2001   KT  -   Re-arranged code and split into each model emulated.
            Code is split into 48k, 128k, +3, tc2048 and ts2048
            segments. 128k uses some of the functions in 48k, +3
            uses some functions in 128, and tc2048/ts2048 use some
            of the functions in 48k. The code has been arranged so
            these functions come in some kind of "override" order,
            read functions changed to use  READ8_HANDLER and write
            functions changed to use WRITE8_HANDLER.
            Added Scorpion256 preliminary.
18/6/2001   DJR -   Added support for Interface 2 cartridges.
xx/xx/2001  KS -    TS-2068 sound fixed.
            Added support for DOCK cartridges for TS-2068.
            Added Spectrum 48k Psycho modified rom driver.
            Added UK-2086 driver.
23/12/2001  KS -    48k machines are now able to run code in screen memory.
                Programs which keep their code in screen memory
                like monitors, tape copiers, decrunchers, etc.
                works now.
                Fixed problem with interrupt vector set to 0xffff (much
            more 128k games works now).
                A useful used trick on the Spectrum is to set
                interrupt vector to 0xffff (using the table
                which contain 0xff's) and put a byte 0x18 hex,
                the opcode for JR, at this address. The first
                byte of the ROM is a 0xf3 (DI), so the JR will
                jump to 0xfff4, where a long JP to the actual
                interrupt routine is put. Due to unideal
                bankswitching in MAME this JP were to 0001 what
                causes Spectrum to reset. Fixing this problem
                made much more software runing (i.e. Paperboy).
            Corrected frames per second value for 48k and 128k
            Sincalir machines.
                There are 50.08 frames per second for Spectrum
                48k what gives 69888 cycles for each frame and
                50.021 for Spectrum 128/+2/+2A/+3 what gives
                70908 cycles for each frame.
            Remaped some Spectrum+ keys.
                Presing F3 to reset was seting 0xf7 on keyboard
                input port. Problem occurred for snapshots of
                some programms where it was readed as pressing
                key 4 (which is exit in Tapecopy by R. Dannhoefer
                for example).
            Added support to load .SP snapshots.
            Added .BLK tape images support.
                .BLK files are identical to .TAP ones, extension
                is an only difference.
08/03/2002  KS -    #FF port emulation added.
                Arkanoid works now, but is not playable due to
                completly messed timings.
25/10/2012  DH - simplified border emulation to be a (manual) partial
                 update with bitmap
                 Removed legacy fff4 interrupt hack, modern version of
                 MAME can handle this just fine

Initialisation values used when determining which model is being emulated:
 48K        Spectrum doesn't use either port.
 128K/+2    Bank switches with port 7ffd only.
 +3/+2a     Bank switches with both ports.

TODO: (the best resource for ZX Spectrum HW informations is the http://www.zxdesign.info/
       blog, a project devored to reverse-engineer the ZX Spectrum in h/w).

 1. No contended memory. The different Spectrum models have different contention timings
    Updated info can be found here: http://scratchpad.wikia.com/wiki/Contended_memory
    Of course, neither "late timing" contention effects are emulated.

 2. Russian models (at least Pentagon and Scorpion) reportedly don't have contended memory.
    Information about screen timings are here: http://www.worldofspectrum.org/rusfaq/

 3. Similarly, contended I/O is not emulated: http://scratchpad.wikia.com/wiki/Contended_IO

 4. Finally, the Floating Bus implementation is incomplete: http://scratchpad.wikia.com/wiki/Floating_bus
    The page hosts a couple of testing programs.

 5. A NTSC model exists: it's not the Timex, but a plain ZX Spectrum made for the Chilean market:
    http://scratchpad.wikia.com/wiki/NTSC_Spectrum

 6. No hi-res colour effects (need contended memory first for accurate timing).

 7. Keyboard autorepeat is broken.

 8. Port decoding on all models needs to be checked and, in case, corrected.

 9. The actual Issue 2/3 effect is a bit more complicated than implemented:
    http://piters.tripod.com/cassport.htm for an explanation/test suite.

10. Tape formats not supported:
    .001, .CSW, .LTP, .PZX, .SPC, .STA, .ITM/.PAN, .TAP(Warajevo), .VOC, .ZXS, .ZXT

11. Disk formats not supported:
    .CPD, .DSK, .FDD, .FDI, .HDF, .IMG, .MDR, .MGT, .SCL, .TRD, .ZXD

12. 128K emulation is not perfect - the 128K machines crash and hang while
    running quite a lot of games.

13. Disk errors occur on some +3 games.

14. EXROM and HOME cartridges are not emulated on Timex machines.

15. The TK90X and TK95 roms output 0 to port #df on start up. The purpose of this port is unknown
    (probably display mode as TS2068) and thus is not emulated.

16. Very detailed infos about the ZX Spectrum +3e can be found at
    http://www.worldofspectrum.org/zxplus3e/

17. The ZX Spectrum is known to support a huge number of peripherals, daisy-chained through its expansion bus.
    In the earlier part of its life cycle, a common configuration consisted of a "docking station"-type
    peripheral providing drive controllers (the disk drives were not part of the packages), a game
    interface (joystick and/or sound chip) and the ZX Printer (which, if present, must terminate the chain).

    Later, all-in-one peripherals became more common. The following list is just a little part of the huge
    catalog of peripherals available to the ZX Spectrum; however these peripherals are both the most successful
    and the better documented ones.

    Except when noted, technical and usage manuals are available at http://www.worldofspectrum.org/

------------------------
Mass storage controllers
------------------------

ZX Interface 1
    up to 8 Microdrives
    1 RS-232 port
    2 ZX Network ports - http://scratchpad.wikia.com/wiki/ZX_Net

Opus Discovery
    up to 2 Disk drives
    1 Centronics port
    1 Joystick port (Kempston)

DISCiPLE Interface
    up to 2 Disk drives
    1 Centronics port
    2 Joystick ports (RH: Sinclair/Kempston, LH: Sinclair)
    2 ZX Network ports

Plus D Interface
    up to 2 Disk drives
    1 Centronics port

Rotronics Wafadrive
    2 "Stringy" drives
    1 RS-232 port
    1 Centronics port

Kempston Disc Interface
    up to 4 Disk drives (KDOS)

Beta 128 Disk Interface
    up to 4 Disk drives (TR-DOS)

Philips Disc ROM
    ???

----------------
Game controllers
----------------

ZX Interface 2
    2 Joystick ports (Sinclair)
    1 ROM slot

Kempston Interface
    1 Joystick port (Kempston)

Protek/AGF/Cursor Interface
    1 Joystick port (Protek)

Fuller Box
    1 Joystick port (Fuller)
    1 G1-AY-3-8912 sound chip - the chip is identical to the one present on the 128 model,
                                but the I/O ports are different: http://scratchpad.wikia.com/wiki/AY-3-8912(a) and
                                http://scratchpad.wikia.com/wiki/Timex_2000_series#Sound_Chip

Mikro-Plus Interface
    1 Joystick port (Mikrogen)
    1 ROM slot - http://www.worldofspectrum.org/showmag.cgi?mag=Crash/Issue19/Pages/Crash1900020.jpg

------------
Misc devices
------------

DK'Tronics Light Pen (identical to TK90X light pen)

AMX Mouse
    Schematics and test software at http://velesoft.speccy.cz/othermouse-cz.htm

Kempston Mouse
    Schematics and test software at http://8bit.yarek.pl/hardware/zx.mouse/

Currah uSpeech
    Speech synthesis through allophones (SP0256-AL2)

MGT Messenger
    Allows connection between ZX Spectrum and SAM Coup??

Soft-ROM
    A development system board - http://www.wearmouth.demon.co.uk/softrom.htm

Multiface 1/128/+3
    The classic snapshot interface for ZX Spectrum, 128 and +3 models, with 8K internal RAM.

SamRam
    A more versatile "Multiface I"-type interface - http://8bit.yarek.pl/upgrade/zx.samram/samram.html


*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/wave.h"
#include "includes/spectrum.h"
#include "formats/tzx_cas.h"
#include "machine/spec_snqk.h"
#include "softlist.h"

/****************************************************************************************************/
/* Spectrum 48k functions */

/*
 bit 7-5: not used
 bit 4: Ear output/Speaker
 bit 3: MIC/Tape Output
 bit 2-0: border colour
*/

WRITE8_MEMBER(spectrum_state::spectrum_port_fe_w)
{
	unsigned char Changed;

	Changed = m_port_fe_data^data;

	/* border colour changed? */
	if ((Changed & 0x07)!=0)
	{
		spectrum_UpdateBorderBitmap();
	}

	if ((Changed & (1<<4))!=0)
	{
		/* DAC output state */
		m_speaker->level_w(BIT(data, 4));
	}

	if ((Changed & (1<<3))!=0)
	{
		/* write cassette data */
		m_cassette->output((data & (1<<3)) ? -1.0 : +1.0);
	}

	m_port_fe_data = data;
}



/* KT: more accurate keyboard reading */
/* DJR: Spectrum+ keys added */
READ8_MEMBER(spectrum_state::spectrum_port_fe_r)
{
	int lines = offset >> 8;
	int data = 0xff;

	int cs_extra1 = m_io_plus0 ? m_io_plus0->read() & 0x1f : 0x1f;
	int cs_extra2 = m_io_plus1 ? m_io_plus1->read() & 0x1f : 0x1f;
	int cs_extra3 = m_io_plus2 ? m_io_plus2->read() & 0x1f : 0x1f;
	int ss_extra1 = m_io_plus3 ? m_io_plus3->read() & 0x1f : 0x1f;
	int ss_extra2 = m_io_plus4 ? m_io_plus4->read() & 0x1f : 0x1f;

	/* Caps - V */
	if ((lines & 1) == 0)
	{
		data &= m_io_line0->read();
		/* CAPS for extra keys */
		if (cs_extra1 != 0x1f || cs_extra2 != 0x1f || cs_extra3 != 0x1f)
			data &= ~0x01;
	}

	/* A - G */
	if ((lines & 2) == 0)
		data &= m_io_line1->read();

	/* Q - T */
	if ((lines & 4) == 0)
		data &= m_io_line2->read();

	/* 1 - 5 */
	if ((lines & 8) == 0)
		data &= m_io_line3->read() & cs_extra1;

	/* 6 - 0 */
	if ((lines & 16) == 0)
		data &= m_io_line4->read() & cs_extra2;

	/* Y - P */
	if ((lines & 32) == 0)
		data &= m_io_line5->read() & ss_extra1;

	/* H - Enter */
	if ((lines & 64) == 0)
		data &= m_io_line6->read();

		/* B - Space */
	if ((lines & 128) == 0)
	{
		data &= m_io_line7->read() & cs_extra3 & ss_extra2;
		/* SYMBOL SHIFT for extra keys */
		if (ss_extra1 != 0x1f || ss_extra2 != 0x1f)
			data &= ~0x02;
	}

	data |= (0xe0); /* Set bits 5-7 - as reset above */

	/* cassette input from wav */
	if (m_cassette->input() > 0.0038 )
	{
		data &= ~0x40;
	}

	/* Issue 2 Spectrums default to having bits 5, 6 & 7 set.
	Issue 3 Spectrums default to having bits 5 & 7 set and bit 6 reset. */
	if (m_io_config->read() & 0x80)
		data ^= (0x40);

	return data;
}

/* kempston joystick interface */
READ8_MEMBER(spectrum_state::spectrum_port_1f_r)
{
	return m_io_kempston->read() & 0x1f;
}

/* fuller joystick interface */
READ8_MEMBER(spectrum_state::spectrum_port_7f_r)
{
	return m_io_fuller->read() | (0xff^0x8f);
}

/* mikrogen joystick interface */
READ8_MEMBER(spectrum_state::spectrum_port_df_r)
{
	return m_io_mikrogen->read() | (0xff^0x1f);
}

READ8_MEMBER(spectrum_state::spectrum_port_ula_r)
{
	int vpos = machine().first_screen()->vpos();

	return vpos<193 ? m_video_ram[(vpos&0xf8)<<2]:0xff;
}

/* Memory Maps */

static ADDRESS_MAP_START (spectrum_mem, AS_PROGRAM, 8, spectrum_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x5aff) AM_RAM AM_SHARE("video_ram")
//  AM_RANGE(0x5b00, 0x7fff) AM_RAM
//  AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

/* ports are not decoded full.
The function decodes the ports appropriately */
static ADDRESS_MAP_START (spectrum_io, AS_IO, 8, spectrum_state )
	AM_RANGE(0x00, 0x00) AM_READWRITE(spectrum_port_fe_r,spectrum_port_fe_w) AM_MIRROR(0xfffe) AM_MASK(0xffff)
	AM_RANGE(0x1f, 0x1f) AM_READ(spectrum_port_1f_r) AM_MIRROR(0xff00)
	AM_RANGE(0x7f, 0x7f) AM_READ(spectrum_port_7f_r) AM_MIRROR(0xff00)
	AM_RANGE(0xdf, 0xdf) AM_READ(spectrum_port_df_r) AM_MIRROR(0xff00)
	AM_RANGE(0x01, 0x01) AM_READ(spectrum_port_ula_r) AM_MIRROR(0xfffe)
ADDRESS_MAP_END

/* Input ports */

/****************************************************************************************************/

static INPUT_PORTS_START( spec_joys )
	PORT_START("JOY_INTF")
	PORT_CONFNAME( 0x0f, 0x01, "Joystick Interface")
	PORT_CONFSETTING(  0x00, "None (No Joystick)" )
	PORT_CONFSETTING(  0x01, "Kempston" )
	PORT_CONFSETTING(  0x02, "Fuller" )
	PORT_CONFSETTING(  0x03, "Mikrogen" )

	PORT_START("KEMPSTON") /* Kempston joystick interface */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Right (Kempston)") PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Left (Kempston)") PORT_CODE(JOYCODE_X_LEFT_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Down (Kempston)") PORT_CODE(JOYCODE_Y_DOWN_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Up (Kempston)") PORT_CODE(JOYCODE_Y_UP_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Fire (Kempston)") PORT_CODE(JOYCODE_BUTTON1) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x01)

	PORT_START("FULLER") /* Fuller joystick interface */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Up (Fuller)") PORT_CODE(JOYCODE_Y_UP_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x02)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Down (Fuller)") PORT_CODE(JOYCODE_Y_DOWN_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x02)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Left (Fuller)") PORT_CODE(JOYCODE_X_LEFT_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x02)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Right (Fuller)") PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x02)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Fire (Fuller)") PORT_CODE(JOYCODE_BUTTON1) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x02)

	PORT_START("MIKROGEN") /* Mikrogen joystick interface */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Up (Mikrogen)") PORT_CODE(JOYCODE_Y_UP_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x03)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Down (Mikrogen)") PORT_CODE(JOYCODE_Y_DOWN_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x03)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Right (Mikrogen)") PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x03)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Left (Mikrogen)") PORT_CODE(JOYCODE_X_LEFT_SWITCH) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x03)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P1 Joystick Fire (Mikrogen)") PORT_CODE(JOYCODE_BUTTON1) PORT_CONDITION("JOY_INTF", 0x0f, EQUALS, 0x03)
INPUT_PORTS_END

/*
Spectrum keyboard is quite complicate to emulate. Each key can have 5 or 6 different functions, depending on which input mode we are in:

-------------------------------------------------------------------------------------------------------------------
    Mode           |  Sequence on Spectrum              |  Sequence in MESS
-------------------------------------------------------------------------------------------------------------------
    KEY Mode       |  Simply press the key              |  Simply press the key
    CAPS Mode      |  Press Key + CAPS SHIFT            |  Press Key + LShift (default mapping)
    SYMBOL Mode    |  Press Key + SYMBOL SHIFT          |  Press Key + RShift (default mapping)
    EXT Mode       |  Press CAPS + SYMBOL once,         |  Press LShift + RShift (to enter EXT Mode),
                   |     then press Key                 |     then press Key
    EXT+Shift Mode |  In EXT Mode, press Key + SHIFT    |  In EXT Mode, press Key + LShift or Key + RShift
                   |    (no matter if CAPS and SYMBOL)  |
    BASIC Mode     |  At BASIC Prompt, press the key    |  At BASIC Prompt, press the key
-------------------------------------------------------------------------------------------------------------------

Number Keys are the only keys not having a function in BASIC Mode (hence, they only have 5 functions)

2009-04: Added natural keyboard support. The commented out PORT_CHARs are not reachable in natural keyboard mode (they are entered
in EXT+Shift Mode on a real Spectrum).
*/

/* TO DO: replace PORT_CHAR('\xD7') with an 'empty' PORT_CHAR. I used \xD7 (multiplication sign) just as a placeholder. There should be no
PORT_CHAR for those functions (which have no equivalent on modern keyboards), but something is needed (to correctly have natural support for
a few keys in SYMBOL Mode) and I found no EMPTY PORT_CHAR in MESS */
INPUT_PORTS_START( spectrum )
	/* PORT_NAME =  KEY Mode    CAPS Mode    SYMBOL Mode   EXT Mode   EXT+Shift Mode   BASIC Mode  */
	PORT_START("LINE0") /* 0xFEFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("z    Z    :      LN       BEEP   COPY") PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("x    X    \xC2\xA3   EXP      INK    CLEAR") PORT_CODE(KEYCODE_X)    PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR('\xA3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("c    C    ?      LPRINT   PAPER  CONT") PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("v    V    /      LLIST    FLASH  CLS") PORT_CODE(KEYCODE_V)      PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR('/')

	PORT_START("LINE1") /* 0xFDFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("a    A    STOP   READ      ~     NEW") PORT_CODE(KEYCODE_A)      PORT_CHAR('a') PORT_CHAR('A')// PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("s    S    NOT    RESTORE   |     SAVE") PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')// PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("d    D    STEP   DATA      \\    DIM") PORT_CODE(KEYCODE_D)      PORT_CHAR('d') PORT_CHAR('D')// PORT_CHAR('\\')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("f    F    TO     SGN       {     FOR") PORT_CODE(KEYCODE_F)      PORT_CHAR('f') PORT_CHAR('F')// PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("g    G    THEN   ABS       }     GOTO") PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')// PORT_CHAR('}')

	PORT_START("LINE2") /* 0xFBFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("q    Q    <=     SIN      ASN      PLOT") PORT_CODE(KEYCODE_Q)   PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("w    W    <>     COS      ACS      DRAW") PORT_CODE(KEYCODE_W)   PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("e    E    >=     TAN      ATN      REM") PORT_CODE(KEYCODE_E)    PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("r    R    <      INT      VERIFY   RUN") PORT_CODE(KEYCODE_R)    PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('<')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("t    T    >      RND      MERGE    RAND") PORT_CODE(KEYCODE_T)   PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR('>')

	/* interface II uses this port for joystick */
	PORT_START("LINE3") /* 0xF7FE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1   EDIT       !    BLUE     DEF FN") PORT_CODE(KEYCODE_1)   PORT_CHAR('1') PORT_CHAR('\xD7') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2   CAPS LOCK  @    RED      FN") PORT_CODE(KEYCODE_2)       PORT_CHAR('2') PORT_CHAR('\xD7') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3   TRUE VID   #    MAGENTA  LINE") PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('\xD7') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4   INV VID    $    GREEN    OPEN#") PORT_CODE(KEYCODE_4)    PORT_CHAR('4') PORT_CHAR('\xD7') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5   Left       %    CYAN     CLOSE#") PORT_CODE(KEYCODE_5)   PORT_CHAR('5') PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR('%')

	/* protek clashes with interface II! uses 5 = left, 6 = down, 7 = up, 8 = right, 0 = fire */
	PORT_START("LINE4") /* 0xEFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0   DEL        _    BLACK    FORMAT") PORT_CODE(KEYCODE_0)   PORT_CHAR('0') PORT_CHAR(8) PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9   GRAPH      )             POINT") PORT_CODE(KEYCODE_9)    PORT_CHAR('9') PORT_CHAR('\xD7') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8   Right      (             CAT") PORT_CODE(KEYCODE_8)      PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7   Up         '    WHITE    ERASE") PORT_CODE(KEYCODE_7)    PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6   Down       &    YELLOW   MOVE") PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR('&')

	PORT_START("LINE5") /* 0xDFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("p    P    \"     TAB      (c)    PRINT") PORT_CODE(KEYCODE_P)    PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('"')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("o    O    ;      PEEK     OUT    POKE") PORT_CODE(KEYCODE_O)     PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("i    I    AT     CODE     IN     INPUT") PORT_CODE(KEYCODE_I)    PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("u    U    OR     CHR$     ]      IF") PORT_CODE(KEYCODE_U)       PORT_CHAR('u') PORT_CHAR('U')// PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("y    Y    AND    STR$     [      RETURN") PORT_CODE(KEYCODE_Y)   PORT_CHAR('y') PORT_CHAR('Y')// PORT_CHAR('[')

	PORT_START("LINE6") /* 0xBFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("l    L    =      USR      ATTR     LET") PORT_CODE(KEYCODE_L)    PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("k    K    +      LEN      SCREEN$  LIST") PORT_CODE(KEYCODE_K)   PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("j    J    -      VAL      VAL$     LOAD") PORT_CODE(KEYCODE_J)   PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("h    H    ^      SQR      CIRCLE   GOSUB") PORT_CODE(KEYCODE_H)  PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR('^')

	PORT_START("LINE7") /* 0x7FFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SYMBOL SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("m    M    .      PI       INVERSE  PAUSE") PORT_CODE(KEYCODE_M)  PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR('.')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("n    N    ,      INKEY$   OVER     NEXT") PORT_CODE(KEYCODE_N)   PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("b    B    *      BIN      BRIGHT   BORDER") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR('*')

	PORT_START("NMI")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NMI") PORT_CODE(KEYCODE_F12)

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x80, 0x00, "Hardware Version" )
	PORT_CONFSETTING(   0x00, "Issue 2" )
	PORT_CONFSETTING(   0x80, "Issue 3" )
	PORT_BIT(0x7f, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_INCLUDE( spec_joys )
INPUT_PORTS_END

/* These keys need not to be mapped in natural mode because Spectrum+ supports both these and the Spectrum sequences above.
   Hence, we can simply keep using such sequences in natural keyboard emulation */
INPUT_PORTS_START( spec_plus )
	PORT_INCLUDE( spectrum )

	PORT_START("PLUS0") /* Spectrum+ Keys (Same as CAPS + 1-5) */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EDIT") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TRUE VID") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INV VID") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PLUS1") /* Spectrum+ Keys (Same as CAPS + 6-0) */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PLUS2") /* Spectrum+ Keys (Same as CAPS + SPACE and CAPS + SYMBOL) */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EXT MODE") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PLUS3") /* Spectrum+ Keys (Same as SYMBOL SHIFT + O/P) */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\"") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PLUS4") /* Spectrum+ Keys (Same as SYMBOL SHIFT + N/M) */
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0xf3, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

/* Machine initialization */

DRIVER_INIT_MEMBER(spectrum_state,spectrum)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	switch (m_ram->size())
	{
		case 48*1024:
		space.install_ram(0x8000, 0xffff, nullptr); // Fall through
		case 16*1024:
		space.install_ram(0x5b00, 0x7fff, nullptr);
	}
}

MACHINE_RESET_MEMBER(spectrum_state,spectrum)
{
	m_port_7ffd_data = -1;
	m_port_1ffd_data = -1;

	if (m_cart && m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0x3fff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_cart));
}

/* F4 Character Displayer */
static const gfx_layout spectrum_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	96,                 /* 96 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( spectrum )
	GFXDECODE_ENTRY( "maincpu", 0x3d00, spectrum_charlayout, 0, 8 )
GFXDECODE_END

void spectrum_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(spectrum_state::spec_interrupt)
{
	m_maincpu->set_input_line(0, HOLD_LINE);
	timer_set(attotime::from_ticks(32, m_maincpu->clock()), 0, 0);
}

DEVICE_IMAGE_LOAD_MEMBER(spectrum_state, spectrum_cart)
{
	UINT32 size = m_cart->common_get_size("rom");

	if (size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

MACHINE_CONFIG_START( spectrum_common, spectrum_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, X1 / 4)        /* This is verified only for the ZX Spectum. Other clones are reported to have different clocks */
	MCFG_CPU_PROGRAM_MAP(spectrum_mem)
	MCFG_CPU_IO_MAP(spectrum_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spectrum_state,  spec_interrupt)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_RESET_OVERRIDE(spectrum_state, spectrum )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)

	MCFG_SCREEN_RAW_PARAMS(X1 / 2, 448, 0, 352,  312, 0, 296)

	MCFG_SCREEN_UPDATE_DRIVER(spectrum_state, screen_update_spectrum)
	MCFG_SCREEN_VBLANK_DRIVER(spectrum_state, screen_eof_spectrum)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(spectrum_state, spectrum )

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", spectrum)
	MCFG_VIDEO_START_OVERRIDE(spectrum_state, spectrum )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_SNAPSHOT_ADD("snapshot", spectrum_state, spectrum, "ach,frz,plusd,prg,sem,sit,sna,snp,snx,sp,z80,zx", 0)
	MCFG_QUICKLOAD_ADD("quickload", spectrum_state, spectrum, "raw,scr", 2) // The delay prevents the screen from being cleared by the RAM test at boot
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(tzx_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)
	MCFG_CASSETTE_INTERFACE("spectrum_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list", "spectrum_cass")

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "spectrum_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(spectrum_state, spectrum_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list", "spectrum_cart")
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( spectrum, spectrum_common )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)               // This configuration is verified only for the original ZX Spectrum.
	MCFG_RAM_DEFAULT_SIZE("48K")        // It's likely, but still to be checked, that many clones were produced only
	MCFG_RAM_EXTRA_OPTIONS("16K")       // in the 48k configuration, while others have extra memory (80k, 128K, 1024K)
	MCFG_RAM_DEFAULT_VALUE(0xff)        // available via bankswitching.
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(spectrum)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROMX_LOAD("spectrum.rom", 0x0000, 0x4000, CRC(ddee531f) SHA1(5ea7c2b824672e914525d1d5c419d71b84a426a2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "sp", "Spanish")
	ROMX_LOAD("48e.rom", 0x0000, 0x4000, CRC(f051746e) SHA1(9e535e2e24231ccb65e33d107f6d0ceb23e99477), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "bsrom118", "BusySoft Upgrade v1.18")
	ROMX_LOAD("bsrom118.rom", 0x0000, 0x4000, CRC(1511cddb) SHA1(ab3c36daad4325c1d3b907b6dc9a14af483d14ec), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "bsrom140", "BusySoft Upgrade v1.40")
	ROMX_LOAD("bsrom140.rom", 0x0000, 0x4000, CRC(07017c6d) SHA1(2ee2dbe6ab96b60d7af1d6cb763b299374c21776), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "groot", "De Groot's Upgrade")
	ROMX_LOAD("groot.rom", 0x0000, 0x4000, CRC(abf18c45) SHA1(51165cde68e218512d3145467074bc7e786bf307), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(5, "48turbo", "48 Turbo")
	ROMX_LOAD("48turbo.rom", 0x0000, 0x4000, CRC(56189781) SHA1(e62a431b0938af414b7ab8b1349a18b3c4407f70), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(6, "psycho", "Maly's Psycho Upgrade")
	ROMX_LOAD("psycho.rom", 0x0000, 0x4000, CRC(cd60b589) SHA1(0853e25857d51dd41b20a6dbc8e80f028c5befaa), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(7, "turbo23", "Turbo 2.3")
	ROMX_LOAD("turbo2_3.rom", 0x0000, 0x4000, CRC(fd3b0413) SHA1(84ea64af06adaf05e68abe1d69454b4fc6888505), ROM_BIOS(8))
	ROM_SYSTEM_BIOS(8, "turbo44", "Turbo 4.4")
	ROMX_LOAD("turbo4_4.rom", 0x0000, 0x4000, CRC(338b6e87) SHA1(21ad93ffe41a4458704c866cca2754f066f6a560), ROM_BIOS(9))
	ROM_SYSTEM_BIOS(9, "imc", "Ian Collier's Upgrade")
	ROMX_LOAD("imc.rom", 0x0000, 0x4000, CRC(d1be99ee) SHA1(dee814271c4d51de257d88128acdb324fb1d1d0d), ROM_BIOS(10))
	ROM_SYSTEM_BIOS(10, "plus4", "ZX Spectrum +4")
	ROMX_LOAD("plus4.rom",0x0000,0x4000, CRC(7e0f47cb) SHA1(c103e89ef58e6ade0c01cea0247b332623bd9a30), ROM_BIOS(11))
	ROM_SYSTEM_BIOS(11, "deutsch", "German unofficial (Andrew Owen)")
	ROMX_LOAD("deutsch.rom",0x0000,0x4000, CRC(1a9190f4) SHA1(795c20324311dd5a56300e6e4ec49b0a694ac0b3), ROM_BIOS(12))
	ROM_SYSTEM_BIOS(12, "hdt", "HDT-ISO HEX-DEC-TEXT")
	ROMX_LOAD("hdt-iso.rom",0x0000,0x4000, CRC(b81c570c) SHA1(2a9745ba3b369a84c4913c98ede66ec87cb8aec1), ROM_BIOS(13))
	ROM_SYSTEM_BIOS(13, "sc", "The Sea Change Minimal ROM V1.78")
	ROMX_LOAD("sc01.rom",0x0000,0x4000, CRC(73b4057a) SHA1(c58ff44a28db47400f09ed362ca0527591218136), ROM_BIOS(14))
	ROM_SYSTEM_BIOS(14, "gosh", "GOSH Wonderful ZX Spectrum ROM V1.32")
	ROMX_LOAD("gw03.rom",0x0000,0x4000, CRC(5585d7c2) SHA1(a701c3d4b698f7d2be537dc6f79e06e4dbc95929), ROM_BIOS(15))
	ROM_SYSTEM_BIOS(15, "1986es", "1986ES Snapshot")
	ROMX_LOAD("1986es.rom",0x0000,0x4000, CRC(9e0fdaaa) SHA1(f9d23f25640c51bcaa63e21ed5dd66bb2d5f63d4), ROM_BIOS(16))
	ROM_SYSTEM_BIOS(16, "jgh", "JGH ROM V0.74 (C) J.G.Harston")
	ROMX_LOAD("jgh.rom",0x0000,0x4000, CRC(7224138e) SHA1(d7f02ed66455f1c08ac0c864c7038a92a88ba94a), ROM_BIOS(17))
	ROM_SYSTEM_BIOS(17, "iso22", "ISO ROM V2.2")
	ROMX_LOAD("isomoje.rom",0x0000,0x4000, CRC(62ab3640) SHA1(04adbdb1380d6ccd4ab26ddd61b9ccbba462a60f), ROM_BIOS(18))
	ROM_SYSTEM_BIOS(18, "iso8", "ISO ROM 8")
	ROMX_LOAD("iso8bm.rom",0x0000,0x4000, CRC(43e9c2fd) SHA1(5752e6f789769475711b91e0a75911fa5232c767), ROM_BIOS(19))
ROM_END

ROM_START(specide)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("zxide.rom", 0x0000, 0x4000, CRC(bd48db54) SHA1(54c2aa958902b5395c260770a0b25c7ba5685de9))
ROM_END

ROM_START(spec80k)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("80-lec.rom", 0x0000, 0x4000, CRC(5b5c92b1) SHA1(bb7a77d66e95d2e28ebb610e543c065e0d428619))
ROM_END

ROM_START(tk90x)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("tk90x.rom",0x0000,0x4000, CRC(3e785f6f) SHA1(9a943a008be13194fb006bddffa7d22d2277813f))
ROM_END

ROM_START(tk95)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("tk95.rom",0x0000,0x4000, CRC(17368e07) SHA1(94edc401d43b0e9a9cdc1d35de4b6462dc414ab3))
ROM_END

ROM_START(inves)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("inves.rom",0x0000,0x4000, CRC(8ff7a4d1) SHA1(d020440638aff4d39467128413ef795677be9c23))
ROM_END

/* Romanian clones */
ROM_START(hc85)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("hc85.rom",0x0000,0x4000, CRC(3ab60fb5) SHA1(a4189db0bcdf8b39ed782b398828efb408fc4817))
ROM_END

ROM_START( hc88 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "hc88.bin", 0x0000, 0x0800, CRC(33be5134) SHA1(b15a6e7085710de8b818e42d329707cb737627e3))
ROM_END

ROM_START(hc90)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("hc90.rom",0x0000,0x4000, CRC(78c14d9a) SHA1(25ef81905bed90497a749770170c24632efb2039))
ROM_END

ROM_START(hc91)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("hc91.rom",0x0000,0x4000, CRC(8bf53761) SHA1(967d5179ba2823e9c8dd9ddfb0430465aaddb554))
ROM_END

ROM_START(cip03)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("cip03.rom",0x0000,0x4000, CRC(c7d0cd3c) SHA1(811055b44fc74076137e2bf8db206b2a70287cc2))
ROM_END

ROM_START(cip01)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("cip01.rom",0x0000,0x4000, CRC(0516a329) SHA1(4e3e0c5719a64d3b4fb224db499b4bef7d146917))
ROM_END

ROM_START(jet)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("jet.rom",0x0000,0x4000, CRC(e56a7d11) SHA1(e76be9ee71bae6aa1c2ff969276fb599ed68cb50))
ROM_END

ROM_START( cobrasp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v1", "V1")
	ROMX_LOAD( "boot64k_v1.bin", 0x0000, 0x0800, CRC(a54aae6d) SHA1(8f5134ce24aea59065ed166ad79e864e17ce812f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2", "V2")
	ROMX_LOAD( "boot64k_v2.bin", 0x0000, 0x0800, CRC(ee91cc89) SHA1(37dea7fe0734068adf99b91fdcbf3119095c350d), ROM_BIOS(2))
ROM_END

ROM_START( cobra80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v1", "V1")
	ROMX_LOAD( "boot80k_v1.bin", 0x0000, 0x0800, CRC(f42d2342) SHA1(8aa1b3b056e311674a051ffc6a49af60cae409f3), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2", "V2")
	ROMX_LOAD( "boot80k_v2.bin", 0x0000, 0x0800, CRC(df6bd954) SHA1(5b858b59e697d0368ea631ead14f5b2aa7954ccd), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v3", "V3")
	ROMX_LOAD( "boot80k_v3.bin", 0x0000, 0x0800, CRC(8580494c) SHA1(91af3f3fa50f2071f8ff081536bdf7e21e9823d9), ROM_BIOS(3))
ROM_END

/* Czechoslovakian clones*/

/* TODO: need to add memory handling for 80K RAM */

ROM_START(dgama87)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("dgama87.rom",0x0000,0x4000, CRC(43104909) SHA1(f62d1f3f35fda467cae468e890995614f6ec2357))
ROM_END

ROM_START(dgama88)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("dgama88.rom",0x0000,0x4000, CRC(4ec7e078) SHA1(09a91f85e82efa7f974d1b88c69636a02063d563))
ROM_END

ROM_START(dgama89)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_SYSTEM_BIOS(0, "default", "Original")
	ROMX_LOAD("dgama89.rom",0x0000,0x4000, CRC(45c29401) SHA1(8466a9da0169666210ccff5d43376d70bae0ae9b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "g81", "Gama 81")
	ROMX_LOAD("g81.rom",0x0000,0x4000, CRC(c169a63b) SHA1(71652005c2e7a4301caa7e95ae989b69cb5a6a0d), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "iso", "ISO")
	ROMX_LOAD("iso.rom",0x0000,0x4000, CRC(2ee3a992) SHA1(2e39995dd032036d33a6dd88a38b750057bca19d), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(3, "isopolak", "ISO Polak")
	ROMX_LOAD("isopolak.rom",0x0000,0x4000, CRC(5e3f1f66) SHA1(61713117c944fc6afcb96c647bdba5ad36fd6a4b), ROM_BIOS(4))
ROM_END

ROM_START(didakt90)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("didakt90.rom",0x0000,0x4000, CRC(76f2db1e) SHA1(daee355a8ee58bc406873c1dd81eecb6161dd4bd))
ROM_END

ROM_START(didakm91)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("didakm91.rom",0x0000,0x4000, CRC(beab69b8) SHA1(71d4d1a05fb936f616bcb05c3a276f79343ecd4d))
ROM_END

ROM_START(didakm92)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("didakm92.bin",0x0000,0x4000, CRC(57264d4f) SHA1(23644fe949cbf527747959d07b72db01de378c4c))
ROM_END

ROM_START(didaktk)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("didaktk.rom",0x0000,0x4000, CRC(8ec8a625) SHA1(cba35517d33a5c97e3d9110f12a417c6c5cdeca8))
ROM_END

ROM_START(didakm93)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("didakm93.rom",0x0000,0x4000, CRC(ec274b1b) SHA1(a3470d8d1a996ee2a1ffff8bd8044da6e907e07e))
ROM_END

ROM_START(mistrum)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("mistrum.rom",0x0000,0x4000, CRC(d496103e) SHA1(cca1c5b059dc3a29ca4282e8621e34a65efaa1a3))
ROM_END

/* Russian clones */

ROM_START(blitzs)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("blitz.rom",0x0000,0x4000, CRC(91e535a8) SHA1(14f09d45dc3803cbdb05c33adb28eb12dbad9dd0))
ROM_END

ROM_START(byte)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("byte.rom",0x0000,0x4000, CRC(c13ba473) SHA1(99f40727185abbb2413f218d69df021ae2e99e45))
ROM_END

ROM_START(orizon)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("orizon.rom",0x0000,0x4000, CRC(ed4d9787) SHA1(3e8b29862e06be03344393c320a64a109fd9aff5))
ROM_END

ROM_START(quorum48)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("quorum48.rom",0x0000,0x4000, CRC(48085b0e) SHA1(8e01581643f7bdfa773f68207a6437911b631e53))
ROM_END

ROM_START(magic6)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("magic6.rom",0x0000,0x4000, CRC(cb63ae06) SHA1(533ad1f50534e6bdeec50eb5a9a4976c3d010dc7))
ROM_END

ROM_START(compani1)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("compani1.rom",0x0000,0x4000, CRC(bcfa6068) SHA1(40074b55c91a947698598e9d6ac5b8495e8cc840))
ROM_END

ROM_START(spektrbk)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("spektr-bk001.rom", 0x0000, 0x4000, CRC(c011eecc) SHA1(35fdc8cd083e50452655997a997873627b131520))
ROM_END

ROM_START(zvezda)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD( "2764-near-cpu_red.bin", 0x0000, 0x2000, CRC(a4ae4938) SHA1(ea1763b9dee29381ddcf882fbc4e404ba5366942))
	ROM_LOAD( "2764-far-cpu_blue.bin", 0x2000, 0x2000, CRC(ebab64bc) SHA1(8c98a8b6e927b02cf602c20a1b50838e60f7785b))
ROM_END

/*    YEAR  NAME      PARENT    COMPAT  MACHINE     INPUT       INIT    COMPANY     FULLNAME */
COMP( 1982, spectrum, 0,        0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Sinclair Research Ltd",    "ZX Spectrum" , 0)
COMP( 1987, spec80k,  spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "ZX Spectrum 80K" , MACHINE_UNOFFICIAL)
COMP( 1995, specide,  spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "ZX Spectrum IDE" , MACHINE_UNOFFICIAL)
COMP( 1986, inves,    spectrum, 0,      spectrum,       spec_plus, spectrum_state,  spectrum,   "Investronica", "Inves Spectrum 48K+" , 0)
COMP( 1985, tk90x,    spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Micro Digital",    "TK 90X Color Computer" , 0)
COMP( 1986, tk95,     spectrum, 0,      spectrum,       spec_plus, spectrum_state,  spectrum,   "Micro Digital",    "TK 95 Color Computer" , 0)
COMP( 1985, hc85,     spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "ICE-Felix",    "HC-85" , 0)
COMP( 1988, hc88,     spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "ICE-Felix",    "HC-88" , MACHINE_NOT_WORKING)
COMP( 1990, hc90,     spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "ICE-Felix",    "HC-90" , 0)
COMP( 1991, hc91,     spectrum, 0,      spectrum,       spec_plus, spectrum_state,  spectrum,   "ICE-Felix",    "HC-91" , 0)
COMP( 1988, cobrasp,   spectrum, 0,     spectrum,       spectrum, spectrum_state,   spectrum,   "ITCI", "Cobra (ITCI)" , MACHINE_NOT_WORKING)
COMP( 1988, cobra80,  spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "ITCI", "Cobra 80K (ITCI)" , MACHINE_NOT_WORKING)
COMP( 1987, cip01,    spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Electronica",  "CIP-01" , 0)   // keyboard should be spectrum, but image was not clear
COMP( 1988, cip03,    spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Electronica",  "CIP-03" , 0)   // keyboard should be spectrum, but image was not clear
COMP( 1990, jet,      spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Electromagnetica", "JET" , 0)  // keyboard should be spectrum, but image was not clear
COMP( 1987, dgama87,  spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Didaktik Skalica", "Didaktik Gama 87" , 0)
COMP( 1988, dgama88,  spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Didaktik Skalica", "Didaktik Gama 88" , 0)
COMP( 1989, dgama89,  spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Didaktik Skalica", "Didaktik Gama 89" , 0)
COMP( 1990, didakt90, spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Didaktik Skalica", "Didaktik 90" , 0)
COMP( 1991, didakm91, spectrum, 0,      spectrum,       spec_plus, spectrum_state,  spectrum,   "Didaktik Skalica", "Didaktik M 91" , 0)
COMP( 1992, didakm92, spectrum, 0,      spectrum,       spec_plus, spectrum_state,  spectrum,   "Didaktik Skalica", "Didaktik M 92" , 0)
COMP( 1992, didaktk,  spectrum, 0,      spectrum,       spec_plus, spectrum_state,  spectrum,   "Didaktik Skalica", "Didaktik Kompakt" , 0)
COMP( 1993, didakm93, spectrum, 0,      spectrum,       spec_plus, spectrum_state,  spectrum,   "Didaktik Skalica", "Didaktik M 93" , 0)
COMP( 1988, mistrum,  spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "Amaterske RADIO",  "Mistrum" , 0)  // keyboard could be spectrum in some models (since it was a build-yourself design)
COMP( 1990, blitzs,   spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Blic" , 0)     // no keyboard images found
COMP( 1990, byte,     spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Byte" , 0)     // no keyboard images found
COMP( 199?, orizon,   spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Orizon-Micro" , 0)     // no keyboard images found
COMP( 1993, quorum48, spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Kvorum 48K" , MACHINE_NOT_WORKING)
COMP( 1993, magic6,   spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Magic 6" , MACHINE_NOT_WORKING)   // keyboard should be spectrum, but image was not clear
COMP( 1990, compani1, spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Kompanion 1" , 0)      // no keyboard images found
COMP( 1990, spektrbk, spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Spektr BK-001" , 0)
COMP( 1990, zvezda,   spectrum, 0,      spectrum,       spectrum, spectrum_state,   spectrum,   "<unknown>",    "Zvezda" , 0)
