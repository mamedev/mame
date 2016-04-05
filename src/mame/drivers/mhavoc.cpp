// license:BSD-3-Clause
// copyright-holders:Mike Appolo
/***************************************************************************

    Atari Major Havoc hardware

    driver by Mike Appolo

    Modified 10/08/2006 by Jess M. Askey to include support for Speech which was not stuffed on production
    Major Havoc PCB's. However, the hardware if stuffed is functional. Speech is used in Major Havoc Return
    to Vaxx.

    Games supported:
        * Alpha One
        * Major Havoc
        * Major Havoc: Return to Vax (including speech) - This version is a hack that includes 3 new levels
                                                          near the end of the game. Level 19 is incomplete.

    Known bugs:
        * none at this time

****************************************************************************

    MAJOR HAVOC (Driver)

    Notes:

    This game was provided in three configurations:
    1) New Machine Purchase
    2) Upgrade kit for Tempest (Kit "A")
    3) Upgrade kit for Space Duel, Gravitar, and Black Window (Kit "B")

    Controls on the machine were:
    A backlit cylinder (roller) on new Major Havoc machines
            or
    A Tempest-like spinner on upgrades


    Memory Map for Major Havoc

    Alpha Processor
                     D  D  D  D  D  D  D  D
    Hex Address      7  6  5  4  3  2  1  0                    Function
    --------------------------------------------------------------------------------
    0000-01FF     |  D  D  D  D  D  D  D  D   | R/W  | Program RAM (1/2K)
    0200-07FF     |  D  D  D  D  D  D  D  D   | R/W  | Paged Program RAM (3K)
    0800-09FF     |  D  D  D  D  D  D  D  D   | R/W  | Program RAM (1/2K)
                  |                           |      |
    1000          |  D  D  D  D  D  D  D  D   |  R   | Gamma Commuication Read Port
                  |                           |      |
    1200          |  D                        |  R   | Right Coin (Player 1=0)
    1200          |     D                     |  R   | Left Coin  (Player 1=0)
    1200          |        D                  |  R   | Aux. Coin  (Player 1=0)
    1200          |           D               |  R   | Diagnostic Step
    1200          |  D                        |  R   | Self Test Switch (Player 1=1)
    1200          |     D                     |  R   | Cabinet Switch (Player 1=1)
    1200          |        D                  |  R   | Aux. Coin Switch (Player 1=1)
    1200          |           D               |  R   | Diagnostic Step
    1200          |              D            |  R   | Gammma Rcvd Flag
    1200          |                 D         |  R   | Gamma Xmtd Flag
    1200          |                    D      |  R   | 2.4 KHz
    1200          |                       D   |  R   | Vector Generator Halt Flag
                  |                           |      |
    1400-141F     |              D  D  D  D   |  W   | ColorRAM
                  |                           |      |
    1600          |  D                        |  W   | Invert X
    1600          |     D                     |  W   | Invert Y
    1600          |        D                  |  W   | Player 1
    1600          |           D               |  W   | Not Used
    1600          |              D            |  W   | Gamma Proc. Reset
    1600          |                 D         |  W   | Beta Proc. Reset
    1600          |                    D      |  W   | Not Used
    1600          |                       D   |  W   | Roller Controller Light
                  |                           |      |
    1640          |                           |  W   | Vector Generator Go
    1680          |                           |  W   | Watchdog Clear
    16C0          |                           |  W   | Vector Generator Reset
                  |                           |      |
    1700          |                           |  W   | IRQ Acknowledge
    1740          |                    D  D   |  W   | Program ROM Page Select
    1780          |                       D   |  W   | Program RAM Page Select
    17C0          |  D  D  D  D  D  D  D  D   |  W   | Gamma Comm. Write Port
                  |                           |      |
    1800-1FFF     |  D  D  D  D  D  D  D  D   | R/W  | Shared Beta RAM(not used)
                  |                           |      |
    2000-3FFF     |  D  D  D  D  D  D  D  D   |  R   | Paged Program ROM (32K)
    4000-4FFF     |  D  D  D  D  D  D  D  D   | R/W  | Vector Generator RAM (4K)
    5000-5FFF     |  D  D  D  D  D  D  D  D   |  R   | Vector Generator ROM (4K)
    6000-7FFF     |  D  D  D  D  D  D  D  D   |  R   | Paged Vector ROM (32K)
    8000-FFFF     |  D  D  D  D  D  D  D  D   |  R   | Program ROM (32K)
    -------------------------------------------------------------------------------

    Gamma Processor

                     D  D  D  D  D  D  D  D
    Hex Address      7  6  5  4  3  2  1  0                    Function
    --------------------------------------------------------------------------------
    0000-07FF     |  D  D  D  D  D  D  D  D   | R/W  | Program RAM (2K)
    2000-203F     |  D  D  D  D  D  D  D  D   | R/W  | Quad-Pokey I/O
                  |                           |      |
    2800          |  D                        |  R   | Fire 1 Switch
    2800          |     D                     |  R   | Shield 1 Switch
    2800          |        D                  |  R   | Fire 2 Switch
    2800          |           D               |  R   | Shield 2 Switch
    2800          |              D            |  R   | N/C (floating, probably reads as 1)
    2800          |                 D         |  R   | /TIRDY (Speech Chip Ready)
    2800          |                    D      |  R   | Alpha Rcvd Flag
    2800          |                       D   |  R   | Alpha Xmtd Flag
                  |                           |      |
    3000          |  D  D  D  D  D  D  D  D   |  R   | Alpha Comm. Read Port
                  |                           |      |
    3800-3803     |  D  D  D  D  D  D  D  D   |  R   | Roller Controller Input
                  |                           |      |
    4000          |                           |  W   | IRQ Acknowledge
    4800          |                    D      |  W   | Left Coin Counter
    4800          |                       D   |  W   | Right Coin Counter
                  |                           |      |
    5000          |  D  D  D  D  D  D  D  D   |  W   | Alpha Comm. Write Port
                  |                           |      |
    5800          |  D  D  D  D  D  D  D  D   |  W   | Speech Data Write / Write Strobe Clear
    5900          |                           |  W   | Speech Write Strobe Set
                  |                           |      |
    6000-61FF     |  D  D  D  D  D  D  D  D   | R/W  | EEROM
    8000-BFFF     |  D  D  D  D  D  D  D  D   |  R   | Program ROM (16K)
    -----------------------------------------------------------------------------



    MAJOR HAVOC DIP SWITCH SETTINGS

    $=Default

    DIP Switch at position 13/14S

                                      1    2    3    4    5    6    7    8
    STARTING LIVES                  _________________________________________
    Free Play   1 Coin   2 Coin     |    |    |    |    |    |    |    |    |
        2         3         5      $|Off |Off |    |    |    |    |    |    |
        3         4         4       | On | On |    |    |    |    |    |    |
        4         5         6       | On |Off |    |    |    |    |    |    |
        5         6         7       |Off | On |    |    |    |    |    |    |
    GAME DIFFICULTY                 |    |    |    |    |    |    |    |    |
    Hard                            |    |    | On | On |    |    |    |    |
    Medium                         $|    |    |Off |Off |    |    |    |    |
    Easy                            |    |    |Off | On |    |    |    |    |
    Demo                            |    |    | On |Off |    |    |    |    |
    BONUS LIFE                      |    |    |    |    |    |    |    |    |
    50,000                          |    |    |    |    | On | On |    |    |
    100,000                        $|    |    |    |    |Off |Off |    |    |
    200,000                         |    |    |    |    |Off | On |    |    |
    No Bonus Life                   |    |    |    |    | On |Off |    |    |
    ATTRACT MODE SOUND              |    |    |    |    |    |    |    |    |
    Silence                         |    |    |    |    |    |    | On |    |
    Sound                          $|    |    |    |    |    |    |Off |    |
    ADAPTIVE DIFFICULTY             |    |    |    |    |    |    |    |    |
    No                              |    |    |    |    |    |    |    | On |
    Yes                            $|    |    |    |    |    |    |    |Off |
                                    -----------------------------------------

        DIP Switch at position 8S

                                      1    2    3    4    5    6    7    8
                                    _________________________________________
    Free Play                       |    |    |    |    |    |    | On |Off |
    1 Coin for 1 Game               |    |    |    |    |    |    |Off |Off |
    1 Coin for 2 Games              |    |    |    |    |    |    | On | On |
    2 Coins for 1 Game             $|    |    |    |    |    |    |Off | On |
    RIGHT COIN MECHANISM            |    |    |    |    |    |    |    |    |
    x1                             $|    |    |    |    |Off |Off |    |    |
    x4                              |    |    |    |    |Off | On |    |    |
    x5                              |    |    |    |    | On |Off |    |    |
    x6                              |    |    |    |    | On | On |    |    |
    LEFT COIN MECHANISM             |    |    |    |    |    |    |    |    |
    x1                             $|    |    |    |Off |    |    |    |    |
    x2                              |    |    |    | On |    |    |    |    |
    BONUS COIN ADDER                |    |    |    |    |    |    |    |    |
    No Bonus Coins                 $|Off |Off |Off |    |    |    |    |    |
    Every 4, add 1                  |Off | On |Off |    |    |    |    |    |
    Every 4, add 2                  |Off | On | On |    |    |    |    |    |
    Every 5, add 1                  | On |Off |Off |    |    |    |    |    |
    Every 3, add 1                  | On |Off | On |    |    |    |    |    |
                                    -----------------------------------------

        2 COIN MINIMUM OPTION: Short pin 6 @13N to ground.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/atari_vg.h"
#include "video/avgdvg.h"
#include "video/vector.h"
#include "sound/tms5220.h"
#include "sound/pokey.h"
#include "machine/nvram.h"
#include "includes/mhavoc.h"

/* Quad pokey hookup (based on schematics):
Address: 543210
         |||||\- pokey A0
         ||||\-- pokey A1
         |||\--- pokey A2
         ||\---- pokey chip number LSB
         |\----- pokey chip number MSB
         \------ pokey A3
*/
READ8_MEMBER(mhavoc_state::quad_pokeyn_r)
{
	static const char *const devname[4] = { "pokey1", "pokey2", "pokey3", "pokey4" };
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset & 0x7) | control;
	pokey_device *pokey = machine().device<pokey_device>(devname[pokey_num]);

	return pokey->read(pokey_reg);
}

