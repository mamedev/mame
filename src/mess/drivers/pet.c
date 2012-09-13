/***************************************************************************
    commodore pet series computer

    PeT mess@utanet.at

    documentation
     vice emulator
     www.funet.fi
     andre fachat (vice emulator, docu, web site, excellent keyboard pictures)

***************************************************************************/

/*

2008 - Driver Updates
---------------------

(most of the informations are taken from http://www.zimmers.net/cbmpics/ )
(also, check
    http://www.6502.org/users/andre/petindex/local/cbm-model-list.1.0.txt
for a more comprehensive list of models)

[CBM systems which belong to this driver]

* Commodore PET 2001 (1977) - board design 1
* PET 2001 Series / CBM 20XX Series (1979) - board design 2, upgraded ROMs

  PET 2001, the first of the series came initially with a calculator style
keyboard, later changed to a larger and better full-stroke one (actually
it was available in two versions: Businness and Home Computer, the latter
also containing graphics on the keys). Plenty of models were released in
this series, depending on the RAM size, the keyboard type and the screen
dimension (see below).

CPU: MOS 6502 (1 MHz)
RAM: 4K and 8K early models (Expandable to 32k addressable, more banked);
    8K, 16K, 32K later models (Expandable again to larger RAM config)
ROM: 18 kilobytes early models; 20 kilobytes later ones
Video: MOS Technology 6545 CRTC (9" Monochrome display; 40 columns x 25
    rows )
Sound: None ine arlier models; Piezo electronic speaker later ones(One
    square wave voice; Three octaves)
Ports: MOS 6520 PIA, MOS 6522 VIA (IEEE-488 edge-connector Port; 2
    Commodore Datasette ports; 'EXPANSION' port; CBM parallel
    programmable "User" port
Keyboard: "Calculator" 69 key QWERTY in early models and Full 69 keys QWERTY
    in later ones (16 key numeric keypad!; 4 direction 2-key cursor-pad )
Additional hardware: Datasette

Model Table (*):

PET 2001-4K     Calculator Keyboard, Black Trim
PET 2001-8K     Calculator Keyboard, Blue Trim, 9" Screen
PET 2001-8C     Calculator Keyboard, Blue Trim, 9" Screen B&W (**)
PET 2001B-8     Business Keyboard, 12" Screen
PET 2001B-16    Business Keyboard, 12" Screen
PET 2001B-32    Business Keyboard, 12" Screen (***)
PET 2001N-8     Home Computer Keyboard, 12" Screen
PET 2001N-16    Home Computer Keyboard, 9" and 12" Screen
PET 2001N-32    Home Computer Keyboard, BASIC 4.0

Notes:  B&W = Black and White Screen
        G&W = Green and White Screen
        Calculator Keyboard models also had a built-in datasette
        Home Computer Keyboard also features graphics on keys

(*) Models 2001B-XX & 2001-XXB are the same. Same applies to 2001N-XX &
    2001-XXN
(**) Also confirmed with Black Trim and 9" Screen G&W
(***) Also confirmed with Black Trim and 9" Screen B&W


* Commodore CBM 3008 / 3016 / 3032 (1979) - board design 2, upgraded ROMs

  In Europe, Philips was already selling a machine called PET, so they
forced Commodore to change name. The result was the CBM 30XX Series, where
XX is equal to the RAM size. These were basically PET models with 9" Screen
and upgraded ROMs. There also existed a German CBM 3001, which was a PET
2001, B or N. Notice that these systems used to boot

CPU: MOS 6502 (1MHz)
RAM: 8K, 16k, and 32k models (Expandable to even more banked memory)
ROM: 20 Kilobytes
Video: MOS Technology 6545 CRTC (9" Monochrome display; 40 columns x 25 rows)
Sound: Piezo electronic speaker (One square wave voice; Three octaves)
Ports: MOS 6520 PIA, MOS 6522 VIA (IEEE-488 edge-connector Port; 2 Commodore
    Datasette ports; 'EXPANSION' port; CBM parallel programmable "User" port)
Keyboard: Full 69 key QWERTY (16 key numeric keypad!; 4 direction 2-key cursor-pad)


* Commodore PET 40XX / CBM 40XX (1980) - board design 2, BASIC 4, 9" Screen
* Commodore PET 40XX / CBM 40XX (1981) - board design 3, BASIC 4, 12" Screen

  While the 40XX machines (again 4008, 4016, 4032) coming with design 2 were
quite similar to the previous ones (except for the new BASIC version), the
machines with design 3 were quite different: there was CRTC support, it was
possible to upgrade to 80 cols and a piezo beeper.

CPU: MOS 6502 (1 MHz)
RAM: 8K, 16k, and 32k models (Expandable to even more banked memory)
ROM: 20 Kilobytes
Video: MOS Technology 6545 CRTC (9" and 12" Monochrome displays; 40
    columns x 25 rows)
Sound: Piezo electronic speaker (One square wave voice; Three octaves)
Ports: MOS 6520 PIA, MOS 6522 VIA (IEEE-488 edge-connector Port; 2 Commodore
    Datasette ports; 'EXPANSION' port; CBM parallel programmable "User" port)
Keyboard: Full 69 key QWERTY (16 key numeric keypad!; 4 direction 2-key cursor-pad)


* Commodore PET 8032 / CBM 8032 (1981) - board design 3, BASIC 4, 12" Screen

  Again, 80 columns screen and piezo beeper; BASIC 4 has a 80 columns editor
ROM. It was possible to expand it to a 8096.

CPU: MOS 6502 (1 MHz)
RAM: 32k standard (Expandable to even more banked memory)
ROM: 20 Kilobytes
Video: MOS Technology 6545 CRTC (12" Monochrome displays; 80 columns x 25 rows)
Sound: Piezo electronic speaker (One square wave voice; Three octaves)
Ports: MOS 6520 PIA, MOS 6522 VIA (IEEE-488 edge-connector Port; 2 Commodore
    Datasette ports; 'EXPANSION' port; CBM parallel programmable "User" port)
Keyboard: Full 73 key QWERTY (11 key numeric keypad!; 4 direction 2-key cursor-pad)


* Commodore PET 8032-SK / CBM 8032-SK - board design 3 & 4, BASIC 4, 12" Screen

  New (smaller) case with detachable keyboard (SK=Separate Keyboard). Board
with design 3 didn't fit very well the new case and required extra cables.
It was possible to expand it to a 8096-SK.

CPU: MOS 6502 (1 MHz)
RAM: 32k standard (8096 expanded to 96k, 64k is banked memory)
ROM: 20 Kilobytes
Video: MOS Technology 6545 CRTC (12" Monochrome display; 80 columns x 25 rows)
Sound: Piezo electronic speaker (One square wave voice; Three octaves)
Ports: MOS 6520 PIA, MOS 6522 VIA (IEEE-488 square-connector Port; 2 Commodore
    Datasette ports; 'EXPANSION' port; CBM parallel programmable "User" port)
Keyboard: Full 73 key QWERTY (11 key numeric keypad!; 4 direction 2-key cursor-pad)


* Commodore CBM 200 Series (198?)

  Rebadged CMB 80XX models: the CBM 200 was basically a CBM 8032-SK and the
CBM 220 was a CBM 8096-SK.


* Commodore Super PET / MMF9000 (1981) - BASIC 4, 12" Screen

  Based on PET 8032. Sold in Germany as MMF (MicroMainFrame) 9000. Machines
sold in Italy had 134kB of RAM.

CPU: MOS Technology 6502 & 6809 (1 MHz clock speeds)
RAM: 96 kilobytes (64K was contained on an expansion board above the motherboard;
    Early SP9000 models had two boards above the motherboard; Some models also
    had 3 or more switches)
ROM: 48 kilobytes
Video: MOS Technology 6545 CRTC (12" Monochrome display; 80 columns x 25 rows;
    3+ character sets, 256 characters each)
Sound: Piezo electronic speaker (One square wave voice; Three octaves)
Ports: 6551 ACIA, MOS 6520 PIA, MOS 6522 VIA (IEEE-488 edge-connector Port; 2
    Commodore Datasette ports; 'EXPANSION' port; RS232 Internal port; CBM
    parallel programmable "User" port; Memory and Processor selection switches)
Keyboard: Full-sized 73 key QWERTY (Multi-Font 62 key keyboard; APL Symbols on
    the front of the keys; 11 key numeric keypad; Editor functions on the front
    of the keys; 2-key 4-direction cursor-pad)


* Commodore CBM SP9000

  Similar to Super PET. Dual uP 6502/6809, 96kB RAM, business keyboard.


* Commodore PET 8296 / CBM 8296 (1984) - BASIC 4, 12" Screen

  Shipped with 128k RAM and separate keyboard, it is a complete redesign of the
PET universal board. It fits into the separate keyboard case, and directly holds
128k RAM, of which 96k can be used as in the 8096. In addition, using "user
jumpers" on the motherboard, the ROM can be completely switched off to access
the RAM "under the ROM", so that the complete 128k RAM are accessible. The "user
jumpers" could be set to connect to the userport, so this could even be done
under program control.

CPU: MOS 6502 (1 MHz)
RAM: 160K (32k standard)
ROM: 24 Kilobytes
Video: MOS Technology 6545 CRTC (12" Monochrome displays; 80 columns x 25 rows)
Sound: Piezo electronic speaker (One square wave voice; Three octaves)
Ports: MOS 6520 PIA, MOS 6522 VIA (IEEE-488 Port; Commodore Datasette port;
    'EXPANSION' port; CBM parallel programmable "User" port)
Keyboard: Full 73 key QWERTY (11 key numeric keypad!; 4 direction 2-key cursor-pad)

see http://www.6502.org/users/andre/petindex/8x96.html and
http://www.sothius.com/hypertxt/welcome.html?./additional/cbm8296addition.html for
more informations


* Commodore PET 8296D / CBM 8296D (1984) - BASIC 4, 12" Screen

  It was a 8296 with built-in 8250-LP disk drive.

* Commodore CBM 8296GD

  Mentioned at http://www.commodore.ca/products/pet/commodore_pet.htm#Commodore%20PET%20Chronology
as a 8296 with a high resolution graphics board and drive. No other info available.

[Other models]


* Teacher's PET     Simply a PET 2001-N with a different label, due to a
                    marketing campaign in US schools
* MDS 6500          Modified 2001N-32 with matching 2040 drive.  500 made.
* "CASSIE"          Synergistics Inc. rebadged 8032


[Board Designs]

All these models can be reduced to four board types (source William Levak's
doc, layouts from PETFAQ):

Static Board (PET 2001)
-----------------------

Four variations based on type of RAM(6550 or 2114) and ROM(6540 or 2316B).
4K or 8K static RAM (selected by jumper).
40 column display
A video interrupt interferes with disk drive operation.
Display timing not compatible with Basic 4.0.
ROM sockets:  A2  2K character      ROM sockets:  A2  2K character
 (2316B)      H1  C000-CFFF           (6540)       H1  C000-C7FF
              H2  D000-DFFF                        H2  D000-D7FF
              H3  E000-E7FF                        H3  E000-E7FF
              H4  F000-FFFF                        H4  F000-F7FF
              H5  C000-CFFF                        H5  C800-CFFF
              H6  D000-DFFF                        H6  D800-DFFF
              H7  F000-FFFF                        H7  F800-FFFF


           IEEE user tape #2
     +------####-####--##-+
     !                    #
     !                    #
     !                    # exp
     !                    # bus
     !                    #
     !                    #    2000 Series
     !                    !       circa 1977/78  Max RAM - 8k
     !       (2k) ROMS    !       [w/daughter board exp to 32k shown]
     !      F F E D D C C !
     !      8 0 0 8 0 8 0 !
     !                    !
tape #       RAM MEMORY   !
 #1  #                    !
     +--------------------+


Dynamic Board (PET/CBM 2001-N/2001-B/4000)
------------------------------------------

4K, 8K, 16K or 32K dynamic RAM (selected by jumper).
40 column display
Can run all versions of 40 column Basic (Basic 1 must be copied to 4K ROMs)
Can be jumpered to replace the older board.
ROM sockets:  UD3   9000-9FFF
              UD4   A000-AFFF
              UD5   B000-BFFF
              UD6   C000-CFFF
              UD7   D000-DFFF
              UD8   E000-E7FF
              UD9   F000-FFFF
              UF10  2K character


            IEEE user tape #1
     +------####-####--##-+
     !                   #!
     !                   #!
     !                   #! exp
     !        ROMS       #! bus
     !    F E D C B A 9  #!
     !                   #!    3000, 4000 Series
     !                    !       (3000 series is European version)
     !                    !       circa 1979/80  Max RAM - 32k
     !                    !
     !                    !
     !                    !
tape #      RAM MEMORY    !
 #2  #                    !
     +--------------------+


80 Column Board (CBM 8000)
--------------------------

16K or 32K RAM (selected by jumper).
Uses CTRC to generate 80 column display.
Can only run the 80 column version of Basic 4.0.
Not compatible with older boards.
ROM sockets:  UA3   2K or 4K character
              UD6   F000-FFFF
              UD7   E000-E7FF
              UD8   D000-DFFF
              UD9   C000-CFFF
              UD10  B000-BFFF
              UD11  A000-AFFF
              UD12  9000-9FFF

The layout is the same of the one used in Universal Boards below.


Universal Board (CBM 8000/PET 4000-12)
--------------------------------------

This is an 80 column board with jumpers for different configurations.
16K or 32K RAM (selected by jumper).
Uses CTRC to generate 40 or 80 column display (selected by jumpers).
Can only run Basic 4.0 versions that support the CRTC.
Can be jumpered to replace all older boards.
ROM sockets:  UA3   2K or 4K character
              UD6   F000-FFFF
              UD7   E000-E7FF
              UD8   D000-DFFF
              UD9   C000-CFFF
              UD10  B000-BFFF
              UD11  A000-AFFF
              UD12  9000-9FFF


           IEEE user tape #1
     +------####-####--##-+
     !                  # # tape
     !                  # #  #2
     !  R       exp bus # !
     !  A                #!
     !  M             9  #!
     !                A  #!     4000, 8000 Series
     !  M          R  B   !        circa 1981     Max RAM - 32k*
     !  E          O  C   !       [8296 layout not shown]
     !  M          M  D   !
     !  O          S  E   !
     !  R             F   !
     !  Y                 !
     !                spkr!
     +--------------------+


[TO DO]

* Verify if pre-CRTC models had differences between NTSC & PAL systems.
If no differences arise we can merge back the following sets:
+ cbm30 (same as pet2001n) & cbm30b (same as pet2001b)
+ cbm40o (same as pet40on) & cbm40ob (same as pet40ob)

* Emulate SuperPET / SP9000 / MMF9000 and their differences

* Emulate CBM 8296 & 8296D (basically only a placeholder now)

* Add proper tape support

* Add sound support, for systems which had it

* Find out answers to the following details:

+ Did BASIC 2 also come as upgrade ROMs for PET 2001 with Static Board? If
this is the case, we need to support the splitted versions of the ROMs as
an alternative BIOS to pet2001.
+ Did final PET 40XX & CBM 40XX with 40 Columns & CRTC support had business or
normal keyboards?

* Find confirmation about some models (which I don't believe ever existed,
    but...):

+ were there PET 2001-16k & PET 2001-32k with Calculator type keyboard?
+ were there CBM 20XX in Europe?
+ what about the following unrealistic RAM configurations: 4004? 8008? 8016?
8064?
+ did a CBM 210 exist?

*/


