// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/konami.h"

#include "includes/ajax.h"

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

WRITE8_MEMBER(ajax_state::ajax_bankswitch_w)
{
	int bank = 0;

	/* rom select */
	if (!(data & 0x80))
		bank += 4;

	/* coin counters */
	coin_counter_w(machine(), 0, data & 0x20);
	coin_counter_w(machine(), 1, data & 0x40);

	/* priority */
	m_priority = data & 0x08;

	/* bank # (ROMS N11 and N12) */
	bank += (data & 0x07);
	membank("bank2")->set_entry(bank);
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

WRITE8_MEMBER(ajax_state::ajax_lamps_w)
{
	set_led_status(machine(), 1, data & 0x02);  /* super weapon lamp */
	set_led_status(machine(), 2, data & 0x04);  /* power up lamps */
	set_led_status(machine(), 5, data & 0x04);  /* power up lamps */
	set_led_status(machine(), 0, data & 0x20);  /* start lamp */
	set_led_status(machine(), 3, data & 0x40);  /* game over lamps */
	set_led_status(machine(), 6, data & 0x40);  /* game over lamps */
	set_led_status(machine(), 4, data & 0x80);  /* game over lamps */
	set_led_status(machine(), 7, data & 0x80);  /* game over lamps */
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

READ8_MEMBER(ajax_state::ajax_ls138_f10_r)
{
	int data = 0, index;
	static const char *const portnames[] = { "SYSTEM", "P1", "DSW1", "DSW2" };

	switch ((offset & 0x01c0) >> 6)
	{
		case 0x00:  /* ??? */
			data = machine().rand();
			break;
		case 0x04:  /* 2P inputs */
			data = ioport("P2")->read();
			break;
		case 0x06:  /* 1P inputs + DIPSW #1 & #2 */
			index = offset & 0x01;
			data = ioport((offset & 0x02) ? portnames[2 + index] : portnames[index])->read();
			break;
		case 0x07:  /* DIPSW #3 */
			data = ioport("DSW3")->read();
			break;

		default:
			logerror("%04x: (ls138_f10) read from an unknown address %02x\n",space.device().safe_pc(), offset);
	}

	return data;
}

WRITE8_MEMBER(ajax_state::ajax_ls138_f10_w)
{
	switch ((offset & 0x01c0) >> 6)
	{
		case 0x00:  /* NSFIRQ + AFR */
			if (offset)
				watchdog_reset_w(space, 0, data);
			else{
				if (m_firq_enable)  /* Cause interrupt on slave CPU */
					m_subcpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
			}
			break;
		case 0x01:  /* Cause interrupt on audio CPU */
			m_audiocpu->set_input_line(0, HOLD_LINE);
			break;
		case 0x02:  /* Sound command number */
			soundlatch_byte_w(space, offset, data);
			break;
		case 0x03:  /* Bankswitch + coin counters + priority*/
			ajax_bankswitch_w(space, 0, data);
			break;
		case 0x05:  /* Lamps + Joystick vibration + Control panel quaking */
			ajax_lamps_w(space, 0, data);
			break;

		default:
			logerror("%04x: (ls138_f10) write %02x to an unknown address %02x\n", space.device().safe_pc(), data, offset);
	}
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

WRITE8_MEMBER(ajax_state::ajax_bankswitch_2_w)
{
	/* enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 5 enables 051316 wraparound */
	m_k051316->wraparound_enable(data & 0x20);

	/* FIRQ control */
	m_firq_enable = data & 0x10;

	/* bank # (ROMS G16 and I16) */
	membank("bank1")->set_entry(data & 0x0f);
}

void ajax_state::machine_start()
{
	UINT8 *MAIN = memregion("maincpu")->base();
	UINT8 *SUB  = memregion("sub")->base();

	membank("bank1")->configure_entries(0,  9,  &SUB[0x10000], 0x2000);
	membank("bank2")->configure_entries(0, 12, &MAIN[0x10000], 0x2000);

	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);

	save_item(NAME(m_priority));
	save_item(NAME(m_firq_enable));
}

void ajax_state::machine_reset()
{
	m_priority = 0;
	m_firq_enable = 0;
}
