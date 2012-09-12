/***************************************************************************

    Hard Drivin' machine hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/m68000/m68000.h"
#include "cpu/dsp32/dsp32.h"
#include "machine/atarigen.h"
#include "machine/asic65.h"
#include "audio/atarijsa.h"
#include "includes/slapstic.h"
#include "includes/harddriv.h"


/*************************************
 *
 *  Constants and macros
 *
 *************************************/

#define DS3_TRIGGER			7777

/* debugging tools */
#define LOG_COMMANDS		0



/*************************************
 *
 *  Static globals
 *
 *************************************/


static void hd68k_update_interrupts(running_machine &machine);



#if 0
#pragma mark * DRIVER/MULTISYNC BOARD
#endif


/*************************************
 *
 *  Initialization
 *
 *************************************/

MACHINE_START( harddriv )
{
	harddriv_state *state = machine.driver_data<harddriv_state>();

	atarigen_init(machine);

	/* predetermine memory regions */
	state->m_sim_memory = (UINT16 *)state->memregion("user1")->base();
	state->m_sim_memory_size = state->memregion("user1")->bytes() / 2;
	state->m_adsp_pgm_memory_word = (UINT16 *)(reinterpret_cast<UINT8 *>(state->m_adsp_pgm_memory.target()) + 1);
}


MACHINE_RESET( harddriv )
{
	harddriv_state *state = machine.driver_data<harddriv_state>();

	/* generic reset */
	atarigen_eeprom_reset(state);
	slapstic_reset();
	atarigen_interrupt_reset(state, hd68k_update_interrupts);

	/* halt several of the DSPs to start */
	if (state->m_adsp != NULL) state->m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	if (state->m_dsp32 != NULL) state->m_dsp32->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	if (state->m_sounddsp != NULL) state->m_sounddsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	/* if we found a 6502, reset the JSA board */
	if (state->m_jsacpu != NULL)
		atarijsa_reset();

	state->m_last_gsp_shiftreg = 0;

	state->m_m68k_adsp_buffer_bank = 0;

	/* reset IRQ states */
	state->m_irq_state = state->m_gsp_irq_state = state->m_msp_irq_state = state->m_adsp_irq_state = state->m_duart_irq_state = 0;

	/* reset the ADSP/DSIII/DSIV boards */
	state->m_adsp_halt = 1;
	state->m_adsp_br = 0;
	state->m_adsp_xflag = 0;
}



/*************************************
 *
 *  68000 interrupt handling
 *
 *************************************/