WRITE8_MEMBER(mhavoc_state::quad_pokeyn_w)
{
	static const char *const devname[4] = { "pokey1", "pokey2", "pokey3", "pokey4" };
	int pokey_num = (offset >> 3) & ~0x04;
	int control = (offset & 0x20) >> 2;
	int pokey_reg = (offset & 0x7) | control;
	pokey_device *pokey = machine().device<pokey_device>(devname[pokey_num]);

	pokey->write(pokey_reg, data);
}


/*************************************
 *
 *  Alpha One: dual POKEY?
 *
 *************************************/
/* dual pokey hookup (presumably, based on the prototype code):
Address: 43210
         ||||\- pokey A0
         |||\-- pokey A1
         ||\--- pokey A2
         |\---- pokey chip number
         \----- pokey A3
*/
READ8_MEMBER(mhavoc_state::dual_pokey_r)
{
	int pokey_num = (offset >> 3) & 0x01;
	int control = (offset & 0x10) >> 1;
	int pokey_reg = (offset & 0x7) | control;

	if (pokey_num == 0)
		return machine().device<pokey_device>("pokey1")->read(pokey_reg);
	else
		return machine().device<pokey_device>("pokey2")->read(pokey_reg);
}


WRITE8_MEMBER(mhavoc_state::dual_pokey_w)
{
	int pokey_num = (offset >> 3) & 0x01;
	int control = (offset & 0x10) >> 1;
	int pokey_reg = (offset & 0x7) | control;

	if (pokey_num == 0)
		machine().device<pokey_device>("pokey1")->write(pokey_reg, data);
	else
		machine().device<pokey_device>("pokey2")->write(pokey_reg, data);
}


