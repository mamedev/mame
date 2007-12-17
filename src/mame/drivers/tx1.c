/*====================================================================*/
/*               TX-1/Buggy Boy  (Tatsumi) Hardware                   */
/*                     Philip J Bennett 2005                          */
/*====================================================================*/

/*
Notes:
======
TX-1
====
* ROM dumps incomplete.
* Arithmetic unit/interface not implemented.
* Several sound related things are guessed (I don't have the relevent schematic sheet or an actual PCB to verify).

Buggy Boy
=========
* Buggy Boy/Speed Buggy and the copyrights are changed via wire jumper on the sound PCB.
* Fiddle with dipswitch 'Do Not Change 2' to skip errors if necessary.
* The 3-monitor version of Buggy Boy is not fully dumped (will be taken care of).

* Arithmetic unit/interface not fully implemented/understood.
* Road hardware emulation is lacking.
* Object drawing is not fully understood (scale parameters and end of line/sprite flags especially).
* Discrete sound emulation is not implemented.
* Character layer scrolling is not implemented.
* Layer mixing is incorrect.

* Buggy Boy controls don't work correctly.
* At some point the slave CPU writes F0 to EVERY 15th memory location. Hmmm. (see PC=fc630)
* Without ROM patches, the game hangs on bootup (sometimes giving error 22): Z80->8086 communication timeout.
  The Z80<->Main CPU comms needs work (although they work fine during the game).

Useful memory locations:
* Score = 0x544
* Timer = 0x54A

Buggy Boy Error Codes             TX-1 Error Codes
=====================             ================

1. Main CPU RAM                   1. Main microprocessor RAM
2. Video (character) RAM          2. Video RAM
3. Road/common RAM                3. Common RAM
4. Sound RAM                      4. Sound RAM
5. Main CPU ROM                   5. Main microprocessor ROM
6. Sound ROM                      6. Sound ROM
8. Auxillary ROM                  10. Interface ROM (time-out error)
12. Arithmetic unit               11. Common RAM (access for arithmetic CPU)
22. Main 8086-Z80 timeout         12. Common RAM (access for arithmetic CPU)
                                  13. Arithmetic RAM
                                  14. Common RAM (access for arithmetic CPU)
                                  15. Object RAM
                                  16. Arithmetic ROM
                                  17. Data ROM (Checksum)
                                  18. Arithmetic unit
*/

#include "driver.h"
#include "machine/8255ppi.h"
#include "sound/ay8910.h"
#include "cpu/i86/i86.h"
#include "rendlay.h"

#define PRINT_CRT 0
#define ROM_PATCHES 1

static int TS, z80_int = 0;

UINT16 *tx1_vram;
static UINT16 *tx1_object_ram;

UINT16 *buggyboy_vram;	/* Tile RAM (three monitor)   */
UINT16 *buggyb1_vram;	/* Tile RAM (single monitor)  */
static UINT8 *z80_ram;	/* Main 8086/Z80 shared RAM   */
UINT16 *bb_objram;
UINT16 *bb_sky;		/* Sky register */
static UINT16 *bb_rcram;

static size_t tx1_objectram_size;
size_t bb_objectram_size;
static size_t bb_rcram_size;

tilemap *tx1_tilemap;
tilemap *buggyboy_tilemap;
tilemap *buggyb1_tilemap;

/* machine/tx1.c */
void MMI_74S516(int ins, UINT16 *data);
READ16_HANDLER( BB_AU_R );
WRITE16_HANDLER( BB_AU_W );

/* video/tx1.c */
WRITE16_HANDLER( tx1_vram_w );
VIDEO_START( tx1 );
VIDEO_UPDATE( tx1 );

PALETTE_INIT( buggyboy );
WRITE16_HANDLER( buggyboy_vram_w );
WRITE16_HANDLER( buggyb1_vram_w );
VIDEO_START( buggyboy );
VIDEO_UPDATE( buggyboy );
VIDEO_START( buggyb1 );
VIDEO_UPDATE( buggyb1 );


static INTERRUPT_GEN( main_irq )
{
	cpunum_set_input_line_and_vector(0, 0, HOLD_LINE, 0x80/4);
}

/* Periodic Z80 interrupt */
static INTERRUPT_GEN( z80_irq )
{
        if(!z80_int){
	   cpunum_set_input_line(2, 0, HOLD_LINE);
	   z80_int = 1;
	}
}


/* HD46505S-2 CRT Controller */
/* Buggy Boy: The main 8086 at F003C expects non-zero value or else jump to 4000:0000 and execute crap */
static READ16_HANDLER(crt_read)
{
       return 1;
}

/* Remove this eventually */
static WRITE16_HANDLER(crt_write)
{
#if PRINT_CRT
	if (ACCESSING_LSB)
	{
		data &= 0xff;
       if(offset==0)
       {
        switch (data)
        {
        	case 0:mame_printf_debug("Horizontal Total");
        	       break;
        	case 1:mame_printf_debug("Horizontal displayed");
               	       break;
        	case 2:mame_printf_debug("Horizontal sync position");
        	       break;
        	case 3:mame_printf_debug("Horizontal sync width");
        	       break;
        	case 4:mame_printf_debug("Vertical total");
        	       break;
        	case 5:mame_printf_debug("Vertical total adj");
        	       break;
        	case 6:mame_printf_debug("Vertical displayed");
        	       break;
        	case 7:mame_printf_debug("Vertical sync position");
        	       break;
        	case 8:mame_printf_debug("Interlace mode");
        	       break;
        	case 9:mame_printf_debug("Max. scan line address");
        	       break;
        	case 0xa:mame_printf_debug("Cursror start");
        	       break;
        	case 0xb:mame_printf_debug("Cursor end");
        	       break;
        	case 0xc:mame_printf_debug("Start address (h)");
        	       break;
        	case 0xd:mame_printf_debug("Start address (l)");
        	       break;
        	case 0xe:mame_printf_debug("Cursor (h)");
        	       break;
        	case 0xf:mame_printf_debug("Cursor (l)");
        	       break;
        	case 0x10:mame_printf_debug("Light pen (h))");
        	       break;
        	case 0x11:mame_printf_debug("Light pen (l)");
        	       break;
        }
       }
        if(offset==1) mame_printf_debug("=         0x%x, %d\n",data,data);
    }
#endif
}


/* Main CPU and Z80 synchronisation*/
static WRITE16_HANDLER(z80_busreq)
{
	if (ACCESSING_LSB)
	{
		if (data & 0xff)
			cpunum_set_input_line(2, INPUT_LINE_HALT, CLEAR_LINE);
		else
			cpunum_set_input_line(2, INPUT_LINE_HALT, ASSERT_LINE);
	}
}


/* Called by main 8086 to wake up slave */
static WRITE16_HANDLER(resume_slave)
{
	cpunum_set_input_line(1, INPUT_LINE_TEST, ASSERT_LINE);
}

static READ16_HANDLER(halt_slave_r)
{
	cpunum_set_input_line(1, INPUT_LINE_TEST, CLEAR_LINE);
	return 0;
}

/* Called by slave 8086 to halt on WAIT instruction */
static void halt_slave(void)
{
	cpunum_set_input_line(1, INPUT_LINE_TEST, CLEAR_LINE);
}


static WRITE8_HANDLER(z80_intreq_w)
{
       z80_int=0;
}

static READ8_HANDLER(z80_intreq_r)
{
       z80_int=0;
       return 0;
}


/* Handlers for main CPU<->Z80 RAM */
static READ16_HANDLER(z80_shared_r)
{
	return z80_ram[offset];
}

static WRITE16_HANDLER(z80_shared_w)
{
	if (ACCESSING_LSB)
		z80_ram[offset] = data;
}