static void hd68k_update_interrupts(running_machine &machine)
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	state->m_maincpu->set_input_line(1, state->m_msp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	state->m_maincpu->set_input_line(2, state->m_adsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	state->m_maincpu->set_input_line(3, state->m_gsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	state->m_maincpu->set_input_line(4, state->m_sound_int_state ? ASSERT_LINE : CLEAR_LINE);	/* /LINKIRQ on STUN Runner */
	state->m_maincpu->set_input_line(5, state->m_irq_state ? ASSERT_LINE : CLEAR_LINE);
	state->m_maincpu->set_input_line(6, state->m_duart_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN( hd68k_irq_gen )
{
	harddriv_state *state = device->machine().driver_data<harddriv_state>();
	state->m_irq_state = 1;
	atarigen_update_interrupts(device->machine());
}


WRITE16_HANDLER( hd68k_irq_ack_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	state->m_irq_state = 0;
	atarigen_update_interrupts(space->machine());
}


void hdgsp_irq_gen(device_t *device, int irqstate)
{
	harddriv_state *state = device->machine().driver_data<harddriv_state>();
	state->m_gsp_irq_state = irqstate;
	atarigen_update_interrupts(device->machine());
}


void hdmsp_irq_gen(device_t *device, int irqstate)
{
	harddriv_state *state = device->machine().driver_data<harddriv_state>();
	state->m_msp_irq_state = irqstate;
	atarigen_update_interrupts(device->machine());
}



/*************************************
 *
 *  68000 access to GSP
 *
 *************************************/

READ16_HANDLER( hd68k_gsp_io_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT16 result;
	offset = (offset / 2) ^ 1;
	state->m_hd34010_host_access = TRUE;
	result = tms34010_host_r(state->m_gsp, offset);
	state->m_hd34010_host_access = FALSE;
	return result;
}


WRITE16_HANDLER( hd68k_gsp_io_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	offset = (offset / 2) ^ 1;
	state->m_hd34010_host_access = TRUE;
	tms34010_host_w(state->m_gsp, offset, data);
	state->m_hd34010_host_access = FALSE;
}



/*************************************
 *
 *  68000 access to MSP
 *
 *************************************/

READ16_HANDLER( hd68k_msp_io_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT16 result;
	offset = (offset / 2) ^ 1;
	state->m_hd34010_host_access = TRUE;
	result = (state->m_msp != NULL) ? tms34010_host_r(state->m_msp, offset) : 0xffff;
	state->m_hd34010_host_access = FALSE;
	return result;
}


WRITE16_HANDLER( hd68k_msp_io_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	offset = (offset / 2) ^ 1;
	if (state->m_msp != NULL)
	{
		state->m_hd34010_host_access = TRUE;
		tms34010_host_w(state->m_msp, offset, data);
		state->m_hd34010_host_access = FALSE;
	}
}



/*************************************
 *
 *  68000 input handlers
 *
 *************************************/

READ16_HANDLER( hd68k_port0_r )
{
	/* port is as follows:

        0x0001 = DIAGN
        0x0002 = /HSYNCB
        0x0004 = /VSYNCB
        0x0008 = EOC12
        0x0010 = EOC8
        0x0020 = SELF-TEST
        0x0040 = COIN2
        0x0080 = COIN1
        0x0100 = SW1 #8
        0x0200 = SW1 #7
            .....
        0x8000 = SW1 #1
    */
	int temp = (space->machine().root_device().ioport("SW1")->read() << 8) | space->machine().root_device().ioport("IN0")->read();
	if (atarigen_get_hblank(*space->machine().primary_screen)) temp ^= 0x0002;
	temp ^= 0x0018;		/* both EOCs always high for now */
	return temp;
}


READ16_HANDLER( hdc68k_port1_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT16 result = state->ioport("a80000")->read();
	UINT16 diff = result ^ state->m_hdc68k_last_port1;

	/* if a new shifter position is selected, use it */
	/* if it's the same shifter position as last time, go back to neutral */
	if ((diff & 0x0100) && !(result & 0x0100))
		state->m_hdc68k_shifter_state = (state->m_hdc68k_shifter_state == 1) ? 0 : 1;
	if ((diff & 0x0200) && !(result & 0x0200))
		state->m_hdc68k_shifter_state = (state->m_hdc68k_shifter_state == 2) ? 0 : 2;
	if ((diff & 0x0400) && !(result & 0x0400))
		state->m_hdc68k_shifter_state = (state->m_hdc68k_shifter_state == 4) ? 0 : 4;
	if ((diff & 0x0800) && !(result & 0x0800))
		state->m_hdc68k_shifter_state = (state->m_hdc68k_shifter_state == 8) ? 0 : 8;

	/* merge in the new shifter value */
	result = (result | 0x0f00) ^ (state->m_hdc68k_shifter_state << 8);

	/* merge in the wheel edge latch bit */
	if (state->m_hdc68k_wheel_edge)
		result ^= 0x4000;

	state->m_hdc68k_last_port1 = result;
	return result;
}


READ16_HANDLER( hda68k_port1_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT16 result = state->ioport("a80000")->read();

	/* merge in the wheel edge latch bit */
	if (state->m_hdc68k_wheel_edge)
		result ^= 0x4000;

	return result;
}


READ16_HANDLER( hdc68k_wheel_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	/* grab the new wheel value and upconvert to 12 bits */
	UINT16 new_wheel = state->ioport("12BADC0")->read() << 4;

	/* hack to display the wheel position */
	if (space->machine().input().code_pressed(KEYCODE_LSHIFT))
		popmessage("%04X", new_wheel);

	/* if we crossed the center line, latch the edge bit */
	if ((state->m_hdc68k_last_wheel / 0xf0) != (new_wheel / 0xf0))
		state->m_hdc68k_wheel_edge = 1;

	/* remember the last value and return the low 8 bits */
	state->m_hdc68k_last_wheel = new_wheel;
	return (new_wheel << 8) | 0xff;
}


READ16_HANDLER( hd68k_adc8_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_adc8_data;
}


READ16_HANDLER( hd68k_adc12_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_adc12_byte ? ((state->m_adc12_data >> 8) & 0x0f) : (state->m_adc12_data & 0xff);
}


READ16_HANDLER( hd68k_sound_reset_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	if (state->m_jsacpu != NULL)
		atarijsa_reset();
	return ~0;
}



/*************************************
 *
 *  68000 output handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_adc_control_w )
{
	static const char *const adc8names[] = { "8BADC0", "8BADC1", "8BADC2", "8BADC3", "8BADC4", "8BADC5", "8BADC6", "8BADC7" };
	static const char *const adc12names[] = { "12BADC0", "12BADC1", "12BADC2", "12BADC3" };
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	COMBINE_DATA(&state->m_adc_control);

	/* handle a write to the 8-bit ADC address select */
	if (state->m_adc_control & 0x08)
	{
		state->m_adc8_select = state->m_adc_control & 0x07;
		state->m_adc8_data = state->ioport(adc8names[state->m_adc8_select])->read();
	}

	/* handle a write to the 12-bit ADC address select */
	if (state->m_adc_control & 0x40)
	{
		state->m_adc12_select = (state->m_adc_control >> 4) & 0x03;
		state->m_adc12_data = space->machine().root_device().ioport(adc12names[state->m_adc12_select])->read() << 4;
	}

	/* bit 7 selects which byte of the 12 bit data to read */
	state->m_adc12_byte = (state->m_adc_control >> 7) & 1;
}


WRITE16_HANDLER( hd68k_wr0_write )
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 1:	/* SEL1 */
		case 2:	/* SEL2 */
		case 3:	/* SEL3 */
		case 4:	/* SEL4 */
		default:
			/* just ignore */
			break;

		case 6:	/* CC1 */
		case 7:	/* CC2 */
			coin_counter_w(space->machine(), offset - 6, data);
			break;
	}
}


WRITE16_HANDLER( hd68k_wr1_write )
{
	if (offset == 0) { //   logerror("Shifter Interface Latch = %02X\n", data);
	} else {				logerror("/WR1(%04X)=%02X\n", offset, data);
	}
}


WRITE16_HANDLER( hd68k_wr2_write )
{
	if (offset == 0) { //   logerror("Steering Wheel Latch = %02X\n", data);
	} else {				logerror("/WR2(%04X)=%02X\n", offset, data);
	}
}


WRITE16_HANDLER( hd68k_nwr_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:	/* CR2 */
		case 1:	/* CR1 */
			set_led_status(space->machine(), offset, data);
			break;
		case 2:	/* LC1 */
			break;
		case 3:	/* LC2 */
			break;
		case 4:	/* ZP1 */
			state->m_m68k_zp1 = data;
			break;
		case 5:	/* ZP2 */
			state->m_m68k_zp2 = data;
			break;
		case 6:	/* /GSPRES */
			logerror("Write to /GSPRES(%d)\n", data);
			if (state->m_gsp != NULL)
				state->m_gsp->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
		case 7:	/* /MSPRES */
			logerror("Write to /MSPRES(%d)\n", data);
			if (state->m_msp != NULL)
				state->m_msp->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


WRITE16_HANDLER( hdc68k_wheel_edge_reset_w )
{
	/* reset the edge latch */
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	state->m_hdc68k_wheel_edge = 0;
}



/*************************************
 *
 *  68000 ZRAM access
 *
 *************************************/

READ16_HANDLER( hd68k_zram_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_eeprom[offset];
}


WRITE16_HANDLER( hd68k_zram_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	if (state->m_m68k_zp1 == 0 && state->m_m68k_zp2 == 1)
		COMBINE_DATA(&state->m_eeprom[offset]);
}



/*************************************
 *
 *  68681 DUART
 *
 *************************************/

void harddriv_duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	harddriv_state *hd_state = device->machine().driver_data<harddriv_state>();
	hd_state->m_duart_irq_state = state;
	atarigen_update_interrupts(device->machine());
}


/*************************************
 *
 *  GSP I/O register writes
 *
 *************************************/

WRITE16_HANDLER( hdgsp_io_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	/* detect an enabling of the shift register and force yielding */
	if (offset == REG_DPYCTL)
	{
		UINT8 new_shiftreg = (data >> 11) & 1;
		if (new_shiftreg != state->m_last_gsp_shiftreg)
		{
			state->m_last_gsp_shiftreg = new_shiftreg;
			if (new_shiftreg)
				space->device().execute().yield();
		}
	}

	/* detect changes to HEBLNK and HSBLNK and force an update before they change */
	if ((offset == REG_HEBLNK || offset == REG_HSBLNK) && data != tms34010_io_register_r(space, offset, 0xffff))
		space->machine().primary_screen->update_partial(space->machine().primary_screen->vpos() - 1);

	tms34010_io_register_w(space, offset, data, mem_mask);
}



/*************************************
 *
 *  GSP protection workarounds
 *
 *************************************/

WRITE16_HANDLER( hdgsp_protection_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	/* this memory address is incremented whenever a protection check fails */
	/* after it reaches a certain value, the GSP will randomly trash a */
	/* register; we just prevent it from ever going above 0 */
	*state->m_gsp_protection = 0;
}


#if 0
#pragma mark -
#pragma mark * ADSP BOARD
#endif

/*************************************
 *
 *  68000 access to ADSP program memory
 *
 *************************************/

READ16_HANDLER( hd68k_adsp_program_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT32 word = state->m_adsp_pgm_memory[offset/2];
	return (!(offset & 1)) ? (word >> 16) : (word & 0xffff);
}


WRITE16_HANDLER( hd68k_adsp_program_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT32 *base = &state->m_adsp_pgm_memory[offset/2];
	UINT32 oldword = *base;
	UINT16 temp;

	if (!(offset & 1))
	{
		temp = oldword >> 16;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0x0000ffff) | (temp << 16);
	}
	else
	{
		temp = oldword & 0xffff;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0xffff0000) | temp;
	}
	*base = oldword;
}