/*************************************
 *
 *  Alpha CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( alpha_map, AS_PROGRAM, 8, mhavoc_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x07ff) AM_RAMBANK("bank1") AM_SHARE("zram0")
	AM_RANGE(0x0800, 0x09ff) AM_RAM
	AM_RANGE(0x0a00, 0x0fff) AM_RAMBANK("bank1") AM_SHARE("zram1")
	AM_RANGE(0x1000, 0x1000) AM_READ(mhavoc_gamma_r)            /* Gamma Read Port */
	AM_RANGE(0x1200, 0x1200) AM_READ_PORT("IN0") AM_WRITENOP    /* Alpha Input Port 0 */
	AM_RANGE(0x1400, 0x141f) AM_RAM AM_SHARE("colorram")    /* ColorRAM */
	AM_RANGE(0x1600, 0x1600) AM_WRITE(mhavoc_out_0_w)           /* Control Signals */
	AM_RANGE(0x1640, 0x1640) AM_DEVWRITE("avg", avg_mhavoc_device, go_w)               /* Vector Generator GO */
	AM_RANGE(0x1680, 0x1680) AM_WRITE(watchdog_reset_w)         /* Watchdog Clear */
	AM_RANGE(0x16c0, 0x16c0) AM_DEVWRITE("avg", avg_mhavoc_device, reset_w)            /* Vector Generator Reset */
	AM_RANGE(0x1700, 0x1700) AM_WRITE(mhavoc_alpha_irq_ack_w)   /* IRQ ack */
	AM_RANGE(0x1740, 0x1740) AM_WRITE(mhavoc_rom_banksel_w)     /* Program ROM Page Select */
	AM_RANGE(0x1780, 0x1780) AM_WRITE(mhavoc_ram_banksel_w)     /* Program RAM Page Select */
	AM_RANGE(0x17c0, 0x17c0) AM_WRITE(mhavoc_gamma_w)           /* Gamma Communication Write Port */
	AM_RANGE(0x1800, 0x1fff) AM_RAM                             /* Shared Beta Ram */
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank2")                        /* Paged Program ROM (32K) */
	AM_RANGE(0x4000, 0x4fff) AM_RAM AM_SHARE("vectorram") AM_REGION("alpha", 0x4000)    /* Vector Generator RAM */
	AM_RANGE(0x5000, 0x7fff) AM_ROM                             /* Vector ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM                 /* Program ROM (32K) */
ADDRESS_MAP_END



/*************************************
 *
 *  Gamma CPU memory handlers
 *
 *************************************/
 
/*
a15 a14 a13 a12 a11 a10 a09 a08 a07 a06 a05 a04 a03 a02 a01 a00
0   0   0   x   x   *   *   *   *   *   *   *   *   *   *   *      RW ZRAM (6116 SRAM@9PQ)
0   0   1   0   0   x   x   x   x   x   *   *   *   *   *   *      RW INPUTS: QCI/O (Quad Pokey)
0   0   1   0   1   x   x   x   x   x   x   x   x   x   x   x      R  INPUTS: SWITCHES
0   0   1   1   0   x   x   x   x   x   x   x   x   x   x   x      R  INPUTS: PORTRD_gamma
0   0   1   1   1   x   x   x   x   x   x   x   x   x   *   *      R  INPUTS: ROLLER (CCI(LETA?), with A2 grounded so only 4 ports are readable)
0   1   0   x   x   x   x   x   x   x   x   x   x   x   x   x      R  OUTPUTS: read of dipswitches @8S encompasses the entire OUTPUTS area
0   1   0   0   0   x   x   x   x   x   x   x   x   x   x   x       W OUTPUTS: IRQACK
0   1   0   0   1   x   x   x   x   x   x   x   x   x   x   x       W OUTPUTS: STROBES
0   1   0   1   0   x   x   x   x   x   x   x   x   x   x   x       W OUTPUTS: PORTWR_gamma
0   1   0   1   1   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?   ?       W OUTPUTS: TISND, unused tms5220 write
 ( supposedly, but the /WS hookup is not traced on pcb yet:
0   1   0   1   1   x?  x?  0   x?  x?  x?  x?  x?  x?  x?  x?      W? OUTPUTS: TISND: Gamma CPU write to octal latch??
0   1   0   1   1   x?  x?  1   x?  x?  x?  x?  x?  x?  x?  x?      W? OUTPUTS: TISND: octal latch output enable to tms5220 and pulse /WS on 5220??
 Is there a way to activate /RS on the TMS5220 to read the status register? is the TMS5220 /INT line connected to the 6502 somehow?
 )
0   1   1   x   x   x   x   *   *   *   *   *   *   *   *   *      RW EEROM: (2804 EEPROM@9QR, pins 22(A9) and 19(A10) are N/C inside the chip)
1   x   *   *   *   *   *   *   *   *   *   *   *   *   *   *      R  ROM 27128 @9S
*/

