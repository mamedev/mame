/***************************************************************************

    Data East 32 bit ARM based games:

    Captain America
    Dragon Gun
    Fighter's History
    Locked 'N Loaded
    Night Slashers
    Tattoo Assassins


    Emulation by Bryan McPhail, mish@tendril.co.uk.  Thank you to Tim,
    Avedis and Stiletto for many things including work on Fighter's
    History protection and tracking down Tattoo Assassins!

    Captain America & Fighter's History - reset with both start buttons
    held down for test mode.  Reset with player 1 start held in Fighter's
    History for 'Pattern Editor'.

    Tattoo Assassins is a prototype, it is thought only 25 test units
    were manufactured and distributed to test arcades before the game
    was recalled.  TA is the only game developed by Data East Pinball
    in USA, rather than Data East Corporation in Japan.

    Tattoo Assassins uses DE Pinball soundboard 520-5077-00 R


    Todo:

    Tattoo Assassins & Dragongun use an unemulated chip (Ace/Jack) for
    special blending effects.  It's exact effect is unclear.

    Video backgrounds in Dragongun?

    Locked'N Loaded (parent set) is a slightly different hardware
    revision: board # DE-0420-1 where the US set is DE-0359-2.
    The sound is _not_ hooked up correctly for this set.



Locked 'n Loaded (US)
Data East Corporation (c) 1994

DE-0359-2 PCB Layout - Same PCB as used for Dragon Gun, see comment below:

------------------------------------------------------------
|     32.220MHz   28.000MHz                 8M-7    8M-3   |
|                                          MBM-05  MBM-03  |
|         NH06-0   HuC6280A                 8M-5    8M-1   |
|           YM2151                         MBM-04  MBM-02  |
--+                                                        |
--+           MBM-07                                       |
|             MAR-07                74                     |
| M6295 M6295 MBM-06                       MBM-01  NH05-0  |
| M6295                                    MBM-00  NH04-0  |
|                                      74                  |
|J                                                         |
|A                                                         |
|M                                                         |
|M                                          2M-5    2M-4   |
|A  113                                    NH03-0  NH01-0  |
|                                  101      2M-7    2M-6   |
--+                                        NH02-0  NH00-0  |
--+ DSW1   146                                             |
|A             93C45                                       |
|U                                +-------------------------+
|X                                |         DE-0406-1       |
--|       ADC0808CCN              |       AUX PCB with      |
  --------------------------------|      Gun Connectors     |
                                  --------------------------+

2M-4 through 2M-7 are empty sockets for additional program ROMs (used by dragon Gun)
Odd numbered 8M are empty sockets
AUX edge connector is a 48 pin type simular to those used on Namco System 11, 12, ect


DE-0360-4 ROM board Layout:

------------------------------------------------------------
| CN2                   TC524256BZ-10 TC524256BZ-10  MAR-17|
|                       TC524256BZ-10 TC524256BZ-10  MAR-18|
| HM65256BLSP-10        TC524256BZ-10 TC524256BZ-10  MAR-19|
| 16 of these chips     TC524256BZ-10 TC524256BZ-10  MAR-20|
| in this area                                       MAR-21|
|                                       Intel i750   MAR-22|
|         187     23.000MHz                          MAR-23|
|MBM-08                                              MAR-24|
|MBM-09             20.0000MHz                       MAR-25|
|MBM-10                                      145     MAR-26|
|MBM-11  186                                         MAR-27|
|MBM-12                                              MAR-28|
|MBM-13                                                    |
|MBM-14 PAL16L8BCN                          Intel i750     |
|MBM-15 PAL16L8BCN                                         |
| CN1                           25.000MHz      PAL16L8BCN  |
------------------------------------------------------------

CN1 = Triple row 32 pin connector
CN2 = Dual row 32 pin connector

Locked 'n Loaded appears to be a conversion of Dragon Gun (c) 1993 as
there are 12 surface mounted GFX roms and 1 surface mounted sample rom
left over from the conversion.  The roms labeled "MAR-xx" are those
from Dragon Gun.



Night Slashers
Data East, 1993

PCB Layout
----------

DE-0397-0   DEC-22VO
|-----------------------------------------------------|
| TA8205AH   Z80          |-----|              HBM-07 |
|         6164    YM2151  | 52  |                     |
|         LX02            |     | HBM-09       HBM-06 |
|     YM3012      32MHz   |-----|                     |
|  JP1   HBM-11   93C45                        HBM-05 |
|CN2     HBM-10           |-----|                     |
|  M6295(1)               | 52  |              HBM-04 |
|     M6295(2)            |     |                     |
|      |-----| |-----|    |-----|    |-----|   HBM-03 |
|J     | 104 | | 153 |               | 52  |          |
|A     |     | |     |     |-----|   |     |   HBM-02 |
|M     |-----| |-----|     | 153 |   |-----|          |
|M                         |     |           28.322MHz|
|A                         |-----|                    |
|    |-----| PAL                                      |
|    | 99  | PROM              |-----| 6164    HBM-01 |
|    |     |                   | 74  | 6164           |
|    |-----|                   |     |         HBM-00 |
|      |-----|  |-----|        |-----|                |
|      | 153 |  | 200 |        |-----| 6164           |
|      |     |  |     |        | 141 | 6164           |
|      |-----|  |-----|        |     |   PAL          |
|TEST_SW                       |-----|   PAL  |-----| |
|                                             | 156 | |
|           LH52250  LH52250   LX-01          |     | |
|  CN3      LH52250  LH52250   LX-00          |-----| |
|-----------------------------------------------------|
Notes:
       The CPU is chip 156. It's an encrypted ARM-based CPU. The CPU is running at 7.0805MHz [28.322/4]

       Z80B      - Goldstar Z8400B, running at 3.5555MHz [32/9]
       YM2151    - Yamaha YM2151 sound chip, running at 3.5555MHz [32/9]
       OKI M6295 - Oki M6295 PCM Sample chip, (1) running at 1.000MHz [32/32]. Sample rate = 1000000 / 132
                                              (2) running at 2.000MHz [32/16]. Sample rate = 2000000 / 132
       6164      - UM6164BK-20 8K x8 SRAM
       LH52250   - Sharp LH52250 32K x8 SRAM
       93C45     - 128bytes x8 Serial EEPROM
       PALs      - PAL 16L8ACN (x 2, near program ROMs, one at 3D labelled 'VM-00', one at 4D labelled 'VM-01')
                   PAL 16l8ACN (near chip 99, located at 8J, labelled 'VM-02')
       HSync     - 15.86kHz
       VSync     - 58Hz
       Custom ICs-
                   DE #            Package Type      Additional #'s (for reference of scratched-off chips on other PCB's)
                   ------------------------------------------------------------------------------------------------------
                   156 (CPU)       100 Pin PQFP      932EV 301801
                   141             160 Pin PQFP      24220F008
                   74              160 Pin PQFP      24220F009
                   99              208 Pin PQFP      L7A0967
                   52 (x3)         128 Pin PQFP      9322EV 298251 VC5259-0001
                   153 (x3)        144 Pin PQFP      L7A0888 9328
                   104             100 Pin PQFP      L7A0717 9148
                   200             100 Pin PQFP      JAPAN 9320PD027  (chip is darker black)

       Other     - There's a small push button near the JAMMA connector to access test mode.
                   All settings are via an on-board menu.

                   All pinouts conform to standard JAMMA pinout.  Joystick is 8-way with 3 buttons used.

                   JP1: 1-2 Sound Output in MONO
                        2-3 Sound Output in STEREO

                   CN2: 4 Pin connector (use when JP1 = 2-3)
                        Pin #   Signal
                         1      L-Speaker +
                         2      L-Speaker -
                         3      R-Speaker -
                         4      R-Speaker +

                   CN3: 15 Pin connector (Player 3)
                        Pin #   Signal
                         1      COIN SW3
                         2      3P PUSH 3
                         3      3P LEFT
                         4      3P RIGHT
                         5      3P UP
                         6      3P DOWN
                         7      3P PUSH 1
                         8      3P PUSH 2
                         9-13   NOT USED
                         14-15  GND


       ROMs      - MAINPRG1.1F     HN27C4096      \
                   MAINPRG2.2F     HN27C4096      /  Main Program (no ROM stickers attached, DE ROM code unknown)

                   Japanese Version
                   LX01-.2F        HN27C4096      \
                   LX00-.1F        HN27C4096      /  Main Program (Japan version)

                   MBH-00.8C   42 pin 16M MASK  (read as 27C160)    \
                   MBH-01.9C   42 pin 16M MASK  (read as 27C160)    |
                   MBH-02.14C  42 pin 16M MASK  (read as 27C160)    |
                   MBH-03.15C  40 pin 4M MASK   (read as MX27C4100) |
                   MBH-04.16C  42 pin 16M MASK  (read as 27C160)    |
                   MBH-05.17C  40 pin 4M MASK   (read as MX27C4100) | GFX
                   MBH-06.18C  32 pin 8M MASK   (read as 27C080)    |
                   MBH-07.19C  32 pin 2M MASK   (read as 27C020)    |
                   MBH-08.16E  40 pin 4M MASK   (read as MX27C2100) |
                   MBH-09.18E  40 pin 4M MASK   (read as MX27C2100) /

                   MBH-10.14L  32 pin 4M MASK   (read as 27C040)    \
                   MBH-11.16L  32 pin 4M MASK   (read as 27C040)    / Sound (Samples)

                   LX02-.17L   27C512                                 Sound Program

                   PROM.9J     Fujitsu MB7124 compatible with 82S147  Labelled 'LN-00'

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "cpu/arm/arm.h"
#include "cpu/h6280/h6280.h"
#include "cpu/m6809/m6809.h"
#include "includes/decocrpt.h"
#include "includes/decoprot.h"
#include "machine/eeprom.h"
#include "includes/deco32.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "sound/bsmt2000.h"

static UINT32 *deco32_ram;
static int raster_enable;
static timer_device *raster_irq_timer;
static UINT8 nslasher_sound_irq;

/**********************************************************************************/


static WRITE32_HANDLER( deco32_pf12_control_w )
{
	COMBINE_DATA(&deco32_pf12_control[offset]);
	space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
}


static WRITE32_HANDLER( deco32_pf34_control_w )
{
	COMBINE_DATA(&deco32_pf34_control[offset]);
	space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos());
}



static TIMER_DEVICE_CALLBACK( interrupt_gen )
{
	cputag_set_input_line(timer.machine, "maincpu", ARM_IRQ_LINE, HOLD_LINE);
}

static READ32_HANDLER( deco32_irq_controller_r )
{
	int vblank;

	switch (offset)
	{
	case 2: /* Raster IRQ ACK - value read is not used */
		cputag_set_input_line(space->machine, "maincpu", ARM_IRQ_LINE, CLEAR_LINE);
		return 0;

	case 3: /* Irq controller

        Bit 0:  1 = Vblank active
        Bit 1:  ? (Hblank active?  Captain America raster IRQ waits for this to go low)
        Bit 2:
        Bit 3:
        Bit 4:  VBL Irq
        Bit 5:  Raster IRQ
        Bit 6:  Lightgun IRQ (on Lock N Load only)
        Bit 7:
        */

        /* ZV03082007 - video_screen_get_vblank() doesn't work for Captain America, as it expects
           that this bit is NOT set in rows 0-7. */
        vblank = space->machine->primary_screen->vpos() > space->machine->primary_screen->visible_area().max_y;
		if (vblank)
			return 0xffffff80 | 0x1 | 0x10; /* Assume VBL takes priority over possible raster/lightgun irq */

		return 0xffffff80 | vblank | (cpu_getiloops(space->cpu) ? 0x40 : 0x20);
//      return 0xffffff80 | vblank | (0x40); //test for lock load guns
	}

	logerror("%08x: Unmapped IRQ read %08x (%08x)\n",cpu_get_pc(space->cpu),offset,mem_mask);
	return 0xffffffff;
}

static WRITE32_HANDLER( deco32_irq_controller_w )
{
	int scanline;

	switch (offset) {
	case 0: /* IRQ enable - probably an irq mask, but only values used are 0xc8 and 0xca */
//      logerror("%08x:  IRQ write %d %08x\n",cpu_get_pc(space->cpu),offset,data);
		raster_enable=(data&0xff)==0xc8; /* 0xca seems to be off */
		break;

	case 1: /* Raster IRQ scanline position, only valid for values between 1 & 239 (0 and 240-256 do NOT generate IRQ's) */
		scanline=(data&0xff);
		if (raster_enable && scanline>0 && scanline<240)
		{
			// needs +16 for the raster to align on captaven intro? (might just be our screen size / visible area / layer offsets need adjusting instead)
			raster_irq_timer->adjust(space->machine->primary_screen->time_until_pos(scanline+16, 320));
		}
		else
			raster_irq_timer->reset();
		break;
	case 2: /* VBL irq ack */
		break;
	}
}

static WRITE32_HANDLER( deco32_sound_w )
{
	soundlatch_w(space,0,data & 0xff);
	cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
}

static READ32_HANDLER( deco32_71_r )
{
	/* Bit 0x80 goes high when sprite DMA is complete, and low
    while it's in progress, we don't bother to emulate it */
	return 0xffffffff;
}

static READ32_HANDLER( captaven_prot_r )
{
	/* Protection/IO chip 75, same as Lemmings & Robocop 2 */
	switch (offset<<2) {
	case 0x0a0: return input_port_read(space->machine, "IN0"); /* Player 1 & 2 controls */
	case 0x158: return input_port_read(space->machine, "IN1"); /* Player 3 & 4 controls */
	case 0xed4: return input_port_read(space->machine, "IN2"); /* Misc */
	}

	logerror("%08x: Unmapped protection read %04x\n",cpu_get_pc(space->cpu),offset<<2);
	return 0xffffffff;
}

static READ32_HANDLER( captaven_soundcpu_r )
{
	/* Top byte - top bit low == sound cpu busy, bottom word is dips */
	return 0xffff0000 | input_port_read(space->machine, "DSW");
}

static READ32_HANDLER( fghthist_control_r )
{
	switch (offset) {
	case 0: return 0xffff0000 | input_port_read(space->machine, "IN0");
	case 1: return 0xffff0000 | input_port_read(space->machine, "IN1"); //check top bits??
	case 2: return 0xfffffffe | eeprom_read_bit(space->machine->device("eeprom"));
	}

	return 0xffffffff;
}