#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6502/m6502.h"

#include "machine/6821pia.h"
#include "machine/6522via.h"
#include "includes/pet.h"
#include "machine/cbmipt.h"
#include "video/mc6845.h"
#include "machine/ram.h"

/* devices config */
#include "includes/cbm.h"
#include "formats/cbm_snqk.h"
#include "machine/ieee488.h"

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START(pet_mem , AS_PROGRAM, 8, pet_state )
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xe7ff) AM_ROM
	AM_RANGE(0xe810, 0xe813) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xe820, 0xe823) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xe840, 0xe84f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
/*  AM_RANGE(0xe900, 0xe91f) AM_DEVREAD_LEGACY("ieee_bus", cbm_ieee_state)    // for debugging */
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pet40_mem , AS_PROGRAM, 8, pet_state )
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xe7ff) AM_ROM
	AM_RANGE(0xe810, 0xe813) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xe820, 0xe823) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xe840, 0xe84f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
	AM_RANGE(0xe880, 0xe880) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe881, 0xe881) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pet80_mem , AS_PROGRAM, 8, pet_state )
	AM_RANGE(0x8000, 0x8fff) AM_RAMBANK("bank1")
	AM_RANGE(0x9000, 0x9fff) AM_RAMBANK("bank2")
	AM_RANGE(0xa000, 0xafff) AM_RAMBANK("bank3")
	AM_RANGE(0xb000, 0xbfff) AM_RAMBANK("bank4")
	AM_RANGE(0xc000, 0xe7ff) AM_RAMBANK("bank6")