static ADDRESS_MAP_START( gamma_map, AS_PROGRAM, 8, mhavoc_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_MIRROR(0x1800)                   /* Program RAM (2K) */
	AM_RANGE(0x2000, 0x203f) AM_READWRITE(quad_pokeyn_r, quad_pokeyn_w) AM_MIRROR(0x07C0) /* Quad Pokey read/write  */
	AM_RANGE(0x2800, 0x2800) AM_READ_PORT("IN1") AM_MIRROR(0x07ff)      /* Gamma Input Port */
	AM_RANGE(0x3000, 0x3000) AM_READ(mhavoc_alpha_r) AM_MIRROR(0x07ff)  /* Alpha Comm. Read Port */
	AM_RANGE(0x3800, 0x3803) AM_READ_PORT("DIAL") AM_MIRROR(0x07fc)     /* Roller Controller Input */
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("DSW2") AM_WRITE(mhavoc_gamma_irq_ack_w) AM_MIRROR(0x07ff) /* DSW at 8S, IRQ Acknowledge */
	AM_RANGE(0x4800, 0x4800) AM_WRITE(mhavoc_out_1_w) AM_MIRROR(0x07ff) /* Coin Counters    */
	AM_RANGE(0x5000, 0x5000) AM_WRITE(mhavoc_alpha_w) AM_MIRROR(0x07ff) /* Alpha Comm. Write Port */
	//AM_RANGE(0x5800, 0x5800) AM_WRITE(mhavocrv_speech_data_w) AM_MIRROR(0x06ff) /* TMS5220 data write */
	//AM_RANGE(0x5900, 0x5900) AM_WRITE(mhavocrv_speech_strobe_w) AM_MIRROR(0x06ff) /* TMS5220 /WS strobe write */
	AM_RANGE(0x6000, 0x61ff) AM_RAM AM_SHARE("nvram") AM_MIRROR(0x1e00) /* EEROM */
	AM_RANGE(0x8000, 0xbfff) AM_ROM AM_MIRROR(0x4000)                   /* Program ROM (16K) */
ADDRESS_MAP_END




/*************************************
 *
 *  Main CPU memory handlers (Alpha One)
 *
 *************************************/

static ADDRESS_MAP_START( alphaone_map, AS_PROGRAM, 8, mhavoc_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x07ff) AM_RAMBANK("bank1") AM_SHARE("zram0")
	AM_RANGE(0x0800, 0x09ff) AM_RAM
	AM_RANGE(0x0a00, 0x0fff) AM_RAMBANK("bank1") AM_SHARE("zram1")
	AM_RANGE(0x1020, 0x103f) AM_READWRITE(dual_pokey_r, dual_pokey_w)
	AM_RANGE(0x1040, 0x1040) AM_READ_PORT("IN0") AM_WRITENOP    /* Alpha Input Port 0 */
	AM_RANGE(0x1060, 0x1060) AM_READ_PORT("IN1")                /* Gamma Input Port */
	AM_RANGE(0x1080, 0x1080) AM_READ_PORT("DIAL")               /* Roller Controller Input*/
	AM_RANGE(0x10a0, 0x10a0) AM_WRITE(alphaone_out_0_w)         /* Control Signals */
	AM_RANGE(0x10a4, 0x10a4) AM_DEVWRITE("avg", avg_mhavoc_device, go_w)               /* Vector Generator GO */
	AM_RANGE(0x10a8, 0x10a8) AM_WRITE(watchdog_reset_w)         /* Watchdog Clear */
	AM_RANGE(0x10ac, 0x10ac) AM_DEVWRITE("avg", avg_mhavoc_device, reset_w)            /* Vector Generator Reset */
	AM_RANGE(0x10b0, 0x10b0) AM_WRITE(mhavoc_alpha_irq_ack_w)   /* IRQ ack */
	AM_RANGE(0x10b4, 0x10b4) AM_WRITE(mhavoc_rom_banksel_w)
	AM_RANGE(0x10b8, 0x10b8) AM_WRITE(mhavoc_ram_banksel_w)
	AM_RANGE(0x10e0, 0x10ff) AM_WRITEONLY AM_SHARE("colorram")  /* ColorRAM */
	AM_RANGE(0x1800, 0x18ff) AM_RAM AM_SHARE("nvram")   /* EEROM */
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank2")                        /* Paged Program ROM (32K) */
	AM_RANGE(0x4000, 0x4fff) AM_RAM AM_SHARE("vectorram") AM_REGION("alpha", 0x4000) /* Vector Generator RAM */
	AM_RANGE(0x5000, 0x7fff) AM_ROM                             /* Vector ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM                             /* Program ROM (32K) */
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

CUSTOM_INPUT_MEMBER(mhavoc_state::clock_r)
{
	/* 2.4kHz (divide 2.5MHz by 1024) */
	return (m_alpha->total_cycles() & 0x400) ? 0 : 1;
}


/* 2008-08 FP:
   IN0 Bit 4 is tested in the second input test, but it's not clear its effect.
   According to the memory map at top it should be Diagnostic Step, but it's
   actually IN0 Bit 5 to have this function. I marked it as UNKNOWN for the moment */
static INPUT_PORTS_START( mhavoc )
	PORT_START("IN0")   /* alpha */
	/* Bits 7-6 = selected based on player_1 */
	/* Bits 5-4 = common */
	/* Bit 3 = Gamma rcvd flag */
	/* Bit 2 = Gamma xmtd flag */
	/* Bit 1 = 2.4kHz (divide 2.5MHz by 1024) */
	/* Bit 0 = Vector generator halt flag */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("avg", avg_mhavoc_device, done_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,clock_r, NULL)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,gamma_xmtd_r, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,gamma_rcvd_r, NULL)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Diag Step/Coin C") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,mhavoc_bit67_r, "COIN\0SERVICE")

	PORT_START("IN1")   /* gamma */
	/* Bits 7-2 = input switches */
	/* Bit 1 = Alpha rcvd flag */
	/* Bit 0 = Alpha xmtd flag */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,alpha_xmtd_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,alpha_rcvd_r, NULL)
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("DIAL")  /* gamma */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40) PORT_REVERSE

	PORT_START("DSW1")  /* DIP Switch at position 13/14S */
	PORT_DIPNAME( 0x01, 0x00, "Adaptive Difficulty" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x0c, "50000")
	PORT_DIPSETTING(    0x00, "100000")
	PORT_DIPSETTING(    0x04, "200000")
	PORT_DIPSETTING(    0x08, DEF_STR( None ))
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ))
	PORT_DIPSETTING(    0x00, DEF_STR( Medium ))
	PORT_DIPSETTING(    0x30, DEF_STR( Hard ))
	PORT_DIPSETTING(    0x20, "Demo")
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3 (2 in Free Play)")
	PORT_DIPSETTING(    0xc0, "4 (3 in Free Play)")
	PORT_DIPSETTING(    0x80, "5 (4 in Free Play)")
	PORT_DIPSETTING(    0x40, "6 (5 in Free Play)")

	PORT_START("DSW2")  /* DIP Switch at position 8S */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Right Coin Mechanism" )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x0c, "x1" )
	PORT_DIPSETTING(    0x08, "x4" )
	PORT_DIPSETTING(    0x04, "x5" )
	PORT_DIPSETTING(    0x00, "x6" )
	PORT_DIPNAME( 0x10, 0x10, "Left Coin Mechanism" )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, "x1" )
	PORT_DIPSETTING(    0x00, "x2" )
	PORT_DIPNAME( 0xe0, 0xe0, "Bonus Credits" )         PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x80, "2 each 4" )
	PORT_DIPSETTING(    0x40, "1 each 3" )
	PORT_DIPSETTING(    0xa0, "1 each 4" )
	PORT_DIPSETTING(    0x60, "1 each 5" )
	PORT_DIPSETTING(    0xe0, DEF_STR( None ) )

	PORT_START("COIN")      /* dummy for player_1 = 0 on alpha */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )      /* Left Coin Switch  */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )      /* Right Coin */

	PORT_START("SERVICE")   /* dummy for player_1 = 1 on alpha */
	PORT_DIPNAME( 0x01, 0x01, "Credit to start" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( mhavocrv )
	PORT_INCLUDE( mhavoc )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,tms5220_r, NULL)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( mhavocp )
	PORT_INCLUDE( mhavoc )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( alphaone )
	PORT_START("IN0")   /* alpha (player_1 = 0) */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER("avg", avg_mhavoc_device, done_r, NULL)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, mhavoc_state,clock_r, NULL)
	PORT_BIT( 0x7c, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")   /* gamma */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DIAL")  /* gamma */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(40) PORT_REVERSE
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( mhavoc, mhavoc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("alpha", M6502, MHAVOC_CLOCK_2_5M)     /* 2.5 MHz */
	MCFG_CPU_PROGRAM_MAP(alpha_map)

	MCFG_CPU_ADD("gamma", M6502, MHAVOC_CLOCK_1_25M)    /* 1.25 MHz */
	MCFG_CPU_PROGRAM_MAP(gamma_map)

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TIMER_DRIVER_ADD_PERIODIC("5k_timer", mhavoc_state, mhavoc_cpu_irq_clock, attotime::from_hz(MHAVOC_CLOCK_5K))

	/* video hardware */
	MCFG_VECTOR_ADD("vector")
	MCFG_SCREEN_ADD("screen", VECTOR)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(400, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 300, 0, 260)
	MCFG_SCREEN_UPDATE_DEVICE("vector", vector_device, screen_update)

	MCFG_DEVICE_ADD("avg", AVG_MHAVOC, 0)
	MCFG_AVGDVG_VECTOR("vector")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	/* FIXME: Outputs 1,2,3 are tied together
	 * This signal and Output 4 are processed separately.
	 * Later they are mixed together again.
	 * ==> DISCRETE emulation, below is just an approximation.
	 */

	MCFG_SOUND_ADD("pokey1", POKEY, MHAVOC_CLOCK_1_25M)
	MCFG_POKEY_ALLPOT_R_CB(IOPORT("DSW1"))
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), CAP_U(0.001), 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("pokey2", POKEY, MHAVOC_CLOCK_1_25M)
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), CAP_U(0.001), 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("pokey3", POKEY, MHAVOC_CLOCK_1_25M)
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), CAP_U(0.001), 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("pokey4", POKEY, MHAVOC_CLOCK_1_25M)
	MCFG_POKEY_OUTPUT_OPAMP(RES_K(1), CAP_U(0.001), 5.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mhavocrv, mhavoc )

	MCFG_SOUND_ADD("tms", TMS5220, MHAVOC_CLOCK/2/9)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( alphaone, mhavoc )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("alpha")
	MCFG_CPU_PROGRAM_MAP(alphaone_map)
	MCFG_DEVICE_REMOVE("gamma")

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0, 580, 0, 500)

	/* sound hardware */
	MCFG_SOUND_REPLACE("pokey1", POKEY, MHAVOC_CLOCK_1_25M)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_REPLACE("pokey2", POKEY, MHAVOC_CLOCK_1_25M)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_REMOVE("pokey3")
	MCFG_DEVICE_REMOVE("pokey4")
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*
 * Notes:
 * the R3 roms are supported as "mhavoc", the R2 roms (with a bug in gameplay)
 * are supported as "mhavoc2".
 * "Return to Vax" - Jess Askey's souped up version (errors on self test)
 * are supported as "mhavocrv".
 * Prototype is supported as "mhavocp"
 * Alpha one is a single-board prototype
 */