static WRITE32_HANDLER( fghthist_eeprom_w )
{
	if (ACCESSING_BITS_0_7) {
		eeprom_device *device = space->machine->device<eeprom_device>("eeprom");
		eeprom_set_clock_line(device, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
		eeprom_write_bit(device, data & 0x10);
		eeprom_set_cs_line(device, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

		deco32_pri_w(space,0,data&0x1,0xffffffff); /* Bit 0 - layer priority toggle */
	}
	else if (!ACCESSING_BITS_8_15)
	{
		// Volume port
	}
}

/**********************************************************************************/

static READ32_HANDLER( dragngun_service_r )
{
//  logerror("%08x:Read service\n",cpu_get_pc(space->cpu));
	return input_port_read(space->machine, "IN2");
}

static READ32_HANDLER( lockload_gun_mirror_r )
{
//logerror("%08x:Read gun %d\n",cpu_get_pc(space->cpu),offset);
//return ((mame_rand(space->machine)%0xffff)<<16) | mame_rand(space->machine)%0xffff;
	if (offset) /* Mirror of player 1 and player 2 fire buttons */
		return input_port_read(space->machine, "IN4") | ((mame_rand(space->machine)%0xff)<<16);
	return input_port_read(space->machine, "IN3") | input_port_read(space->machine, "LIGHT0_X") | (input_port_read(space->machine, "LIGHT0_X")<<16) | (input_port_read(space->machine, "LIGHT0_X")<<24); //((mame_rand(space->machine)%0xff)<<16);
}

static READ32_HANDLER( dragngun_prot_r )
{
//  logerror("%08x:Read prot %08x (%08x)\n",cpu_get_pc(space->cpu),offset<<1,mem_mask);

	static int strobe=0;
	if (!strobe) strobe=8;
	else strobe=0;

//definitely vblank in locked load

	switch (offset<<1) {
	case 0x140/2: return 0xffff0000 | input_port_read(space->machine, "IN0"); /* IN0 */
	case 0xadc/2: return 0xffff0000 | input_port_read(space->machine, "IN1") | strobe; /* IN1 */
	case 0x6a0/2: return 0xffff0000 | input_port_read(space->machine, "DSW"); /* IN2 (Dip switch) */
	}
	return 0xffffffff;
}

static int dragngun_lightgun_port;

static READ32_HANDLER( dragngun_lightgun_r )
{
	/* Ports 0-3 are read, but seem unused */
	switch (dragngun_lightgun_port) {
	case 4: return input_port_read(space->machine, "LIGHT0_X");
	case 5: return input_port_read(space->machine, "LIGHT1_X");
	case 6: return input_port_read(space->machine, "LIGHT0_Y");
	case 7: return input_port_read(space->machine, "LIGHT1_Y");
	}

//  logerror("Illegal lightgun port %d read \n",dragngun_lightgun_port);
	return 0;
}

static WRITE32_HANDLER( dragngun_lightgun_w )
{
//  logerror("Lightgun port %d\n",dragngun_lightgun_port);
	dragngun_lightgun_port=offset;
}

static READ32_DEVICE_HANDLER( dragngun_eeprom_r )
{
	return 0xfffffffe | eeprom_read_bit(device);
}

static WRITE32_DEVICE_HANDLER( dragngun_eeprom_w )
{
	if (ACCESSING_BITS_0_7) {
		eeprom_set_clock_line(device, (data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
		eeprom_write_bit(device, data & 0x1);
		eeprom_set_cs_line(device, (data & 0x4) ? CLEAR_LINE : ASSERT_LINE);
		return;
	}
	logerror("%s:Write control 1 %08x %08x\n",cpuexec_describe_context(device->machine),offset,data);
}

/**********************************************************************************/

static int tattass_eprom_bit;

static READ32_HANDLER( tattass_prot_r )
{
	switch (offset<<1) {
	case 0x280: return input_port_read(space->machine, "IN0") << 16;
	case 0x4c4: return input_port_read(space->machine, "IN1") << 16;
	case 0x35a: return tattass_eprom_bit << 16;
	}

	logerror("%08x:Read prot %08x (%08x)\n",cpu_get_pc(space->cpu),offset<<1,mem_mask);

	return 0xffffffff;
}

static WRITE32_HANDLER( tattass_prot_w )
{
	/* Only sound port of chip is used - no protection */
	if (offset==0x700/4) {
		/* 'Swap bits 0 and 3 to correct for design error from BSMT schematic' */
		int soundcommand = (data>>16)&0xff;
		soundcommand = BITSWAP8(soundcommand,7,6,5,4,0,2,1,3);
		soundlatch_w(space,0,soundcommand);
	}
}

static WRITE32_HANDLER( tattass_control_w )
{
	static int lastClock=0;
	static char buffer[32];
	static int bufPtr=0;
	static int pendingCommand=0; /* 1 = read, 2 = write */
	static int readBitCount=0;
	static int byteAddr=0;
	eeprom_device *eeprom = space->machine->device<eeprom_device>("eeprom");
	address_space *eeprom_space = eeprom->space();

	/* Eprom in low byte */
	if (mem_mask==0x000000ff) { /* Byte write to low byte only (different from word writing including low byte) */
		/*
            The Tattoo Assassins eprom seems strange...  It's 1024 bytes in size, and 8 bit
            in width, but offers a 'multiple read' mode where a bit stream can be read
            starting at any byte boundary.

            Multiple read mode:
            Write 110aa000      [Read command, top two bits of address, 4 zeroes]
            Write 00000000      [8 zeroes]
            Write aaaaaaaa      [Bottom 8 bits of address]

            Then bits are read back per clock, for as many bits as needed (NOT limited to byte
            boundaries).

            Write mode:
            Write 000aa000      [Write command, top two bits of address, 4 zeroes]
            Write 00000000      [8 zeroes]
            Write aaaaaaaa      [Bottom 8 bits of address]
            Write dddddddd      [8 data bits]

        */
		if ((data&0x40)==0) {
			if (bufPtr) {
				int i;
				logerror("Eprom reset (bit count %d): ",readBitCount);
				for (i=0; i<bufPtr; i++)
					logerror("%s",buffer[i] ? "1" : "0");
				logerror("\n");

			}
			bufPtr=0;
			pendingCommand=0;
			readBitCount=0;
		}

		/* Eprom has been clocked */
		if (lastClock==0 && data&0x20 && data&0x40) {
			if (bufPtr>=32) {
				logerror("Eprom overflow!");
				bufPtr=0;
			}

			/* Handle pending read */
			if (pendingCommand==1) {
				int d=readBitCount/8;
				int m=7-(readBitCount%8);
				int a=(byteAddr+d)%1024;
				int b=eeprom_space->read_byte(a);

				tattass_eprom_bit=(b>>m)&1;

				readBitCount++;
				lastClock=data&0x20;
				return;
			}

			/* Handle pending write */
			if (pendingCommand==2) {
				buffer[bufPtr++]=(data&0x10)>>4;

				if (bufPtr==32) {
					int b=(buffer[24]<<7)|(buffer[25]<<6)|(buffer[26]<<5)|(buffer[27]<<4)
						|(buffer[28]<<3)|(buffer[29]<<2)|(buffer[30]<<1)|(buffer[31]<<0);

					eeprom_space->write_byte(byteAddr, b);
				}
				lastClock=data&0x20;
				return;
			}

			buffer[bufPtr++]=(data&0x10)>>4;
			if (bufPtr==24) {
				/* Decode addr */
				byteAddr=(buffer[3]<<9)|(buffer[4]<<8)
						|(buffer[16]<<7)|(buffer[17]<<6)|(buffer[18]<<5)|(buffer[19]<<4)
						|(buffer[20]<<3)|(buffer[21]<<2)|(buffer[22]<<1)|(buffer[23]<<0);

				/* Check for read command */
				if (buffer[0] && buffer[1]) {
					tattass_eprom_bit=(eeprom_space->read_byte(byteAddr)>>7)&1;
					readBitCount=1;
					pendingCommand=1;
				}

				/* Check for write command */
				else if (buffer[0]==0x0 && buffer[1]==0x0) {
					pendingCommand=2;
				}
				else {
					logerror("Detected unknown eprom command\n");
				}
			}

		} else {
			if (!(data&0x40)) {
				logerror("Cs set low\n");
				bufPtr=0;
			}
		}

		lastClock=data&0x20;
		return;
	}

	/* Volume in high byte */
	if (mem_mask==0x0000ff00) {
		//TODO:  volume attenuation == ((data>>8)&0xff);
		return;
	}

	/* Playfield control - Only written in full word memory accesses */
	deco32_pri_w(space,0,data&0x3,0xffffffff); /* Bit 0 - layer priority toggle, Bit 1 - BG2/3 Joint mode (8bpp) */

	/* Sound board reset control */
	if (data&0x80)
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, CLEAR_LINE);
	else
		cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);

	/* bit 0x4 fade cancel? */
	/* bit 0x8 ?? */
	/* Bit 0x100 ?? */
	//logerror("%08x: %08x data\n",data,mem_mask);
}

/**********************************************************************************/

static READ32_HANDLER( nslasher_prot_r )
{

	switch (offset<<1) {
	case 0x280: return input_port_read(space->machine, "IN0") << 16| 0xffff; /* IN0 */
	case 0x4c4: return input_port_read(space->machine, "IN1") << 16| 0xffff; /* IN1 */
	case 0x35a: return (eeprom_read_bit(space->machine->device("eeprom"))<< 16) | 0xffff; // Debug switch in low word??
	}

	//logerror("%08x: Read unmapped prot %08x (%08x)\n",cpu_get_pc(space->cpu),offset<<1,mem_mask);

	return 0xffffffff;
}

static WRITE32_HANDLER( nslasher_eeprom_w )
{
	if (ACCESSING_BITS_0_7)
	{
		eeprom_device *device = space->machine->device<eeprom_device>("eeprom");
		eeprom_set_clock_line(device, (data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
		eeprom_write_bit(device, data & 0x10);
		eeprom_set_cs_line(device, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

		deco32_pri_w(space,0,data&0x3,0xffffffff); /* Bit 0 - layer priority toggle, Bit 1 - BG2/3 Joint mode (8bpp) */
	}
}


static WRITE32_HANDLER( nslasher_prot_w )
{
	//logerror("%08x:write prot %08x (%08x) %08x\n",cpu_get_pc(space->cpu),offset<<1,mem_mask,data);

	/* Only sound port of chip is used - no protection */
	if (offset==0x700/4) {

		/* bit 1 of nslasher_sound_irq specifies IRQ command writes */
		soundlatch_w(space,0,(data>>16)&0xff);
		nslasher_sound_irq |= 0x02;
		cputag_set_input_line(space->machine, "audiocpu", 0, (nslasher_sound_irq != 0) ? ASSERT_LINE : CLEAR_LINE);
	}
}

/**********************************************************************************/

static ADDRESS_MAP_START( captaven_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM

	AM_RANGE(0x100000, 0x100007) AM_READ(deco32_71_r)
	AM_RANGE(0x100000, 0x100003) AM_WRITE(buffer_spriteram32_w)
	AM_RANGE(0x108000, 0x108003) AM_WRITENOP /* ? */
	AM_RANGE(0x110000, 0x110fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram) /* Sprites */
	AM_RANGE(0x120000, 0x127fff) AM_RAM AM_BASE(&deco32_ram) /* Main RAM */
	AM_RANGE(0x128000, 0x128fff) AM_READ(captaven_prot_r)
	AM_RANGE(0x1280c8, 0x1280cb) AM_WRITE(deco32_sound_w)
	AM_RANGE(0x130000, 0x131fff) AM_RAM_WRITE(deco32_nonbuffered_palette_w) AM_BASE_GENERIC(paletteram) /* Palette RAM */
	AM_RANGE(0x148000, 0x14800f) AM_READWRITE(deco32_irq_controller_r, deco32_irq_controller_w)
	AM_RANGE(0x160000, 0x167fff) AM_RAM /* Extra work RAM */
	AM_RANGE(0x168000, 0x168003) AM_READ(captaven_soundcpu_r)

	AM_RANGE(0x178000, 0x178003) AM_WRITE(deco32_pri_w)

	AM_RANGE(0x180000, 0x18001f) AM_RAM_WRITE(deco32_pf12_control_w) AM_BASE(&deco32_pf12_control)
	AM_RANGE(0x190000, 0x191fff) AM_RAM_WRITE(deco32_pf1_data_w) AM_BASE(&deco32_pf1_data)
	AM_RANGE(0x192000, 0x193fff) AM_WRITE(deco32_pf1_data_w) /* Mirror address - bug in program code */
	AM_RANGE(0x194000, 0x195fff) AM_RAM_WRITE(deco32_pf2_data_w) AM_BASE(&deco32_pf2_data)
	AM_RANGE(0x1a0000, 0x1a1fff) AM_RAM AM_BASE(&deco32_pf1_rowscroll)
	AM_RANGE(0x1a4000, 0x1a5fff) AM_RAM AM_BASE(&deco32_pf2_rowscroll)

	AM_RANGE(0x1c0000, 0x1c001f) AM_RAM_WRITE(deco32_pf34_control_w) AM_BASE(&deco32_pf34_control)
	AM_RANGE(0x1d0000, 0x1d1fff) AM_RAM_WRITE(deco32_pf3_data_w) AM_BASE(&deco32_pf3_data)
	AM_RANGE(0x1e0000, 0x1e1fff) AM_RAM AM_BASE(&deco32_pf3_rowscroll)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fghthist_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x001fff) AM_ROM AM_WRITE(deco32_pf1_data_w)
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM AM_BASE(&deco32_ram)
	AM_RANGE(0x120020, 0x12002f) AM_READ(fghthist_control_r)
	AM_RANGE(0x12002c, 0x12002f) AM_WRITE(fghthist_eeprom_w)
	AM_RANGE(0x1201fc, 0x1201ff) AM_WRITE(deco32_sound_w)

	AM_RANGE(0x140000, 0x140003) AM_WRITENOP /* VBL irq ack */

	AM_RANGE(0x168000, 0x169fff) AM_RAM_WRITE(deco32_buffered_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x16c008, 0x16c00b) AM_WRITE(deco32_palette_dma_w)
	//
	AM_RANGE(0x178000, 0x178fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x179000, 0x179fff) AM_RAM AM_BASE_GENERIC(spriteram2) // ?
	AM_RANGE(0x17c010, 0x17c013) AM_WRITE(buffer_spriteram32_w)

	AM_RANGE(0x182000, 0x183fff) AM_RAM_WRITE(deco32_pf1_data_w) AM_BASE(&deco32_pf1_data)
	AM_RANGE(0x184000, 0x185fff) AM_RAM_WRITE(deco32_pf2_data_w) AM_BASE(&deco32_pf2_data)
	AM_RANGE(0x192000, 0x192fff) AM_RAM AM_BASE(&deco32_pf1_rowscroll)
	AM_RANGE(0x194000, 0x194fff) AM_RAM AM_BASE(&deco32_pf2_rowscroll)
	AM_RANGE(0x1a0000, 0x1a001f) AM_RAM_WRITE(deco32_pf12_control_w) AM_BASE(&deco32_pf12_control)

	AM_RANGE(0x1c2000, 0x1c3fff) AM_RAM_WRITE(deco32_pf3_data_w) AM_BASE(&deco32_pf3_data)
	AM_RANGE(0x1c4000, 0x1c5fff) AM_RAM_WRITE(deco32_pf4_data_w) AM_BASE(&deco32_pf4_data)
	AM_RANGE(0x1d2000, 0x1d2fff) AM_RAM AM_BASE(&deco32_pf3_rowscroll)
	AM_RANGE(0x1d4000, 0x1d4fff) AM_RAM AM_BASE(&deco32_pf4_rowscroll)
	AM_RANGE(0x1e0000, 0x1e001f) AM_RAM_WRITE(deco32_pf34_control_w) AM_BASE(&deco32_pf34_control)

	AM_RANGE(0x16c000, 0x16c01f) AM_READNOP
	AM_RANGE(0x17c000, 0x17c03f) AM_READNOP

	AM_RANGE(0x200000, 0x200fff) AM_READWRITE(deco16_146_fghthist_prot_r, deco16_146_fghthist_prot_w) AM_BASE(&deco32_prot_ram)
	AM_RANGE(0x208800, 0x208803) AM_WRITENOP /* ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( fghthsta_memmap, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM AM_BASE(&deco32_ram)
	AM_RANGE(0x140000, 0x140003) AM_WRITENOP /* VBL irq ack */
	AM_RANGE(0x150000, 0x150003) AM_WRITE(fghthist_eeprom_w) /* Volume port/Eprom */

	AM_RANGE(0x168000, 0x169fff) AM_RAM_WRITE(deco32_buffered_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x16c008, 0x16c00b) AM_WRITE(deco32_palette_dma_w)
	AM_RANGE(0x16c010, 0x16c013) AM_READNOP

	AM_RANGE(0x178000, 0x179fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x17c010, 0x17c013) AM_WRITE(buffer_spriteram32_w)
	AM_RANGE(0x17c020, 0x17c023) AM_READNOP

	AM_RANGE(0x182000, 0x183fff) AM_RAM_WRITE(deco32_pf1_data_w) AM_BASE(&deco32_pf1_data)
	AM_RANGE(0x184000, 0x185fff) AM_RAM_WRITE(deco32_pf2_data_w) AM_BASE(&deco32_pf2_data)
	AM_RANGE(0x192000, 0x192fff) AM_RAM AM_BASE(&deco32_pf1_rowscroll)
	AM_RANGE(0x194000, 0x194fff) AM_RAM AM_BASE(&deco32_pf2_rowscroll)
	AM_RANGE(0x1a0000, 0x1a001f) AM_RAM_WRITE(deco32_pf12_control_w) AM_BASE(&deco32_pf12_control)

	AM_RANGE(0x1c2000, 0x1c3fff) AM_RAM_WRITE(deco32_pf3_data_w) AM_BASE(&deco32_pf3_data)
	AM_RANGE(0x1c4000, 0x1c5fff) AM_RAM_WRITE(deco32_pf4_data_w) AM_BASE(&deco32_pf4_data)
	AM_RANGE(0x1d2000, 0x1d2fff) AM_RAM AM_BASE(&deco32_pf3_rowscroll)
	AM_RANGE(0x1d4000, 0x1d4fff) AM_RAM AM_BASE(&deco32_pf4_rowscroll)
	AM_RANGE(0x1e0000, 0x1e001f) AM_RAM_WRITE(deco32_pf34_control_w) AM_BASE(&deco32_pf34_control)

	AM_RANGE(0x200000, 0x200fff) AM_READWRITE(deco16_146_fghthist_prot_r, deco16_146_fghthist_prot_w) AM_BASE(&deco32_prot_ram)
ADDRESS_MAP_END

// the video drawing (especially sprite) code on this is too slow to cope with proper partial updates
// raster effects appear to need some work on it anyway?
static ADDRESS_MAP_START( dragngun_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM AM_BASE(&deco32_ram)
	AM_RANGE(0x120000, 0x120fff) AM_READ(dragngun_prot_r)
	AM_RANGE(0x1204c0, 0x1204c3) AM_WRITE(deco32_sound_w)
	AM_RANGE(0x128000, 0x12800f) AM_READWRITE(deco32_irq_controller_r, deco32_irq_controller_w)
	AM_RANGE(0x130000, 0x131fff) AM_RAM_WRITE(deco32_buffered_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x138000, 0x138003) AM_NOP /* Palette dma complete in bit 0x8? ack?  return 0 else tight loop */
	AM_RANGE(0x138008, 0x13800b) AM_WRITE(deco32_palette_dma_w)

//  AM_RANGE(0x180000, 0x18001f) AM_RAM_WRITE(deco32_pf12_control_w) AM_BASE(&deco32_pf12_control)
	AM_RANGE(0x180000, 0x18001f) AM_RAM AM_BASE(&deco32_pf12_control)
	AM_RANGE(0x190000, 0x191fff) AM_RAM_WRITE(deco32_pf1_data_w) AM_BASE(&deco32_pf1_data)
	AM_RANGE(0x194000, 0x195fff) AM_RAM_WRITE(deco32_pf2_data_w) AM_BASE(&deco32_pf2_data)
	AM_RANGE(0x1a0000, 0x1a0fff) AM_RAM AM_BASE(&deco32_pf1_rowscroll)
	AM_RANGE(0x1a4000, 0x1a4fff) AM_RAM AM_BASE(&deco32_pf2_rowscroll)

//  AM_RANGE(0x1c0000, 0x1c001f) AM_RAM_WRITE(deco32_pf34_control_w) AM_BASE(&deco32_pf34_control)
	AM_RANGE(0x1c0000, 0x1c001f) AM_RAM AM_BASE(&deco32_pf34_control)
	AM_RANGE(0x1d0000, 0x1d1fff) AM_RAM_WRITE(deco32_pf3_data_w) AM_BASE(&deco32_pf3_data)
	AM_RANGE(0x1d4000, 0x1d5fff) AM_RAM_WRITE(deco32_pf4_data_w) AM_BASE(&deco32_pf4_data)
	AM_RANGE(0x1e0000, 0x1e0fff) AM_RAM AM_BASE(&deco32_pf3_rowscroll)
	AM_RANGE(0x1e4000, 0x1e4fff) AM_RAM AM_BASE(&deco32_pf4_rowscroll)

	AM_RANGE(0x204800, 0x204fff) AM_RAM // ace? 0x10 byte increments only  // 13f ff stuff


	AM_RANGE(0x208000, 0x208fff) AM_RAM AM_BASE(&dragngun_sprite_layout_0_ram)
	AM_RANGE(0x20c000, 0x20cfff) AM_RAM AM_BASE(&dragngun_sprite_layout_1_ram)
	AM_RANGE(0x210000, 0x217fff) AM_RAM AM_BASE(&dragngun_sprite_lookup_0_ram)
	AM_RANGE(0x218000, 0x21ffff) AM_RAM AM_BASE(&dragngun_sprite_lookup_1_ram)
	AM_RANGE(0x220000, 0x221fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram) /* Main spriteram */

	AM_RANGE(0x228000, 0x2283ff) AM_RAM //0x10 byte increments only

	AM_RANGE(0x230000, 0x230003) AM_WRITE(dragngun_spriteram_dma_w)

	AM_RANGE(0x300000, 0x3fffff) AM_ROM

	AM_RANGE(0x400000, 0x400003) AM_DEVREADWRITE8_MODERN("oki3", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x410000, 0x410003) AM_WRITENOP /* Some kind of serial bit-stream - digital volume control? */
	AM_RANGE(0x420000, 0x420003) AM_DEVREADWRITE("eeprom", dragngun_eeprom_r, dragngun_eeprom_w)
	AM_RANGE(0x438000, 0x438003) AM_READ(dragngun_lightgun_r)
	AM_RANGE(0x430000, 0x43001f) AM_WRITE(dragngun_lightgun_w)
	AM_RANGE(0x440000, 0x440003) AM_READ(dragngun_service_r)

	AM_RANGE(0x500000, 0x500003) AM_WRITE(dragngun_sprite_control_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lockload_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM AM_BASE(&deco32_ram)
	AM_RANGE(0x120000, 0x120fff) AM_READ(dragngun_prot_r)
	AM_RANGE(0x1204c0, 0x1204c3) AM_WRITE(deco32_sound_w)
	AM_RANGE(0x128000, 0x12800f) AM_READWRITE(deco32_irq_controller_r, deco32_irq_controller_w)

	AM_RANGE(0x130000, 0x131fff) AM_RAM_WRITE(deco32_buffered_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x138000, 0x138003) AM_READONLY AM_WRITENOP //palette dma complete in bit 0x8? ack?  return 0 else tight loop
	AM_RANGE(0x138008, 0x13800b) AM_WRITE(deco32_palette_dma_w)

	AM_RANGE(0x170000, 0x170007) AM_READ(lockload_gun_mirror_r) /* Not on Dragongun */
	AM_RANGE(0x178008, 0x17800f) AM_WRITENOP /* Gun read ACK's */

//  AM_RANGE(0x180000, 0x18001f) AM_RAM_WRITE(deco32_pf12_control_w) AM_BASE(&deco32_pf12_control)
	AM_RANGE(0x180000, 0x18001f) AM_RAM AM_BASE(&deco32_pf12_control)
	AM_RANGE(0x190000, 0x191fff) AM_RAM_WRITE(deco32_pf1_data_w) AM_BASE(&deco32_pf1_data)
	AM_RANGE(0x194000, 0x195fff) AM_RAM_WRITE(deco32_pf2_data_w) AM_BASE(&deco32_pf2_data)
	AM_RANGE(0x1a0000, 0x1a0fff) AM_RAM AM_BASE(&deco32_pf1_rowscroll)
	AM_RANGE(0x1a4000, 0x1a4fff) AM_RAM AM_BASE(&deco32_pf2_rowscroll)

//  AM_RANGE(0x1c0000, 0x1c001f) AM_RAM_WRITE(deco32_pf34_control_w) AM_BASE(&deco32_pf34_control)
	AM_RANGE(0x1c0000, 0x1c001f) AM_RAM AM_BASE(&deco32_pf34_control)
	AM_RANGE(0x1d0000, 0x1d1fff) AM_RAM_WRITE(deco32_pf3_data_w) AM_BASE(&deco32_pf3_data)
	AM_RANGE(0x1d4000, 0x1d5fff) AM_RAM_WRITE(deco32_pf4_data_w) AM_BASE(&deco32_pf4_data)
	AM_RANGE(0x1e0000, 0x1e0fff) AM_RAM AM_BASE(&deco32_pf3_rowscroll)
	AM_RANGE(0x1e4000, 0x1e4fff) AM_RAM AM_BASE(&deco32_pf4_rowscroll)

	AM_RANGE(0x204800, 0x204fff) AM_RAM				//0x10 byte increments only
	AM_RANGE(0x208000, 0x208fff) AM_RAM AM_BASE(&dragngun_sprite_layout_0_ram)
	AM_RANGE(0x20c000, 0x20cfff) AM_RAM AM_BASE(&dragngun_sprite_layout_1_ram)
	AM_RANGE(0x210000, 0x217fff) AM_RAM AM_BASE(&dragngun_sprite_lookup_0_ram)
	AM_RANGE(0x218000, 0x21ffff) AM_RAM AM_BASE(&dragngun_sprite_lookup_1_ram)
	AM_RANGE(0x220000, 0x221fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram) /* Main spriteram */

	AM_RANGE(0x228000, 0x2283ff) AM_RAM				//0x10 byte increments only
	AM_RANGE(0x230000, 0x230003) AM_WRITE(dragngun_spriteram_dma_w)

	AM_RANGE(0x300000, 0x3fffff) AM_ROM

	AM_RANGE(0x400000, 0x400003) AM_DEVREADWRITE8_MODERN("oki3", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x420000, 0x420003) AM_DEVREADWRITE("eeprom", dragngun_eeprom_r, dragngun_eeprom_w)
//  AM_RANGE(0x430000, 0x43001f) AM_WRITE(dragngun_lightgun_w)
//  AM_RANGE(0x438000, 0x438003) AM_READ(dragngun_lightgun_r)
	AM_RANGE(0x440000, 0x440003) AM_READ(dragngun_service_r)
	AM_RANGE(0x500000, 0x500003) AM_WRITE(dragngun_sprite_control_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tattass_map, ADDRESS_SPACE_PROGRAM, 32 )

	AM_RANGE(0x000000, 0x0f7fff) AM_ROM
	AM_RANGE(0x0f8000, 0x0fffff) AM_ROM AM_WRITENOP
	AM_RANGE(0x100000, 0x11ffff) AM_RAM AM_BASE(&deco32_ram)
	AM_RANGE(0x120000, 0x120003) AM_NOP				/* ACIA (unused) */
	AM_RANGE(0x130000, 0x130003) AM_WRITENOP		/* Coin port (unused?) */
	AM_RANGE(0x140000, 0x140003) AM_WRITENOP		/* Vblank ack */
	AM_RANGE(0x150000, 0x150003) AM_WRITE(tattass_control_w) /* Volume port/Eprom/Priority */

	AM_RANGE(0x162000, 0x162fff) AM_RAM				/* 'Jack' RAM!? */
	AM_RANGE(0x163000, 0x16309f) AM_RAM_WRITE(deco32_ace_ram_w) AM_BASE(&deco32_ace_ram)
	AM_RANGE(0x164000, 0x164003) AM_WRITENOP /* Palette control BG2/3 ($1a constant) */
	AM_RANGE(0x164004, 0x164007) AM_WRITENOP /* Palette control Obj1 ($6 constant) */
	AM_RANGE(0x164008, 0x16400b) AM_WRITENOP /* Palette control Obj2 ($5 constant) */
	AM_RANGE(0x16400c, 0x16400f) AM_WRITENOP
	AM_RANGE(0x168000, 0x169fff) AM_RAM_WRITE(deco32_buffered_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x16c000, 0x16c003) AM_WRITENOP
	AM_RANGE(0x16c008, 0x16c00b) AM_WRITE(deco32_palette_dma_w)

	AM_RANGE(0x170000, 0x171fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x174000, 0x174003) AM_WRITENOP /* Sprite DMA mode (2) */
	AM_RANGE(0x174010, 0x174013) AM_WRITE(buffer_spriteram32_w)
	AM_RANGE(0x174018, 0x17401b) AM_WRITENOP /* Sprite 'CPU' (unused) */
	AM_RANGE(0x178000, 0x179fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram2)

	AM_RANGE(0x17c000, 0x17c003) AM_WRITENOP /* Sprite DMA mode (2) */
	AM_RANGE(0x17c010, 0x17c013) AM_WRITE(buffer_spriteram32_2_w)
	AM_RANGE(0x17c018, 0x17c01b) AM_WRITENOP /* Sprite 'CPU' (unused) */

	AM_RANGE(0x182000, 0x183fff) AM_RAM_WRITE(deco32_pf1_data_w) AM_BASE(&deco32_pf1_data)
	AM_RANGE(0x184000, 0x185fff) AM_RAM_WRITE(deco32_pf2_data_w) AM_BASE(&deco32_pf2_data)
	AM_RANGE(0x192000, 0x193fff) AM_RAM AM_BASE(&deco32_pf1_rowscroll)
	AM_RANGE(0x194000, 0x195fff) AM_RAM AM_BASE(&deco32_pf2_rowscroll)
	AM_RANGE(0x1a0000, 0x1a001f) AM_RAM_WRITE(deco32_pf12_control_w) AM_BASE(&deco32_pf12_control)

	AM_RANGE(0x1c2000, 0x1c3fff) AM_RAM_WRITE(deco32_pf3_data_w) AM_BASE(&deco32_pf3_data)
	AM_RANGE(0x1c4000, 0x1c5fff) AM_RAM_WRITE(deco32_pf4_data_w) AM_BASE(&deco32_pf4_data)
	AM_RANGE(0x1d2000, 0x1d3fff) AM_RAM AM_BASE(&deco32_pf3_rowscroll)
	AM_RANGE(0x1d4000, 0x1d5fff) AM_RAM AM_BASE(&deco32_pf4_rowscroll)
	AM_RANGE(0x1e0000, 0x1e001f) AM_RAM_WRITE(deco32_pf34_control_w) AM_BASE(&deco32_pf34_control)

	AM_RANGE(0x200000, 0x200fff) AM_READWRITE(tattass_prot_r, tattass_prot_w) AM_BASE(&deco32_prot_ram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nslasher_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x11ffff) AM_RAM AM_BASE(&deco32_ram)
	AM_RANGE(0x120000, 0x1200ff) AM_NOP							/* ACIA (unused) */
	AM_RANGE(0x140000, 0x140003) AM_WRITENOP					/* Vblank ack */
	AM_RANGE(0x150000, 0x150003) AM_WRITE(nslasher_eeprom_w)	/* Volume port/Eprom/Priority */

	AM_RANGE(0x163000, 0x16309f) AM_RAM_WRITE(deco32_ace_ram_w) AM_BASE(&deco32_ace_ram) /* 'Ace' RAM!? */
	AM_RANGE(0x164000, 0x164003) AM_WRITENOP /* Palette control BG2/3 ($1a constant) */
	AM_RANGE(0x164004, 0x164007) AM_WRITENOP /* Palette control Obj1 ($4 constant) */
	AM_RANGE(0x164008, 0x16400b) AM_WRITENOP /* Palette control Obj2 ($6 constant) */
	AM_RANGE(0x16400c, 0x16400f) AM_WRITENOP
	AM_RANGE(0x168000, 0x169fff) AM_RAM_WRITE(deco32_buffered_palette_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x16c000, 0x16c003) AM_WRITENOP
	AM_RANGE(0x16c008, 0x16c00b) AM_WRITE(deco32_palette_dma_w)

	AM_RANGE(0x170000, 0x171fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x174000, 0x174003) AM_WRITENOP /* Sprite DMA mode (2) */
	AM_RANGE(0x174010, 0x174013) AM_WRITE(buffer_spriteram32_w)
	AM_RANGE(0x174018, 0x17401b) AM_WRITENOP /* Sprite 'CPU' (unused) */
	AM_RANGE(0x178000, 0x179fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram2)
	AM_RANGE(0x17c000, 0x17c003) AM_WRITENOP /* Sprite DMA mode (2) */
	AM_RANGE(0x17c010, 0x17c013) AM_WRITE(buffer_spriteram32_2_w)
	AM_RANGE(0x17c018, 0x17c01b) AM_WRITENOP /* Sprite 'CPU' (unused) */

	AM_RANGE(0x182000, 0x183fff) AM_RAM_WRITE(deco32_pf1_data_w) AM_BASE(&deco32_pf1_data)
	AM_RANGE(0x184000, 0x185fff) AM_RAM_WRITE(deco32_pf2_data_w) AM_BASE(&deco32_pf2_data)
	AM_RANGE(0x192000, 0x193fff) AM_RAM AM_BASE(&deco32_pf1_rowscroll)
	AM_RANGE(0x194000, 0x195fff) AM_RAM AM_BASE(&deco32_pf2_rowscroll)
	AM_RANGE(0x1a0000, 0x1a001f) AM_RAM_WRITE(deco32_pf12_control_w) AM_BASE(&deco32_pf12_control)

	AM_RANGE(0x1c2000, 0x1c3fff) AM_RAM_WRITE(deco32_pf3_data_w) AM_BASE(&deco32_pf3_data)
	AM_RANGE(0x1c4000, 0x1c5fff) AM_RAM_WRITE(deco32_pf4_data_w) AM_BASE(&deco32_pf4_data)
	AM_RANGE(0x1d2000, 0x1d3fff) AM_RAM AM_BASE(&deco32_pf3_rowscroll)
	AM_RANGE(0x1d4000, 0x1d5fff) AM_RAM AM_BASE(&deco32_pf4_rowscroll)
	AM_RANGE(0x1e0000, 0x1e001f) AM_RAM_WRITE(deco32_pf34_control_w) AM_BASE(&deco32_pf34_control)

	AM_RANGE(0x200000, 0x200fff) AM_READWRITE(nslasher_prot_r, nslasher_prot_w) AM_BASE(&deco32_prot_ram)
ADDRESS_MAP_END

/******************************************************************************/

static UINT8 bsmt_latch;
static UINT8 bsmt_reset;

static WRITE8_HANDLER(deco32_bsmt_reset_w)
{
	UINT8 diff = data ^ bsmt_reset;
	bsmt_reset = data;
	if ((diff & 0x80) && !(data & 0x80))
		devtag_reset(space->machine, "bsmt");
}

static WRITE8_HANDLER(deco32_bsmt0_w)
{
	bsmt_latch = data;
}

static WRITE8_DEVICE_HANDLER(deco32_bsmt1_w)
{
	bsmt2000_data_w(device, offset^ 0xff, ((bsmt_latch << 8) | data), 0xffff);
	cputag_set_input_line(device->machine, "audiocpu", M6809_IRQ_LINE, HOLD_LINE); /* BSMT is ready */
}

static READ8_HANDLER(deco32_bsmt_status_r)
{
	return 0x80;
}

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE_MODERN("oki1", okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_DEVREADWRITE_MODERN("oki2", okim6295_device, read, write)
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8")
	AM_RANGE(0x1fec00, 0x1fec01) AM_WRITE(h6280_timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_WRITE(h6280_irq_status_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tattass_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_WRITE(deco32_bsmt_reset_w)
	AM_RANGE(0x2002, 0x2003) AM_READ(soundlatch_r)
	AM_RANGE(0x2006, 0x2007) AM_READ(deco32_bsmt_status_r)
	AM_RANGE(0x6000, 0x6000) AM_WRITE(deco32_bsmt0_w)
	AM_RANGE(0xa000, 0xa0ff) AM_DEVWRITE("bsmt", deco32_bsmt1_w)
	AM_RANGE(0x2000, 0xffff) AM_ROM
ADDRESS_MAP_END

static READ8_HANDLER(latch_r)
{
	/* bit 1 of nslasher_sound_irq specifies IRQ command writes */
	nslasher_sound_irq &= ~0x02;
	cputag_set_input_line(space->machine, "audiocpu", 0, (nslasher_sound_irq != 0) ? ASSERT_LINE : CLEAR_LINE);
	return soundlatch_r(space,0);
}

static ADDRESS_MAP_START( nslasher_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xb000, 0xb000) AM_DEVREADWRITE_MODERN("oki1", okim6295_device, read, write)
	AM_RANGE(0xc000, 0xc000) AM_DEVREADWRITE_MODERN("oki2", okim6295_device, read, write)
	AM_RANGE(0xd000, 0xd000) AM_READ(latch_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nslasher_io_sound, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_REGION("audiocpu", 0)
ADDRESS_MAP_END

/**********************************************************************************/

/* Notes (2002.02.05) :

When the "Continue Coin" Dip Switch is set to "2 Start/1 Continue",
the "Coinage" Dip Switches have no effect.

START, BUTTON1 and COIN effects :

  2 players, common coin slots

STARTn starts a game for player n. It adds 100 energy points each time it is pressed
(provided there are still some credits, and energy is <= 900).

BUTTON1n selects the character for player n.

COIN1n adds credit(s)/coin(s).

  2 players, individual coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It also adds 100 energy points for each credit
inserted for the player. It then selects the character for player n.

COIN1n adds 100 energy points (based on "Coinage") for player n when ingame if energy
<= 900, else adds credit(s)/coin(s) for player n.

  4 players, common coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It gives 100 energy points. It then selects the
character for player n.

  4 players, individual coin slots

NO STARTn button !

BUTTON1n starts a game for player n. It also adds 100 energy points for each credit
inserted for the player. It then selects the character for player n.

COIN1n adds 100 energy points (based on "Coinage") for player n when ingame if energy
<= 900, else adds credit(s)/coin(s) for player n.

*/

static INPUT_PORTS_START( captaven )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")	/* Dip switch bank 1 */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Continue Coin" )
	PORT_DIPSETTING(      0x0080, "1 Start/1 Continue" )
	PORT_DIPSETTING(      0x0000, "2 Start/1 Continue" )

	/* Dip switch bank 2 */
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Coin Slots" )
	PORT_DIPSETTING(      0x1000, "Common" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x2000, 0x2000, "Play Mode" )
	PORT_DIPSETTING(      0x2000, "2 Player" )
	PORT_DIPSETTING(      0x0000, "4 Player" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( fghthist )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dragngun )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) //check  //test BUTTON F2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) /* Would be a dipswitch, but only 1 present on board */
	PORT_DIPNAME( 0x0100, 0x0000, "Reset" ) /* Behaves like Reset */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Stage Select" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) //check  //test BUTTON F2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( lockload )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) //check  //test BUTTON F2
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED ) /* Would be a dipswitch, but only 1 present on board */
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, "Reset" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Debug Mode" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_VBLANK ) //IPT_VBLANK )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0x00f8, IP_ACTIVE_LOW, IPT_UNUSED ) //check  //test BUTTON F2

	PORT_START("IN3")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* mirror of fire buttons */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_START("IN4")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( tattass )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( nslasher )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

/**********************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8	/* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 16, 0, 24, 8 },
	{ 64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+4, 64*8+5, 64*8+6, 64*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout spritelayout_5bpp =
{
	16,16,
	RGN_FRAC(1,2),
	5,
	{ RGN_FRAC(1,2), 16, 0, 24, 8 },
	{ 64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+4, 64*8+5, 64*8+6, 64*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static const gfx_layout tilelayout2 =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(1,4)+0, 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static const gfx_layout spritelayout2 =
{
	16,16,
	RGN_FRAC(1,5),
	5,
	{ 0x800000*8, 0x600000*8, 0x400000*8, 0x200000*8, 0 },
	{
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
		0,1,2,3,4,5,6,7
	},
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout spritelayout4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{0,1,2,3},
	{3*8,2*8,1*8,0*8,7*8,6*8,5*8,4*8,
	 11*8,10*8,9*8,8*8,15*8,14*8,13*8,12*8},
	{0*128,1*128,2*128,3*128,4*128,5*128,6*128,7*128,
	 8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*16*8
};

static const gfx_layout spritelayout5 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{4,5,6,7},
	{3*8,2*8,1*8,0*8,7*8,6*8,5*8,4*8,
	 11*8,10*8,9*8,8*8,15*8,14*8,13*8,12*8},
	{0*128,1*128,2*128,3*128,4*128,5*128,6*128,7*128,
	 8*128,9*128,10*128,11*128,12*128,13*128,14*128,15*128 },
	16*16*8
};

static GFXDECODE_START( captaven )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        512, 32 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,        512, 32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout2,      1024,  4 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,        0, 32 )	/* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( fghthist )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,          0,  16 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,        256,  16 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,        512,  32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,     1024, 128 )	/* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( dragngun )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,        512, 16 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,        768, 16 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout2,      1024,  4 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout4,       0, 32 )	/* Sprites 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout5,       0, 32 )	/* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( tattass )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,          0, 32 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,          0, 32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,        512, 32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout2,    1536, 16 )	/* Sprites 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout,     1024+256, 32 )	/* Sprites 16x16 */
GFXDECODE_END

static GFXDECODE_START( nslasher )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,          0, 32 )	/* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,          0, 32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,        512, 32 )	/* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout_5bpp,1024, 16 )	/* Sprites 16x16 */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout,     1536, 32 )	/* Sprites 16x16 */
GFXDECODE_END

/**********************************************************************************/

static void sound_irq(running_device *device, int state)
{
	cputag_set_input_line(device->machine, "audiocpu", 1, state); /* IRQ 2 */
}

static void sound_irq_nslasher(running_device *device, int state)
{
	/* bit 0 of nslasher_sound_irq specifies IRQ from sound chip */
	if (state)
		nslasher_sound_irq |= 0x01;
	else
		nslasher_sound_irq &= ~0x01;
	cputag_set_input_line(device->machine, "audiocpu", 0, (nslasher_sound_irq != 0) ? ASSERT_LINE : CLEAR_LINE);
}

static WRITE8_DEVICE_HANDLER( sound_bankswitch_w )
{
	okim6295_device *oki1 = device->machine->device<okim6295_device>("oki1");
	okim6295_device *oki2 = device->machine->device<okim6295_device>("oki2");
	oki1->set_bank_base(((data >> 0)& 1) * 0x40000);
	oki2->set_bank_base(((data >> 1)& 1) * 0x40000);
}

static const ym2151_interface ym2151_config =
{
	sound_irq,
	sound_bankswitch_w
};

static const ym2151_interface ym2151_interface_nslasher =
{
	sound_irq_nslasher,
	sound_bankswitch_w
};

static const eeprom_interface eeprom_interface_tattass =
{
	10,				// address bits 10  ==> } 1024 byte eprom
	8,				// data bits    8
};

/**********************************************************************************/

static MACHINE_RESET( deco32 )
{
	raster_irq_timer = machine->device<timer_device>("int_timer");
}

static INTERRUPT_GEN( deco32_vbl_interrupt )
{
	cpu_set_input_line(device, ARM_IRQ_LINE, HOLD_LINE);
}

static INTERRUPT_GEN( tattass_snd_interrupt )
{
	cpu_set_input_line(device, M6809_FIRQ_LINE, HOLD_LINE);
}

static MACHINE_CONFIG_START( captaven, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM, XTAL_28MHz/4) /* verified on pcb (Data East 101 custom)*/
	MDRV_CPU_PROGRAM_MAP(captaven_map)
	MDRV_CPU_VBLANK_INT("screen", deco32_vbl_interrupt)

	MDRV_CPU_ADD("audiocpu", H6280, XTAL_32_22MHz/4/3)  /* pin 10 is 32mhz/4, pin 14 is High so internal divisor is 3 (verified on pcb) */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_RESET(deco32)

	MDRV_TIMER_ADD("int_timer", interrupt_gen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(42*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(captaven)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(captaven)
	MDRV_VIDEO_EOF(captaven)
	MDRV_VIDEO_UPDATE(captaven)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, XTAL_32_22MHz/9) /* verified on pcb */
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.42)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.42)

	MDRV_OKIM6295_ADD("oki1", XTAL_32_22MHz/32, OKIM6295_PIN7_HIGH)  /* verified on pcb; pin 7 is floating to 2.5V (left unconnected), so I presume High */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", XTAL_32_22MHz/16, OKIM6295_PIN7_HIGH) /* verified on pcb; pin 7 is floating to 2.5V (left unconnected), so I presume High */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fghthist, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM, 28000000/4)
	MDRV_CPU_PROGRAM_MAP(fghthist_map)
	MDRV_CPU_VBLANK_INT("screen", deco32_vbl_interrupt)

	MDRV_CPU_ADD("audiocpu", H6280, 32220000/8)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(42*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(fghthist)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(fghthist)
	MDRV_VIDEO_UPDATE(fghthist)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.42)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.42)

	MDRV_OKIM6295_ADD("oki1", 32220000/32, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", 32220000/16, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( fghthsta, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM, 28000000/4)
	MDRV_CPU_PROGRAM_MAP(fghthsta_memmap)
	MDRV_CPU_VBLANK_INT("screen", deco32_vbl_interrupt)

	MDRV_CPU_ADD("audiocpu", H6280, 32220000/8)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(42*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(fghthist)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(fghthist)
	MDRV_VIDEO_UPDATE(fghthist)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.42)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.42)

	MDRV_OKIM6295_ADD("oki1", 32220000/32, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", 32220000/16, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dragngun, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM, 28000000/4)
	MDRV_CPU_PROGRAM_MAP(dragngun_map)
	MDRV_CPU_VBLANK_INT("screen", deco32_vbl_interrupt)

	MDRV_CPU_ADD("audiocpu", H6280, 32220000/8)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_RESET(deco32)
	MDRV_EEPROM_93C46_ADD("eeprom")

	MDRV_TIMER_ADD("int_timer", interrupt_gen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(42*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(dragngun)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(dragngun)
	MDRV_VIDEO_UPDATE(dragngun)
	MDRV_VIDEO_EOF(dragngun)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.42)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.42)

	MDRV_OKIM6295_ADD("oki1", 32220000/32, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", 32220000/16, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)

	MDRV_OKIM6295_ADD("oki3", 32220000/32, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( lockload, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM, 28000000/4)
	MDRV_CPU_PROGRAM_MAP(lockload_map)
	MDRV_CPU_VBLANK_INT_HACK(deco32_vbl_interrupt,2) // From 2

	MDRV_CPU_ADD("audiocpu", H6280, 32220000/8)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_MACHINE_RESET(deco32)
	MDRV_EEPROM_93C46_ADD("eeprom")

	MDRV_TIMER_ADD("int_timer", interrupt_gen)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(42*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(dragngun)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(lockload)
	MDRV_VIDEO_UPDATE(dragngun)
	MDRV_VIDEO_EOF(dragngun)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.42)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.42)

	MDRV_OKIM6295_ADD("oki1", 32220000/32, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MDRV_OKIM6295_ADD("oki2", 32220000/16, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)

	MDRV_OKIM6295_ADD("oki3", 32220000/32, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tattass, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM, 28000000/4) /* Unconfirmed */
	MDRV_CPU_PROGRAM_MAP(tattass_map)
	MDRV_CPU_VBLANK_INT("screen", deco32_vbl_interrupt)

	MDRV_CPU_ADD("audiocpu", M6809, 2000000)
	MDRV_CPU_PROGRAM_MAP(tattass_sound_map)
	MDRV_CPU_PERIODIC_INT(tattass_snd_interrupt, 489) /* Fixed FIRQ of 489Hz as measured on real (pinball) machine */

	MDRV_EEPROM_ADD("eeprom", eeprom_interface_tattass)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(42*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(tattass)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(nslasher)
	MDRV_VIDEO_UPDATE(nslasher)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("bsmt", BSMT2000, 24000000)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( nslasher, driver_device )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", ARM, 28322000/4)
	MDRV_CPU_PROGRAM_MAP(nslasher_map)
	MDRV_CPU_VBLANK_INT("screen", deco32_vbl_interrupt)

	MDRV_CPU_ADD("audiocpu", Z80, 32220000/9)
	MDRV_CPU_PROGRAM_MAP(nslasher_sound)
	MDRV_CPU_IO_MAP(nslasher_io_sound)

	MDRV_QUANTUM_TIME(HZ(6000))	/* to improve main<->audio comms */

	MDRV_EEPROM_93C46_ADD("eeprom")

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(42*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(nslasher)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(nslasher)
	MDRV_VIDEO_UPDATE(nslasher)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 32220000/9)
	MDRV_SOUND_CONFIG(ym2151_interface_nslasher)
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.40)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.40)

	MDRV_OKIM6295_ADD("oki1", 32220000/32, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.80)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.80)

	MDRV_OKIM6295_ADD("oki2", 32220000/16, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.10)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.10)
MACHINE_CONFIG_END

/**********************************************************************************/

ROM_START( captaven )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "hn_00-4.1e",	0x000000, 0x20000, CRC(147fb094) SHA1(6bd759c42f4b7f9e1c3f2d3ece0b3ec72de1a982) )
	ROM_LOAD32_BYTE( "hn_01-4.1h",	0x000001, 0x20000, CRC(11ecdb95) SHA1(832b56f05ae7e15e67fbdd321da8c1cc5e7629a0) )
	ROM_LOAD32_BYTE( "hn_02-4.1k",	0x000002, 0x20000, CRC(35d2681f) SHA1(3af7d959dc4842238a7f79926adf449cb7f0b2e9) )
	ROM_LOAD32_BYTE( "hn_03-4.1m",	0x000003, 0x20000, CRC(3b59ba05) SHA1(400e868e59977e56a4fa1870321c643983ba4162) )
	ROM_LOAD32_BYTE( "man-12.3e",	0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",	0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",	0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",	0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "hj_08.17k",	0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "man-00.8a",	0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) /* Encrypted tiles */

	ROM_REGION( 0x500000, "gfx2", 0 )
	ROM_LOAD( "man-05.16a",	0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x140000,  0x40000 )
	ROM_CONTINUE(			0x280000,  0x40000 )
	ROM_CONTINUE(			0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a",	0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x180000,  0x40000 )
	ROM_CONTINUE(			0x2c0000,  0x40000 )
	ROM_CONTINUE(			0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a",	0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x1c0000,  0x40000 )
	ROM_CONTINUE(			0x300000,  0x40000 )
	ROM_CONTINUE(			0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a",	0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x200000,  0x40000 )
	ROM_CONTINUE(			0x340000,  0x40000 )
	ROM_CONTINUE(			0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a",	0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x240000,  0x40000 )
	ROM_CONTINUE(			0x380000,  0x40000 )
	ROM_CONTINUE(			0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "man-06.17a",	0x000000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD16_BYTE( "man-07.18a",	0x000001,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD16_BYTE( "man-08.17c",	0x200000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD16_BYTE( "man-09.21c",	0x200001,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k",	0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k",	0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )
ROM_END

ROM_START( captavena )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "hn_00.e1",	0x000000, 0x20000, CRC(12dd0c71) SHA1(77bd0e5f1b105ec70de5e76cb9c8138f02a496be) )
	ROM_LOAD32_BYTE( "hn_01.h1",	0x000001, 0x20000, CRC(ac5ea492) SHA1(e08fa2b3e3a40cba6dcdf07049d67056d59ed72a) )
	ROM_LOAD32_BYTE( "hn_02.k1",	0x000002, 0x20000, CRC(0c5e13f6) SHA1(d9ebf503db7da8663f45fe307e432545651cfc13) )
	ROM_LOAD32_BYTE( "hn_03.l1",	0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",	0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",	0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",	0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",	0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "hj_08.17k",	0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "man-00.8a",	0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) /* Encrypted tiles */

	ROM_REGION( 0x500000, "gfx2", 0 )
	ROM_LOAD( "man-05.16a",	0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x140000,  0x40000 )
	ROM_CONTINUE(			0x280000,  0x40000 )
	ROM_CONTINUE(			0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a",	0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x180000,  0x40000 )
	ROM_CONTINUE(			0x2c0000,  0x40000 )
	ROM_CONTINUE(			0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a",	0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x1c0000,  0x40000 )
	ROM_CONTINUE(			0x300000,  0x40000 )
	ROM_CONTINUE(			0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a",	0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x200000,  0x40000 )
	ROM_CONTINUE(			0x340000,  0x40000 )
	ROM_CONTINUE(			0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a",	0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x240000,  0x40000 )
	ROM_CONTINUE(			0x380000,  0x40000 )
	ROM_CONTINUE(			0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "man-06.17a",	0x000000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD16_BYTE( "man-07.18a",	0x000001,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD16_BYTE( "man-08.17c",	0x200000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD16_BYTE( "man-09.21c",	0x200001,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k",	0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k",	0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )
ROM_END

ROM_START( captavene )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "hg_00-4.1e",	0x000000, 0x20000, CRC(7008d43c) SHA1(a39143e13075ebc58ecc576391f04d2649675dfb) )
	ROM_LOAD32_BYTE( "hg_01-4.1h",	0x000001, 0x20000, CRC(53dc1042) SHA1(4547ad20e5bc3b9cedae53f73f1628fa3493aafa) )
	ROM_LOAD32_BYTE( "hg_02-4.1k",	0x000002, 0x20000, CRC(9e3f9ee2) SHA1(a56a68bdac58a337be48b346b6939c3f68da8e9d) )
	ROM_LOAD32_BYTE( "hg_03-4.1m",	0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",	0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",	0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",	0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",	0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "hj_08.17k",	0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "man-00.8a",	0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) /* Encrypted tiles */

	ROM_REGION( 0x500000, "gfx2", 0 )
	ROM_LOAD( "man-05.16a",	0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x140000,  0x40000 )
	ROM_CONTINUE(			0x280000,  0x40000 )
	ROM_CONTINUE(			0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a",	0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x180000,  0x40000 )
	ROM_CONTINUE(			0x2c0000,  0x40000 )
	ROM_CONTINUE(			0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a",	0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x1c0000,  0x40000 )
	ROM_CONTINUE(			0x300000,  0x40000 )
	ROM_CONTINUE(			0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a",	0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x200000,  0x40000 )
	ROM_CONTINUE(			0x340000,  0x40000 )
	ROM_CONTINUE(			0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a",	0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x240000,  0x40000 )
	ROM_CONTINUE(			0x380000,  0x40000 )
	ROM_CONTINUE(			0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "man-06.17a",	0x000000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD16_BYTE( "man-07.18a",	0x000001,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD16_BYTE( "man-08.17c",	0x200000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD16_BYTE( "man-09.21c",	0x200001,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k",	0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k",	0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8a.12l", 0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8a.4h",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8a.5h",  0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r8b.14c", 0x0600, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( captavenu )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "hh_00-19.1e",	0x000000, 0x20000, CRC(08b870e0) SHA1(44c837e3c5dfc9764d89b0ebb3e9b7a40fe4d76f) )
	ROM_LOAD32_BYTE( "hh_01-19.1h",	0x000001, 0x20000, CRC(0dc0feca) SHA1(cb1c97aac59dabcf6c37bc1562cf2f62bca951f1) )
	ROM_LOAD32_BYTE( "hh_02-19.1k",	0x000002, 0x20000, CRC(26ef94c0) SHA1(985fae62a6a7ca7e1e64dba2db053b08206c65e7) )
	ROM_LOAD32_BYTE( "hn_03-4.1m",	0x000003, 0x20000, CRC(3b59ba05) SHA1(400e868e59977e56a4fa1870321c643983ba4162) )
	ROM_LOAD32_BYTE( "man-12.3e",	0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",	0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",	0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",	0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "hj_08.17k",	0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "man-00.8a",	0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) /* Encrypted tiles */

	ROM_REGION( 0x500000, "gfx2", 0 )
	ROM_LOAD( "man-05.16a",	0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x140000,  0x40000 )
	ROM_CONTINUE(			0x280000,  0x40000 )
	ROM_CONTINUE(			0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a",	0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x180000,  0x40000 )
	ROM_CONTINUE(			0x2c0000,  0x40000 )
	ROM_CONTINUE(			0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a",	0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x1c0000,  0x40000 )
	ROM_CONTINUE(			0x300000,  0x40000 )
	ROM_CONTINUE(			0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a",	0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x200000,  0x40000 )
	ROM_CONTINUE(			0x340000,  0x40000 )
	ROM_CONTINUE(			0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a",	0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x240000,  0x40000 )
	ROM_CONTINUE(			0x380000,  0x40000 )
	ROM_CONTINUE(			0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "man-06.17a",	0x000000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD16_BYTE( "man-07.18a",	0x000001,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD16_BYTE( "man-08.17c",	0x200000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD16_BYTE( "man-09.21c",	0x200001,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k",	0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k",	0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )
ROM_END

ROM_START( captavenuu )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "hh-00.1e",	0x000000, 0x20000, CRC(c34da654) SHA1(a1988a6a45991db6dee10b484049f6703b4671c9) )
	ROM_LOAD32_BYTE( "hh-01.1h",	0x000001, 0x20000, CRC(55abe63f) SHA1(98772eff3ebb5a4f243c7a77d398eb142d1505cb) )
	ROM_LOAD32_BYTE( "hh-02.1k",	0x000002, 0x20000, CRC(6096a9fb) SHA1(aa81189b9c185dc5d59f888afcb17a1e4935c241) )
	ROM_LOAD32_BYTE( "hh-03.1m",	0x000003, 0x20000, CRC(93631ded) SHA1(b4c8a6cbf586f895e637c0ed38f0842327624423) )
	ROM_LOAD32_BYTE( "man-12.3e",	0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",	0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",	0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",	0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "hj_08.17k",	0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "man-00.8a",	0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) /* Encrypted tiles */

	ROM_REGION( 0x500000, "gfx2", 0 )
	ROM_LOAD( "man-05.16a",	0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x140000,  0x40000 )
	ROM_CONTINUE(			0x280000,  0x40000 )
	ROM_CONTINUE(			0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a",	0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x180000,  0x40000 )
	ROM_CONTINUE(			0x2c0000,  0x40000 )
	ROM_CONTINUE(			0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a",	0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x1c0000,  0x40000 )
	ROM_CONTINUE(			0x300000,  0x40000 )
	ROM_CONTINUE(			0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a",	0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x200000,  0x40000 )
	ROM_CONTINUE(			0x340000,  0x40000 )
	ROM_CONTINUE(			0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a",	0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x240000,  0x40000 )
	ROM_CONTINUE(			0x380000,  0x40000 )
	ROM_CONTINUE(			0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "man-06.17a",	0x000000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD16_BYTE( "man-07.18a",	0x000001,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD16_BYTE( "man-08.17c",	0x200000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD16_BYTE( "man-09.21c",	0x200001,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k",	0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k",	0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )
ROM_END

ROM_START( captavenua )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "hh_00-4.2e",   0x000000, 0x20000, CRC(0e1acc05) SHA1(7eb6206efad233f9f4ee51102f9fe6b58f0719ea) )
	ROM_LOAD32_BYTE( "hh_01-4.2h",   0x000001, 0x20000, CRC(4ff0351d) SHA1(15fc2662ff0d32986c4d4d074b985ad853da34e1) )
	ROM_LOAD32_BYTE( "hh_02-4.2k",   0x000002, 0x20000, CRC(e84c0665) SHA1(d846f04315af49abeca00314b3d23e1d8c638dcd) )
	ROM_LOAD32_BYTE( "hh_03-4.2m",   0x000003, 0x20000, CRC(bc050740) SHA1(bee425e76734251444c9cfa9287e1eb9383625bc) )
	ROM_LOAD32_BYTE( "man-12.3e",	0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",	0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",	0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",	0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "hj_08.17k",	0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "man-00.8a",	0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) /* Encrypted tiles */

	ROM_REGION( 0x500000, "gfx2", 0 )
	ROM_LOAD( "man-05.16a",	0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x140000,  0x40000 )
	ROM_CONTINUE(			0x280000,  0x40000 )
	ROM_CONTINUE(			0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a",	0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x180000,  0x40000 )
	ROM_CONTINUE(			0x2c0000,  0x40000 )
	ROM_CONTINUE(			0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a",	0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x1c0000,  0x40000 )
	ROM_CONTINUE(			0x300000,  0x40000 )
	ROM_CONTINUE(			0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a",	0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x200000,  0x40000 )
	ROM_CONTINUE(			0x340000,  0x40000 )
	ROM_CONTINUE(			0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a",	0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x240000,  0x40000 )
	ROM_CONTINUE(			0x380000,  0x40000 )
	ROM_CONTINUE(			0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "man-06.17a",	0x000000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD16_BYTE( "man-07.18a",	0x000001,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD16_BYTE( "man-08.17c",	0x200000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD16_BYTE( "man-09.21c",	0x200001,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k",	0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k",	0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )
ROM_END

ROM_START( captavenj )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "hj_00-2.1e",	0x000000, 0x20000, CRC(10b1faaf) SHA1(9d76885200a846b4751c8d44ff591e2aff7c4148) )
	ROM_LOAD32_BYTE( "hj_01-2.1h",	0x000001, 0x20000, CRC(62c59f27) SHA1(20bbb7f3ff63a8c795686c1d56d51e90305daa77) )
	ROM_LOAD32_BYTE( "hj_02-2.1k",	0x000002, 0x20000, CRC(ce946cad) SHA1(9f1e92f5149e8a8d0236d5a7ba854ee100fd8488) )
	ROM_LOAD32_BYTE( "hj_03-2.1m",	0x000003, 0x20000, CRC(140cf9ce) SHA1(e2260ca4cea2fd7b64b8a78fd5444a7628bdafbb) )
	ROM_LOAD32_BYTE( "man-12.3e",	0x080000, 0x20000, CRC(d6261e98) SHA1(f3707be37ca926d9a341b9253a6bb2f3de0e25f6) )
	ROM_LOAD32_BYTE( "man-13.3h",	0x080001, 0x20000, CRC(40f0764d) SHA1(a6715c4a2accacf96f41c885579f314367c70dde) )
	ROM_LOAD32_BYTE( "man-14.3k",	0x080002, 0x20000, CRC(7cb9a4bd) SHA1(0af1a7bf0fcfa3cc14b38d92f19e97ad6e5541dd) )
	ROM_LOAD32_BYTE( "man-15.3m",	0x080003, 0x20000, CRC(c7854fe8) SHA1(ffa87dcda44fa0111de6ab317b77dd2bde015890) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "hj_08.17k",	0x00000,  0x10000,  CRC(361fbd16) SHA1(c4bbaf74e09c263044be74bb2c98caf6cfcab618) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "man-00.8a",	0x000000,  0x80000,  CRC(7855a607) SHA1(fa0be080515482281e5a12fe172eeb9a21af0820) ) /* Encrypted tiles */

	ROM_REGION( 0x500000, "gfx2", 0 )
	ROM_LOAD( "man-05.16a",	0x000000,  0x40000,  CRC(d44d1995) SHA1(e88e1a59a4b24ad058f21538f6e9bbba94a166b4) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x140000,  0x40000 )
	ROM_CONTINUE(			0x280000,  0x40000 )
	ROM_CONTINUE(			0x3c0000,  0x40000 )
	ROM_LOAD( "man-04.14a",	0x040000,  0x40000,  CRC(541492a1) SHA1(2e0ab12555fc46001a815e76e3a0cd21f385f82a) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x180000,  0x40000 )
	ROM_CONTINUE(			0x2c0000,  0x40000 )
	ROM_CONTINUE(			0x400000,  0x40000 )
	ROM_LOAD( "man-03.12a",	0x080000,  0x40000,  CRC(2d9c52b2) SHA1(8f6f4fe4f1a63099f889068991b34f9432b04fd7) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x1c0000,  0x40000 )
	ROM_CONTINUE(			0x300000,  0x40000 )
	ROM_CONTINUE(			0x440000,  0x40000 )
	ROM_LOAD( "man-02.11a",	0x0c0000,  0x40000,  CRC(07674c05) SHA1(08b33721d7eba4a1ff2e282f77eeb56535a52923) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x200000,  0x40000 )
	ROM_CONTINUE(			0x340000,  0x40000 )
	ROM_CONTINUE(			0x480000,  0x40000 )
	ROM_LOAD( "man-01.10a",	0x100000,  0x40000,  CRC(ae714ada) SHA1(b4d5806265d422c8b837489afe93731f584e4adf) ) /* Encrypted tiles */
	ROM_CONTINUE(			0x240000,  0x40000 )
	ROM_CONTINUE(			0x380000,  0x40000 )
	ROM_CONTINUE(			0x4c0000,  0x40000 )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "man-06.17a",	0x000000,  0x100000,  CRC(a9a64297) SHA1(e4cb441207b1907461c90c32c05a461c9bd30756) )
	ROM_LOAD16_BYTE( "man-07.18a",	0x000001,  0x100000,  CRC(b1db200c) SHA1(970bb15e90194dd285f53594aca5dec3405e75d5) )
	ROM_LOAD16_BYTE( "man-08.17c",	0x200000,  0x100000,  CRC(28e98e66) SHA1(55dbbd945eada81f7dcc874fdcb0b9e62ea453f0) )
	ROM_LOAD16_BYTE( "man-09.21c",	0x200001,  0x100000,  CRC(1921245d) SHA1(88d3b69a38c18c83d5658d057b95974f1bd371e6) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "man-10.14k",	0x000000,  0x80000,  CRC(0132c578) SHA1(70952f39508360bab51e1151531536f0ea6bbe06) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "man-11.16k",	0x000000,  0x80000,  CRC(0dc60a4c) SHA1(4d0daa6a0272852a37f341a0cdc48baee0ad9dd8) )
ROM_END

ROM_START( dragngun )
	ROM_REGION(0x400000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "kb02.a9",  0x000000, 0x40000, CRC(4fb9cfea) SHA1(e20fbae32682fc5fdc82070d2d6c73b5b7ac13f8) )
	ROM_LOAD32_BYTE( "kb06.c9",  0x000001, 0x40000, CRC(2395efec) SHA1(3c08299a6cdeebf9d3d5d367ab435eec76986194) )
	ROM_LOAD32_BYTE( "kb00.a5",  0x000002, 0x40000, CRC(1539ff35) SHA1(6c82fe01f5ebf5cdd3a914cc823499fa6a26f9a9) )
	ROM_LOAD32_BYTE( "kb04.c5",  0x000003, 0x40000, CRC(5b5c1ec2) SHA1(3c5c02b7e432cf1861e0c8db23b302dc47774a42) )
	ROM_LOAD32_BYTE( "kb03.a10", 0x300000, 0x40000, CRC(6c6a4f42) SHA1(ae96fe81f9ba587eb3194dbffa0233413d63c4c6) )
	ROM_LOAD32_BYTE( "kb07.c10", 0x300001, 0x40000, CRC(2637e8a1) SHA1(7bcd1b1f3a4e6aaa0a3b78ca77dc666948c87547) )
	ROM_LOAD32_BYTE( "kb01.a7",  0x300002, 0x40000, CRC(d780ba8d) SHA1(0e315c718c038962b6020945b48bcc632de6f5e1) )
	ROM_LOAD32_BYTE( "kb05.c7",  0x300003, 0x40000, CRC(fbad737b) SHA1(04e16abe8c4cec4f172bea29516535511db9db90) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "kb10.n25",  0x00000,  0x10000,  CRC(ec56f560) SHA1(feb9491683ba7f1000edebb568d6b3471fcc87fb) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "kb08.a15",  0x00000,  0x10000,  CRC(8fe4e5f5) SHA1(922b94f8ce0c35e965259c11e95891ef4be913d4) ) /* Encrypted tiles */
	ROM_LOAD16_BYTE( "kb09.a17",  0x00001,  0x10000,  CRC(e9dcac3f) SHA1(0621e601ffae73bbf69623042c9c8ab0526c3de6) )

	ROM_REGION( 0x120000, "gfx2", 0 )
	ROM_LOAD( "mar-00.bin",  0x00000,  0x80000,  CRC(d0491a37) SHA1(cc0ae1e9e5f42ba30159fb79bccd2e237cd037d0) ) /* Encrypted tiles */
	ROM_LOAD( "mar-01.bin",  0x90000,  0x80000,  CRC(d5970365) SHA1(729baf1efbef15c9f3e1d700717f5ba4f10d3014) )

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD( "mar-02.bin",  0x000000, 0x40000,  CRC(c6cd4baf) SHA1(350286829a330b64f463d0a9cbbfdb71eecf5188) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x100000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x200000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x300000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mar-03.bin",  0x040000, 0x40000,  CRC(793006d7) SHA1(7d8aba2fe75917f580a3a931a7defe5939a0874e) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x140000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x240000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x340000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mar-04.bin",  0x080000, 0x40000,  CRC(56631a2b) SHA1(0fa3d6215df8ce923c153b96f39161ba88b2dd53) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x180000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x280000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x380000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mar-05.bin",  0x0c0000, 0x40000,  CRC(ac16e7ae) SHA1(dca32e0a677a99f47a7b8e8f105483c57382f218) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x1c0000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x2c0000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x3c0000, 0x40000 ) /* 3/4 */

	ROM_REGION( 0x800000, "gfx4", 0 )
	ROM_LOAD32_BYTE( "mar-09.bin", 0x000000, 0x100000,  CRC(18fec9e1) SHA1(1290a9c13b4fd7d2197b39ec616206796e3a17a8) )
	ROM_LOAD32_BYTE( "mar-10.bin", 0x400000, 0x100000,  CRC(73126fbc) SHA1(9b9c31335e4db726863b219072c83810008f88f9) )
	ROM_LOAD32_BYTE( "mar-11.bin", 0x000001, 0x100000,  CRC(1fc638a4) SHA1(003dcfbb65a8f32a1a030502a11432287cf8b4e0) )
	ROM_LOAD32_BYTE( "mar-12.bin", 0x400001, 0x100000,  CRC(4c412512) SHA1(ccd5014bc9f9648cf5fa56bb8d54fc72a7099ca3) )
	ROM_LOAD32_BYTE( "mar-13.bin", 0x000002, 0x100000,  CRC(d675821c) SHA1(ff195422d0bef62d1f9c7784bba1e6b7ab5cd211) )
	ROM_LOAD32_BYTE( "mar-14.bin", 0x400002, 0x100000,  CRC(22d38c71) SHA1(62273665975f3e6000fa4b01755aeb70e5dd002d) )
	ROM_LOAD32_BYTE( "mar-15.bin", 0x000003, 0x100000,  CRC(ec976b20) SHA1(c120b3c56d5e02162e41dc7f726c260d0f8d2f1a) )
	ROM_LOAD32_BYTE( "mar-16.bin", 0x400003, 0x100000,  CRC(8b329bc8) SHA1(6e34eb6e2628a01a699d20a5155afb2febc31255) )

	ROM_REGION( 0x100000, "gfx5", 0 ) /* Video data - unused for now */
	ROM_LOAD( "mar-17.bin",  0x00000,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) )
	ROM_LOAD( "mar-18.bin",  0x00000,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
	ROM_LOAD( "mar-19.bin",  0x00000,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) )
	ROM_LOAD( "mar-20.bin",  0x00000,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) )
	ROM_LOAD( "mar-21.bin",  0x00000,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
	ROM_LOAD( "mar-22.bin",  0x00000,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) )
	ROM_LOAD( "mar-23.bin",  0x00000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) )
	ROM_LOAD( "mar-24.bin",  0x00000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
	ROM_LOAD( "mar-25.bin",  0x00000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) )
	ROM_LOAD( "mar-26.bin",  0x00000,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) )
	ROM_LOAD( "mar-27.bin",  0x00000,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
	ROM_LOAD( "mar-28.bin",  0x00000,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mar-06.n17", 0x000000, 0x80000,  CRC(3e006c6e) SHA1(55786e0fde2bf6ba9802f3f4fa8d4c21625b976a) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mar-08.n21", 0x000000, 0x80000,  CRC(b9281dfd) SHA1(449faf5d36f3b970d0a9b483e2152a5f68604a77) )

	ROM_REGION(0x80000, "oki3", 0 )
	ROM_LOAD( "mar-07.n19", 0x000000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )
ROM_END

ROM_START( fghthist )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_WORD( "kx00-unknown.bin", 0x000000, 0x80000, CRC(fe5eaba1) SHA1(c8a3784af487a1bbd2150abf4b1c8f3ad33da8a4) )
	ROM_LOAD32_WORD( "kx01-unknown.bin", 0x000002, 0x80000, CRC(3fb8d738) SHA1(2fca7a3ea483f01c97fb28a0adfa6d7980d8236c) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "kz02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) /* Encrypted tiles */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) /* Encrypted tiles */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbf02-16.16d",  0x000001,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD16_BYTE( "mbf04-16.18d",  0x000000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD16_BYTE( "mbf03-16.17d",  0x400001,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD16_BYTE( "mbf05-16.19d",  0x400000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) /* MB7124H type prom */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
ROM_END

ROM_START( fghthistu )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_WORD( "kz00-1.1f", 0x000000, 0x80000, CRC(3a3dd15c) SHA1(689b51adf73402b12191a75061b8e709468c91bc) )
	ROM_LOAD32_WORD( "kz01-1.2f", 0x000002, 0x80000, CRC(86796cd6) SHA1(c397c07d7a1d03ba96ccb2fe7a0ad25b8331e945) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "kz02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) /* Encrypted tiles */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) /* Encrypted tiles */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbf02-16.16d",  0x000001,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD16_BYTE( "mbf04-16.18d",  0x000000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD16_BYTE( "mbf03-16.17d",  0x400001,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD16_BYTE( "mbf05-16.19d",  0x400000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) /* MB7124H type prom */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
ROM_END

ROM_START( fghthista )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_WORD( "le-00.1f", 0x000000, 0x80000, CRC(a5c410eb) SHA1(e2b0cb2351782e1155ecc4029010beb7326fd874) )
	ROM_LOAD32_WORD( "le-01.2f", 0x000002, 0x80000, CRC(7e148aa2) SHA1(b21e16604c4d29611f91d629deb9f041eaf41e9b) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "kz02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) /* Encrypted tiles */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) /* Encrypted tiles */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbf02-16.16d",  0x000001,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD16_BYTE( "mbf04-16.18d",  0x000000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD16_BYTE( "mbf03-16.17d",  0x400001,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD16_BYTE( "mbf05-16.19d",  0x400000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) /* MB7124H type prom */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
ROM_END

ROM_START( fghthistj )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_WORD( "kw00-3.1f", 0x000000, 0x80000, CRC(ade9581a) SHA1(c1302e921f119ff9baeb52f9c338df652e64a9ee) )
	ROM_LOAD32_WORD( "kw01-3.2f", 0x000002, 0x80000, CRC(63580acf) SHA1(03372b168fe461542dd1cf64b4021d948d07e15c) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "kz02.18k",  0x00000,  0x10000,  CRC(5fd2309c) SHA1(2fb7af54d5cd9bf7dd6fb4f6b82aa52b03294f1f) ) /* Labeled KW02- but the same as the other sets */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "mbf00-8.8a",  0x000000,  0x100000,  CRC(d3e9b580) SHA1(fc4676e0ecc6c32441ff66fa1f990cc3158237db) ) /* Encrypted tiles */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mbf01-8.9a",  0x000000,  0x100000,  CRC(0c6ed2eb) SHA1(8e37ef4b1f0b6d3370a08758bfd602cb5f221282) ) /* Encrypted tiles */

	ROM_REGION( 0x800000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbf02-16.16d",  0x000001,  0x200000,  CRC(c19c5953) SHA1(e6ed26f932c6c86bbd1fc4c000aa2f510c268009) )
	ROM_LOAD16_BYTE( "mbf04-16.18d",  0x000000,  0x200000,  CRC(f6a23fd7) SHA1(74e5559f17cd591aa25d2ed6c34ac9ed89e2e9ba) )
	ROM_LOAD16_BYTE( "mbf03-16.17d",  0x400001,  0x200000,  CRC(37d25c75) SHA1(8219d31091b4317190618edd8acc49f97cba6a1e) )
	ROM_LOAD16_BYTE( "mbf05-16.19d",  0x400000,  0x200000,  CRC(137be66d) SHA1(3fde345183ce04a7a65b4cedfd050d771df7d026) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbf06.15k",  0x000000,  0x80000,  CRC(fb513903) SHA1(7727a49ff7977f159ed36d097020edef3b5b36ba) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbf07.16k",  0x000000,  0x80000,  CRC(51d4adc7) SHA1(22106ed7a05db94adc5a783ce34529e29d24d41a) )

	ROM_REGION(512, "proms", 0 )
	ROM_LOAD( "kt-00.8j",  0,  512,  CRC(7294354b) SHA1(14fe42ad5d26d022c0fe9a46a4a9017af2296f40) ) /* MB7124H type prom */

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "ve-00.3d",  0x0000, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
	ROM_LOAD( "ve-01.4d",  0x0200, 0x0104, NO_DUMP ) /* PAL16L8 is read protected */
ROM_END

ROM_START( lockload ) /* Board No. DE-0420-1 + Bottom board DE-0421-0 slightly different hardware, likely a unique PCB and not a Dragongun conversion */
	ROM_REGION(0x400000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "nl-00-1.b5", 0x000002, 0x80000, CRC(7a39bf8d) SHA1(8b1a6407bab74b3960a243a6c04c0005a82126f1) )
	ROM_LOAD32_BYTE( "nl-01-1.b8", 0x000000, 0x80000, CRC(d23afcb7) SHA1(de7b5bc936a87cc6511d588b0bf082bbf745581c) )
	ROM_LOAD32_BYTE( "nl-02-1.d5", 0x000003, 0x80000, CRC(730e0168) SHA1(fdfa0d335c03c2c528326f90948e642f9ea43150) )
	ROM_LOAD32_BYTE( "nl-03-1.d8", 0x000001, 0x80000, CRC(51a53ece) SHA1(ee2c8858844a47fa1e83c30c06d78cf49219dc33) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "nm-06-.n22",  0x00000,  0x10000,  CRC(31d1c245) SHA1(326e35e7ebd8ea761d90e856c50d86512327f2a5) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "nl-04-.b15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) /* Encrypted tiles */
	ROM_LOAD16_BYTE( "nl-05-.b17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) /* Encrypted tiles */
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "gfx3", 0 )
	ROM_LOAD( "mbm-02.b21",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x200000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x400000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x600000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x040000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x240000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x440000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x640000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mbm-03.b22",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x280000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x480000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x680000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x0c0000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x2c0000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x4c0000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x6c0000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mbm-04.e21",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x300000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x500000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x700000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x140000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x340000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x540000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x740000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mbm-05.e22",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x380000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x580000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x780000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x1c0000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x3c0000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x5c0000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x7c0000, 0x40000 ) /* 3/4 */

	ROM_REGION( 0x800000, "gfx4", 0 )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000000, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400000, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000001, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400001, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000002, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400002, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000003, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400003, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )

	ROM_REGION( 0x100000, "gfx5", ROMREGION_ERASE00 ) /* Video data - unique PCB and this region is not used? */

	ROM_REGION(0x100000, "oki1", 0 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )

	ROM_REGION(0x80000, "oki2", ROMREGION_ERASE00 ) /* Sound data - unique PCB and this region is not used? */

	ROM_REGION(0x80000, "oki3", 0 )
	ROM_LOAD( "mbm-07.n19",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) ) /* Does this go here or "oki2" ?? */
ROM_END

ROM_START( lockloadu )
	ROM_REGION(0x400000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_BYTE( "nh-00-0.b5", 0x000002, 0x80000, CRC(b8a57164) SHA1(b700a08db2ad1aa1bf0a32635ffbd5d3f08713ee) )
	ROM_LOAD32_BYTE( "nh-01-0.b8", 0x000000, 0x80000, CRC(e371ac50) SHA1(c448b54bc8962844b490994607b21b0c806d7714) )
	ROM_LOAD32_BYTE( "nh-02-0.d5", 0x000003, 0x80000, CRC(3e361e82) SHA1(b5445d44f2a775c141fdc561d5489234c39445a4) )
	ROM_LOAD32_BYTE( "nh-03-0.d8", 0x000001, 0x80000, CRC(d08ee9c3) SHA1(9a85710a11940df047e83e8d5977a23d6c67d665) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "nh-06-0.n25",  0x00000,  0x10000,  CRC(7a1af51d) SHA1(54e6b16d3f5b787d3c6eb7203d8854e6e0fb9803) )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "nh-04-0.b15",  0x00000,  0x10000,  CRC(f097b3d9) SHA1(5748de9a796afddd78dad7f5c184269ee533c51c) ) /* Encrypted tiles */
	ROM_LOAD16_BYTE( "nh-05-0.b17",  0x00001,  0x10000,  CRC(448fec1e) SHA1(9a107959621cbb3688fd3ad9a8320aa5584f7d13) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "mbm-00.d15",  0x00000, 0x80000,  CRC(b97de8ff) SHA1(59508f7135af22c2ac89db78874b1e8a68c53434) ) /* Encrypted tiles */
	ROM_LOAD( "mbm-01.d17",  0x80000, 0x80000,  CRC(6d4b8fa0) SHA1(56e2b9adb4d010ba2592eccba654a24141441141) )

	ROM_REGION( 0x800000, "gfx3", 0 )
	ROM_LOAD( "mbm-02.b23",  0x000000, 0x40000,  CRC(e723019f) SHA1(15361d3e6db5707a7f0ead4254463c50163c864c) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x200000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x400000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x600000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x040000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x240000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x440000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x640000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mbm-03.b26",  0x080000, 0x40000,  CRC(e0d09894) SHA1(be2faa81cf92b6fadfb2ec4ca2173157b05071ec) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x280000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x480000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x680000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x0c0000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x2c0000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x4c0000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x6c0000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mbm-04.e23",  0x100000, 0x40000,  CRC(9e12466f) SHA1(51eaadfaf45d02d72b61052a606f97f36b3964fd) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x300000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x500000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x700000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x140000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x340000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x540000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x740000, 0x40000 ) /* 3/4 */
	ROM_LOAD( "mbm-05.e26",  0x180000, 0x40000,  CRC(6ff02dc0) SHA1(5862e2189a09f963d5ec58ca4aa1c06210a3c7ef) ) /* Encrypted tiles 0/4 */
	ROM_CONTINUE(            0x380000, 0x40000 ) /* 2 bpp per 0x40000 chunk, 1/4 */
	ROM_CONTINUE(            0x580000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x780000, 0x40000 ) /* 3/4 */
	ROM_CONTINUE(            0x1c0000, 0x40000 ) /* Next block 2bpp 0/4 */
	ROM_CONTINUE(            0x3c0000, 0x40000 ) /* 1/4 */
	ROM_CONTINUE(            0x5c0000, 0x40000 ) /* 2/4 */
	ROM_CONTINUE(            0x7c0000, 0x40000 ) /* 3/4 */

	ROM_REGION( 0x800000, "gfx4", 0 )
	ROM_LOAD32_BYTE( "mbm-08.a14",  0x000000, 0x100000,  CRC(5358a43b) SHA1(778637fc63a0957338c7da3adb2555ffada177c4) )
	ROM_LOAD32_BYTE( "mbm-09.a16",  0x400000, 0x100000,  CRC(2cce162f) SHA1(db5795465a36971861e8fb7436db0805717ad101) )
	ROM_LOAD32_BYTE( "mbm-10.a19",  0x000001, 0x100000,  CRC(232e1c91) SHA1(868d4eb4873ecc210cbb3a266cae0b6ad8f11add) )
	ROM_LOAD32_BYTE( "mbm-11.a20",  0x400001, 0x100000,  CRC(8a2a2a9f) SHA1(d11e0ea2785e35123bc56a8f18ce22f58432b599) )
	ROM_LOAD32_BYTE( "mbm-12.a21",  0x000002, 0x100000,  CRC(7d221d66) SHA1(25c9c20485e443969c0bf4d74c4211c3881dabcd) )
	ROM_LOAD32_BYTE( "mbm-13.a22",  0x400002, 0x100000,  CRC(678b9052) SHA1(ae8fc921813e53e9dbc3960e772c1c4de94c22a7) )
	ROM_LOAD32_BYTE( "mbm-14.a23",  0x000003, 0x100000,  CRC(5aaaf929) SHA1(5ee30db9b83db664d77e6b5e0988ce3366460df6) )
	ROM_LOAD32_BYTE( "mbm-15.a25",  0x400003, 0x100000,  CRC(789ce7b1) SHA1(3fb390ce0620ce7a63f7f46eac1ff0eb8ed76d26) )

	ROM_REGION( 0x100000, "gfx5", ROMREGION_ERASE00 ) /* Video data - same as Dragongun, probably leftover from a conversion */
//  ROM_LOAD( "mar-17.bin",  0x00000,  0x100000,  CRC(7799ed23) SHA1(ae28ad4fa6033a3695fa83356701b3774b26e6b0) )
//  ROM_LOAD( "mar-18.bin",  0x00000,  0x100000,  CRC(ded66da9) SHA1(5134cb47043cc190a35ebdbf1912166669f9c055) )
//  ROM_LOAD( "mar-19.bin",  0x00000,  0x100000,  CRC(bdd1ed20) SHA1(2435b23210b8fee4d39c30d4d3c6ea40afaa3b93) )
//  ROM_LOAD( "mar-20.bin",  0x00000,  0x100000,  CRC(fa0462f0) SHA1(1a52617ad4d7abebc0f273dd979f4cf2d6a0306b) )
//  ROM_LOAD( "mar-21.bin",  0x00000,  0x100000,  CRC(2d0a28ae) SHA1(d87f6f71bb76880e4d4f1eab8e0451b5c3df69a5) )
//  ROM_LOAD( "mar-22.bin",  0x00000,  0x100000,  CRC(c85f3559) SHA1(a5d5cf9b18c9ef6a92d7643ca1ec9052de0d4a01) )
//  ROM_LOAD( "mar-23.bin",  0x00000,  0x100000,  CRC(ba907d6a) SHA1(1fd99b66e6297c8d927c1cf723a613b4ee2e2f90) )
//  ROM_LOAD( "mar-24.bin",  0x00000,  0x100000,  CRC(5cec45c8) SHA1(f99a26afaca9d9320477e469b09e3873bc8c156f) )
//  ROM_LOAD( "mar-25.bin",  0x00000,  0x100000,  CRC(d65d895c) SHA1(4508dfff95a7aff5109dc74622cbb4503b0b5840) )
//  ROM_LOAD( "mar-26.bin",  0x00000,  0x100000,  CRC(246a06c5) SHA1(447252be976a5059925f4ad98df8564b70198f62) )
//  ROM_LOAD( "mar-27.bin",  0x00000,  0x100000,  CRC(3fcbd10f) SHA1(70fc7b88bbe35bbae1de14364b03d0a06d541de5) )
//  ROM_LOAD( "mar-28.bin",  0x00000,  0x100000,  CRC(5a2ec71d) SHA1(447c404e6bb696f7eb7c61992a99b9be56f5d6b0) )

	ROM_REGION(0x100000, "oki1", 0 )
	ROM_LOAD( "mbm-06.n17",  0x00000, 0x100000,  CRC(f34d5999) SHA1(265b5f4e8598bcf9183bf9bd95db69b01536acb2) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbm-07.n21",  0x00000, 0x80000,  CRC(414f3793) SHA1(ed5f63e57390d503193fd1e9f7294ae1da6d3539) )

	ROM_REGION(0x80000, "oki3", 0 )
	ROM_LOAD( "mar-07.n19",  0x00000, 0x80000,  CRC(40287d62) SHA1(c00cb08bcdae55bcddc14c38e88b0484b1bc9e3e) )	// same as dragngun, unused?
ROM_END

ROM_START( tattass )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_WORD( "pp44.cpu", 0x000000, 0x80000, CRC(c3ca5b49) SHA1(c6420b0c20df1ae166b279504880ade65b1d8048) )
	ROM_LOAD32_WORD( "pp45.cpu", 0x000002, 0x80000, CRC(d3f30de0) SHA1(5a0aa0f96d29299b3b337b4b51bc84e447eb74d0) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(6947be8a) SHA1(4ac6c3c7f54501f23c434708cea6bf327bc8cf95) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "gfx3", 0 )
	ROM_LOAD( "ob1_c0.b0",  0x000000, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD( "ob1_c1.b0",  0x200000, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD( "ob1_c2.b0",  0x400000, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD( "ob1_c3.b0",  0x600000, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD( "ob1_c4.b0",  0x800000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD( "ob1_c0.b1",  0x080000, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD( "ob1_c1.b1",  0x280000, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD( "ob1_c2.b1",  0x480000, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD( "ob1_c3.b1",  0x680000, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD( "ob1_c4.b1",  0x880000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD( "ob1_c0.b2",  0x100000, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD( "ob1_c1.b2",  0x300000, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD( "ob1_c2.b2",  0x500000, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD( "ob1_c3.b2",  0x700000, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD( "ob1_c4.b2",  0x900000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD( "ob1_c0.b3",  0x180000, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD( "ob1_c1.b3",  0x380000, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD( "ob1_c2.b3",  0x580000, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD( "ob1_c3.b3",  0x780000, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD( "ob1_c4.b3",  0x980000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "gfx4", 0 )
	ROM_LOAD32_BYTE( "ob2_c0.b0",  0x000001, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD32_BYTE( "ob2_c1.b0",  0x000003, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD32_BYTE( "ob2_c2.b0",  0x000000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD32_BYTE( "ob2_c3.b0",  0x000002, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD32_BYTE( "ob2_c0.b1",  0x200001, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD32_BYTE( "ob2_c1.b1",  0x200003, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD32_BYTE( "ob2_c2.b1",  0x200000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD32_BYTE( "ob2_c3.b1",  0x200002, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD32_BYTE( "ob2_c0.b2",  0x400001, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD32_BYTE( "ob2_c1.b2",  0x400003, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD32_BYTE( "ob2_c2.b2",  0x400000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD32_BYTE( "ob2_c3.b2",  0x400002, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD32_BYTE( "ob2_c0.b3",  0x600001, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD32_BYTE( "ob2_c1.b3",  0x600003, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD32_BYTE( "ob2_c2.b3",  0x600000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD32_BYTE( "ob2_c3.b3",  0x600002, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x200000, "bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(b945c18d) SHA1(6556bbb4a7057df3680132f24687fa944006c784) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(10b2110c) SHA1(83e5938ed22da2874022e1dc8df76c72d95c448d) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(3b73abe2) SHA1(195096e2302e84123b23b4ccd982fb3ab9afe42c) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(986066b5) SHA1(9dd1a14de81733617cf51293674a8e26fc5cec68) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( tattassa )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD32_WORD( "rev232a.000", 0x000000, 0x80000, CRC(1a357112) SHA1(d7f78f90970fd56ca1452a4c138168568b06d868) )
	ROM_LOAD32_WORD( "rev232a.001", 0x000002, 0x80000, CRC(550245d4) SHA1(c1b2b31768da9becebd907a8622d05aa68ecaa29) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "u7.snd",  0x00000, 0x10000,  CRC(6947be8a) SHA1(4ac6c3c7f54501f23c434708cea6bf327bc8cf95) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "abak_b01.s02",  0x000000, 0x80000,  CRC(bc805680) SHA1(ccdbca23fc843ef82a3524020999542f43b3c618) )
	ROM_LOAD16_BYTE( "abak_b01.s13",  0x000001, 0x80000,  CRC(350effcd) SHA1(0452d95be9fc28bd00d846a2cc5828899d69601e) )
	ROM_LOAD16_BYTE( "abak_b23.s02",  0x100000, 0x80000,  CRC(91abdc21) SHA1(ba08e59bc0417e863d35ea295cf58cfe8faf57b5) )
	ROM_LOAD16_BYTE( "abak_b23.s13",  0x100001, 0x80000,  CRC(80eb50fe) SHA1(abfe1a5417ceff9d6d52372d11993bf9b1db9432) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "bbak_b01.s02",  0x000000, 0x80000,  CRC(611be9a6) SHA1(86263c8beb562e0607a65aa30fbbe030a044cd75) )
	ROM_LOAD16_BYTE( "bbak_b01.s13",  0x000001, 0x80000,  CRC(097e0604) SHA1(6ae241b37b6bb15fc66679cf66f500b8f8a19f44) )
	ROM_LOAD16_BYTE( "bbak_b23.s02",  0x100000, 0x80000,  CRC(3836531a) SHA1(57bead820ac396ee0ed8fb2ac5c15929896d75bf) )
	ROM_LOAD16_BYTE( "bbak_b23.s13",  0x100001, 0x80000,  CRC(1210485a) SHA1(9edc4c96f389e231066ef164a7b2851cd7ade038) )

	ROM_REGION( 0xa00000, "gfx3", 0 )
	ROM_LOAD( "ob1_c0.b0",  0x000000, 0x80000,  CRC(053fecca) SHA1(319efc71042238d9012d2c3dddab9d11205decc6) )
	ROM_LOAD( "ob1_c1.b0",  0x200000, 0x80000,  CRC(e183e6bc) SHA1(d9cce277861967f403a882879e1baefa84696bdc) )
	ROM_LOAD( "ob1_c2.b0",  0x400000, 0x80000,  CRC(1314f828) SHA1(6a91543d4e70af30de287ba775c69ffb1cde719d) )
	ROM_LOAD( "ob1_c3.b0",  0x600000, 0x80000,  CRC(c63866df) SHA1(a897835d8a33002f1bd54f27d1a6393c4e1864b9) )
	ROM_LOAD( "ob1_c4.b0",  0x800000, 0x80000,  CRC(f71cdd1b) SHA1(6fbccdbe460c8ddfeed972ebe766a6f8a2d4c466) )

	ROM_LOAD( "ob1_c0.b1",  0x080000, 0x80000,  CRC(385434b0) SHA1(ea764bd9844e13f5b10207022135dbe07bf0258a) )
	ROM_LOAD( "ob1_c1.b1",  0x280000, 0x80000,  CRC(0a3ec489) SHA1(1a2e1252d6acda43019ded5a31ae60bef40e4bd9) )
	ROM_LOAD( "ob1_c2.b1",  0x480000, 0x80000,  CRC(52f06081) SHA1(c630f45b110b9423dfb0bf92359fdb28b75c8cf1) )
	ROM_LOAD( "ob1_c3.b1",  0x680000, 0x80000,  CRC(a8a5cfbe) SHA1(7afc8f7c7f3826a276e4840e4fc8b8bb645dd3bd) )
	ROM_LOAD( "ob1_c4.b1",  0x880000, 0x80000,  CRC(09d0acd6) SHA1(1b162f5b76852e49ae6a24db2031d66ca59d87e9) )

	ROM_LOAD( "ob1_c0.b2",  0x100000, 0x80000,  CRC(946e9f59) SHA1(46a0d35641b381fe553caa00451c30f1950b5dfd) )
	ROM_LOAD( "ob1_c1.b2",  0x300000, 0x80000,  CRC(9f66ad54) SHA1(6e6ac6edee2f2dda46e7cd85db8d79c8335c73cd) )
	ROM_LOAD( "ob1_c2.b2",  0x500000, 0x80000,  CRC(a8df60eb) SHA1(c971e66eec6accccaf2bdd87dde7adde79322da9) )
	ROM_LOAD( "ob1_c3.b2",  0x700000, 0x80000,  CRC(a1a753be) SHA1(1666a32bb69db36dba029a835592d00a21ad8c5e) )
	ROM_LOAD( "ob1_c4.b2",  0x900000, 0x80000,  CRC(b65b3c4b) SHA1(f636a682b506e3ce5ca07ba8fd3166158d1ab667) )

	ROM_LOAD( "ob1_c0.b3",  0x180000, 0x80000,  CRC(cbbbc696) SHA1(6f2383655461ac35f3178e0f7c0146cff89c8295) )
	ROM_LOAD( "ob1_c1.b3",  0x380000, 0x80000,  CRC(f7b1bdee) SHA1(1d505d8d4ede55246de0b5fbc6ca20f836699b60) )
	ROM_LOAD( "ob1_c2.b3",  0x580000, 0x80000,  CRC(97815619) SHA1(b1b694310064971aa5438671d0f9992b7e4bf277) )
	ROM_LOAD( "ob1_c3.b3",  0x780000, 0x80000,  CRC(fc3ccb7a) SHA1(4436fcbd830912589bd6c838eb63b7d41a2bb56e) )
	ROM_LOAD( "ob1_c4.b3",  0x980000, 0x80000,  CRC(dfdfd0ff) SHA1(79dc686351d41d635359936efe97c7ade305dc84) )

	ROM_REGION( 0x800000, "gfx4", 0 )
	ROM_LOAD32_BYTE( "ob2_c0.b0",  0x000001, 0x80000,  CRC(9080ebe4) SHA1(1cfabe045532e16f203fe054449149451a280f56) )
	ROM_LOAD32_BYTE( "ob2_c1.b0",  0x000003, 0x80000,  CRC(c0464970) SHA1(2bd87c9a7ed0742a8f1ee0c0de225e18a0351168) )
	ROM_LOAD32_BYTE( "ob2_c2.b0",  0x000000, 0x80000,  CRC(35a2e621) SHA1(ff7687e30c379cbcee4f80c0c737cef891509881) )
	ROM_LOAD32_BYTE( "ob2_c3.b0",  0x000002, 0x80000,  CRC(99c7cc2d) SHA1(c761e5b7f1e2afdafef36390f7141ebcb5e8f254) )
	ROM_LOAD32_BYTE( "ob2_c0.b1",  0x200001, 0x80000,  CRC(2c2c15c9) SHA1(fdc48fab6dad97d16d4e77479fa77bb320eb3767) )
	ROM_LOAD32_BYTE( "ob2_c1.b1",  0x200003, 0x80000,  CRC(d2c49a14) SHA1(49d92233d6d5f77fbbf9d31607c568efef6d94f0) )
	ROM_LOAD32_BYTE( "ob2_c2.b1",  0x200000, 0x80000,  CRC(fbe957e8) SHA1(4f0bb0e434771316bcd8796878ffd3e5cafebb2b) )
	ROM_LOAD32_BYTE( "ob2_c3.b1",  0x200002, 0x80000,  CRC(d7238829) SHA1(6fef08a518be69251852d3204413b4b8b6615be2) )
	ROM_LOAD32_BYTE( "ob2_c0.b2",  0x400001, 0x80000,  CRC(aefa1b01) SHA1(bbd4b432b36d64f80065c56559ea9675acf3151e) )
	ROM_LOAD32_BYTE( "ob2_c1.b2",  0x400003, 0x80000,  CRC(4af620ca) SHA1(f3753235b2e72f011c9b94f26a425b9a79577201) )
	ROM_LOAD32_BYTE( "ob2_c2.b2",  0x400000, 0x80000,  CRC(8e58be07) SHA1(d8a8662e800da0892d70c628de0ca27ff983006c) )
	ROM_LOAD32_BYTE( "ob2_c3.b2",  0x400002, 0x80000,  CRC(1b5188c5) SHA1(4792a36b889a2c2dfab9ec78d848d3d8bf10d20f) )
	ROM_LOAD32_BYTE( "ob2_c0.b3",  0x600001, 0x80000,  CRC(a2a5dafd) SHA1(2baadcfe9ae8fa30ae4226caa10fe3d58f8af3e0) )
	ROM_LOAD32_BYTE( "ob2_c1.b3",  0x600003, 0x80000,  CRC(6f0afd05) SHA1(6a4bf3466a77d14b3bc18377537f86108774badd) )
	ROM_LOAD32_BYTE( "ob2_c2.b3",  0x600000, 0x80000,  CRC(90fe5f4f) SHA1(2149e9eae152556c632ebd4d0b2de49e40916a77) )
	ROM_LOAD32_BYTE( "ob2_c3.b3",  0x600002, 0x80000,  CRC(e3517e6e) SHA1(68ac60570423d8f0d7cff3db1901c9c050d0be91) )

	ROM_REGION(0x200000, "bsmt", 0 )
	ROM_LOAD( "u17.snd",  0x000000, 0x80000,  CRC(b945c18d) SHA1(6556bbb4a7057df3680132f24687fa944006c784) )
	ROM_LOAD( "u21.snd",  0x080000, 0x80000,  CRC(10b2110c) SHA1(83e5938ed22da2874022e1dc8df76c72d95c448d) )
	ROM_LOAD( "u36.snd",  0x100000, 0x80000,  CRC(3b73abe2) SHA1(195096e2302e84123b23b4ccd982fb3ab9afe42c) )
	ROM_LOAD( "u37.snd",  0x180000, 0x80000,  CRC(986066b5) SHA1(9dd1a14de81733617cf51293674a8e26fc5cec68) )

	ROM_REGION( 0x400, "eeprom", 0 )
	ROM_LOAD( "eeprom-tattass.bin", 0x0000, 0x0400, CRC(7140f40c) SHA1(4fb7897933046b6adaf00b4626d5fd23d0e8a666) )
ROM_END

ROM_START( nslasher )
	ROM_REGION(0x100000, "maincpu", 0 ) /* Encrypted ARM 32 bit code */
	ROM_LOAD32_WORD( "mainprg.1f", 0x000000, 0x80000, CRC(507acbae) SHA1(329a2bb244f2f3adb8d75cab5aa2dcb129d70712) )
	ROM_LOAD32_WORD( "mainprg.2f", 0x000002, 0x80000, CRC(931fc7ee) SHA1(54eb12abfa3f332ce9b43a45ec424aaee88641a6) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )


	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) /* Encrypted tiles */

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) /* Encrypted tiles */

	ROM_REGION( 0xa00000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbh-02.14c",  0x000001,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD16_BYTE( "mbh-04.16c",  0x000000,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD16_BYTE( "mbh-03.15c",  0x400001,  0x80000,  CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD16_BYTE( "mbh-05.17c",  0x400000,  0x80000,  CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD32_BYTE( "mbh-06.18c",  0x500000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD32_BYTE( "mbh-07.19c",  0x900000,  0x40000,  CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbh-08.16e",  0x000001,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD16_BYTE( "mbh-09.18e",  0x000000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )

ROM_END

ROM_START( nslasherj )
	ROM_REGION(0x100000, "maincpu", 0 ) /* Encrypted ARM 32 bit code */
	ROM_LOAD32_WORD( "lx-00.1f", 0x000000, 0x80000, CRC(6ed5fb88) SHA1(84350da7939a479968a523c84e254e3ee54b8da2) )
	ROM_LOAD32_WORD( "lx-01.2f", 0x000002, 0x80000, CRC(a6df2152) SHA1(6fe7e0b2e71c5f807951dcc81a6a3cff55247961) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) /* Encrypted tiles */

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) /* Encrypted tiles */

	ROM_REGION( 0xa00000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbh-02.14c",  0x000001,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD16_BYTE( "mbh-04.16c",  0x000000,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD16_BYTE( "mbh-03.15c",  0x400001,  0x80000,  CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD16_BYTE( "mbh-05.17c",  0x400000,  0x80000,  CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD32_BYTE( "mbh-06.18c",  0x500000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD32_BYTE( "mbh-07.19c",  0x900000,  0x40000,  CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbh-08.16e",  0x000001,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD16_BYTE( "mbh-09.18e",  0x000000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )
ROM_END

ROM_START( nslashers )
	ROM_REGION(0x100000, "maincpu", 0 ) /* Encrypted ARM 32 bit code */
	ROM_LOAD32_WORD( "ly-00.1f", 0x000000, 0x80000, CRC(fa0646f9) SHA1(7f9633bda230a0ced59171cdc5ab40a6d56c3d34) )
	ROM_LOAD32_WORD( "ly-01.2f", 0x000002, 0x80000, CRC(ae508149) SHA1(3592949e5fb2770adb9c9daa4e38c4e75f3e2554) )

	ROM_REGION(0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "sndprg.17l",  0x00000,  0x10000,  CRC(18939e92) SHA1(50b37a78d9d2259d4b140dd17393c4e5ca92bca5) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mbh-00.8c",  0x000000,  0x200000,  CRC(a877f8a3) SHA1(79253525f360a73161894f31e211e4d6b38d307a) ) /* Encrypted tiles */

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "mbh-01.9c",  0x000000,  0x200000,  CRC(1853dafc) SHA1(b1183c0db301cbed9f079c782202dbfc553b198e) ) /* Encrypted tiles */

	ROM_REGION( 0xa00000, "gfx3", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbh-02.14c",  0x000001,  0x200000, CRC(b2f158a1) SHA1(4f8c0b324813db198fe1dad7fff4185b828b94de) )
	ROM_LOAD16_BYTE( "mbh-04.16c",  0x000000,  0x200000, CRC(eecfe06d) SHA1(2df817fe5e2ea31207b217bb03dc58979c05d0d2) )
	ROM_LOAD16_BYTE( "mbh-03.15c",  0x400001,  0x80000,  CRC(787787e3) SHA1(531444e3f28aa9a7539a5a76ca94a9d6b97274c5) )
	ROM_LOAD16_BYTE( "mbh-05.17c",  0x400000,  0x80000,  CRC(1d2b7c17) SHA1(ae0b8e0448a1a8180fb424fb0bc8a4302f8ff602) )
	ROM_LOAD32_BYTE( "mbh-06.18c",  0x500000,  0x100000, CRC(038c2127) SHA1(5bdb215305f1a419fde27a83b623a38b9328e560) )
	ROM_LOAD32_BYTE( "mbh-07.19c",  0x900000,  0x40000,  CRC(bbd22323) SHA1(6ab665b2e6d04cdadc48c52e15098e978b61fe10) )

	ROM_REGION( 0x100000, "gfx4", 0 ) /* Sprites */
	ROM_LOAD16_BYTE( "mbh-08.16e",  0x000001,  0x80000,  CRC(cdd7f8cb) SHA1(910bbe8783c0ba722e9d6399b332d658fa059fdb) )
	ROM_LOAD16_BYTE( "mbh-09.18e",  0x000000,  0x80000,  CRC(33fa2121) SHA1(eb0e99d29b1ad9995df28e5b7cfc89d53efb53c3) )

	ROM_REGION(0x80000, "oki1", 0 )
	ROM_LOAD( "mbh-10.14l", 0x000000,  0x80000,  CRC(c4d6b116) SHA1(c5685bce6a6c6a74ca600ebf766ba1007f0dc666) )

	ROM_REGION(0x80000, "oki2", 0 )
	ROM_LOAD( "mbh-11.16l", 0x000000,  0x80000,  CRC(0ec40b6b) SHA1(9fef44149608ae2a00f6a75a6f77f2efcab6e78e) )
ROM_END


static DRIVER_INIT( captaven )
{
	deco56_decrypt_gfx(machine, "gfx1");
	deco56_decrypt_gfx(machine, "gfx2");
}

static DRIVER_INIT( dragngun )
{
	UINT32 *ROM = (UINT32 *)memory_region(machine, "maincpu");
	const UINT8 *SRC_RAM = memory_region(machine, "gfx1");
	UINT8 *DST_RAM = memory_region(machine, "gfx2");

	deco74_decrypt_gfx(machine, "gfx1");
	deco74_decrypt_gfx(machine, "gfx2");
	deco74_decrypt_gfx(machine, "gfx3");

	memcpy(DST_RAM+0x80000,SRC_RAM,0x10000);
	memcpy(DST_RAM+0x110000,SRC_RAM+0x10000,0x10000);

	ROM[0x1b32c/4]=0xe1a00000;//  NOP test switch lock
}

static DRIVER_INIT( fghthist )
{
	deco56_decrypt_gfx(machine, "gfx1");
	deco74_decrypt_gfx(machine, "gfx2");

	decoprot_reset(machine);
}

static DRIVER_INIT( lockload )
{
	UINT8 *RAM = memory_region(machine, "maincpu");
//  UINT32 *ROM = (UINT32 *)memory_region(machine, "maincpu");

	deco74_decrypt_gfx(machine, "gfx1");
	deco74_decrypt_gfx(machine, "gfx2");
	deco74_decrypt_gfx(machine, "gfx3");

	memcpy(RAM+0x300000,RAM+0x100000,0x100000);
	memset(RAM+0x100000,0,0x100000);

//  ROM[0x3fe3c0/4]=0xe1a00000;//  NOP test switch lock
//  ROM[0x3fe3cc/4]=0xe1a00000;//  NOP test switch lock
//  ROM[0x3fe40c/4]=0xe1a00000;//  NOP test switch lock
}

static DRIVER_INIT( tattass )
{
	UINT8 *RAM = memory_region(machine, "gfx1");
	UINT8 *tmp = auto_alloc_array(machine, UINT8, 0x80000);

	/* Reorder bitplanes to make decoding easier */
	memcpy(tmp,RAM+0x80000,0x80000);
	memcpy(RAM+0x80000,RAM+0x100000,0x80000);
	memcpy(RAM+0x100000,tmp,0x80000);

	RAM = memory_region(machine, "gfx2");
	memcpy(tmp,RAM+0x80000,0x80000);
	memcpy(RAM+0x80000,RAM+0x100000,0x80000);
	memcpy(RAM+0x100000,tmp,0x80000);

	auto_free(machine, tmp);

	deco56_decrypt_gfx(machine, "gfx1"); /* 141 */
	deco56_decrypt_gfx(machine, "gfx2"); /* 141 */
}

static DRIVER_INIT( nslasher )
{
	UINT8 *RAM = memory_region(machine, "gfx1");
	UINT8 *tmp = auto_alloc_array(machine, UINT8, 0x80000);

	/* Reorder bitplanes to make decoding easier */
	memcpy(tmp,RAM+0x80000,0x80000);
	memcpy(RAM+0x80000,RAM+0x100000,0x80000);
	memcpy(RAM+0x100000,tmp,0x80000);

	RAM = memory_region(machine, "gfx2");
	memcpy(tmp,RAM+0x80000,0x80000);
	memcpy(RAM+0x80000,RAM+0x100000,0x80000);
	memcpy(RAM+0x100000,tmp,0x80000);

	auto_free(machine, tmp);

	deco56_decrypt_gfx(machine, "gfx1"); /* 141 */
	deco74_decrypt_gfx(machine, "gfx2");

	deco156_decrypt(machine);

	soundlatch_setclearedvalue(machine, 0xff);

	/* The board for Night Slashers is very close to the Fighter's History and
    Tattoo Assassins boards, but has an encrypted ARM cpu. */

}

/**********************************************************************************/

GAME( 1991, captaven,   0,        captaven, captaven, captaven, ROT0, "Data East Corporation", "Captain America and The Avengers (Asia Rev 1.4)", 0 )
GAME( 1991, captavena,  captaven, captaven, captaven, captaven, ROT0, "Data East Corporation", "Captain America and The Avengers (Asia Rev 1.0)", 0 )
GAME( 1991, captavene,  captaven, captaven, captaven, captaven, ROT0, "Data East Corporation", "Captain America and The Avengers (UK Rev 1.4)", 0 )
GAME( 1991, captavenu,  captaven, captaven, captaven, captaven, ROT0, "Data East Corporation", "Captain America and The Avengers (US Rev 1.9)", 0 )
GAME( 1991, captavenuu, captaven, captaven, captaven, captaven, ROT0, "Data East Corporation", "Captain America and The Avengers (US Rev 1.6)", 0 )
GAME( 1991, captavenua, captaven, captaven, captaven, captaven, ROT0, "Data East Corporation", "Captain America and The Avengers (US Rev 1.4)", 0 )
GAME( 1991, captavenj,  captaven, captaven, captaven, captaven, ROT0, "Data East Corporation", "Captain America and The Avengers (Japan Rev 0.2)", 0 )
GAME( 1993, dragngun,   0,        dragngun, dragngun, dragngun, ROT0, "Data East Corporation", "Dragon Gun (US)", GAME_IMPERFECT_GRAPHICS  )
GAME( 1993, fghthist,   0,        fghthist, fghthist, fghthist, ROT0, "Data East Corporation", "Fighter's History (World ver 43-07)", 0 )
GAME( 1993, fghthistu,  fghthist, fghthist, fghthist, fghthist, ROT0, "Data East Corporation", "Fighter's History (US ver 42-03)", 0 )
GAME( 1993, fghthista,  fghthist, fghthsta, fghthist, fghthist, ROT0, "Data East Corporation", "Fighter's History (US ver 42-05, alternate hardware )", 0 )
GAME( 1993, fghthistj,  fghthist, fghthist, fghthist, fghthist, ROT0, "Data East Corporation", "Fighter's History (Japan ver 42-03)", 0 )
GAME( 1994, lockload,   0,        lockload, lockload, lockload, ROT0, "Data East Corporation", "Locked 'n Loaded (World)", GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1994, lockloadu,  lockload, lockload, lockload, lockload, ROT0, "Data East Corporation", "Locked 'n Loaded (US)", GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1994, tattass,    0,        tattass,  tattass,  tattass,  ROT0, "Data East Pinball",     "Tattoo Assassins (US prototype)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, tattassa,   tattass,  tattass,  tattass,  tattass,  ROT0, "Data East Pinball",     "Tattoo Assassins (Asia prototype)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, nslasher,   0,        nslasher, nslasher, nslasher, ROT0, "Data East Corporation", "Night Slashers (Korea Rev 1.3)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, nslasherj,  nslasher, nslasher, nslasher, nslasher, ROT0, "Data East Corporation", "Night Slashers (Japan Rev 1.2)", GAME_IMPERFECT_GRAPHICS )
GAME( 1994, nslashers,  nslasher, nslasher, nslasher, nslasher, ROT0, "Data East Corporation", "Night Slashers (Over Sea Rev 1.2)", GAME_IMPERFECT_GRAPHICS )