#if 1
	AM_RANGE(0xe800, 0xefff) AM_RAMBANK("bank7")
#else
	AM_RANGE(0xe810, 0xe813) AM_DEVREADWRITE_LEGACY("pia_0", pia6821_r, pia6821_w)
	AM_RANGE(0xe820, 0xe823) AM_DEVREADWRITE_LEGACY("pia_1", pia6821_r, pia6821_w)
	AM_RANGE(0xe840, 0xe84f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
	AM_RANGE(0xe880, 0xe880) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe881, 0xe881) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
#endif
	AM_RANGE(0xf000, 0xffff) AM_READ_BANK("bank8")
	AM_RANGE(0xf000, 0xffef) AM_WRITE_BANK("bank8")
	AM_RANGE(0xfff1, 0xffff) AM_WRITE_BANK("bank9")
ADDRESS_MAP_END


/* 0xe880 crtc
   0xefe0 6702 encoder
   0xeff0 acia6551

   0xeff8 super pet system latch
61432        SuperPET system latch
        bit 0    1=6502, 0=6809
        bit 1    0=read only
        bit 3    diagnostic sense: set to 1 to switch to 6502

61436        SuperPET bank select latch
        bit 0-3  bank
        bit 7    1=enable system latch

*/
static ADDRESS_MAP_START( superpet_mem , AS_PROGRAM, 8, pet_state )
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_SHARE("memory")
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xa000, 0xe7ff) AM_ROM
	AM_RANGE(0xe810, 0xe813) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xe820, 0xe823) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xe840, 0xe84f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
	AM_RANGE(0xe880, 0xe880) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe881, 0xe881) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	/* 0xefe0, 0xefe3, mos 6702 */
	/* 0xeff0, 0xeff3, acia6551 */
	AM_RANGE(0xeff8, 0xefff) AM_READWRITE_LEGACY(superpet_r, superpet_w)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( superpet_m6809_mem, AS_PROGRAM, 8, pet_state )
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_SHARE("memory")	/* same memory as m6502 */
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("videoram")	/* same memory as m6502 */
    AM_RANGE(0x9000, 0x9fff) AM_RAMBANK("bank1")	/* 64 kbyte ram turned in */
	AM_RANGE(0xa000, 0xe7ff) AM_ROM
	AM_RANGE(0xe810, 0xe813) AM_DEVREADWRITE("pia_0", pia6821_device, read, write)
	AM_RANGE(0xe820, 0xe823) AM_DEVREADWRITE("pia_1", pia6821_device, read, write)
	AM_RANGE(0xe840, 0xe84f) AM_DEVREADWRITE("via6522_0", via6522_device, read, write)
	AM_RANGE(0xe880, 0xe880) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe881, 0xe881) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xeff8, 0xefff) AM_READWRITE_LEGACY(superpet_r, superpet_w)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Input Ports
 *
 *************************************/