ROM_START( mhavoc )
	/* Alpha Processor ROMs */
	ROM_REGION( 0x20000, "alpha", 0 )   /* 152KB for ROMs */
	/* Vector Generator ROM */
	ROM_LOAD( "136025.210",   0x05000, 0x2000, CRC(c67284ca) SHA1(d9adad80c266d36429444f483cac4ebcf1fec7b8) )

	/* Program ROM */
	ROM_LOAD( "136025.216",   0x08000, 0x4000, CRC(522a9cc0) SHA1(bbd75e01c45220e1c87bd1e013cf2c2fb9f376b2) )
	ROM_LOAD( "136025.217",   0x0c000, 0x4000, CRC(ea3d6877) SHA1(27823c1b546c073b37ff11a8cb25312ea71673c2) )

	/* Paged Program ROM */
	ROM_LOAD( "136025.215",   0x10000, 0x4000, CRC(a4d380ca) SHA1(c3cdc76054be2f904b1fb6f28c3c027eba5c3a70) ) /* page 0+1 */
	ROM_LOAD( "136025.318",   0x14000, 0x4000, CRC(ba935067) SHA1(05ad81e7a1982b9d8fddb48502546f48b5dc21b7) ) /* page 2+3 */

	/* Paged Vector Generator ROM */
	ROM_LOAD( "136025.106",   0x18000, 0x4000, CRC(2ca83c76) SHA1(cc1adca32f70af30c4590e9fd6b056b051ccdb38) ) /* page 0+1 */
	ROM_LOAD( "136025.107",   0x1c000, 0x4000, CRC(5f81c5f3) SHA1(be4055727a2d4536e37ec20150deffdb5af5b01f) ) /* page 2+3 */

	/* Gamma Processor ROM */
	ROM_REGION( 0x10000, "gamma", 0 )
	ROM_LOAD( "136025.108",   0x08000, 0x4000, CRC(93faf210) SHA1(7744368a1d520f986d1c4246113a7e24fcdd6d04) ) /* mirrored to c000-ffff for reset+interrupt vectors */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "036408-01.b1",   0x0000, 0x0100, BAD_DUMP CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END