/*************************************
 *
 *  68000 access to ADSP data memory
 *
 *************************************/

READ16_HANDLER( hd68k_adsp_data_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_adsp_data_memory[offset];
}


WRITE16_HANDLER( hd68k_adsp_data_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	COMBINE_DATA(&state->m_adsp_data_memory[offset]);

	/* any write to $1FFF is taken to be a trigger; synchronize the CPUs */
	if (offset == 0x1fff)
	{
		logerror("%06X:ADSP sync address written (%04X)\n", space->device().safe_pcbase(), data);
		space->machine().scheduler().synchronize();
		state->m_adsp->signal_interrupt_trigger();
	}
	else
		logerror("%06X:ADSP W@%04X (%04X)\n", space->device().safe_pcbase(), offset, data);
}



/*************************************
 *
 *  68000 access to ADSP output memory
 *
 *************************************/

READ16_HANDLER( hd68k_adsp_buffer_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
/*  logerror("hd68k_adsp_buffer_r(%04X)\n", offset);*/
	return state->m_som_memory[state->m_m68k_adsp_buffer_bank * 0x2000 + offset];
}


WRITE16_HANDLER( hd68k_adsp_buffer_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	COMBINE_DATA(&state->m_som_memory[state->m_m68k_adsp_buffer_bank * 0x2000 + offset]);
}