static INPUT_PORTS_START( pet )
	PORT_INCLUDE( pet_keyboard )	/* ROW0 -> ROW9 */

	PORT_INCLUDE( pet_special )		/* SPECIAL */

	PORT_INCLUDE( pet_config )		/* CFG */
INPUT_PORTS_END


static INPUT_PORTS_START( petb )
	PORT_INCLUDE( pet_business_keyboard )	/* ROW0 -> ROW9 */

	PORT_INCLUDE( pet_special )				/* SPECIAL */

	PORT_INCLUDE( pet_config )				/* CFG */

    PORT_MODIFY("CFG")
	PORT_BIT( 0x180, 0x000, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( cbm8096 )
	PORT_INCLUDE( petb )

    PORT_MODIFY("CFG")
	PORT_DIPNAME( 0x08, 0x08, "CBM8096, 8296 Expansion Memory")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
INPUT_PORTS_END


static INPUT_PORTS_START (superpet)
	PORT_INCLUDE( petb )

    PORT_MODIFY("CFG")
	PORT_DIPNAME( 0x04, 0x04, "CPU Select")
	PORT_DIPSETTING(	0x00, "M6502" )
	PORT_DIPSETTING(	0x04, "M6809" )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/


static const unsigned char pet_palette[] =
{
	0,0,0, /* black */
	0,0x80,0, /* green */
};

static const gfx_layout pet_charlayout =
{
	8,8,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	},
	8*8
};

static const gfx_layout pet80_charlayout =
{
	8,16,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{ 0,1,2,3,4,5,6,7 },
	/* y offsets */
	{
		0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8
	},
	8*16
};

static GFXDECODE_START( pet )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pet_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0800, pet_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( pet80 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pet80_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, pet80_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( superpet )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pet80_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, pet80_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x2000, pet80_charlayout, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x3000, pet80_charlayout, 0, 1 )
GFXDECODE_END