ROM_START( mhavoc2 )
	/* Alpha Processor ROMs */
	ROM_REGION( 0x20000, "alpha", 0 )
	/* Vector Generator ROM */
	ROM_LOAD( "136025.110",   0x05000, 0x2000, CRC(16eef583) SHA1(277252bd716dd96d5b98ec5e33a3a6a3bc1a9abf) )

	/* Program ROM */
	ROM_LOAD( "136025.103",   0x08000, 0x4000, CRC(bf192284) SHA1(4c2dc3ba75122e521ebf2c42f89b31737613c2df) )
	ROM_LOAD( "136025.104",   0x0c000, 0x4000, CRC(833c5d4e) SHA1(932861b2a329172247c1a5d0a6498a00a1fce814) )

	/* Paged Program ROM - switched to 2000-3fff */
	ROM_LOAD( "136025.101",   0x10000, 0x4000, CRC(2b3b591f) SHA1(39fd6fdd14367906bc0102bde15d509d3289206b) ) /* page 0+1 */
	ROM_LOAD( "136025.109",   0x14000, 0x4000, CRC(4d766827) SHA1(7697bf6f92bff0e62850ed75ff66008a08583ef7) ) /* page 2+3 */

	/* Paged Vector Generator ROM */
	ROM_LOAD( "136025.106",   0x18000, 0x4000, CRC(2ca83c76) SHA1(cc1adca32f70af30c4590e9fd6b056b051ccdb38) ) /* page 0+1 */
	ROM_LOAD( "136025.107",   0x1c000, 0x4000, CRC(5f81c5f3) SHA1(be4055727a2d4536e37ec20150deffdb5af5b01f) ) /* page 2+3 */

	/* the last 0x1000 is used for the 2 RAM pages */

	/* Gamma Processor ROM */
	ROM_REGION( 0x10000, "gamma", 0 )
	ROM_LOAD( "136025.108",   0x08000, 0x4000, CRC(93faf210) SHA1(7744368a1d520f986d1c4246113a7e24fcdd6d04) ) /* mirrored to c000-ffff for reset+interrupt vectors */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "036408-01.b1",   0x0000, 0x0100, BAD_DUMP CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END