/*************************************
 *
 *  68000 access to ADSP control regs
 *
 *************************************/

static TIMER_CALLBACK( deferred_adsp_bank_switch )
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	if (LOG_COMMANDS && state->m_m68k_adsp_buffer_bank != param && machine.input().code_pressed(KEYCODE_L))
	{
		static FILE *commands;
		if (!commands) commands = fopen("commands.log", "w");
		if (commands)
		{
			INT16 *base = (INT16 *)&state->m_som_memory[param * 0x2000];
			INT16 *end = base + (UINT16)*base;
			INT16 *current = base + 1;
			INT16 *table = base + 1 + (UINT16)*current++;

			fprintf(commands, "\n---------------\n");

			while ((current + 5) < table)
			{
				int offset = (int)(current - base);
				int c1 = *current++;
				int c2 = *current++;
				int c3 = *current++;
				int c4 = *current++;
				fprintf(commands, "Cmd @ %04X = %04X  %d-%d @ %d\n", offset, c1, c2, c3, c4);
				while (current < table)
				{
					UINT32 rslope, lslope;
					rslope = (UINT16)*current++,
					rslope |= *current++ << 16;
					if (rslope == 0xffffffff)
					{
						fprintf(commands, "  (end)\n");
						break;
					}
					lslope = (UINT16)*current++,
					lslope |= *current++ << 16;
					fprintf(commands, "  L=%08X R=%08X count=%d\n",
							(int)lslope, (int)rslope, (int)*current++);
				}
			}
			fprintf(commands, "\nTable:\n");
			current = table;
			while (current < end)
				fprintf(commands, "  %04X\n", *current++);
		}
	}

	state->m_m68k_adsp_buffer_bank = param;
	logerror("ADSP bank = %d\n", param);
}