static INPUT_PORTS_START( tx1 )
	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x0700, 0x0300,  DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0000, "A" )
	PORT_DIPSETTING(      0x0100, "B" )
	PORT_DIPSETTING(      0x0200, "C" )
	PORT_DIPSETTING(      0x0300, "D" )
	PORT_DIPSETTING(      0x0400, "E" )
	PORT_DIPSETTING(      0x0500, "F" )
	PORT_DIPSETTING(      0x0600, "G" )
	PORT_DIPSETTING(      0x0700, "H" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(      0x0000, "A" )
	PORT_DIPSETTING(      0x0800, "B" )
	PORT_DIPSETTING(      0x1000, "C" )
	PORT_DIPSETTING(      0x1800, "D" )

	PORT_DIPNAME( 0xe000, 0xe000, "Bonus Adder" )
	PORT_DIPSETTING(    0x0000, "No Bonus" )
	PORT_DIPSETTING(    0x2000, "2 Coin Units for 1 Credit" )
	PORT_DIPSETTING(    0x4000, "3 Coin Units for 1 Credit" )
	PORT_DIPSETTING(    0x6000, "4 Coin Units for 1 Credit" )
	PORT_DIPSETTING(    0x8000, "5 Coin Units for 1 Credit" )
	PORT_DIPSETTING(    0xa000, "4 Coin Units for 2 Credit" )
	PORT_DIPSETTING(    0xc000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0xe000, "No Bonus" )

	/* These don't correspond to the Atari manual :( */
	PORT_DIPNAME( 0x0003, 0x0003, "Game Cost" )
	PORT_DIPSETTING(      0x0000, "1 Coin Unit for 1 Credit" )
	PORT_DIPSETTING(      0x0001, "2 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x0002, "3 Coin Units for 1 Credit" )
	PORT_DIPSETTING(      0x0003, "4 Coin Units for 1 Credit" )

	PORT_DIPNAME( 0x0004, 0x0004, "Left Coin Mechanism" )
	PORT_DIPSETTING(    0x0000, "1 Coin for 1 Coin Unit" )
	PORT_DIPSETTING(    0x0004, "1 Coin for 2 Coin Units" )

	PORT_DIPNAME( 0x0018, 0x0000, "Right Coin Mechanism" )
	PORT_DIPSETTING(    0x0000, "1 Coin Units 1 Credit" )
	PORT_DIPSETTING(    0x0008, "1 Coin Units 4 Credit" )
	PORT_DIPSETTING(    0x0010, "1 Coin Units 5 Credit" )
	PORT_DIPSETTING(    0x0018, "1 Coin Unit 6 Credit" )

	PORT_START_TAG("ANALOG_A")
	PORT_BIT( 0x000f, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1) /* Accelerator */
	PORT_BIT( 0xf0, 0x10, IPT_PEDAL ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2) /* Brake */
	PORT_START_TAG("ANALOG_B")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1) /* Steering */
	/* Wire jumper setting on sound PCB */
	PORT_DIPNAME( 0xf0, 0x80,  DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x10, "1" )
	PORT_DIPSETTING(      0x20, "2" )
	PORT_DIPSETTING(      0x40, "3" )
	PORT_DIPSETTING(      0x80, "4" )

	PORT_START_TAG("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE
INPUT_PORTS_END


static INPUT_PORTS_START( buggyboy )
	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0xe000, "Free-Play" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x1800, DEF_STR( 1C_5C ) )

	PORT_DIPNAME( 0x0700, 0x0700,  "Do not change 1" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0600, "6" )
	PORT_DIPSETTING(      0x0700, "7" )

	PORT_DIPNAME( 0x0003, 0x0003,  "Do not change 2" )       /* Dipswitch 0 is unconnected */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )

	PORT_DIPNAME( 0x0004, 0x0004, "Message" )
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_DIPNAME( 0x0008, 0x0008, "Do not Change 3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )

	PORT_DIPNAME( 0x0030, 0x0030, "Time Rank" )
	PORT_DIPSETTING(      0x0000, "A" )
	PORT_DIPSETTING(      0x0010, "B" )
	PORT_DIPSETTING(      0x0020, "C" )
	PORT_DIPSETTING(      0x0030, "D" )

	PORT_DIPNAME( 0x00c0, 0x0040, "Game Rank" )
	PORT_DIPSETTING(      0x0000, "A")
	PORT_DIPSETTING(      0x0040, "B" )
	PORT_DIPSETTING(      0x0080, "C" )
	PORT_DIPSETTING(      0x00c0, "D" )

	PORT_START_TAG("YM2149_IC19_A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE /* Gear shift */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) PORT_TOGGLE


	/* Read by 8255 I think */
	PORT_START_TAG("YM2149_IC19_B")
	PORT_DIPNAME( 0xff, 0x80,  "Sound PCB Jumper:" )
	PORT_DIPSETTING(      0x00, "0" )
	PORT_DIPSETTING(      0x01, "1" )
	PORT_DIPSETTING(      0x02, "2" )
	PORT_DIPSETTING(      0x04, "3" )
	PORT_DIPSETTING(      0x08, "4" )
	PORT_DIPSETTING(      0x10, "5" )
	PORT_DIPSETTING(      0x20, "Speed Buggy/Data East" )
	PORT_DIPSETTING(      0x40, "Buggy Boy/Taito" )
	PORT_DIPSETTING(      0x80, "Buggy Boy/Tatsumi" )

	/* Fix me */
	PORT_START_TAG("ANALOG_A")
	PORT_BIT( 0x0f, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1) /* Accelerator */
	PORT_BIT( 0xf0, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1) /* Steering */
	PORT_START_TAG("ANALOG_B")
	PORT_BIT( 0xf0, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2) /* Brake */

INPUT_PORTS_END

static INPUT_PORTS_START( buggyb1 )
	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0xe000, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0xe000, "Free-Play" )

	PORT_DIPNAME( 0x1800, 0x0800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x1800, DEF_STR( 1C_6C ) )

	PORT_DIPNAME( 0x0700, 0x0700,  "Do not change 1" )
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0500, "5" )
	PORT_DIPSETTING(      0x0600, "6" )
	PORT_DIPSETTING(      0x0700, "7" )

	PORT_DIPNAME( 0x0003, 0x0003,  "Do not change 2" )       /* Dipswitch 0 is unconnected */
	PORT_DIPSETTING(      0x0000, "0" )
	PORT_DIPSETTING(      0x0001, "1" )
	PORT_DIPSETTING(      0x0002, "2" )
	PORT_DIPSETTING(      0x0003, "3" )

	PORT_DIPNAME( 0x0004, 0x0004, "Message" )
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japanese ) )

	PORT_DIPNAME( 0x0008, 0x0008, "Do not Change 3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )

	PORT_DIPNAME( 0x0030, 0x0030, "Time Rank" )
	PORT_DIPSETTING(      0x0000, "A" )
	PORT_DIPSETTING(      0x0010, "B" )
	PORT_DIPSETTING(      0x0020, "C" )
	PORT_DIPSETTING(      0x0030, "D" )

	PORT_DIPNAME( 0x00c0, 0x0040, "Game Rank" )
	PORT_DIPSETTING(      0x0000, "A")
	PORT_DIPSETTING(      0x0040, "B" )
	PORT_DIPSETTING(      0x0080, "C" )
	PORT_DIPSETTING(      0x00c0, "D" )

	PORT_START_TAG("YM2149_IC19_A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_TOGGLE  /* Gear shift */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) PORT_TOGGLE

	/* Wire jumper setting on sound PCB */
	PORT_START_TAG("YM2149_IC19_B")
	PORT_DIPNAME( 0xff, 0x80,  "Sound PCB Jumper:" )
	PORT_DIPSETTING(      0x00, "0" )
	PORT_DIPSETTING(      0x01, "1" )
	PORT_DIPSETTING(      0x02, "2" )
	PORT_DIPSETTING(      0x04, "3" )
	PORT_DIPSETTING(      0x08, "4" )
	PORT_DIPSETTING(      0x10, "5" )
	PORT_DIPSETTING(      0x20, "Speed Buggy/Data East" )
	PORT_DIPSETTING(      0x40, "Buggy Boy/Taito" )
	PORT_DIPSETTING(      0x80, "Buggy Boy/Tatsumi" )

	PORT_START_TAG("ANALOG_A")
	PORT_BIT( 0x0f, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)  /* Steering */
	PORT_BIT( 0xf0, 0x10, IPT_PEDAL ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1) /* Accelerator */
	PORT_START_TAG("ANALOG_B")
	PORT_BIT( 0xf0, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)  /* Brake */
INPUT_PORTS_END


/********************/
/* TX-1 Memory Maps */
/********************/
static ADDRESS_MAP_START( tx1_master, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x00fff) AM_MIRROR(0x1000) AM_RAM              /* RAM */
	AM_RANGE(0x02000, 0x02fff) AM_MIRROR(0x1000) AM_RAM              /* RAM */
	AM_RANGE(0x04000, 0x04fff) AM_MIRROR(0x1000) AM_RAM              /* Backup RAM */
	AM_RANGE(0x06000, 0x06009) AM_READWRITE(crt_read, crt_write)
	AM_RANGE(0x08000, 0x09fff) AM_READWRITE(MRA16_RAM,tx1_vram_w) AM_BASE(&tx1_vram)
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE(1)                    /* Road/common RAM */
	AM_RANGE(0x0b000, 0x0b001) AM_READWRITE (input_port_0_word_r, z80_busreq)
	AM_RANGE(0x0c000, 0x0c001) AM_RAM                                 /* /SCOLST */
	AM_RANGE(0x0d000, 0x0d001) AM_RAM                                 /* /SLINCS */
	AM_RANGE(0x0e000, 0x0e001) AM_RAM                                 /* /SLOCK  */
	AM_RANGE(0x0f000, 0x0f003) AM_READWRITE (MRA16_NOP, resume_slave)          /* Watchdog and slave resume */
	AM_RANGE(0x10000, 0x13fff) AM_ROM                                         /* Z80 ROM */
	AM_RANGE(0x16000, 0x16fff) AM_READWRITE (z80_shared_r, z80_shared_w)
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

/* Lot of unknown writes - I haven't investigated further */
static ADDRESS_MAP_START( tx1_slave, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x007ff) AM_MIRROR(0x800) AM_RAM               /* Arithmetic RAM (IC169, IC158) A10 is grounded */
	AM_RANGE(0x01000, 0x01fff) AM_RAM AM_SHARE(1)   /* Common RAM (IC195, IC210) */
	AM_RANGE(0x02000, 0x022ff) AM_RAM AM_BASE(&tx1_object_ram) AM_SIZE(&tx1_objectram_size)   /* Object RAM (IC43, IC68) A10-A8 are grounded*/
	AM_RANGE(0x02400, 0x027ff) AM_RAM               /* /BANKCS */
	AM_RANGE(0x02800, 0x02bff) AM_READWRITE(halt_slave_r, MWA16_NOP)  /* /DRAK */
	AM_RANGE(0x02C00, 0x02fff) AM_RAM               /* /FLGCS */
	AM_RANGE(0x03000, 0x03fff) AM_RAM               /* SN74S516 arithmetic unit */
	AM_RANGE(0x04000, 0x07fff) AM_ROM               /* ROM mirror? */
	AM_RANGE(0x08000, 0x0bfff) AM_ROM               /* ROM mirror? */
	AM_RANGE(0x0c000, 0x0ffff) AM_ROM               /* ROM mirror? */
	AM_RANGE(0xfc000, 0xfffff) AM_ROM
ADDRESS_MAP_END


/* Sound */
static ADDRESS_MAP_START( tx1_sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x3000, 0x37ff) AM_RAM AM_MIRROR(0x800) AM_BASE (&z80_ram)
	AM_RANGE(0x4000, 0x4000) AM_READ (z80_intreq_r)                                  /* Unconfirmed */
	AM_RANGE(0x5000, 0x5003) AM_READWRITE(ppi8255_0_r, ppi8255_0_w)
	AM_RANGE(0x6000, 0x6003) AM_RAM                                  /* 8253A PIT */
	AM_RANGE(0x7000, 0x70DF) AM_RAM                                  /* Propigate input signals into 8255 */
	AM_RANGE(0xb00b, 0xb00b) AM_RAM                                  /* Read once - TS?*/
ADDRESS_MAP_END


static ADDRESS_MAP_START( tx1_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x40, 0x40) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x41, 0x41) AM_WRITE(AY8910_control_port_0_w)
ADDRESS_MAP_END


/*************************/
/* Buggy Boy Memory Maps */
/*************************/

static READ16_HANDLER(input_port_0A_r)
{
	return (readinputport(0) & 0xfffe) | TS;
}

static ADDRESS_MAP_START( buggyb1_master, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM                             /* 1kB RAM - battery backed */
	AM_RANGE(0x04000, 0x04fff) AM_READWRITE(crt_read, crt_write)  /* HD46505S-2 or equivalent CRT controller */
	AM_RANGE(0x08000, 0x08fff) AM_READWRITE(MRA16_RAM,buggyb1_vram_w) AM_BASE(&buggyb1_vram)         /* Character mapped RAM */
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE(1) AM_BASE(&bb_rcram) AM_SIZE(&bb_rcram_size)     /* Road/common RAM - 1800-18ff & 1c00-1cff are read by road H/W */
	AM_RANGE(0x0b000, 0x0b001) AM_READWRITE (input_port_0A_r, z80_busreq)     /* Dipswitches and Z80 busreq */
	AM_RANGE(0x0c000, 0x0c001) AM_RAM                                         /* /SCOLW  */
	AM_RANGE(0x0d000, 0x0d003) AM_RAM                                         /* /SLINCS */
	AM_RANGE(0x0e000, 0x0e001) AM_RAM AM_BASE(&bb_sky)                        /* /SKYCS  */
	AM_RANGE(0x0f000, 0x0f003) AM_READWRITE (MRA16_NOP, resume_slave)          /* Watchdog and slave resume */
	AM_RANGE(0x10000, 0x17fff) AM_ROM                                         /* Z80 ROM */
	AM_RANGE(0x18000, 0x18fff) AM_READWRITE (z80_shared_r, z80_shared_w)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( buggyboy_master, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x03fff) AM_RAM                             /* 1kB RAM - battery backed */
	AM_RANGE(0x04000, 0x04fff) AM_READWRITE(crt_read, crt_write)  /* HD46505S-2 or equivalent CRT controller */
	AM_RANGE(0x08000, 0x09fff) AM_READWRITE(MRA16_RAM,buggyboy_vram_w) AM_BASE(&buggyboy_vram)           /* Character mapped RAM */
	AM_RANGE(0x0a000, 0x0afff) AM_RAM AM_SHARE(1) AM_BASE(&bb_rcram) AM_SIZE(&bb_rcram_size)            /* Road/common RAM */
	AM_RANGE(0x0b000, 0x0b001) AM_READWRITE (input_port_0A_r, z80_busreq)     /* Dipswitches and Z80 busreq */
	AM_RANGE(0x0c000, 0x0c001) AM_RAM                                         /* /SCOLW  */
	AM_RANGE(0x0d000, 0x0d003) AM_RAM                                         /* /SLINCS */
	AM_RANGE(0x0e000, 0x0e001) AM_RAM AM_BASE(&bb_sky)                        /* /SKYCS  */
	AM_RANGE(0x0f000, 0x0f003) AM_READWRITE (MRA16_NOP, resume_slave)          /* Watchdog and slave resume */
	AM_RANGE(0x10000, 0x17fff) AM_ROM                                         /* Z80 ROM */
	AM_RANGE(0x18000, 0x18fff) AM_READWRITE (z80_shared_r, z80_shared_w)
	AM_RANGE(0x20000, 0x2ffff) AM_ROM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END


/*
2400-24FF appear to be road control (R/W)

From the single monitor schematics:

-GAS = 24XX:
-BASET0 = 2400-F, 2410-F
-BASET1 = 2420-F, 2430-F
-BSET   = 2440-F, 2450-F
-HASET  = 2460-F, 2470-F
-HSET   = 2480-F, 2490-F
-WASET  = 24A0-F, 24B0-F
-FLAGS  = 24E0-F, 24F0-F
*/

static READ16_HANDLER(GAS_r)
{
       if(offset >= 0xe0/2)  halt_slave();
       return 0;
}

static WRITE16_HANDLER(GAS_w)
{
       if(offset >= 0xe0/2)  halt_slave();
}


/*
From single monitor PCB TC-041:

TZ0310 IC87 (PAL12L6)
============================
/SPCS  =  !A15.!A14.!A13.A12 + A14.!A13.!A12 + !A14.A12.A9 + !A14.A13./A12 + !A14.!A11 + A15
/PRCS (Program ROM)  =  !A15.!A14.A13.A12 + !A15.!A14.!A13.A12 + !A15.!A14.!A12
/ARCRCS (Road/Common RAM)  =  A15.!A14.A13.A12 + !A15.!A14.!A12 + A14 + A15
/INSALD =  /AT3RD.!AT3WDRART + !AT3WDRART.A15 + !AT3WDRART.!A13 + !AT3WDRART.A14 + !AT3WDRART.!A12 + !A7
/CNTST  =  /AT3RD.!AT3WDRART + !AT3WDRART.A15 + !AT3WDRART.!A13 + !AT3WDRART.A14 + !AT3WDRART.!A12 + !A8
LS245 IC24/55 /G (Program ROM and RAM data bus)  =  !A15.!A14.A13.!A11 + !A15.!A14.A13.A12 + !A15.!A14.!A13.A12 + /ADEN


TZ0311 IC88 (PAL10L8) (protection fuse was set, yet it read)
============================

/ARTCS (Arithmetic unit and interface)  =  !(A12.A13.!A14.!A15)
/GAS (Road registers)  =  !(Q.ADT/#R + Q.!Q.ADT/#R.!ADT/#R)            <-- ???
/EXPCS (Expansion RAM)  =  !(A11.!A12.A13.!A14.!A15)
/PRMHCS (Program RAM high-byte) = !(!A12.!A13.!A14.!A15.!/ADHE)
/PRMLCS (Program RAM low-byte) = !(!A0.!A12.!A13.!A14.!A15)
/OCRCS (Object RAM)  =  !(!A10.!A11.!A12.A13.!A14.!A15)
/MLPCS (Arithmetic Unit Instruction = A1-A3)  =  !(!A10.!A11.A12.A13.!A14.!A15)
/DPRCS (Function Data ROM)  =  !(A10.A11.A12.A13.!A14.!A15)
*/

/* Memory map and ROM contents are the same for both versions */
static ADDRESS_MAP_START( buggyboy_slave, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x00fff) AM_RAM                           /* 4kB RAM */
	AM_RANGE(0x01000, 0x01fff) AM_RAM AM_SHARE(1)               /* Road/Common RAM */
	AM_RANGE(0x02000, 0x022ff) AM_RAM AM_BASE(&bb_objram) AM_SIZE(&bb_objectram_size)    /* RAM A10-A8 are grounded */
	AM_RANGE(0x02400, 0x024ff) AM_READWRITE (GAS_r, GAS_w)      /* Road control registers */
	AM_RANGE(0x03000, 0x03fff) AM_READWRITE (BB_AU_R, BB_AU_W)  /* SN74S516 arithmetic unit */
	AM_RANGE(0x04000, 0x07fff) AM_ROM                           /* ROM mirror */
	AM_RANGE(0x08000, 0x0bfff) AM_ROM                           /* ROM mirror */
	AM_RANGE(0x0c000, 0x0ffff) AM_ROM                           /* ROM mirror */
	AM_RANGE(0xfc000, 0xfffff) AM_ROM                           /* ROM */
ADDRESS_MAP_END


/* Connected in place of dipswitch A bit 0 */
/* Accessed on startup as some sort of acknowledgement */
static WRITE8_HANDLER(_TS_)
{
//       TS = 1;
}

/* Buggy Boy Sound PCB TC033A. */
static ADDRESS_MAP_START( buggyboy_sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM  AM_BASE(&z80_ram)                /* 2kB RAM - shared with Main CPU */
	AM_RANGE(0x6000, 0x6000) AM_READ(input_port_4_r)                  /* Steering and accelerator */
	AM_RANGE(0x6001, 0x6001) AM_READ(input_port_5_r)                  /* Brake (NOT USED) */
	AM_RANGE(0x6800, 0x6803) AM_READWRITE(ppi8255_0_r, ppi8255_0_w)   /* Digital inputs (coins etc.) - check me */
	AM_RANGE(0x7000, 0x7003) AM_RAM                                   /* 8253 Programmable Interval Timer (Engine Sound) */
	AM_RANGE(0xc024, 0xc024) AM_WRITE(_TS_)                           /* /TS? */
	AM_RANGE(0x7800, 0x7800) AM_WRITE(z80_intreq_w)                     /* /INTREQ? */
ADDRESS_MAP_END

/* Buggy Boy (single monitor) Sound PCB TC-043 (no 8255). */
static ADDRESS_MAP_START( buggyb1_sound_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM AM_BASE(&z80_ram)                     /* 2kB RAM - shared with Main 8086 */
	AM_RANGE(0x5000, 0x5003) AM_RAM                                       /* 8253 Programmable Interval Timer (Engine Sound) */
	AM_RANGE(0x6000, 0x6000) AM_READ(input_port_4_r)                      /* Steering and accelerator */
	AM_RANGE(0x6001, 0x6001) AM_READ(input_port_5_r)                      /* Brake (NOT USED) */
	AM_RANGE(0xc024, 0xc024) AM_WRITE(_TS_)                               /* /TS? */
	AM_RANGE(0x7000, 0x7001) AM_WRITE(z80_intreq_w)                       /* /INTREQ */
ADDRESS_MAP_END


/* Common to both versions*/
static ADDRESS_MAP_START( buggyboy_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x40, 0x40) AM_READWRITE(AY8910_read_port_0_r, AY8910_write_port_0_w)
	AM_RANGE(0x41, 0x41) AM_WRITE(AY8910_control_port_0_w)

	AM_RANGE(0x80, 0x80) AM_READWRITE(AY8910_read_port_1_r, AY8910_write_port_1_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(AY8910_control_port_1_w)
ADDRESS_MAP_END


/* Common to TX-1 and Buggy Boy */
static const gfx_layout char_layout =
{
	8,8,
	2048,
	2,
	{ 0, 8*8*2048 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout bb_object_layout =
{
	8,8,
	4096,
	2,
	{ 0,  8*8*4096 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tx1_object_layout =
{
	8,8,
	2048,
	2,
	{ 0, 8*8*2048 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( bb )
	GFXDECODE_ENTRY( REGION_GFX1, 0, char_layout, 0, 256 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, bb_object_layout, 256, 2048  )
	GFXDECODE_ENTRY( REGION_GFX3, 0, bb_object_layout, 256, 2048  )
	GFXDECODE_ENTRY( REGION_GFX4, 0, bb_object_layout, 256, 2048  )
GFXDECODE_END

static GFXDECODE_START( tx1 )
	GFXDECODE_ENTRY( REGION_GFX1, 0, char_layout, 0, 16 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, tx1_object_layout, 0, 16 )
	GFXDECODE_ENTRY( REGION_GFX3, 0, tx1_object_layout, 0, 16 )
GFXDECODE_END


/****************************/
/*    TX-1 Sound Hardware   */
/****************************/

static const struct AY8910interface tx1_ay8910_interface =
{
	0,                        /* Probably hooked up to discrete sound filters */
	0,
        0,
        0,
};

/****************************/
/* Buggy Boy Sound Hardware */
/****************************/

/*
According to the single monitor schematics:

                YM-2149 IC 24
                =============

       Port A                     Port B
       ======                     ======

0: Engine sound control       0: Coin Counter 1
1: Engine sound control       1: Coin Counter 2
2: Engine sound control       2: Coin Counter 3 (Unused)
3: Engine sound control       3: Engine sound 4066B
4: Engine sound control       4: Noise EN1
5: Engine sound control       5: Noise EN2
6: Engine sound control       6: Enable YM IC24 on rear right speaker
7: Engine Noise control       7: Enable YM IC19 on rear left speaker

                YM-2149 IC 19
                =============

       Port A                     Port B
       ======                     ======

0: Engine sound control       0: Coin Counter 1
1: Engine sound control       1: Coin Counter 2
2: Engine sound control       2: Coin Counter 3 (Unused)
3: Engine sound control       3: Engine sound 4066B
4: Engine sound control       4: Noise EN1
5: Engine sound control       5: Noise EN2
6: Engine sound control       6: Enable YM IC24 on rear right speaker
7: Engine Noise control       7: Enable YM IC19 on rear left speaker
*/


static WRITE8_HANDLER(BB_YM2149_1_A_w)
{

}

static WRITE8_HANDLER(BB_YM2149_1_B_w)
{
    coin_counter_w(0,data & 0x01);
    coin_counter_w(1,data & 0x02);
}


/* YM2149 IC19 - front left */
static const struct AY8910interface buggyboy_ay8910_interface_1 =
{
	input_port_2_r,
	input_port_3_r,
	0,
	0,
};


/* YM2149 IC24 - front right */
static const struct AY8910interface buggyboy_ay8910_interface_2 =
{
	0,
	0,
	BB_YM2149_1_A_w,
	BB_YM2149_1_B_w,
};

static WRITE8_HANDLER(TX1_COIN_COUNTER)
{
       coin_counter_w(0,data & 0x80);
       coin_counter_w(1,data & 0x40);
}
static const ppi8255_interface tx1_ppi8255_intf =
{
	1,
	{ input_port_2_r },         /* Accelerator and brake */
	{ input_port_3_r },         /* Steering and sound wire jumpers */
	{ input_port_4_r },         /* Coin and shift inputs */
	{ NULL },
	{ NULL },
	{ TX1_COIN_COUNTER },
};


/* Buggy Boy (3-monitor) uses a 8255 instead of the YM2149 ports for inputs */
static const ppi8255_interface buggyboy_ppi8255_intf =
{
	1,
	{ input_port_2_r },
	{ NULL },
	{ input_port_3_r },
	{ NULL },
	{ NULL },
	{ NULL },
};

static MACHINE_RESET( buggyb1 )
{
       UINT8 *rom = (UINT8 *)memory_region(REGION_CPU1);

/*
    The main CPU /TEST line is connected to the /BUSAK output of the Z80.

    TODO: Improve communications.
*/
#if ROM_PATCHES
       rom[0xf00d0]=0x3b;     /* Patch out cmp Al,05 - z80 comms */
       rom[0xf00d1]=0xc0;
#endif
}


static MACHINE_RESET( buggyboy )
{
       UINT8 *rom = (UINT8 *)memory_region(REGION_CPU1);

       ppi8255_init(&buggyboy_ppi8255_intf);

#ifdef ROM_PATCHES
       rom[0xf00d0]=0x3b;     /* Patch out cmp Al,05 - z80 comms */
       rom[0xf00d1]=0xc0;
#endif
}

static MACHINE_RESET( tx1 )
{
	UINT8 *rom = (UINT8 *)memory_region(REGION_CPU1);
	UINT8 *rom_b = (UINT8 *)memory_region(REGION_CPU2);

	ppi8255_init(&tx1_ppi8255_intf);

	rom = rom;
	rom_b = rom_b;

       /* For tx1a: Needed to skip the startup tests */
#if 0
	rom_b[0xfcf38]=0x3B;	/* Patch out AU check */
	rom_b[0xfcf39]=0xc0;
	rom_b[0xfcf3a]=0x90;
	rom_b[0xfcf3b]=0x90;
	rom_b[0xfcf3c]=0x90;

	rom_b[0xfce68]=0x3b;
	rom_b[0xfce69]=0xc0;
	rom_b[0xfce6a]=0x90;

	rom_b[0xfcf43]=0x3B;	/* Patch out AU check */
	rom_b[0xfcf44]=0xc0;
	rom_b[0xfcf45]=0x90;
	rom_b[0xfcf46]=0x90;
	rom_b[0xfcf47]=0x90;

	rom[0xf3c38]=0x3b;	/* Main ROM Checksum */
	rom[0xf3c39]=0xc0;
	rom[0xf0251]=0x3b;	/* Patch out cmp Al,?? */
	rom[0xf0252]=0xc0;
	rom[0xf0265]=0x3b;	/* Patch out cmp Al,?? */
	rom[0xf0266]=0xc0;
	rom[0xf0271]=0x90;	/* NOP */
	rom[0xf0280]=0x3b;	/* Patch out cmp Al,??  */
	rom[0xf0281]=0xc0;
#endif
}

static MACHINE_DRIVER_START( tx1 )
	MDRV_CPU_ADD(I8086,5000000 )
	MDRV_CPU_PROGRAM_MAP(tx1_master,0)
	MDRV_CPU_PERIODIC_INT(main_irq, 46 )        /* To do: measure HD46505 CUDISP output rate */
	//MDRV_WATCHDOG_TIME_INIT(ATTOTIME_IN_SEC(5))  /* To do: measure watchdog time interval */

	MDRV_CPU_ADD(I8086,5000000 )
	MDRV_CPU_PROGRAM_MAP(tx1_slave,0)

	MDRV_CPU_ADD(Z80,3750000 )                  /* Guess */
	MDRV_CPU_PROGRAM_MAP(tx1_sound_prg,0)
	MDRV_CPU_IO_MAP(tx1_sound_io,0)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 915.52734375/2 )         /* Guess */

	MDRV_MACHINE_RESET(tx1)
	MDRV_INTERLEAVE(100)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_GFXDECODE(tx1)
	MDRV_PALETTE_LENGTH(32768)
	MDRV_DEFAULT_LAYOUT(layout_triphsxs)

	MDRV_SCREEN_ADD("left", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_SCREEN_ADD("middle", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_SCREEN_ADD("right", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_VIDEO_START(tx1)
	MDRV_VIDEO_UPDATE(tx1)

	MDRV_SPEAKER_STANDARD_MONO("Mono")                           /* Mono for the time being */

	MDRV_SOUND_ADD(AY8910, 1875000)                              /* Guess */
	MDRV_SOUND_CONFIG(tx1_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "Mono", 0.5)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( buggyboy )
	MDRV_CPU_ADD(I8086,5000000 )
	MDRV_CPU_PROGRAM_MAP(buggyboy_master,0)
	MDRV_CPU_PERIODIC_INT(main_irq, 46 )    /* To do: measure HD46505 CUDISP output rate */
	//MDRV_WATCHDOG_TIME_INIT(ATTOTIME_IN_SEC(5))  /* To do: measure watchdog time interval */

	MDRV_CPU_ADD(I8086,5000000 )
	MDRV_CPU_PROGRAM_MAP(buggyboy_slave,0)

	MDRV_CPU_ADD(Z80,3750000 ) /* audio CPU */
	MDRV_CPU_PROGRAM_MAP(buggyboy_sound_prg,0)
	MDRV_CPU_PERIODIC_INT(z80_irq, 915.52734375/2 )  /* To do: verify against real PCB */
	MDRV_CPU_IO_MAP(buggyboy_sound_io,0)

	MDRV_MACHINE_RESET(buggyboy)
	MDRV_INTERLEAVE(100)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_GFXDECODE(bb)
	MDRV_DEFAULT_LAYOUT(layout_triphsxs)

	MDRV_SCREEN_ADD("left", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_SCREEN_ADD("middle", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_SCREEN_ADD("right", 0x000)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-1)

	MDRV_PALETTE_LENGTH(256)
	MDRV_COLORTABLE_LENGTH(256+(256*4)+(2048*4))
	MDRV_PALETTE_INIT(buggyboy)
	MDRV_VIDEO_START(buggyboy)
	MDRV_VIDEO_UPDATE(buggyboy)

	/* Sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("Front Left", "Front Right")
     /* MDRV_SPEAKER_STANDARD_STEREO("Rear Left", "Rear Right") */

	MDRV_SOUND_ADD(AY8910, 1875000)
	MDRV_SOUND_CONFIG(buggyboy_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "Front Left", 0.15)

	MDRV_SOUND_ADD(AY8910, 1875000)
	MDRV_SOUND_CONFIG(buggyboy_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "Front Right", 0.15)
MACHINE_DRIVER_END


/* Need to verify clocks on real PCB */
static MACHINE_DRIVER_START( buggyb1 )
	MDRV_CPU_ADD(I8086,5000000 )
	MDRV_CPU_PROGRAM_MAP(buggyb1_master,0)
	MDRV_CPU_PERIODIC_INT(main_irq, 46 )    /* To do: measure HD46505 CUDISP output rate */
	//MDRV_WATCHDOG_TIME_INIT(ATTOTIME_IN_SEC(5))  /* To do: measure watchdog time interval */

	MDRV_CPU_ADD(I8086,5000000 )
	MDRV_CPU_PROGRAM_MAP(buggyboy_slave,0)

	MDRV_CPU_ADD(Z80,3750000 ) /* audio CPU */
	MDRV_CPU_PROGRAM_MAP(buggyb1_sound_prg,0)
	MDRV_CPU_PERIODIC_INT(z80_irq, 915.52734375/2 )  /* To do: verify against real PCB*/
	MDRV_CPU_IO_MAP(buggyboy_sound_io,0)

	/* Confirm this stuff */
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)
	MDRV_MACHINE_RESET( buggyb1 )
	MDRV_INTERLEAVE(100)                                    /* ? */

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 255, 0, 239)
	MDRV_GFXDECODE(bb)

	MDRV_PALETTE_LENGTH(256)
	MDRV_COLORTABLE_LENGTH(256+(256*4)+(2048*4))     /* 256 for chars, 2048 for objects */
	MDRV_PALETTE_INIT(buggyboy)
	MDRV_VIDEO_START(buggyb1)
	MDRV_VIDEO_UPDATE(buggyb1)

	/* Sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("Front Left", "Front Right")
     /* MDRV_SPEAKER_STANDARD_STEREO("Rear Left", "Rear Right") */

	MDRV_SOUND_ADD(AY8910, 1875000)
	MDRV_SOUND_CONFIG(buggyboy_ay8910_interface_1)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "Front Left", 0.15)

	MDRV_SOUND_ADD(AY8910, 1875000)
	MDRV_SOUND_CONFIG(buggyboy_ay8910_interface_2)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "Front Right", 0.15)
MACHINE_DRIVER_END


ROM_START( tx1 )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "8411-136027-157.11", 0x10000, 0x2000, CRC(10ae3075) SHA1(69c5f62f2473aba848383eed3cecf15e273d86ca) )   /* Z80 ROM - checksummed by main CPU */

	ROM_LOAD16_BYTE( "tx1_1c.ic22", 0xf0000, 0x4000, CRC(eedcee83) SHA1(7fa0590b142fb13c6562126a9bdd5a1e032880c7) )
	ROM_LOAD16_BYTE( "tx1_2c.ic29", 0xf0001, 0x4000, CRC(294bf5bf) SHA1(02b425caba8a187c58211bab27988205eb044558) )
	ROM_LOAD16_BYTE( "tx1_4c.ic54", 0xf8001, 0x4000, CRC(15bb8ef2) SHA1(83968f010ec555fcd0548a80562fb23a892b5afb) )
	ROM_LOAD16_BYTE( "tx1_3c.ic45", 0xf8000, 0x4000, CRC(21a8aa55) SHA1(21bc4adefb22a95fcd7a4e305bf0b05e2cb34129) )

	ROM_REGION( 0x100000, REGION_CPU2, 0 )
	ROM_LOAD16_BYTE( "tx1_9c.ic146", 0xfc000, 0x2000, CRC(b65eeea2) SHA1(b5f26e17520c598132b93c5cd7af7ebd03b10012) )
	ROM_RELOAD(                  0x4000, 0x2000 )
	ROM_RELOAD(                  0x8000, 0x2000 )
	ROM_RELOAD(                  0xc000, 0x2000 )
	ROM_LOAD16_BYTE( "tx1_8c.ic132", 0xfc001, 0x2000, CRC(0d63dadc) SHA1(0954174b25c08967d3efb31f5721fd05502d66dd) )
	ROM_RELOAD(                  0x4001, 0x2000 )
	ROM_RELOAD(                  0x8001, 0x2000 )
	ROM_RELOAD(                  0xc001, 0x2000 )
	/* Sound */
	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "8411-136027-157.11", 0x00000, 0x2000, CRC(10ae3075) SHA1(69c5f62f2473aba848383eed3cecf15e273d86ca) )

	/* 8x8 characters */
	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "tx1_20a.ic174", 0x0000, 0x4000, CRC(dbe595fc) SHA1(1ed2f775f0a1b46a2ffbc056eb4ef732ed546d3c) )
	ROM_LOAD( "tx1_21a.ic204", 0x4000, 0x4000, CRC(cd3441ad) SHA1(8e6597b3177b8aaa34ed3373d85fc4b6231e1333) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "tx1_16b.ic203", 0x0000, 0x4000, CRC(1141c965) SHA1(4b90c1428bcbd72d0449c064856a5596269b3fc6) )
	ROM_LOAD( "tx1_18b.ic258", 0x4000, 0x4000, NO_DUMP )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "tx1_15b.ic173", 0x0000, 0x4000, CRC(30d1a8d5) SHA1(b4c585b7b8a8920bb3949d643e9e10c17d4009a0) )
	ROM_LOAD( "tx1_17b.ic232", 0x4000, 0x4000, CRC(364bb354) SHA1(a26581ca1088b979285471e2c6595048df84d75e) )

	/* Road LUTs */
	ROM_REGION( 0x40000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "tx1_5a.ic56", 0x0000, 0x2000, CRC(5635b8c1) SHA1(5cc9437a2ff0843f1917f2451852d4561c240b24) )
	ROM_LOAD( "tx1_6a.ic66", 0x2000, 0x2000, CRC(03d83cf8) SHA1(5c0cfc6bf02ad2b3f37e1ceb493f69eb9829ab1e) )
	ROM_LOAD( "tx1_7a.ic76", 0x4000, 0x2000, CRC(ad56013a) SHA1(ae3a91f58f30daff334754476db33ad1d12569fc) )

	/* Arithmetic Unit Function Data ROMs */
	ROM_REGION( 0x10000, REGION_USER1, 0 )
	ROM_LOAD16_BYTE( "tx1_10b.ic184", 0x0000, 0x4000, CRC(acf754e8) SHA1(06779e18636f0799efdaa09396e9ccd59f426257) )
	ROM_LOAD16_BYTE( "tx1_11b.ic185", 0x0001, 0x4000, CRC(f89d3e20) SHA1(4b4cf679b7e3d63cded9989d2b667941f718ff57) )

	ROM_REGION( 0x50000, REGION_USER2, 0 )
	ROM_LOAD( "tx1_14b.ic106", 0x0000, 0x4000, CRC(68c63d6e) SHA1(110e02b99c44d31041be588bd14642e26890ecbd) ) /* Object chunk sequence LUT */
	ROM_LOAD( "tx1_13b.ic73", 0x4000, 0x4000, CRC(b0c581b2) SHA1(20926bc15e7c97045b219b828acfcdd99b8712a6) ) /* Object chunk sequence LUT */

	ROM_REGION( 0x50000, REGION_USER3, 0 )
	ROM_LOAD( "tx1_12b.ic48", 0x0000, 0x2000, CRC(4b3d7956) SHA1(fc2432dd69f3be7007d4fd6f7c86c7c19453b1ba) ) /* Object LUT */
	ROM_LOAD( "tx1_19b.ic281", 0x2000, 0x4000, NO_DUMP ) /* Object palette LUT */

	ROM_REGION( 0x10000, REGION_PROMS, 0 )
	ROM_LOAD( "xb01b.ic213",0x0000, 0x200, CRC(f6b8b70b) SHA1(b79374acf11d71db1e4ad3c494ac5f500a52677b) )
	ROM_LOAD( "xb02b.ic223",0x0200, 0x200, CRC(22c77af6) SHA1(1be8585b95316b4fc5712cdaef699e676320cd4d) )
	ROM_LOAD( "xb03a.ic25", 0x0300, 0x100, CRC(616a7a85) SHA1(b7c1060ecb128154092441212de64dc304aa3fcd) )
	ROM_LOAD( "xb04a.ic276",0x0500, 0x200, CRC(92bf5533) SHA1(4d9127417325af66099234178ab2641d23ee9d22) )
	ROM_LOAD( "xb04a.ic277",0x0700, 0x200, CRC(92bf5533) SHA1(4d9127417325af66099234178ab2641d23ee9d22) )
	ROM_LOAD( "xb05a.ic36", 0x0800, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "xb05a.ic57", 0x0900, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "xb05a.ic8",  0x0a00, 0x100, CRC(3b387d01) SHA1(1229548e3052ad34eeee9598743091d19f6b8f88) )
	ROM_LOAD( "xb06a.ic37", 0x0b00, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "xb06a.ic58", 0x0c00, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "xb06a.ic9",  0x0d00, 0x100, CRC(f6f4d7d9) SHA1(866024b76b26d6942bd4e1d2494686299414f6be) )
	ROM_LOAD( "xb07a.ic10", 0x0e00, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )
	ROM_LOAD( "xb07a.ic38", 0x0f00, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )
	ROM_LOAD( "xb07a.ic59", 0x1000, 0x100, CRC(824e7532) SHA1(917ce74d2bae6af90f2c4e41d12a69f884320915) )
	ROM_LOAD( "xb08.ic116", 0x1100, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )
	ROM_LOAD( "xb08.ic148", 0x1200, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )
	ROM_LOAD( "xb08.ic85",  0x1300, 0x100, CRC(5aeef5cc) SHA1(e123bf01d556178b0cf9d495bcce445f3f8421cd) )
	ROM_LOAD( "xb09.ic33",  0x1500, 0x200, CRC(fafb6917) SHA1(30eb182c7623026dce7dba9e249bc8a9eb7a7f3e) )
	ROM_LOAD( "xb10.ic40",  0x1700, 0x200, CRC(93deb894) SHA1(5ae9a21298c836fe649a52f3df2b4067f9012b91) )
	ROM_LOAD( "xb11.ic49",  0x1900, 0x200, CRC(aa5ed232) SHA1(f33e7bc2dd33ac6d75fb06b93c4dd58e5d10010d) )
	ROM_LOAD( "xb12.ic50",  0x1b00, 0x200, CRC(6b424cea) SHA1(83127326c20116b0a4be1126e163f9c6755e19dc) )
ROM_END

ROM_START( tx1a )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "8411-136027-157.11", 0x10000, 0x2000, CRC(10ae3075) SHA1(69c5f62f2473aba848383eed3cecf15e273d86ca) )   /* Z80 ROM - checksummed by main CPU */

	ROM_LOAD16_BYTE( "8412-136027-244.22", 0xf0000, 0x4000, CRC(2e9cefa2) SHA1(4ca04eae446e8df08ab793488a79217ed1a27875) )
	ROM_LOAD16_BYTE( "8412-136027-245.29", 0xf0001, 0x4000, CRC(ade7895c) SHA1(1c33a574cae46fddb4cadb85f5de17f02ae7a596) )
	ROM_LOAD16_BYTE( "8412-136027-250.54", 0xf8001, 0x4000, CRC(c8c9368f) SHA1(0972d54d506216eb2b204cf22ccdff9210fb7b10) )
	ROM_LOAD16_BYTE( "8412-136027-249.45", 0xf8000, 0x4000, CRC(9bcb82db) SHA1(d1528c9b9c4c2848bdba15e4632927476d544f40))

	ROM_REGION( 0x100000, REGION_CPU2, 0 )
	ROM_LOAD16_BYTE( "8411-136027-152.146", 0xfc000, 0x2000, CRC(b65eeea2) SHA1(b5f26e17520c598132b93c5cd7af7ebd03b10012) )
	ROM_RELOAD(                  0x4000, 0x2000 )
	ROM_RELOAD(                  0x8000, 0x2000 )
	ROM_RELOAD(                  0xc000, 0x2000 )
	ROM_LOAD16_BYTE( "8411-136027-151.132", 0xfc001, 0x2000, CRC(0d63dadc) SHA1(0954174b25c08967d3efb31f5721fd05502d66dd) )
	ROM_RELOAD(                  0x4001, 0x2000 )
	ROM_RELOAD(                  0x8001, 0x2000 )
	ROM_RELOAD(                  0xc001, 0x2000 )

	/* Sound */
	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "8411-136027-157.11", 0x00000, 0x2000, CRC(10ae3075) SHA1(69c5f62f2473aba848383eed3cecf15e273d86ca) )

	/* 8x8 characters */
	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "8411-136027-155.174", 0x0000, 0x4000, CRC(e59a6b72) SHA1(c10efa77ab421ac60b97227a8d547f50f8415670) )
	ROM_LOAD( "8411-136027-156.204", 0x4000, 0x4000, CRC(60f3c616) SHA1(59c4361891e4274e27e6279c919e8fd6803af7cf) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "8411-136027-114.203", 0x0000, 0x4000, CRC(fc91328b) SHA1(e57fd2056b65d37cf2e1f0af56616c6555df3006) )
	ROM_LOAD( "8411-136027-116.258", 0x4000, 0x4000, CRC(5745f671) SHA1(6e471633cd6de9926b3361a84430c088e1f6a097) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "8411-136027-115.173", 0x0000, 0x4000, CRC(720e5873) SHA1(151d9063c35b26f5876cf94bdf0c2665ec701bbd) )
	ROM_LOAD( "8411-136027-117.232", 0x4000, 0x4000, CRC(3c68d0bc) SHA1(2dbaf2a268b90214fd61c016ac945d4371057826) )

	/* Road LUTs */
	ROM_REGION( 0x40000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "8411-136027-146.56", 0x0000, 0x2000, CRC(5635b8c1) SHA1(5cc9437a2ff0843f1917f2451852d4561c240b24) )
	ROM_LOAD( "8411-136027-147.66", 0x2000, 0x2000, CRC(03d83cf8) SHA1(5c0cfc6bf02ad2b3f37e1ceb493f69eb9829ab1e) )
	ROM_LOAD( "8411-136027-148.76", 0x4000, 0x2000, CRC(ad56013a) SHA1(ae3a91f58f30daff334754476db33ad1d12569fc) )

	/* Arithmetic Unit Function Data ROMs */
	ROM_REGION( 0x10000, REGION_USER1, 0 )
	ROM_LOAD16_BYTE( "8411-136027-153.184", 0x0000, 0x4000, CRC(acf754e8) SHA1(06779e18636f0799efdaa09396e9ccd59f426257) )
	ROM_LOAD16_BYTE( "8411-136027-154.185", 0x0001, 0x4000, CRC(f89d3e20) SHA1(4b4cf679b7e3d63cded9989d2b667941f718ff57) )

	ROM_REGION( 0x50000, REGION_USER2, 0 )
	ROM_LOAD( "8411-136027-119.106", 0x0000, 0x4000, CRC(88eec0fb) SHA1(81d7a69dc1a4b3b81d7f28d97a3f80697cdcc6eb) ) /* Object chunk sequence LUT */
	ROM_LOAD( "8411-136027-120.73", 0x4000, 0x4000, CRC(407cbe65) SHA1(e1c11b65f3c6abde6d55afeaffdb39cdd6d66377) ) /* Object chunk sequence LUT */

	ROM_REGION( 0x50000, REGION_USER3, 0 )
	ROM_LOAD( "8411-136027-113.48", 0x0000, 0x2000, CRC(4b3d7956) SHA1(fc2432dd69f3be7007d4fd6f7c86c7c19453b1ba) ) /* Object LUT */
	ROM_LOAD( "8411-136027-118.281", 0x2000, 0x4000, CRC(de418dc7) SHA1(1233e2f7499ec5a73a40ee336d3fe26c06187784) ) /* Object palette LUT */

	/* PROMs not dumped. May well be the same as the Tazmi set. */
ROM_END

/* The single monitor is the parent set at the moment, as the 3-monitor dump is incomplete */

/* Single Monitor Version */
ROM_START( buggyb1 )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "bug35s.21", 0x10000, 0x4000, CRC(65d9af57) SHA1(17b09404942d17e7254550c43b56ae96a8c55680) ) /* Z80 ROM - checksummed by main CPU */

	ROM_LOAD16_BYTE( "bug1a.214", 0x20000, 0x8000, CRC(92797c25) SHA1(8f7434abbd7f557d3202abb01b1e4899c82c67a5) )
	ROM_LOAD16_BYTE( "bug4a.175", 0x20001, 0x8000, CRC(40ce3930) SHA1(4bf62ebeea1549a13a21a32cb860717f064b186a) )

	ROM_LOAD16_BYTE( "bug2s.213", 0xf0000, 0x8000, CRC(abcc8ad2) SHA1(aeb695c3675d40a951fe8272cbf0f6673435dab8) )
	ROM_LOAD16_BYTE( "bug5s.174", 0xf0001, 0x8000, CRC(5e352d8d) SHA1(350c206b5241d5628e673ce1108f728c8c4f980c) )

	ROM_REGION( 0x100000, REGION_CPU2, 0 )
	ROM_LOAD16_BYTE( "bug8s.26", 0x4000, 0x2000, CRC(efd66282) SHA1(8355422c0732c92951659930eb399129fe8d6230) )
	ROM_RELOAD(                  0x8000, 0x2000 )
	ROM_RELOAD(                  0xc000, 0x2000 )
	ROM_RELOAD(                  0xfc000, 0x2000 )

	ROM_LOAD16_BYTE( "bug7s.25", 0x4001, 0x2000, CRC(bd75b5eb) SHA1(f2b55f84f4c968df177a56103924ac64705285cd) )
	ROM_RELOAD(                  0x8001, 0x2000 )
	ROM_RELOAD(                  0xc001, 0x2000 )
	ROM_RELOAD(                  0xfc001, 0x2000 )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )
	ROM_LOAD( "bug35s.21", 0x00000, 0x4000, CRC(65d9af57) SHA1(17b09404942d17e7254550c43b56ae96a8c55680) )

	/* 8x8 characters */
	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bug34s.46", 0x00000, 0x4000, CRC(8ea8fec4) SHA1(75e67c9a59a86fcdedf2a70fafd303baa552aa18) )
	ROM_LOAD( "bug33s.47", 0x04000, 0x4000, CRC(459c2b03) SHA1(ff62a86195042a349fbe799c638cf590fe9572bb) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "bug26s.147", 0x0000, 0x8000, CRC(14033710) SHA1(e05afeb557ce14055fa8b4f6d8805307feaa1660) )
	ROM_LOAD( "bug19s.144", 0x8000, 0x8000, CRC(838e0697) SHA1(0e9aff2c4065d79350ddb55edff57a899c33ef1c) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "bug28s.146", 0x0000, 0x8000, CRC(8b47d227) SHA1(a3e57594ad0085e8b1bd327c580eb36237f3e3d2) )
	ROM_LOAD( "bug21s.143", 0x8000, 0x8000, CRC(876a5666) SHA1(db485cdf35f63c080c919ee86374f63e577092c3) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "bug30s.145", 0x0000, 0x8000, CRC(11d8e2a8) SHA1(9bf198229a12d331e8e7352b7ee3f39f6891f517) )
	ROM_LOAD( "bug23s.142", 0x8000, 0x8000, CRC(015db5d8) SHA1(39ef8b44f2eb9399fb1555cffa6763e06d59c181) )

	/* Road */
	ROM_REGION( 0x40000, REGION_GFX6, 0)
	ROM_LOAD( "bb3s", 0x000, 0x200, CRC(2ab3d5ff) SHA1(9f8359cb4ba2e7d15dbb9dc21cd71c0902cd2153) )
	ROM_LOAD( "bb4s", 0x200, 0x200, CRC(630f68a4) SHA1(d730f050353c688f81d090e33e00cd35e7b7b6fa) )
	ROM_LOAD( "bb5s", 0x400, 0x200, CRC(65925c9e) SHA1(d1ff1cb9f83c09e52a96632945e4edfedc335fd4) )
	ROM_LOAD( "bug11s.225", 0x1000, 0x4000, CRC(771af4e1) SHA1(a42b164dd0567c78c0d308ee48d63e5a284897bb) ) /* Road Lookup */

	/* Arithmetic Unit Function Data ROMs */
	ROM_REGION( 0x10000, REGION_USER1, 0 )
	ROM_LOAD( "bug9.138",  0x0800, 0x800, CRC(7d84135b) SHA1(3c669c4e796e83672aceeb6de1aeea28f9f2fef0) )
	ROM_CONTINUE(          0x1800, 0x800 )
	ROM_CONTINUE(          0x2800, 0x800 )
	ROM_CONTINUE(          0x3800, 0x800 )
	ROM_CONTINUE(          0x4800, 0x800 )
	ROM_CONTINUE(          0x5800, 0x800 )
	ROM_CONTINUE(          0x6800, 0x800 )
	ROM_CONTINUE(          0x7800, 0x800 )

	ROM_LOAD( "bug10.95",  0x0000, 0x800, CRC(b518dd6f) SHA1(7cefa2f9438306c81dc83cd260928c835eb9b712) )
	ROM_CONTINUE(          0x1000, 0x800 )
	ROM_CONTINUE(          0x2000, 0x800 )
	ROM_CONTINUE(          0x3000, 0x800 )
	ROM_CONTINUE(          0x4000, 0x800 )
	ROM_CONTINUE(          0x5000, 0x800 )
	ROM_CONTINUE(          0x6000, 0x800 )
	ROM_CONTINUE(          0x7000, 0x800 )

	ROM_REGION( 0x100000, REGION_USER2, 0 )
	ROM_LOAD( "bug16s.139", 0x0000, 0x8000, CRC(1903a9ad) SHA1(526c404c15e3f04b4afb27dee66e9deb0a6b9704) ) /* Object chunk sequence LUT */
	ROM_LOAD( "bug17s.140", 0x8000, 0x8000, CRC(82cabdd4) SHA1(94324fcf83c373621fc40553473ae3cb552ab704) ) /* Object chunk sequence LUT */

	ROM_REGION( 0x10000, REGION_USER3, 0 )
	ROM_LOAD( "bug13.32", 0x0000, 0x2000, CRC(53604d7a) SHA1(bfa304cd885162ece7a5f54988d9880fc541eb3a) )   /* Object LUT */
	ROM_LOAD( "bug18s.141", 0x2000, 0x4000, CRC(67786327) SHA1(32cc1f5bc654497c968ddcd4af29720c6d659482) ) /* Object palette LUT */

	ROM_REGION( 0x10000, REGION_PROMS, 0 )
	ROM_LOAD( "bb10.41", 0x000, 0x100, CRC(f2368398) SHA1(53f28dba11bb494d033bb279abf138975c84b20d) )  /* RED */
	ROM_LOAD( "bb11.40", 0x100, 0x100, CRC(bf77f624) SHA1(b042d293d2094dbabb32d628fd9addd832f084ef) )  /* GREEN */
	ROM_LOAD( "bb12.39", 0x200, 0x100, CRC(10a2e8d1) SHA1(51a8c51ecbbb7bd04ae46fb5598d2c8de8097581) )  /* BLUE */
	ROM_LOAD( "bb13.42", 0x300, 0x100, CRC(40d10dfa) SHA1(e40b4c424827937fec6df1a27b19b8dc09d3274a) )
	ROM_LOAD( "bb14.19", 0x400, 0x100, CRC(0b821e0b) SHA1(b9401b9364fb99e15f562df91dcfdec1b989af2d) )  /* Character colour LUT */

	ROM_LOAD( "bb9.190", 0x500, 0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )  /* Object colour LUT (odd) */
	ROM_LOAD( "bb9.162", 0xd00, 0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )  /* Object colour LUT (even)*/
	ROM_LOAD( "bb7.188", 0x1500, 0x100, CRC(b57b609f) SHA1(2dea375437c62cb4c64b21d5e6ddc09397b6ab35) )  /* Road colour LUT */

	ROM_LOAD( "bb8.31",  0x1600, 0x100, CRC(2330ff4f) SHA1(e86eb63ce47572bcbbf325f9bb749d10d96bf2e7) )  /* Object tile LUT */
	ROM_LOAD( "bb1.163", 0x1700, 0x200, CRC(0ddbd36d) SHA1(7a08901a350c315d46ab8d0aa881db384b9f37d2) )  /* Arithmetic instruction PROM */
	ROM_LOAD( "bb2.162", 0x1900, 0x200, CRC(71d47de1) SHA1(2da9aeb3f2ebb1114631c8042a37c4f4c18e741b) )  /* Arithmetic instruction PROM */
	ROM_LOAD( "bb6.224", 0x1b00, 0x200, CRC(ad43e02a) SHA1(c50a398020508f52ddf8d45881f211d17d096fa1) )  /* Road video timing? */
ROM_END

/* 3 Monitor Version */
ROM_START( buggyboy )
	ROM_REGION( 0x100000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "bug1a.230", 0x20000, 0x8000, CRC(92797c25) SHA1(8f7434abbd7f557d3202abb01b1e4899c82c67a5) )
	ROM_LOAD16_BYTE( "bug4a.173", 0x20001, 0x8000, CRC(40ce3930) SHA1(4bf62ebeea1549a13a21a32cb860717f064b186a) )

	ROM_LOAD16_BYTE( "bug2d.231", 0xf0000, 0x4000, CRC(f67bd593) SHA1(b6e3f9d5534b0addbba4aa4c813dda21a27cafa2) )
	ROM_LOAD16_BYTE( "bug5d.174", 0xf0001, 0x4000, CRC(d0dd6ffc) SHA1(377581ace86ea0f713aa7dc8f96f27cbcda1b2ea) )

	ROM_LOAD16_BYTE( "bug3b.232", 0xf8000, 0x4000, CRC(43cce3f0) SHA1(17de3728d809d386f6a7a330c8c8701975d4ebed) )
	ROM_LOAD16_BYTE( "bug6b.175", 0xf8001, 0x4000, CRC(8f000dfa) SHA1(5fd78a03a00f547bbb431839f78a8d10a4ba8e3e) )

	ROM_LOAD16_BYTE( "bug35.11",  0x10000, 0x4000, CRC(7aa16e9e) SHA1(ea54e56270f70351a62a78fa32027bb41ef9861e) ) /* Z80 ROM - tested by main CPU */

	ROM_REGION( 0x100000, REGION_CPU2, 0 )
	ROM_LOAD16_BYTE( "bug8a.061", 0x4000, 0x2000, CRC(512291cd) SHA1(60f87133c86b88b982ba4680f96d0ac55970cb8d) )
	ROM_RELOAD(                  0x8000,  0x2000 )
	ROM_RELOAD(                  0xc000,  0x2000 )
	ROM_RELOAD(                  0xfc000, 0x2000 )

	ROM_LOAD16_BYTE( "bug7a.060", 0x4001, 0x2000, CRC(d24dfdef) SHA1(37d05a8bf9567380523df01265afb9780e39ea2a) )
	ROM_RELOAD(                  0x8001,  0x2000 )
	ROM_RELOAD(                  0xc001,  0x2000 )
	ROM_RELOAD(                  0xfc001, 0x2000 )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )  /* Sound */
	ROM_LOAD( "bug35.11", 0x00000, 0x4000,  CRC(7aa16e9e) SHA1(ea54e56270f70351a62a78fa32027bb41ef9861e) )

	/* 8x8 Characters */
	ROM_REGION( 0x20000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bug34s.46", 0x00000, 0x4000, CRC(8ea8fec4) SHA1(75e67c9a59a86fcdedf2a70fafd303baa552aa18) )
	ROM_LOAD( "bug33s.47", 0x04000, 0x4000, CRC(459c2b03) SHA1(ff62a86195042a349fbe799c638cf590fe9572bb) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "bug26s.147", 0x0000, 0x8000, CRC(14033710) SHA1(e05afeb557ce14055fa8b4f6d8805307feaa1660) )
	ROM_LOAD( "bug19s.144", 0x8000, 0x8000, CRC(838e0697) SHA1(0e9aff2c4065d79350ddb55edff57a899c33ef1c) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX3, ROMREGION_DISPOSE )
	ROM_LOAD( "bug28s.146", 0x0000, 0x8000, CRC(8b47d227) SHA1(a3e57594ad0085e8b1bd327c580eb36237f3e3d2) )
	ROM_LOAD( "bug21s.143", 0x8000, 0x8000, CRC(876a5666) SHA1(db485cdf35f63c080c919ee86374f63e577092c3) )

	/* 8x8 object chunks */
	ROM_REGION( 0x40000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "bug30s.145", 0x0000, 0x8000, CRC(11d8e2a8) SHA1(9bf198229a12d331e8e7352b7ee3f39f6891f517) )
	ROM_LOAD( "bug23s.142", 0x8000, 0x8000, CRC(015db5d8) SHA1(39ef8b44f2eb9399fb1555cffa6763e06d59c181) )

	/* Object Lookups */
	ROM_REGION( 0x40000, REGION_GFX5, ROMREGION_DISPOSE )
	ROM_LOAD( "bug16s.139", 0x0000, 0x8000, CRC(1903a9ad) SHA1(526c404c15e3f04b4afb27dee66e9deb0a6b9704) )
	ROM_LOAD( "bug17s.140", 0x8000, 0x8000, CRC(82cabdd4) SHA1(94324fcf83c373621fc40553473ae3cb552ab704) )
	ROM_LOAD( "bug18s.141", 0x10000, 0x4000, CRC(67786327) SHA1(32cc1f5bc654497c968ddcd4af29720c6d659482) )

	/* Road */
	ROM_REGION( 0x40000, REGION_GFX6, 0)
	ROM_LOAD( "bb3s", 0x000, 0x200, CRC(2ab3d5ff) SHA1(9f8359cb4ba2e7d15dbb9dc21cd71c0902cd2153) )
	ROM_LOAD( "bb4s", 0x200, 0x200, CRC(630f68a4) SHA1(d730f050353c688f81d090e33e00cd35e7b7b6fa) )
	ROM_LOAD( "bb5s", 0x400, 0x200, CRC(65925c9e) SHA1(d1ff1cb9f83c09e52a96632945e4edfedc335fd4) )
	ROM_LOAD( "bug11s.225", 0x1000, 0x4000, CRC(771af4e1) SHA1(a42b164dd0567c78c0d308ee48d63e5a284897bb) ) /* Road Lookup */

	/* Arithmetic Unit Function Data ROMs */
	ROM_REGION( 0x10000, REGION_USER1, 0 )
	ROM_LOAD( "bug9.138",  0x0800, 0x800, CRC(7d84135b) SHA1(3c669c4e796e83672aceeb6de1aeea28f9f2fef0) )
	ROM_CONTINUE(          0x1800, 0x800 )
	ROM_CONTINUE(          0x2800, 0x800 )
	ROM_CONTINUE(          0x3800, 0x800 )
	ROM_CONTINUE(          0x4800, 0x800 )
	ROM_CONTINUE(          0x5800, 0x800 )
	ROM_CONTINUE(          0x6800, 0x800 )
	ROM_CONTINUE(          0x7800, 0x800 )

	ROM_LOAD( "bug10.95",  0x0000, 0x800, CRC(b518dd6f) SHA1(7cefa2f9438306c81dc83cd260928c835eb9b712) )
	ROM_CONTINUE(          0x1000, 0x800 )
	ROM_CONTINUE(          0x2000, 0x800 )
	ROM_CONTINUE(          0x3000, 0x800 )
	ROM_CONTINUE(          0x4000, 0x800 )
	ROM_CONTINUE(          0x5000, 0x800 )
	ROM_CONTINUE(          0x6000, 0x800 )
	ROM_CONTINUE(          0x7000, 0x800 )

	ROM_REGION( 0x100000, REGION_USER2, 0 )
	ROM_LOAD( "bug16s.139", 0x0000, 0x8000, CRC(1903a9ad) SHA1(526c404c15e3f04b4afb27dee66e9deb0a6b9704) ) /* Object chunk sequence LUT */
	ROM_LOAD( "bug17s.140", 0x8000, 0x8000, CRC(82cabdd4) SHA1(94324fcf83c373621fc40553473ae3cb552ab704) ) /* Object chunk sequence LUT */

	ROM_REGION( 0x10000, REGION_USER3, 0 )
	ROM_LOAD( "bug13.32", 0x0000, 0x2000, CRC(53604d7a) SHA1(bfa304cd885162ece7a5f54988d9880fc541eb3a) )   /* Object LUT */
	ROM_LOAD( "bug18s.141", 0x2000, 0x4000, CRC(67786327) SHA1(32cc1f5bc654497c968ddcd4af29720c6d659482) ) /* Object palette LUT */

	ROM_REGION( 0x10000, REGION_PROMS, 0 )
	ROM_LOAD( "bb10.41", 0x000, 0x100, CRC(f2368398) SHA1(53f28dba11bb494d033bb279abf138975c84b20d) )  /* RED */
	ROM_LOAD( "bb11.40", 0x100, 0x100, CRC(bf77f624) SHA1(b042d293d2094dbabb32d628fd9addd832f084ef) )  /* GREEN */
	ROM_LOAD( "bb12.39", 0x200, 0x100, CRC(10a2e8d1) SHA1(51a8c51ecbbb7bd04ae46fb5598d2c8de8097581) )  /* BLUE */
	ROM_LOAD( "bb13.42", 0x300, 0x100, CRC(40d10dfa) SHA1(e40b4c424827937fec6df1a27b19b8dc09d3274a) )
	ROM_LOAD( "bb14.19", 0x400, 0x100, CRC(0b821e0b) SHA1(b9401b9364fb99e15f562df91dcfdec1b989af2d) )  /* Character colour LUT */

	ROM_LOAD( "bb9.190", 0x500, 0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )  /* Object colour LUT (odd) */
	ROM_LOAD( "bb9.162", 0xd00, 0x800, CRC(6fc807d1) SHA1(3442cbb21bbedf6291a3fe1747d479445f613d26) )  /* Object colour LUT (even)*/
	ROM_LOAD( "bb7.188", 0x1500,0x100, CRC(b57b609f) SHA1(2dea375437c62cb4c64b21d5e6ddc09397b6ab35) )  /* Road colour LUT */

	ROM_LOAD( "bb8.31", 0x1600, 0x100, CRC(2330ff4f) SHA1(e86eb63ce47572bcbbf325f9bb749d10d96bf2e7) )  /* Object tile LUT */
	ROM_LOAD( "bb1.163", 0x1700, 0x200, CRC(0ddbd36d) SHA1(7a08901a350c315d46ab8d0aa881db384b9f37d2) )  /* Arithmetic instruction PROM */
	ROM_LOAD( "bb2.162", 0x1900, 0x200, CRC(71d47de1) SHA1(2da9aeb3f2ebb1114631c8042a37c4f4c18e741b) )  /* Arithmetic instruction PROM */
ROM_END

GAME( 1983, tx1,      0,       tx1,      tx1,      0, ROT0, "Tazmi",   "TX-1",                             GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1983, tx1a,     0,       tx1,      tx1,      0, ROT0, "Tazmi",   "TX-1 (Atari/Namco/Taito License)", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1985, buggyboy, buggyb1, buggyboy, buggyboy, 0, ROT0, "Tatsumi", "Buggy Boy",                        GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
GAME( 1986, buggyb1,  0,       buggyb1,  buggyb1,  0, ROT0, "Tatsumi", "Buggy Boy (Single Monitor)",       GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS | GAME_NOT_WORKING )