ROM_START( mhavocrv )
	/* Alpha Processor ROMs */
	ROM_REGION( 0x20000, "alpha", 0 )   /* 152KB for ROMs */
	/* Vector Generator ROM */
	ROM_LOAD( "136025.210",   0x05000, 0x2000, CRC(c67284ca) SHA1(d9adad80c266d36429444f483cac4ebcf1fec7b8) )

	/* Program ROM */
	ROM_LOAD( "136025.916",   0x08000, 0x4000, CRC(1255bd7f) SHA1(e277fe7b23ce8cf1294b6bfa5548b24a6c8952ce) )
	ROM_LOAD( "136025.917",   0x0c000, 0x4000, CRC(21889079) SHA1(d1ad6d9fa1432912e376bca50ceeefac2bfd6ac3) )

	/* Paged Program ROM */
	ROM_LOAD( "136025.915",   0x10000, 0x4000, CRC(4c7235dc) SHA1(67cafc2ce438ec389550efb46c554a7fe7b45efc) ) /* page 0+1 */
	ROM_LOAD( "136025.918",   0x14000, 0x4000, CRC(84735445) SHA1(21aacd862ce8911d257c6f48ead119ee5bb0b60d) ) /* page 2+3 */

	/* Paged Vector Generator ROM */
	ROM_LOAD( "136025.106",   0x18000, 0x4000, CRC(2ca83c76) SHA1(cc1adca32f70af30c4590e9fd6b056b051ccdb38) ) /* page 0+1 */
	ROM_LOAD( "136025.907",   0x1c000, 0x4000, CRC(4deea2c9) SHA1(c4107581748a3f2d2084de2a4f120abd67a52189) ) /* page 2+3 */

	/* Gamma Processor ROM */
	ROM_REGION( 0x10000, "gamma", 0 )
	ROM_LOAD( "136025.908",   0x08000, 0x4000, CRC(c52ec664) SHA1(08120a385f71b17ec02a3c2ef856ff835a91773e) ) /* mirrored to c000-ffff for reset+interrupt vectors */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "036408-01.b1",   0x0000, 0x0100, BAD_DUMP CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END


ROM_START( mhavocp )
	/* Alpha Processor ROMs */
	ROM_REGION( 0x20000, "alpha", 0 )
	/* Vector Generator ROM */
	ROM_LOAD( "136025.010",   0x05000, 0x2000, CRC(3050c0e6) SHA1(f19a9538996d949cdca7e6abd4f04e8ff6e0e2c1) )

	/* Program ROM */
	ROM_LOAD( "136025.016",   0x08000, 0x4000, CRC(94caf6c0) SHA1(8734411280bd0484c99a59231b97ad64d6e787e8) )
	ROM_LOAD( "136025.017",   0x0c000, 0x4000, CRC(05cba70a) SHA1(c069e6dec3e5bc278103156d0908ab93f3784be1) )

	/* Paged Program ROM - switched to 2000-3fff */
	ROM_LOAD( "136025.015",   0x10000, 0x4000, CRC(c567c11b) SHA1(23b89389f59bb6a040342adfe583818a91ce5bff) )
	ROM_LOAD( "136025.018",   0x14000, 0x4000, CRC(a8c35ccd) SHA1(c243a5407557390a64c6560d857f5031f839973f) )

	/* Paged Vector Generator ROM */
	ROM_LOAD( "136025.006",   0x18000, 0x4000, CRC(e272ed41) SHA1(0de395d1c4300a64da7f45746d7b550779e36a21) )
	ROM_LOAD( "136025.007",   0x1c000, 0x4000, CRC(e152c9d8) SHA1(79d0938fa9ad262c7f28c5a8ad21004a4dec9ed8) )

	/* the last 0x1000 is used for the 2 RAM pages */

	/* Gamma Processor ROM */
	ROM_REGION( 0x10000, "gamma", 0 )
	ROM_LOAD( "136025.008",   0x8000, 0x4000, CRC(22ea7399) SHA1(eeda8cc40089506063835a62c3273e7dd3918fd5) ) /* mirrored to c000-ffff for reset+interrupt vectors */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "036408-01.b1",   0x0000, 0x0100, BAD_DUMP CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END