WRITE16_HANDLER( hd68k_adsp_control_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	/* bit 3 selects the value; data is ignored */
	int val = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:
		case 1:
			/* LEDs */
			break;

		case 3:
			logerror("ADSP bank = %d (deferred)\n", val);
			space->machine().scheduler().synchronize(FUNC(deferred_adsp_bank_switch), val);
			break;

		case 5:
			/* connected to the /BR (bus request) line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			state->m_adsp_br = !val;
			logerror("ADSP /BR = %d\n", !state->m_adsp_br);
			if (state->m_adsp_br || state->m_adsp_halt)
				state->m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				state->m_adsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				space->device().execute().spin();
			}
			break;

		case 6:
			/* connected to the /HALT line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			state->m_adsp_halt = !val;
			logerror("ADSP /HALT = %d\n", !state->m_adsp_halt);
			if (state->m_adsp_br || state->m_adsp_halt)
				state->m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				state->m_adsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				space->device().execute().spin();
			}
			break;

		case 7:
			logerror("ADSP reset = %d\n", val);
			state->m_adsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			space->device().execute().yield();
			break;

		default:
			logerror("ADSP control %02X = %04X\n", offset, data);
			break;
	}
}


WRITE16_HANDLER( hd68k_adsp_irq_clear_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	logerror("%06X:68k clears ADSP interrupt\n", space->device().safe_pcbase());
	state->m_adsp_irq_state = 0;
	atarigen_update_interrupts(space->machine());
}


READ16_HANDLER( hd68k_adsp_irq_state_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int result = 0xfffd;
	if (state->m_adsp_xflag) result ^= 2;
	if (state->m_adsp_irq_state) result ^= 1;
	logerror("%06X:68k reads ADSP interrupt state = %04x\n", space->device().safe_pcbase(), result);
	return result;
}



/*************************************
 *
 *  ADSP memory-mapped I/O
 *
 *************************************/

READ16_HANDLER( hdadsp_special_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	switch (offset & 7)
	{
		case 0:	/* /SIMBUF */
			if (state->m_adsp_eprom_base + state->m_adsp_sim_address < state->m_sim_memory_size)
				return state->m_sim_memory[state->m_adsp_eprom_base + state->m_adsp_sim_address++];
			else
				return 0xff;

		case 1:	/* /SIMLD */
			break;

		case 2:	/* /SOMO */
			break;

		case 3:	/* /SOMLD */
			break;

		default:
			logerror("%04X:hdadsp_special_r(%04X)\n", space->device().safe_pcbase(), offset);
			break;
	}
	return 0;
}


WRITE16_HANDLER( hdadsp_special_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	switch (offset & 7)
	{
		case 1:	/* /SIMCLK */
			state->m_adsp_sim_address = data;
			break;

		case 2:	/* SOMLATCH */
			state->m_som_memory[(state->m_m68k_adsp_buffer_bank ^ 1) * 0x2000 + (state->m_adsp_som_address++ & 0x1fff)] = data;
			break;

		case 3:	/* /SOMCLK */
			state->m_adsp_som_address = data;
			break;

		case 5:	/* /XOUT */
			state->m_adsp_xflag = data & 1;
			break;

		case 6:	/* /GINT */
			logerror("%04X:ADSP signals interrupt\n", space->device().safe_pcbase());
			state->m_adsp_irq_state = 1;
			atarigen_update_interrupts(space->machine());
			break;

		case 7:	/* /MP */
			state->m_adsp_eprom_base = 0x10000 * data;
			break;

		default:
			logerror("%04X:hdadsp_special_w(%04X)=%04X\n", space->device().safe_pcbase(), offset, data);
			break;
	}
}



#if 0
#pragma mark -
#pragma mark * DS III BOARD
#endif

/*************************************
 *
 *  General DS III I/O
 *
 *************************************/

static void update_ds3_irq(harddriv_state *state)
{
	/* update the IRQ2 signal to the ADSP2101 */
	if (!(!state->m_ds3_g68flag && state->m_ds3_g68irqs) && !(state->m_ds3_gflag && state->m_ds3_gfirqs))
		state->m_adsp->set_input_line(ADSP2100_IRQ2, ASSERT_LINE);
	else
		state->m_adsp->set_input_line(ADSP2100_IRQ2, CLEAR_LINE);
}


WRITE16_HANDLER( hd68k_ds3_control_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int val = (offset >> 3) & 1;

	switch (offset & 7)
	{
		case 0:
			/* SRES - reset sound CPU */
			break;

		case 1:
			/* XRES - reset sound helper CPU */
			break;

		case 2:
			/* connected to the /BR (bus request) line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			state->m_adsp_br = !val;
			if (state->m_adsp_br)
				state->m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				state->m_adsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				space->device().execute().spin();
			}
			break;

		case 3:
			state->m_adsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			if (val && !state->m_ds3_reset)
			{
				state->m_ds3_gflag = 0;
				state->m_ds3_gcmd = 0;
				state->m_ds3_gfirqs = 0;
				state->m_ds3_g68irqs = !state->m_ds3_gfirqs;
				state->m_ds3_send = 0;
				update_ds3_irq(state);
			}
			state->m_ds3_reset = val;
			space->device().execute().yield();
			logerror("DS III reset = %d\n", val);
			break;

		case 7:
			/* LED */
			break;

		default:
			logerror("DS III control %02X = %04X\n", offset, data);
			break;
	}
}



/*************************************
 *
 *  DS III graphics I/O
 *
 *************************************/

READ16_HANDLER( hd68k_ds3_girq_state_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int result = 0x0fff;
	if (state->m_ds3_g68flag) result ^= 0x8000;
	if (state->m_ds3_gflag) result ^= 0x4000;
	if (state->m_ds3_g68irqs) result ^= 0x2000;
	if (!state->m_adsp_irq_state) result ^= 0x1000;
	return result;
}


READ16_HANDLER( hd68k_ds3_gdata_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	offs_t pc = space->device().safe_pc();

	state->m_ds3_gflag = 0;
	update_ds3_irq(state);

	logerror("%06X:hd68k_ds3_gdata_r(%04X)\n", space->device().safe_pcbase(), state->m_ds3_gdata);

	/* attempt to optimize the transfer if conditions are right */
	if (&space->device() == state->m_maincpu && pc == state->m_ds3_transfer_pc &&
		!(!state->m_ds3_g68flag && state->m_ds3_g68irqs) && !(state->m_ds3_gflag && state->m_ds3_gfirqs))
	{
		UINT32 destaddr = state->m_maincpu->state_int(M68K_A1);
		UINT16 count68k = state->m_maincpu->state_int(M68K_D1);
		UINT16 mstat = state->m_adsp->state_int(ADSP2100_MSTAT);
		UINT16 i6 = state->m_adsp->state_int((mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC);
		UINT16 l6 = state->m_adsp->state_int(ADSP2100_L6) - 1;
		UINT16 m7 = state->m_adsp->state_int(ADSP2100_M7);

		logerror("%06X:optimizing 68k transfer, %d words\n", state->m_maincpu->pcbase(), count68k);

		while (count68k > 0 && state->m_adsp_data_memory[0x16e6] > 0)
		{
			space->write_word(destaddr, state->m_ds3_gdata);
			{
				state->m_adsp_data_memory[0x16e6]--;
				state->m_ds3_gdata = state->m_adsp_pgm_memory[i6] >> 8;
				i6 = (i6 & ~l6) | ((i6 + m7) & l6);
			}
			count68k--;
		}
		state->m_maincpu->set_state_int(M68K_D1, count68k);
		state->m_adsp->set_state_int((mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC, i6);
		state->m_adsp_speedup_count[1]++;
	}

	/* if we just cleared the IRQ, we are going to do some VERY timing critical reads */
	/* it is important that all the CPUs be in sync before we continue, so spin a little */
	/* while to let everyone else catch up */
	space->device().execute().spin_until_trigger(DS3_TRIGGER);
	space->machine().scheduler().trigger(DS3_TRIGGER, attotime::from_usec(5));

	return state->m_ds3_gdata;
}


WRITE16_HANDLER( hd68k_ds3_gdata_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	logerror("%06X:hd68k_ds3_gdata_w(%04X)\n", space->device().safe_pcbase(), state->m_ds3_gdata);

	COMBINE_DATA(&state->m_ds3_g68data);
	state->m_ds3_g68flag = 1;
	state->m_ds3_gcmd = offset & 1;
	state->m_adsp->signal_interrupt_trigger();
	update_ds3_irq(state);
}



/*************************************
 *
 *  DS III sound I/O
 *
 *************************************/

READ16_HANDLER( hd68k_ds3_sirq_state_r )
{
	return 0x4000;
}


READ16_HANDLER( hd68k_ds3_sdata_r )
{
	return 0;
}


WRITE16_HANDLER( hd68k_ds3_sdata_w )
{
}


/*************************************
 *
 *  DS III internal I/O
 *
 *************************************/

READ16_HANDLER( hdds3_special_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int result;

	switch (offset & 7)
	{
		case 0:
			state->m_ds3_g68flag = 0;
			update_ds3_irq(state);
			return state->m_ds3_g68data;

		case 1:
			result = 0x0fff;
			if (state->m_ds3_gcmd) result ^= 0x8000;
			if (state->m_ds3_g68flag) result ^= 0x4000;
			if (state->m_ds3_gflag) result ^= 0x2000;
			return result;

		case 6:
			logerror("ADSP r @ %04x\n", state->m_ds3_sim_address);
			if (state->m_ds3_sim_address < state->m_sim_memory_size)
				return state->m_sim_memory[state->m_ds3_sim_address];
			else
				return 0xff;
	}
	return 0;
}


WRITE16_HANDLER( hdds3_special_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	/* IMPORTANT! these data values also write through to the underlying RAM */
	state->m_adsp_data_memory[offset] = data;

	switch (offset & 7)
	{
		case 0:
			logerror("%04X:ADSP sets gdata to %04X\n", space->device().safe_pcbase(), data);
			state->m_ds3_gdata = data;
			state->m_ds3_gflag = 1;
			update_ds3_irq(state);

			/* once we've written data, trigger the main CPU to wake up again */
			space->machine().scheduler().trigger(DS3_TRIGGER);
			break;

		case 1:
			logerror("%04X:ADSP sets interrupt = %d\n", space->device().safe_pcbase(), (data >> 1) & 1);
			state->m_adsp_irq_state = (data >> 1) & 1;
			hd68k_update_interrupts(space->machine());
			break;

		case 2:
			state->m_ds3_send = (data >> 0) & 1;
			break;

		case 3:
			state->m_ds3_gfirqs = (data >> 1) & 1;
			state->m_ds3_g68irqs = !state->m_ds3_gfirqs;
			update_ds3_irq(state);
			break;

		case 4:
			state->m_ds3_sim_address = (state->m_ds3_sim_address & 0xffff0000) | (data & 0xffff);
			break;

		case 5:
			state->m_ds3_sim_address = (state->m_ds3_sim_address & 0xffff) | ((data << 16) & 0x00070000);
			break;
	}
}


READ16_HANDLER( hdds3_control_r )
{
	logerror("adsp2101 control r @ %04X\n", 0x3fe0 + offset);
	return 0;
}


WRITE16_HANDLER( hdds3_control_w )
{
	if (offset != 0x1e && offset != 0x1f)
		logerror("adsp2101 control w @ %04X = %04X\n", 0x3fe0 + offset, data);
}



/*************************************
 *
 *  DS III program memory handlers
 *
 *************************************/

READ16_HANDLER( hd68k_ds3_program_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT32 *base = &state->m_adsp_pgm_memory[offset & 0x1fff];
	UINT32 word = *base;
	return (!(offset & 0x2000)) ? (word >> 8) : (word & 0xff);
}


WRITE16_HANDLER( hd68k_ds3_program_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT32 *base = &state->m_adsp_pgm_memory[offset & 0x1fff];
	UINT32 oldword = *base;
	UINT16 temp;

	if (!(offset & 0x2000))
	{
		temp = oldword >> 8;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0x000000ff) | (temp << 8);
	}
	else
	{
		temp = oldword & 0xff;
		COMBINE_DATA(&temp);
		oldword = (oldword & 0xffffff00) | (temp & 0xff);
	}
	*base = oldword;
}



#if 0
#pragma mark -
#pragma mark * DSK BOARD
#endif

/*************************************
 *
 *  DSK board IRQ generation
 *
 *************************************/

void hddsk_update_pif(dsp32c_device &device, UINT32 pins)
{
	atarigen_state *atarigen = device.machine().driver_data<atarigen_state>();
	atarigen->m_sound_int_state = ((pins & DSP32_OUTPUT_PIF) != 0);
	hd68k_update_interrupts(device.machine());
}



/*************************************
 *
 *  DSK board control handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_dsk_control_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 0:	/* DSPRESTN */
			state->m_dsp32->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 1:	/* DSPZN */
			state->m_dsp32->set_input_line(INPUT_LINE_HALT, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 2:	/* ZW1 */
			break;

		case 3:	/* ZW2 */
			break;

		case 4:	/* ASIC65 reset */
			asic65_reset(space->machine(), !val);
			break;

		case 7:	/* LED */
			break;

		default:
			logerror("hd68k_dsk_control_w(%d) = %d\n", offset & 7, val);
			break;
	}
}



/*************************************
 *
 *  DSK board RAM/ZRAM/ROM handlers
 *
 *************************************/

READ16_HANDLER( hd68k_dsk_ram_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_dsk_ram[offset];
}


WRITE16_HANDLER( hd68k_dsk_ram_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	COMBINE_DATA(&state->m_dsk_ram[offset]);
}


READ16_HANDLER( hd68k_dsk_zram_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_dsk_zram[offset];
}


WRITE16_HANDLER( hd68k_dsk_zram_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	COMBINE_DATA(&state->m_dsk_zram[offset]);
}


READ16_HANDLER( hd68k_dsk_small_rom_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_dsk_rom[offset & 0x1ffff];
}


READ16_HANDLER( hd68k_dsk_rom_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_dsk_rom[offset];
}



/*************************************
 *
 *  DSK board DSP32C I/O handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_dsk_dsp32_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	state->m_dsk_pio_access = TRUE;
	state->m_dsp32->pio_w(offset, data);
	state->m_dsk_pio_access = FALSE;
}


READ16_HANDLER( hd68k_dsk_dsp32_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT16 result;
	state->m_dsk_pio_access = TRUE;
	result = state->m_dsp32->pio_r(offset);
	state->m_dsk_pio_access = FALSE;
	return result;
}


/*************************************
 *
 *  DSP32C synchronization
 *
 *************************************/

static TIMER_CALLBACK( rddsp32_sync_cb )
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	*state->m_dataptr[param] = state->m_dataval[param];
}


