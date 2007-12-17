/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "cpu/konami/konami.h"
#include "video/konamiic.h"

UINT8 *ajax_sharedram;
extern UINT8 ajax_priority;
static int firq_enable;

/*  ajax_bankswitch_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at H11:

    Bit Description
    --- -----------
    7   MRB3    Selects ROM N11/N12
    6   CCOUNT2 Coin Counter 2  (*)
    5   CCOUNT1 Coin Counter 1  (*)
    4   SRESET  Slave CPU Reset?
    3   PRI0    Layer Priority Selector
    2   MRB2    \
    1   MRB1     |  ROM Bank Select
    0   MRB0    /

    (*) The Coin Counters are handled by the Konami Custom 051550
*/

static WRITE8_HANDLER( ajax_bankswitch_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	int bankaddress = 0;

	/* rom select */
	if (!(data & 0x80))	bankaddress += 0x8000;

	/* coin counters */
	coin_counter_w(0,data & 0x20);
	coin_counter_w(1,data & 0x40);

	/* priority */
	ajax_priority = data & 0x08;

	/* bank # (ROMS N11 and N12) */
	bankaddress += 0x10000 + (data & 0x07)*0x2000;
	memory_set_bankptr(2,&RAM[bankaddress]);
}

/*  ajax_lamps_w:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at B9:

    Bit Description
    --- -----------
    7   LAMP7 & LAMP8 - Game over lamps (*)
    6   LAMP3 & LAMP4 - Game over lamps (*)
    5   LAMP1 - Start lamp (*)
    4   Control panel quaking (**)
    3   Joystick vibration (**)
    2   LAMP5 & LAMP6 - Power up lamps (*)
    1   LAMP2 - Super weapon lamp (*)
    0   unused

    (*) The Lamps are handled by the M54585P
    (**)Vibration/Quaking handled by these chips:
        Chip        Location    Description
        ----        --------    -----------
        PS2401-4    B21         ???
        UPA1452H    B22         ???
        LS74        H2          Dual +ve edge trigger D-type Flip-flop with SET and RESET
        LS393       C20         Dual -ve edge trigger 4-bit Binary Ripple Counter with Resets
*/

static WRITE8_HANDLER( ajax_lamps_w )
{
	set_led_status(1,data & 0x02);	/* super weapon lamp */
	set_led_status(2,data & 0x04);	/* power up lamps */
	set_led_status(5,data & 0x04);	/* power up lamps */
	set_led_status(0,data & 0x20);	/* start lamp */
	set_led_status(3,data & 0x40);	/* game over lamps */
	set_led_status(6,data & 0x40);	/* game over lamps */
	set_led_status(4,data & 0x80);	/* game over lamps */
	set_led_status(7,data & 0x80);	/* game over lamps */
}

/*  ajax_ls138_f10:
    The LS138 1-of-8 Decoder/Demultiplexer at F10 selects what to do:

    Address R/W Description
    ------- --- -----------
    0x0000  (r) ??? I think this read is because a CPU core bug
            (w) 0x0000  NSFIRQ  Trigger FIRQ on the M6809
                0x0020  AFR     Watchdog reset (handled by the 051550)
    0x0040  (w) SOUND           Cause interrupt on the Z80
    0x0080  (w) SOUNDDATA       Sound code number
    0x00c0  (w) MBL1            Enables the LS273 at H11 (Banking + Coin counters)
    0x0100  (r) MBL2            Enables 2P Inputs reading
    0x0140  (w) MBL3            Enables the LS273 at B9 (Lamps + Vibration)
    0x0180  (r) MIO1            Enables 1P Inputs + DIPSW #1 & #2 reading
    0x01c0  (r) MIO2            Enables DIPSW #3 reading
*/

READ8_HANDLER( ajax_ls138_f10_r )
{
	int data = 0;

	switch ((offset & 0x01c0) >> 6){
		case 0x00:	/* ??? */
			data = mame_rand(Machine);
			break;
		case 0x04:	/* 2P inputs */
			data = readinputport(5);
			break;
		case 0x06:	/* 1P inputs + DIPSW #1 & #2 */
			if (offset & 0x02)
				data = readinputport(offset & 0x01);
			else
				data = readinputport(3 + (offset & 0x01));
			break;
		case 0x07:	/* DIPSW #3 */
			data = readinputport(2);
			break;

		default:
			logerror("%04x: (ls138_f10) read from an unknown address %02x\n",activecpu_get_pc(), offset);
	}

	return data;
}

WRITE8_HANDLER( ajax_ls138_f10_w )
{
	switch ((offset & 0x01c0) >> 6){
		case 0x00:	/* NSFIRQ + AFR */
			if (offset)
				watchdog_reset_w(0, data);
			else{
				if (firq_enable)	/* Cause interrupt on slave CPU */
					cpunum_set_input_line(1, M6809_FIRQ_LINE, HOLD_LINE);
			}
			break;
		case 0x01:	/* Cause interrupt on audio CPU */
			cpunum_set_input_line(2, 0, HOLD_LINE);
			break;
		case 0x02:	/* Sound command number */
			soundlatch_w(offset,data);
			break;
		case 0x03:	/* Bankswitch + coin counters + priority*/
			ajax_bankswitch_w(0, data);
			break;
		case 0x05:	/* Lamps + Joystick vibration + Control panel quaking */
			ajax_lamps_w(0, data);
			break;

		default:
			logerror("%04x: (ls138_f10) write %02x to an unknown address %02x\n",activecpu_get_pc(), data, offset);
	}
}

/* Shared RAM between the 052001 and the 6809 (6264SL at I8) */
READ8_HANDLER( ajax_sharedram_r )
{
	return ajax_sharedram[offset];
}

WRITE8_HANDLER( ajax_sharedram_w )
{
	ajax_sharedram[offset] = data;
}

/*  ajax_bankswitch_w_2:
    Handled by the LS273 Octal +ve edge trigger D-type Flip-flop with Reset at K14:

    Bit Description
    --- -----------
    7   unused
    6   RMRD    Enable char ROM reading through the video RAM
    5   RVO     enables 051316 wraparound
    4   FIRQST  FIRQ control
    3   SRB3    \
    2   SRB2     |
    1   SRB1     |  ROM Bank Select
    0   SRB0    /
*/

WRITE8_HANDLER( ajax_bankswitch_2_w )
{
	UINT8 *RAM = memory_region(REGION_CPU2);
	int bankaddress;

	/* enable char ROM reading through the video RAM */
	K052109_set_RMRD_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 5 enables 051316 wraparound */
	K051316_wraparound_enable(0, data & 0x20);

	/* FIRQ control */
	firq_enable = data & 0x10;

	/* bank # (ROMS G16 and I16) */
	bankaddress = 0x10000 + (data & 0x0f)*0x2000;
	memory_set_bankptr(1,&RAM[bankaddress]);
}


MACHINE_RESET( ajax )
{
	firq_enable = 1;
}

INTERRUPT_GEN( ajax_interrupt )
{
	if (K051960_is_IRQ_enabled())
		cpunum_set_input_line(0, KONAMI_IRQ_LINE, HOLD_LINE);
}