ROM_START( alphaone )
	ROM_REGION( 0x20000, "alpha", 0 )
	/* Vector Generator ROM */
	ROM_LOAD( "vec5000.tw",   0x05000, 0x1000, CRC(2a4c149f) SHA1(b60a0b29958bee9b5f7c1d88163680b626bb76dd) )

	/* Program ROM */
	ROM_LOAD( "8000.tw",      0x08000, 0x2000, CRC(962d4da2) SHA1(2299f850aed7470a80a21526143f7b412a879cb1) )
	ROM_LOAD( "a000.tw",      0x0a000, 0x2000, CRC(f739a791) SHA1(1e70e446fc7dd27683ad71e768ebb2bc1d4fedd3) )
	ROM_LOAD( "twjk1.bin",    0x0c000, 0x2000, CRC(1ead0b34) SHA1(085e05526d029bcff7c8ae050cde73f52ee13846) )
	ROM_LOAD( "e000.tw",      0x0e000, 0x1000, CRC(6b1d7d2b) SHA1(36ac8b53e2fe01ed281c94afec02484ef676ddad) )
	ROM_RELOAD(               0x0f000, 0x1000 )

	/* Paged Program ROM - switched to 2000-3fff */
	ROM_LOAD( "page01.tw",    0x10000, 0x4000, CRC(cbf3b05a) SHA1(1dfaf9300a252c9c921f06167160a59cdf329726) )

	/* Paged Vector Generator ROM */
	ROM_LOAD( "vec_pg01.tw",  0x18000, 0x4000, CRC(e392a94d) SHA1(b5843da97d7aa5767c87c29660115efc5ad9ad54) )
	ROM_LOAD( "vec_pg23.tw",  0x1c000, 0x4000, CRC(1ff74292) SHA1(90e61c48544c62d905e207bba5c67ae7694e86a5) )

	/* the last 0x1000 is used for the 2 RAM pages */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "036408-01.b1",   0x0000, 0x0100, BAD_DUMP CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END


ROM_START( alphaonea )
	ROM_REGION( 0x20000, "alpha", 0 )
	/* Vector Generator ROM */
	ROM_LOAD( "vec5000.tw",   0x05000, 0x1000, CRC(2a4c149f) SHA1(b60a0b29958bee9b5f7c1d88163680b626bb76dd) )

	/* Program ROM */
	ROM_LOAD( "8000.tw",      0x08000, 0x2000, CRC(962d4da2) SHA1(2299f850aed7470a80a21526143f7b412a879cb1) )
	ROM_LOAD( "a000.tw",      0x0a000, 0x2000, CRC(f739a791) SHA1(1e70e446fc7dd27683ad71e768ebb2bc1d4fedd3) )
	ROM_LOAD( "c000.tw",      0x0c000, 0x2000, CRC(f21fb1ac) SHA1(2590147e75611a3f87397e7b0baa7020e7528ac8) )
	ROM_LOAD( "e000.tw",      0x0e000, 0x1000, CRC(6b1d7d2b) SHA1(36ac8b53e2fe01ed281c94afec02484ef676ddad) )
	ROM_RELOAD(               0x0f000, 0x1000 )

	/* Paged Program ROM - switched to 2000-3fff */
	ROM_LOAD( "page01.tw",    0x10000, 0x4000, CRC(cbf3b05a) SHA1(1dfaf9300a252c9c921f06167160a59cdf329726) )

	/* Paged Vector Generator ROM */
	ROM_LOAD( "vec_pg01.tw",  0x18000, 0x4000, CRC(e392a94d) SHA1(b5843da97d7aa5767c87c29660115efc5ad9ad54) )
	ROM_LOAD( "vec_pg23.tw",  0x1c000, 0x4000, CRC(1ff74292) SHA1(90e61c48544c62d905e207bba5c67ae7694e86a5) )

	/* the last 0x1000 is used for the 2 RAM pages */

	/* AVG PROM */
	ROM_REGION( 0x100, "user1", 0 )
	ROM_LOAD( "136002-125.6c",   0x0000, 0x0100, BAD_DUMP CRC(5903af03) SHA1(24bc0366f394ad0ec486919212e38be0f08d0239) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, mhavoc,   0,      mhavoc,   mhavoc, driver_device,   0,        ROT0, "Atari",         "Major Havoc (rev 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, mhavoc2,  mhavoc, mhavoc,   mhavoc, driver_device,   0,        ROT0, "Atari",         "Major Havoc (rev 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, mhavocrv, mhavoc, mhavocrv, mhavocrv, mhavoc_state, mhavocrv, ROT0, "Atari / JMA",   "Major Havoc (Return to Vax)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, mhavocp,  mhavoc, mhavoc,   mhavocp, driver_device,  0,        ROT0, "Atari",         "Major Havoc (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, alphaone, mhavoc, alphaone, alphaone, driver_device, 0,        ROT0, "Atari",         "Alpha One (prototype, 3 lives)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, alphaonea,mhavoc, alphaone, alphaone, driver_device, 0,        ROT0, "Atari",         "Alpha One (prototype, 5 lives)", MACHINE_SUPPORTS_SAVE )