WRITE32_HANDLER( rddsp32_sync0_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	if (state->m_dsk_pio_access)
	{
		UINT32 *dptr = &state->m_rddsp32_sync[0][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		state->m_dataptr[state->m_next_msp_sync % MAX_MSP_SYNC] = dptr;
		state->m_dataval[state->m_next_msp_sync % MAX_MSP_SYNC] = newdata;
		space->machine().scheduler().synchronize(FUNC(rddsp32_sync_cb), state->m_next_msp_sync++ % MAX_MSP_SYNC);
	}
	else
		COMBINE_DATA(&state->m_rddsp32_sync[0][offset]);
}


WRITE32_HANDLER( rddsp32_sync1_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	if (state->m_dsk_pio_access)
	{
		UINT32 *dptr = &state->m_rddsp32_sync[1][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		state->m_dataptr[state->m_next_msp_sync % MAX_MSP_SYNC] = dptr;
		state->m_dataval[state->m_next_msp_sync % MAX_MSP_SYNC] = newdata;
		space->machine().scheduler().synchronize(FUNC(rddsp32_sync_cb), state->m_next_msp_sync++ % MAX_MSP_SYNC);
	}
	else
		COMBINE_DATA(&state->m_rddsp32_sync[1][offset]);
}



#if 0
#pragma mark -
#pragma mark * DSPCOM BOARD
#endif

/*************************************
 *
 *  DSPCOM control handlers
 *
 *************************************/

WRITE16_HANDLER( hddspcom_control_w )
{
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 2:	/* ASIC65 reset */
			asic65_reset(space->machine(), !val);
			break;

		default:
			logerror("hddspcom_control_w(%d) = %d\n", offset & 7, val);
			break;
	}
}



#if 0
#pragma mark -
#pragma mark * GAME-SPECIFIC PROTECTION
#endif

/*************************************
 *
 *  Race Drivin' slapstic handling
 *
 *************************************/

WRITE16_HANDLER( rd68k_slapstic_w )
{
	slapstic_tweak(space, offset & 0x3fff);
}


READ16_HANDLER( rd68k_slapstic_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int bank = slapstic_tweak(space, offset & 0x3fff) * 0x4000;
	return state->m_m68k_slapstic_base[bank + (offset & 0x3fff)];
}



/*************************************
 *
 *  Steel Talons SLOOP handling
 *
 *************************************/

static int st68k_sloop_tweak(harddriv_state *state, offs_t offset)
{
	static int last_offset;

	if (last_offset == 0)
	{
		switch (offset)
		{
			case 0x78e8:
				state->m_st68k_sloop_bank = 0;
				break;
			case 0x6ca4:
				state->m_st68k_sloop_bank = 1;
				break;
			case 0x15ea:
				state->m_st68k_sloop_bank = 2;
				break;
			case 0x6b28:
				state->m_st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return state->m_st68k_sloop_bank;
}


WRITE16_HANDLER( st68k_sloop_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	st68k_sloop_tweak(state, offset & 0x3fff);
}


READ16_HANDLER( st68k_sloop_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int bank = st68k_sloop_tweak(state, offset) * 0x4000;
	return state->m_m68k_slapstic_base[bank + (offset & 0x3fff)];
}


READ16_HANDLER( st68k_sloop_alt_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	if (state->m_st68k_last_alt_sloop_offset == 0x00fe)
	{
		switch (offset*2)
		{
			case 0x22c:
				state->m_st68k_sloop_bank = 0;
				break;
			case 0x1e2:
				state->m_st68k_sloop_bank = 1;
				break;
			case 0x1fa:
				state->m_st68k_sloop_bank = 2;
				break;
			case 0x206:
				state->m_st68k_sloop_bank = 3;
				break;
		}
	}
	state->m_st68k_last_alt_sloop_offset = offset*2;
	return state->m_m68k_sloop_alt_base[offset];
}


static int st68k_protosloop_tweak(harddriv_state *state, offs_t offset)
{
	static int last_offset;

	if (last_offset == 0)
	{
		switch (offset)
		{
			case 0x0001:
				state->m_st68k_sloop_bank = 0;
				break;
			case 0x0002:
				state->m_st68k_sloop_bank = 1;
				break;
			case 0x0003:
				state->m_st68k_sloop_bank = 2;
				break;
			case 0x0004:
				state->m_st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return state->m_st68k_sloop_bank;
}


WRITE16_HANDLER( st68k_protosloop_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	st68k_protosloop_tweak(state, offset & 0x3fff);
}


READ16_HANDLER( st68k_protosloop_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int bank = st68k_protosloop_tweak(state, offset) * 0x4000;
	return state->m_m68k_slapstic_base[bank + (offset & 0x3fff)];
}



#if 0
#pragma mark -
#pragma mark * GSP OPTIMIZATIONS
#endif

/*************************************
 *
 *  GSP Optimizations - case 1
 *  Works for:
 *      Hard Drivin'
 *      STUN Runner
 *
 *************************************/

READ16_HANDLER( hdgsp_speedup_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int result = state->m_gsp_speedup_addr[0][offset];

	/* if both this address and the other important address are not $ffff */
	/* then we can spin until something gets written */
	if (result != 0xffff && state->m_gsp_speedup_addr[1][0] != 0xffff &&
		&space->device() == state->m_gsp && space->device().safe_pc() == state->m_gsp_speedup_pc)
	{
		state->m_gsp_speedup_count[0]++;
		space->device().execute().spin_until_interrupt();
	}

	return result;
}


WRITE16_HANDLER( hdgsp_speedup1_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	COMBINE_DATA(&state->m_gsp_speedup_addr[0][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (state->m_gsp_speedup_addr[0][offset] == 0xffff)
		state->m_gsp->signal_interrupt_trigger();
}


WRITE16_HANDLER( hdgsp_speedup2_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	COMBINE_DATA(&state->m_gsp_speedup_addr[1][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (state->m_gsp_speedup_addr[1][offset] == 0xffff)
		state->m_gsp->signal_interrupt_trigger();
}



/*************************************
 *
 *  GSP Optimizations - case 2
 *  Works for:
 *      Race Drivin'
 *
 *************************************/

READ16_HANDLER( rdgsp_speedup1_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int result = state->m_gsp_speedup_addr[0][offset];

	/* if this address is equal to $f000, spin until something gets written */
	if (&space->device() == state->m_gsp && space->device().safe_pc() == state->m_gsp_speedup_pc &&
		(result & 0xff) < space->device().state().state_int(TMS34010_A1))
	{
		state->m_gsp_speedup_count[0]++;
		space->device().execute().spin_until_interrupt();
	}

	return result;
}


WRITE16_HANDLER( rdgsp_speedup1_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	COMBINE_DATA(&state->m_gsp_speedup_addr[0][offset]);
	if (&space->device() != state->m_gsp)
		state->m_gsp->signal_interrupt_trigger();
}



#if 0
#pragma mark -
#pragma mark * MSP OPTIMIZATIONS
#endif

/*************************************
 *
 *  MSP Optimizations
 *
 *************************************/

READ16_HANDLER( hdmsp_speedup_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int data = state->m_msp_speedup_addr[offset];

	if (data == 0 && &space->device() == state->m_msp && space->device().safe_pc() == state->m_msp_speedup_pc)
	{
		state->m_msp_speedup_count[0]++;
		space->device().execute().spin_until_interrupt();
	}

	return data;
}


WRITE16_HANDLER( hdmsp_speedup_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	COMBINE_DATA(&state->m_msp_speedup_addr[offset]);
	if (offset == 0 && state->m_msp_speedup_addr[offset] != 0)
		state->m_msp->signal_interrupt_trigger();
}


#if 0
#pragma mark -
#pragma mark * ADSP OPTIMIZATIONS
#endif

/*************************************
 *
 *  ADSP Optimizations
 *
 *************************************/

READ16_HANDLER( hdadsp_speedup_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int data = state->m_adsp_data_memory[0x1fff];

	if (data == 0xffff && &space->device() == state->m_adsp && space->device().safe_pc() <= 0x3b)
	{
		state->m_adsp_speedup_count[0]++;
		space->device().execute().spin_until_interrupt();
	}

	return data;
}


READ16_HANDLER( hdds3_speedup_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	int data = *state->m_ds3_speedup_addr;

	if (data != 0 && &space->device() == state->m_adsp && space->device().safe_pc() == state->m_ds3_speedup_pc)
	{
		state->m_adsp_speedup_count[2]++;
		space->device().execute().spin_until_interrupt();
	}

	return data;
}