static const mc6845_interface crtc_pet40 = {
	"screen",
	8,
	NULL,
	pet40_update_row,
	NULL,
	DEVCB_LINE(pet_display_enable_changed),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static const mc6845_interface crtc_pet80 = {
	"screen",
	16,
	NULL,
	pet80_update_row,
	NULL,
	DEVCB_LINE(pet_display_enable_changed),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

void pet_state::palette_init()
{
	int i;

	for ( i = 0; i < sizeof(pet_palette) / 3; i++ ) {
		palette_set_color_rgb(machine(), i, pet_palette[i*3], pet_palette[i*3+1], pet_palette[i*3+2]);
	}
}

VIDEO_START_MEMBER(pet_state,pet_crtc)
{
}

static IEEE488_INTERFACE( ieee488_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("pia_1", pia6821_device, cb1_w),
	DEVCB_DEVICE_LINE_MEMBER("pia_1", pia6821_device, ca1_w),
	DEVCB_NULL
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( pet_general, pet_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_8MHz/8)
	MCFG_CPU_PROGRAM_MAP(pet_mem)
	MCFG_CPU_VBLANK_INT("screen", pet_frame_interrupt)


    /* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(320, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 320 - 1, 0, 200 - 1)
	MCFG_SCREEN_UPDATE_STATIC( pet )

	MCFG_GFXDECODE( pet )
	MCFG_PALETTE_LENGTH(ARRAY_LENGTH(pet_palette) / 3)

	/* cassette */
	MCFG_CASSETTE_ADD( CASSETTE_TAG, cbm_cassette_interface )
	MCFG_CASSETTE_ADD( CASSETTE2_TAG, cbm_cassette_interface )

	/* via */
	MCFG_VIA6522_ADD( "via6522_0", 0, pet_via)

	/* pias */
	MCFG_PIA6821_ADD( "pia_0", pet_pia0)
	MCFG_PIA6821_ADD( "pia_1", pet_pia1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pet, pet_general )
	MCFG_QUICKLOAD_ADD("quickload", cbm_pet, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_FRAGMENT_ADD(pet_cartslot)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("8K,16K")

	/* IEEE bus */
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c4040")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( petb, pet )
	MCFG_PIA6821_MODIFY( "pia_0", petb_pia0 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pet2001, pet_general )
	MCFG_QUICKLOAD_ADD("quickload", cbm_pet1, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_FRAGMENT_ADD(pet_cartslot)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")
	MCFG_RAM_EXTRA_OPTIONS("4K")

	/* IEEE bus */
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c4040")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pet40, pet )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( pet40_mem)

	MCFG_MC6845_ADD("crtc", MC6845, XTAL_17_73447MHz/3	/* This is a wild guess and mostly likely incorrect */, crtc_pet40)

	MCFG_VIDEO_START_OVERRIDE(pet_state, pet_crtc )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DEVICE( "crtc", mc6845_device, screen_update )

	MCFG_FRAGMENT_ADD(pet4_cartslot)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pet40pal, pet40 )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pet80, pet_general )
	MCFG_QUICKLOAD_ADD("quickload", cbm_pet, "p00,prg", CBM_QUICKLOAD_DELAY_SECONDS)
	MCFG_FRAGMENT_ADD(pet_cartslot)

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( pet80_mem)

    /* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 250 - 1)
	MCFG_SCREEN_UPDATE_DEVICE( "crtc", mc6845_device, screen_update )

	MCFG_MC6845_ADD("crtc", MC6845, XTAL_12MHz / 2	/* This is a wild guess and mostly likely incorrect */, crtc_pet80)

	MCFG_GFXDECODE( pet80 )
	MCFG_VIDEO_START_OVERRIDE(pet_state, pet_crtc )

	MCFG_PIA6821_MODIFY( "pia_0", petb_pia0 )

	MCFG_FRAGMENT_ADD(pet4_cartslot)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")

	/* IEEE bus */
	MCFG_CBM_IEEE488_ADD(ieee488_intf, "c8050")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pet80pal, pet80 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( superpet, pet80 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( superpet_mem)

	/* m6809 cpu */
	MCFG_CPU_ADD("m6809", M6809, 1000000)
	MCFG_CPU_PROGRAM_MAP(superpet_m6809_mem)
	MCFG_CPU_VBLANK_INT("screen", pet_frame_interrupt)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_GFXDECODE( superpet )

	MCFG_PIA6821_MODIFY( "pia_0", petb_pia0 )
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/


/* PET 2001 - Board type 1 (Static Board) */
/* BASIC 1 - "*** COMMODORE BASIC ***" at boot */
/* Four board variations depending on
    ROM sockets: either 6540 or 2316B
    RAM: either 6550 or 2114
The ROM content is the same in both cases, but the labels differ. Below we use labels
from the 2316B version. For documentation sake, these would have been the labels based on
the board with 6540 ROMs

901439-01.h1 / 901439-09.h1
901439-05.h5
901439-02.h2
901439-06.h6
901439-03.h3
901439-04.h4
901439-07.h7
901439-08.a2

Also, in some board with 2316B, the location of the ROMs changes h1 <-> h5, h2 <-> h6 and
h4 <-> h7
*/

ROM_START( pet2001 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "basic1", "BASIC 1r" )
	ROMX_LOAD( "901447-09.h1", 0xc000, 0x800, CRC(03cf16d0) SHA1(1330580c0614d3556a389da4649488ba04a60908), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic1o", "BASIC 1" )
	ROMX_LOAD( "901447-01.h1", 0xc000, 0x800, CRC(a055e33a) SHA1(831db40324113ee996c434d38b4add3fd1f820bd), ROM_BIOS(2) )
	ROM_LOAD( "901447-02.h5", 0xc800, 0x800, CRC(69fd8a8f) SHA1(70c0f4fa67a70995b168668c957c3fcf2c8641bd) )
	ROM_LOAD( "901447-03.h2", 0xd000, 0x800, CRC(d349f2d4) SHA1(4bf2c20c51a63d213886957485ebef336bb803d0) )
	ROM_LOAD( "901447-04.h6", 0xd800, 0x800, CRC(850544eb) SHA1(d293972d529023d8fd1f493149e4777b5c253a69) )
	ROM_LOAD( "901447-05.h3", 0xe000, 0x800, CRC(9e1c5cea) SHA1(f02f5fb492ba93dbbd390f24c10f7a832dec432a) )
	ROM_LOAD( "901447-06.h4", 0xf000, 0x800, CRC(661a814a) SHA1(960717282878e7de893d87242ddf9d1512be162e) )
	ROM_LOAD( "901447-07.h7", 0xf800, 0x800, CRC(c4f47ad1) SHA1(d440f2510bc52e20c3d6bc8b9ded9cea7f462a9c) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "901447-08.a2", 0x0000, 0x800, CRC(54f32f45) SHA1(3e067cc621e4beafca2b90cb8f6dba975df2855b) )
ROM_END


/* PET 2001-N / 2001-B / CBM 30XX / PET 40XX (early) / CBM 40XX (early) - Board type 2 (Dynamic Board) */
/* BASIC 1 upgraded / BASIC 2 - "### COMMODORE BASIC ###" at boot */
/* Again different kind of ROM sockets can be found. */
/* These boards would support BASIC 1 as well, if the content would be put on 4K ROMs */

/* BASIC 2 */
ROM_START( pet2001n )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-01.ud6", 0xc000, 0x1000, CRC(63a7fe4a) SHA1(3622111f486d0e137022523657394befa92bde44) )	// BASIC 2
	ROM_LOAD( "901465-02.ud7", 0xd000, 0x1000, CRC(ae4cb035) SHA1(1bc0ebf27c9bb62ad71bca40313e874234cab6ac) )	// BASIC 2
	ROM_LOAD( "901447-24.ud8", 0xe000, 0x800,  CRC(e459ab32) SHA1(5e5502ce32f5a7e387d65efe058916282041e54b) )	// Screen Editor (40 columns, no CRTC, Normal Keyb)
	ROM_LOAD( "901465-03.ud9", 0xf000, 0x1000, CRC(f02238e2) SHA1(38742bdf449f629bcba6276ef24d3daeb7da6e84) )	// Kernal

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "901447-10.uf10", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

/* BASIC 2 - Business Keyboard (Number keys, etc.) */
ROM_START( pet2001b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-01.ud6", 0xc000, 0x1000, CRC(63a7fe4a) SHA1(3622111f486d0e137022523657394befa92bde44) )	// BASIC 2
	ROM_LOAD( "901465-02.ud7", 0xd000, 0x1000, CRC(ae4cb035) SHA1(1bc0ebf27c9bb62ad71bca40313e874234cab6ac) )	// BASIC 2
	ROM_LOAD( "901474-01.ud8", 0xe000, 0x800,  CRC(05db957e) SHA1(174ace3a8c0348cd21d39cc864e2adc58b0101a9) )	// Screen Editor (40 columns, no CRTC, Business Keyb)
	ROM_LOAD( "901465-03.ud9", 0xf000, 0x1000, CRC(f02238e2) SHA1(38742bdf449f629bcba6276ef24d3daeb7da6e84) )	// Kernal

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "901447-10.uf10", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

#define rom_cbm30		rom_pet2001n
#define rom_cbm30b		rom_pet2001b

/* BASIC 4, but 40 columns only and no CRTC */
ROM_START( pet40on )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "basic4", "BASIC 4r" )
	ROMX_LOAD( "901465-23.ud5", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(1) )	// BASIC 4
	ROM_SYSTEM_BIOS( 1, "basic4o", "BASIC 4" )
	ROMX_LOAD( "901465-19.ud5", 0xb000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(2) )
	ROM_LOAD( "901465-20.ud6", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud7", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901447-29.ud8", 0xe000, 0x800,  CRC(e5714d4c) SHA1(e88f56e5c54b0e8d8d4e8cb39a4647c803c1f51c) )	// Screen Editor (40 columns, no CRTC, Normal Keyb)
	ROM_LOAD( "901465-22.ud9", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "901447-10.uf10", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

/* BASIC 4, but 40 columns only and no CRTC - Business Keyboard (Number keys, etc.) */
ROM_START( pet40ob )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "basic4", "BASIC 4r" )
	ROMX_LOAD( "901465-23.ud5", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "basic4o", "BASIC 4" )
	ROMX_LOAD( "901465-19.ud5", 0xb000, 0x1000, CRC(3a5f5721) SHA1(bc2b7c99495fea3eda950ee9e3d6cabe448a452b), ROM_BIOS(2) )
	ROM_LOAD( "901465-20.ud6", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud7", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901474-02.ud8", 0xe000, 0x800,  CRC(75ff4af7) SHA1(0ca5c4e8f532f914cb0bf86ea9900f20f0a655ce) )	// Screen Editor (40 columns, no CRTC, Business Keyb)
	ROM_LOAD( "901465-22.ud9", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "901447-10.uf10", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

#define rom_cbm40o		rom_pet40on
#define rom_cbm40ob		rom_pet40ob

/* PET 40XX (later) / PET 80XX / CBM 400XX (later) / CBM 80XX - Board type 3 (80 columns Board) & 4 (Universal Board) */
/* BASIC 4, Board 3: 80 columns only using CRTC - Board 4: 40 or 80 columns using CRTC, changed through a jumper on the board */
/* Board 3 ones can only run the 80 columns BASIC 4, Board 4 can run all BASIC versions supporting CRTC */

/* 40 columns - 60 Hz */
ROM_START( pet40n )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901499-01.ud7", 0xe000, 0x800,  CRC(5f85bdf8) SHA1(8cbf086c1ce4dfb2a2fe24c47476dfb878493dee) )	// Screen Editor (40 columns, CRTC 60Hz, Normal Keyb?)
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "901447-10.ua3", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

/* 40 columns - 50 Hz */
ROM_START( cbm40n )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901498-01.ud7", 0xe000, 0x800,  CRC(3370e359) SHA1(05af284c914d53a52987b5f602466de75765f650) )	// Screen Editor (40 columns, CRTC 50Hz, Normal Keyb?)
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "901447-10.ua3", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

/* 80 columns - 60 Hz */
ROM_START( pet40b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901474-03.ud7", 0xe000, 0x800,  CRC(5674dd5e) SHA1(c605fa343fd77c73cbe1e0e9567e2f014f6e7e30) )	// Screen Editor (80 columns, CRTC 60Hz, Business Keyb)
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901447-10.ua3", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

/* 80 columns - 50 Hz */
ROM_START( cbm40b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_SYSTEM_BIOS( 0, "default", "BASIC 4" )
	ROMX_LOAD( "901474-04.ud7", 0xe000, 0x800,  CRC(abb000e7) SHA1(66887061b6c4ebef7d6efb90af9afd5e2c3b08ba), ROM_BIOS(1) )	// Screen Editor (80 columns, CRTC 50Hz, Business Keyb)
	ROM_SYSTEM_BIOS( 1, "alt", "BASIC 4 (alt Editor)" )
	ROMX_LOAD( "901474-04a.ud7", 0xe000, 0x800, BAD_DUMP CRC(845a44e6) SHA1(81975eab31a8f4f51ae2a20d099a567c7b3f2dd1), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "old", "BASIC 4 (Editor dated 3681)" )
	ROMX_LOAD( "901474-04o.ud7", 0xe000, 0x800, CRC(c1ffca3a) SHA1(7040b283ba39e9630e3d147f7d076b7abc39bc70), ROM_BIOS(3) )
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901447-10.ua3", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

/* 80 columns - 60 Hz - can be expanded to 96k RAM */
ROM_START( pet80 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901474-03.ud7", 0xe000, 0x800,  CRC(5674dd5e) SHA1(c605fa343fd77c73cbe1e0e9567e2f014f6e7e30) )	// Screen Editor (80 columns, CRTC 60Hz, Business Keyb)
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901447-10.ua3", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

/* 80 columns - 50 Hz - can be expanded to 96k RAM */
ROM_START( cbm80 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_SYSTEM_BIOS( 0, "default", "BASIC 4" )
	ROMX_LOAD( "901474-04.ud7", 0xe000, 0x800,  CRC(abb000e7) SHA1(66887061b6c4ebef7d6efb90af9afd5e2c3b08ba), ROM_BIOS(1) )	// Screen Editor (80 columns, CRTC 50Hz, Business Keyb)
	ROM_SYSTEM_BIOS( 1, "alt", "BASIC 4 (alt Editor)" )
	ROMX_LOAD( "901474-04a.ud7", 0xe000, 0x800, BAD_DUMP CRC(845a44e6) SHA1(81975eab31a8f4f51ae2a20d099a567c7b3f2dd1), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "old", "BASIC 4 (Editor dated 3681)" )
	ROMX_LOAD( "901474-04o.ud7", 0xe000, 0x800, CRC(c1ffca3a) SHA1(7040b283ba39e9630e3d147f7d076b7abc39bc70), ROM_BIOS(3) )
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901447-10.ua3", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END


/* Two different layouts are documented for the m6809 */
/* The 2532 EEPROM listed below (2516 for the .u21 one) could have been replaced by three 2764 EEPROM.
In this case the labels would have been as follows

970018-12.u47 (contents: u17+u18 below)
970019-12.u48 (contents: u19+u20 below)
970020-12.u49 (contents: u21+u22 below)

*/

ROM_START( superpet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901474-04.ud7", 0xe000, 0x800,  CRC(abb000e7) SHA1(66887061b6c4ebef7d6efb90af9afd5e2c3b08ba) )	// Screen Editor (80 columns, CRTC 50Hz, Business Keyb)
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x10000, "m6809", 0 )
	ROM_LOAD( "901898-01.u17", 0xa000, 0x1000, CRC(728a998b) SHA1(0414b3ab847c8977eb05c2fcc72efcf2f9d92871) )
	ROM_LOAD( "901898-02.u18", 0xb000, 0x1000, CRC(6beb7c62) SHA1(df154939b934d0aeeb376813ec1ba0d43c2a3378) )
	ROM_LOAD( "901898-03.u19", 0xc000, 0x1000, CRC(5db4983d) SHA1(6c5b0cce97068f8841112ba6d5cd8e568b562fa3) )
	ROM_LOAD( "901898-04.u20", 0xd000, 0x1000, CRC(f55fc559) SHA1(b42a2050a319a1ffca7868a8d8d635fadd37ec37) )
	ROM_LOAD( "901897-01.u21", 0xe000, 0x800,  CRC(b2cee903) SHA1(e8ce8347451a001214a5e71a13081b38b4be23bc) )
	ROM_LOAD( "901898-05.u22", 0xf000, 0x1000, CRC(f42df0cb) SHA1(9b4a5134d20345171e7303445f87c4e0b9addc96) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "901640-01.ub3", 0x0000, 0x1000, CRC(ee8229c4) SHA1(bf346f11595a3e65e55d6aeeaa2c0cec807b66c7) )
ROM_END

#define rom_sp9000		rom_superpet


/* CBM 8296 / 8296D - only ROM loading added */
ROM_START( cbm8296 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "324746-01.ue7", 0xb000, 0x3000, CRC(7935b528) SHA1(5ab17ee70467152bf2130e3f48a2aa81e9df93c9) )	// BASIC 4 // FIX ME!!
	ROM_CONTINUE(			   0xf000, 0x1000 )
	ROM_LOAD( "901474-04.ue8", 0xe000, 0x800,  CRC(c1ffca3a) SHA1(7040b283ba39e9630e3d147f7d076b7abc39bc70) )	// Dated 0384, coincides with 3681 above according to Andr? Fachat's notes

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "901447-10.uc5", 0x0000, 0x800, CRC(d8408674) SHA1(0157a2d55b7ac4eaeb38475889ebeea52e2593db) )	// Character Generator
ROM_END

#define rom_cbm8296d	rom_cbm8296


/* PAL regional variants */

ROM_START( cbm80ger )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "german.bin",    0xe000, 0x800,  CRC(1c1e597d) SHA1(7ac75ed73832847623c9f4f197fe7fb1a73bb41c) )
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "chargen.de", 0x0000, 0x800, CRC(3bb8cb87) SHA1(a4f0df13473d7f9cd31fd62cfcab11318e2fb1dc) )
ROM_END

ROM_START( cbm80swe )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "swedish.bin",   0xe000, 0x800,  CRC(75901dd7) SHA1(2ead0d83255a344a42bb786428353ca48d446d03) )		// It had a label "8000-UD7, SCREEN-04"
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "901447-14.ua3", 0x0000, 0x800, CRC(48c77d29) SHA1(aa7c8ff844d16ec05e2b32acc586c58d9e35388c) )	// Character Generator
ROM_END

/* This had only the CharGen dumped, the editor needs to be dumped as well (and the other ones verified)!!  */
ROM_START( cbm30nor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-01.ud6", 0xc000, 0x1000, CRC(63a7fe4a) SHA1(3622111f486d0e137022523657394befa92bde44) )	// BASIC 2
	ROM_LOAD( "901465-02.ud7", 0xd000, 0x1000, CRC(ae4cb035) SHA1(1bc0ebf27c9bb62ad71bca40313e874234cab6ac) )	// BASIC 2
	ROM_LOAD( "901474-01.ud8", 0xe000, 0x800,  BAD_DUMP CRC(05db957e) SHA1(174ace3a8c0348cd21d39cc864e2adc58b0101a9) )	// Screen Editor to be redumped
	ROM_LOAD( "901465-03.ud9", 0xf000, 0x1000, CRC(f02238e2) SHA1(38742bdf449f629bcba6276ef24d3daeb7da6e84) )	// Kernal

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "norwegian.uf10", 0x0000, 0x800, CRC(7c00534a) SHA1(2c46bd5f5351530ceb52686e5196de995e28e24f) )	// Character Generator dumped from a CBM 3032 // FIX ME!!
ROM_END

/* This had only the CharGen dumped, the editor needs to be dumped as well (and the other ones verified)!!  */
ROM_START( cbm80hun )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "901474-04.ud7", 0xe000, 0x800,  BAD_DUMP CRC(abb000e7) SHA1(66887061b6c4ebef7d6efb90af9afd5e2c3b08ba) )	// Screen Editor (80 columns, CRTC 50Hz, Business Keyb)
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "hungarian.ua3", 0x0000, 0x800, CRC(a02d8122) SHA1(2fedbb59068b457d98f28de79f1817e25f745604) )	// Character Generator // FIX ME!!
ROM_END

/* Swedish M6809 roms needed */
ROM_START( mmf9000s )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "901465-23.ud10", 0xb000, 0x1000, CRC(ae3deac0) SHA1(975ee25e28ff302879424587e5fb4ba19f403adc) )	// BASIC 4
	ROM_LOAD( "901465-20.ud9", 0xc000, 0x1000, CRC(0fc17b9c) SHA1(242f98298931d21eaacb55fe635e44b7fc192b0a) )	// BASIC 4
	ROM_LOAD( "901465-21.ud8", 0xd000, 0x1000, CRC(36d91855) SHA1(1bb236c72c726e8fb029c68f9bfa5ee803faf0a8) )	// BASIC 4
	ROM_LOAD( "swedish.bin",   0xe000, 0x800,  CRC(75901dd7) SHA1(2ead0d83255a344a42bb786428353ca48d446d03) )
	ROM_LOAD( "901465-22.ud6", 0xf000, 0x1000, CRC(cc5298a1) SHA1(96a0fa56e0c937da92971d9c99d504e44e898806) )	// Kernal

	ROM_REGION( 0x20000, "m6809", 0 )
	ROM_LOAD( "901898-01.u17", 0xa000, 0x1000, BAD_DUMP CRC(728a998b) SHA1(0414b3ab847c8977eb05c2fcc72efcf2f9d92871) )
	ROM_LOAD( "901898-02.u18", 0xb000, 0x1000, BAD_DUMP CRC(6beb7c62) SHA1(df154939b934d0aeeb376813ec1ba0d43c2a3378) )
	ROM_LOAD( "901898-03.u19", 0xc000, 0x1000, BAD_DUMP CRC(5db4983d) SHA1(6c5b0cce97068f8841112ba6d5cd8e568b562fa3) )
	ROM_LOAD( "901898-04.u20", 0xd000, 0x1000, BAD_DUMP CRC(f55fc559) SHA1(b42a2050a319a1ffca7868a8d8d635fadd37ec37) )
	ROM_LOAD( "901897-01.u21", 0xe000, 0x800,  BAD_DUMP CRC(b2cee903) SHA1(e8ce8347451a001214a5e71a13081b38b4be23bc) )
	ROM_LOAD( "901898-05.u22", 0xf000, 0x1000, BAD_DUMP CRC(f42df0cb) SHA1(9b4a5134d20345171e7303445f87c4e0b9addc96) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD("skand.gen.ub3", 0x0000, 0x1000, CRC(da1cd630) SHA1(35f472114ff001259bdbae073ae041b0759e32cb) )	// Actual label was "901640-01 SKAND.GEN."
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/* YEAR    NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY                             FULLNAME */

COMP(1977, pet2001,  0,        0,        pet2001,  pet, pet_state,      pet2001, "Commodore Business Machines",  "PET 2001", GAME_NO_SOUND)
COMP(1979, pet2001n, pet2001,  0,        pet,      pet, pet_state,      pet,     "Commodore Business Machines",  "PET 2001-N", GAME_NO_SOUND)
COMP(1979, pet2001b, pet2001,  0,        petb,     petb, pet_state,     pet,     "Commodore Business Machines",  "PET 2001-B", GAME_NO_SOUND)
COMP(1979, cbm30,    pet2001,  0,        pet,      pet, pet_state,      pet,     "Commodore Business Machines",  "CBM 30xx", GAME_NO_SOUND)
COMP(1979, cbm30b,   pet2001,  0,        petb,     petb, pet_state,     pet,     "Commodore Business Machines",  "CBM 30xx (Business keyboard)", GAME_NO_SOUND)
COMP(1979, cbm30nor, pet2001,  0,        petb,     petb, pet_state,     pet,     "Commodore Business Machines",  "CBM 30xx (Norway, Business keyboard)", GAME_NO_SOUND)

/* So called, THIN-40 */
COMP(1980, pet40on,  pet2001,  0,        pet,      pet, pet_state,      pet,     "Commodore Business Machines",  "PET 40xx (Basic 4, no CRTC, Normal keyboard)", GAME_NO_SOUND)
COMP(1980, pet40ob,  pet2001,  0,        petb,     petb, pet_state,     pet,     "Commodore Business Machines",  "PET 40xx (Basic 4, no CRTC, Business keyboard)", GAME_NO_SOUND)
COMP(1980, cbm40o,   pet2001,  0,        pet,      pet, pet_state,      pet,     "Commodore Business Machines",  "CBM 40xx (Basic 4, no CRTC, Normal keyboard)", GAME_NO_SOUND)
COMP(1980, cbm40ob,  pet2001,  0,        petb,     petb, pet_state,     pet,     "Commodore Business Machines",  "CBM 40xx (Basic 4, no CRTC, Business keyboard)", GAME_NO_SOUND)

COMP(1981, pet80,    0,        0,        pet80,    cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "PET 80xx (Basic 4, CRTC 60Hz, 80 columns)", GAME_NO_SOUND)
COMP(1981, cbm80,    pet80,    0,        pet80pal, cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "CBM 80xx (Basic 4, CRTC 50Hz, 80 columns)", GAME_NO_SOUND)
COMP(1981, cbm80ger, pet80,    0,        pet80pal, cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "CBM 80xx (Germany, Basic 4, CRTC 50Hz, 80 columns)", GAME_NO_SOUND)
COMP(1981, cbm80hun, pet80,    0,        pet80pal, cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "CBM 80xx (Hungary, Basic 4, CRTC 50Hz, 80 columns)", GAME_NO_SOUND)
COMP(1981, cbm80swe, pet80,    0,        pet80pal, cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "CBM 80xx (Sweden, Basic 4, CRTC 50Hz, 80 columns)", GAME_NO_SOUND)

/* So called, FAT-40 */
COMP(1981, pet40b,   pet80,    0,        pet80,    cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "PET 40xx (Basic 4, CRTC 60Hz, 80 columns)", GAME_NO_SOUND)
COMP(1981, pet40n,   pet2001,  0,        pet40,    pet, pet_state,      pet,     "Commodore Business Machines",  "PET 40xx (Basic 4, CRTC 60Hz, 40 columns)", GAME_NO_SOUND)
COMP(1981, cbm40b,   pet80,    0,        pet80pal, cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "CBM 40xx (Basic 4, CRTC 50Hz, 80 columns)", GAME_NO_SOUND)
COMP(1981, cbm40n,   pet2001,  0,        pet40pal, pet, pet_state,      pet,     "Commodore Business Machines",  "CBM 40xx (Basic 4, CRTC 50Hz, 40 columns)", GAME_NO_SOUND)

COMP(1981, superpet, 0,        0,        superpet, superpet, pet_state, superpet,"Commodore Business Machines",  "SuperPET (CRTC 50Hz)", GAME_NO_SOUND | GAME_NOT_WORKING)
COMP(1981, sp9000,   superpet, 0,        superpet, superpet, pet_state, superpet,"Commodore Business Machines",  "CBM SP9000 / MicroMainFrame 9000 (CRTC 50Hz)", GAME_NO_SOUND | GAME_NOT_WORKING)
COMP(198?, mmf9000s, superpet, 0,        superpet, superpet, pet_state, superpet,"Commodore Business Machines",  "MicroMainFrame 9000 (Sweden, CRTC 50Hz)", GAME_NO_SOUND | GAME_NOT_WORKING)

COMP(1984, cbm8296,  pet80,    0,        pet80pal, cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "CBM 8296 (Basic 4, CRTC 50Hz, 80 columns)", GAME_NO_SOUND)
COMP(1984, cbm8296d, pet80,    0,        pet80pal, cbm8096, pet_state,  pet80,   "Commodore Business Machines",  "CBM 8296D", GAME_NO_SOUND)
