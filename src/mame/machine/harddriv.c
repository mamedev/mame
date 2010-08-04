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

#define DUART_CLOCK			(36864000)
#define DS3_TRIGGER			7777

/* debugging tools */
#define LOG_COMMANDS		0



/*************************************
 *
 *  Static globals
 *
 *************************************/


static void hd68k_update_interrupts(running_machine *machine);



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
	harddriv_state *state = machine->driver_data<harddriv_state>();

	atarigen_init(machine);

	/* predetermine memory regions */
	state->sim_memory = (UINT16 *)memory_region(machine, "user1");
	state->sim_memory_size = memory_region_length(machine, "user1") / 2;
	state->adsp_pgm_memory_word = (UINT16 *)((UINT8 *)state->adsp_pgm_memory + 1);
}


MACHINE_RESET( harddriv )
{
	harddriv_state *state = machine->driver_data<harddriv_state>();

	/* generic reset */
	atarigen_eeprom_reset(state);
	slapstic_reset();
	atarigen_interrupt_reset(state, hd68k_update_interrupts);

	/* halt several of the DSPs to start */
	if (state->adsp != NULL) cpu_set_input_line(state->adsp, INPUT_LINE_HALT, ASSERT_LINE);
	if (state->dsp32 != NULL) cpu_set_input_line(state->dsp32, INPUT_LINE_HALT, ASSERT_LINE);
	if (state->sounddsp != NULL) cpu_set_input_line(state->sounddsp, INPUT_LINE_HALT, ASSERT_LINE);

	/* if we found a 6502, reset the JSA board */
	if (state->jsacpu != NULL)
		atarijsa_reset();

	state->last_gsp_shiftreg = 0;

	state->m68k_adsp_buffer_bank = 0;

	/* reset IRQ states */
	state->irq_state = state->gsp_irq_state = state->msp_irq_state = state->adsp_irq_state = state->duart_irq_state = 0;

	/* reset the DUART */
	memset(state->duart_read_data, 0, sizeof(state->duart_read_data));
	memset(state->duart_write_data, 0, sizeof(state->duart_write_data));
	state->duart_output_port = 0;

	/* reset the ADSP/DSIII/DSIV boards */
	state->adsp_halt = 1;
	state->adsp_br = 0;
	state->adsp_xflag = 0;
}



/*************************************
 *
 *  68000 interrupt handling
 *
 *************************************/

static void hd68k_update_interrupts(running_machine *machine)
{
	harddriv_state *state = machine->driver_data<harddriv_state>();
	cpu_set_input_line(state->maincpu, 1, state->msp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(state->maincpu, 2, state->adsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(state->maincpu, 3, state->gsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(state->maincpu, 4, state->sound_int_state ? ASSERT_LINE : CLEAR_LINE);	/* /LINKIRQ on STUN Runner */
	cpu_set_input_line(state->maincpu, 5, state->irq_state ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(state->maincpu, 6, state->duart_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN( hd68k_irq_gen )
{
	harddriv_state *state = device->machine->driver_data<harddriv_state>();
	state->irq_state = 1;
	atarigen_update_interrupts(device->machine);
}


WRITE16_HANDLER( hd68k_irq_ack_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	state->irq_state = 0;
	atarigen_update_interrupts(space->machine);
}


void hdgsp_irq_gen(running_device *device, int irqstate)
{
	harddriv_state *state = device->machine->driver_data<harddriv_state>();
	state->gsp_irq_state = irqstate;
	atarigen_update_interrupts(device->machine);
}


void hdmsp_irq_gen(running_device *device, int irqstate)
{
	harddriv_state *state = device->machine->driver_data<harddriv_state>();
	state->msp_irq_state = irqstate;
	atarigen_update_interrupts(device->machine);
}



/*************************************
 *
 *  68000 access to GSP
 *
 *************************************/

READ16_HANDLER( hd68k_gsp_io_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT16 result;
	offset = (offset / 2) ^ 1;
	state->hd34010_host_access = TRUE;
	result = tms34010_host_r(state->gsp, offset);
	state->hd34010_host_access = FALSE;
	return result;
}


WRITE16_HANDLER( hd68k_gsp_io_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	offset = (offset / 2) ^ 1;
	state->hd34010_host_access = TRUE;
	tms34010_host_w(state->gsp, offset, data);
	state->hd34010_host_access = FALSE;
}



/*************************************
 *
 *  68000 access to MSP
 *
 *************************************/

READ16_HANDLER( hd68k_msp_io_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT16 result;
	offset = (offset / 2) ^ 1;
	state->hd34010_host_access = TRUE;
	result = (state->msp != NULL) ? tms34010_host_r(state->msp, offset) : 0xffff;
	state->hd34010_host_access = FALSE;
	return result;
}


WRITE16_HANDLER( hd68k_msp_io_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	offset = (offset / 2) ^ 1;
	if (state->msp != NULL)
	{
		state->hd34010_host_access = TRUE;
		tms34010_host_w(state->msp, offset, data);
		state->hd34010_host_access = FALSE;
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
	int temp = input_port_read(space->machine, "IN0");
	if (atarigen_get_hblank(*space->machine->primary_screen)) temp ^= 0x0002;
	temp ^= 0x0018;		/* both EOCs always high for now */
	return temp;
}


READ16_HANDLER( hdc68k_port1_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT16 result = input_port_read(space->machine, "a80000");
	UINT16 diff = result ^ state->hdc68k_last_port1;

	/* if a new shifter position is selected, use it */
	/* if it's the same shifter position as last time, go back to neutral */
	if ((diff & 0x0100) && !(result & 0x0100))
		state->hdc68k_shifter_state = (state->hdc68k_shifter_state == 1) ? 0 : 1;
	if ((diff & 0x0200) && !(result & 0x0200))
		state->hdc68k_shifter_state = (state->hdc68k_shifter_state == 2) ? 0 : 2;
	if ((diff & 0x0400) && !(result & 0x0400))
		state->hdc68k_shifter_state = (state->hdc68k_shifter_state == 4) ? 0 : 4;
	if ((diff & 0x0800) && !(result & 0x0800))
		state->hdc68k_shifter_state = (state->hdc68k_shifter_state == 8) ? 0 : 8;

	/* merge in the new shifter value */
	result = (result | 0x0f00) ^ (state->hdc68k_shifter_state << 8);

	/* merge in the wheel edge latch bit */
	if (state->hdc68k_wheel_edge)
		result ^= 0x4000;

	state->hdc68k_last_port1 = result;
	return result;
}


READ16_HANDLER( hda68k_port1_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT16 result = input_port_read(space->machine, "a80000");

	/* merge in the wheel edge latch bit */
	if (state->hdc68k_wheel_edge)
		result ^= 0x4000;

	return result;
}


READ16_HANDLER( hdc68k_wheel_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	/* grab the new wheel value and upconvert to 12 bits */
	UINT16 new_wheel = input_port_read(space->machine, "12BADC0") << 4;

	/* hack to display the wheel position */
	if (input_code_pressed(space->machine, KEYCODE_LSHIFT))
		popmessage("%04X", new_wheel);

	/* if we crossed the center line, latch the edge bit */
	if ((state->hdc68k_last_wheel / 0xf0) != (new_wheel / 0xf0))
		state->hdc68k_wheel_edge = 1;

	/* remember the last value and return the low 8 bits */
	state->hdc68k_last_wheel = new_wheel;
	return (new_wheel << 8) | 0xff;
}


READ16_HANDLER( hd68k_adc8_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->adc8_data;
}


READ16_HANDLER( hd68k_adc12_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->adc12_byte ? ((state->adc12_data >> 8) & 0x0f) : (state->adc12_data & 0xff);
}


READ16_HANDLER( hd68k_sound_reset_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	if (state->jsacpu != NULL)
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	COMBINE_DATA(&state->adc_control);

	/* handle a write to the 8-bit ADC address select */
	if (state->adc_control & 0x08)
	{
		state->adc8_select = state->adc_control & 0x07;
		state->adc8_data = input_port_read(space->machine, adc8names[state->adc8_select]);
	}

	/* handle a write to the 12-bit ADC address select */
	if (state->adc_control & 0x40)
	{
		state->adc12_select = (state->adc_control >> 4) & 0x03;
		state->adc12_data = input_port_read(space->machine, adc12names[state->adc12_select]) << 4;
	}

	/* bit 7 selects which byte of the 12 bit data to read */
	state->adc12_byte = (state->adc_control >> 7) & 1;
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
			coin_counter_w(space->machine, offset - 6, data);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:	/* CR2 */
		case 1:	/* CR1 */
			set_led_status(space->machine, offset, data);
			break;
		case 2:	/* LC1 */
			break;
		case 3:	/* LC2 */
			break;
		case 4:	/* ZP1 */
			state->m68k_zp1 = data;
			break;
		case 5:	/* ZP2 */
			state->m68k_zp2 = data;
			break;
		case 6:	/* /GSPRES */
			logerror("Write to /GSPRES(%d)\n", data);
			if (state->gsp != NULL)
				cpu_set_input_line(state->gsp, INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
		case 7:	/* /MSPRES */
			logerror("Write to /MSPRES(%d)\n", data);
			if (state->msp != NULL)
				cpu_set_input_line(state->msp, INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


WRITE16_HANDLER( hdc68k_wheel_edge_reset_w )
{
	/* reset the edge latch */
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	state->hdc68k_wheel_edge = 0;
}



/*************************************
 *
 *  68000 ZRAM access
 *
 *************************************/

READ16_HANDLER( hd68k_zram_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->eeprom[offset];
}


WRITE16_HANDLER( hd68k_zram_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	if (state->m68k_zp1 == 0 && state->m68k_zp2 == 1)
		COMBINE_DATA(&state->eeprom[offset]);
}



/*************************************
 *
 *  68000 DUART interface
 *
 *************************************/

/*
                                    DUART registers

            Read                                Write
            ----------------------------------  -------------------------------------------
    0x00 =  Mode Register A (MR1A, MR2A)        Mode Register A (MR1A, MR2A)
    0x02 =  Status Register A (SRA)             Clock-Select Register A (CSRA)
    0x04 =  Clock-Select Register A 1 (CSRA)    Command Register A (CRA)
    0x06 =  Receiver Buffer A (RBA)             Transmitter Buffer A (TBA)
    0x08 =  Input Port Change Register (IPCR)   Auxiliary Control Register (ACR)
    0x0a =  Interrupt Status Register (ISR)     Interrupt Mask Register (IMR)
    0x0c =  Counter Mode: Current MSB of        Counter/Timer Upper Register (CTUR)
                    Counter (CUR)
    0x0e =  Counter Mode: Current LSB of        Counter/Timer Lower Register (CTLR)
                    Counter (CLR)
    0x10 = Mode Register B (MR1B, MR2B)         Mode Register B (MR1B, MR2B)
    0x12 = Status Register B (SRB)              Clock-Select Register B (CSRB)
    0x14 = Clock-Select Register B 2 (CSRB)     Command Register B (CRB)
    0x16 = Receiver Buffer B (RBB)              Transmitter Buffer B (TBB)
    0x18 = Interrupt-Vector Register (IVR)      Interrupt-Vector Register (IVR)
    0x1a = Input Port (IP)                      Output Port Configuration Register (OPCR)
    0x1c = Start-Counter Command 3              Output Port Register (OPR): Bit Set Command 3
    0x1e = Stop-Counter Command 3               Output Port Register (OPR): Bit Reset Command 3
*/


INLINE int duart_clock(harddriv_state *state)
{
	int mode = (state->duart_write_data[0x04] >> 4) & 7;
	if (mode != 3)
		logerror("DUART: unsupported clock mode %d\n", mode);
	return DUART_CLOCK / 16;
}


INLINE attotime duart_clock_period(harddriv_state *state)
{
	return ATTOTIME_IN_HZ(duart_clock(state));
}


TIMER_DEVICE_CALLBACK( hd68k_duart_callback )
{
	harddriv_state *state = timer.machine->driver_data<harddriv_state>();
	logerror("DUART timer fired\n");
	if (state->duart_write_data[0x05] & 0x08)
	{
		logerror("DUART interrupt generated\n");
		state->duart_read_data[0x05] |= 0x08;
		state->duart_irq_state = (state->duart_read_data[0x05] & state->duart_write_data[0x05]) != 0;
		atarigen_update_interrupts(timer.machine);
	}
	timer.adjust(attotime_mul(duart_clock_period(state), 65536));
}


READ16_HANDLER( hd68k_duart_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	switch (offset)
	{
		case 0x00:		/* Mode Register A (MR1A, MR2A) */
		case 0x08:		/* Mode Register B (MR1B, MR2B) */
			return (state->duart_write_data[0x00] << 8) | 0x00ff;
		case 0x01:		/* Status Register A (SRA) */
		case 0x02:		/* Clock-Select Register A 1 (CSRA) */
		case 0x03:		/* Receiver Buffer A (RBA) */
		case 0x04:		/* Input Port Change Register (IPCR) */
		case 0x05:		/* Interrupt Status Register (ISR) */
		case 0x06:		/* Counter Mode: Current MSB of Counter (CUR) */
		case 0x07:		/* Counter Mode: Current LSB of Counter (CLR) */
		case 0x09:		/* Status Register B (SRB) */
		case 0x0a:		/* Clock-Select Register B 2 (CSRB) */
		case 0x0b:		/* Receiver Buffer B (RBB) */
		case 0x0c:		/* Interrupt-Vector Register (IVR) */
		case 0x0d:		/* Input Port (IP) */
			return (state->duart_read_data[offset] << 8) | 0x00ff;
		case 0x0e:		/* Start-Counter Command 3 */
		{
			int reps = (state->duart_write_data[0x06] << 8) | state->duart_write_data[0x07];
			state->duart_timer->adjust(attotime_mul(duart_clock_period(state), reps));
			logerror("DUART timer started (period=%f)\n", attotime_to_double(attotime_mul(duart_clock_period(state), reps)));
			return 0x00ff;
		}
		case 0x0f:		/* Stop-Counter Command 3 */
			{
				int reps = attotime_to_double(attotime_mul(state->duart_timer->time_left(), duart_clock(state)));
				state->duart_timer->reset();
				state->duart_read_data[0x06] = reps >> 8;
				state->duart_read_data[0x07] = reps & 0xff;
				logerror("DUART timer stopped (final count=%04X)\n", reps);
			}
			state->duart_read_data[0x05] &= ~0x08;
			state->duart_irq_state = (state->duart_read_data[0x05] & state->duart_write_data[0x05]) != 0;
			atarigen_update_interrupts(space->machine);
			return 0x00ff;
	}
	return 0x00ff;
}


WRITE16_HANDLER( hd68k_duart_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	if (ACCESSING_BITS_8_15)
	{
		int newdata = (data >> 8) & 0xff;
		state->duart_write_data[offset] = newdata;

		switch (offset)
		{
			case 0x00:		/* Mode Register A (MR1A, MR2A) */
			case 0x01:		/* Clock-Select Register A (CSRA) */
			case 0x02:		/* Command Register A (CRA) */
			case 0x03:		/* Transmitter Buffer A (TBA) */
			case 0x04:		/* Auxiliary Control Register (ACR) */
			case 0x05:		/* Interrupt Mask Register (IMR) */
			case 0x06:		/* Counter/Timer Upper Register (CTUR) */
			case 0x07:		/* Counter/Timer Lower Register (CTLR) */
			case 0x08:		/* Mode Register B (MR1B, MR2B) */
			case 0x09:		/* Clock-Select Register B (CSRB) */
			case 0x0a:		/* Command Register B (CRB) */
			case 0x0b:		/* Transmitter Buffer B (TBB) */
			case 0x0c:		/* Interrupt-Vector Register (IVR) */
			case 0x0d:		/* Output Port Configuration Register (OPCR) */
				break;
			case 0x0e:		/* Output Port Register (OPR): Bit Set Command 3 */
				state->duart_output_port |= newdata;
				break;
			case 0x0f:		/* Output Port Register (OPR): Bit Reset Command 3 */
				state->duart_output_port &= ~newdata;
				break;
		}
		logerror("DUART write %02X @ %02X\n", (data >> 8) & 0xff, offset);
	}
	else
		logerror("Unexpected DUART write %02X @ %02X\n", data, offset);
}



/*************************************
 *
 *  GSP I/O register writes
 *
 *************************************/

WRITE16_HANDLER( hdgsp_io_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	/* detect an enabling of the shift register and force yielding */
	if (offset == REG_DPYCTL)
	{
		UINT8 new_shiftreg = (data >> 11) & 1;
		if (new_shiftreg != state->last_gsp_shiftreg)
		{
			state->last_gsp_shiftreg = new_shiftreg;
			if (new_shiftreg)
				cpu_yield(space->cpu);
		}
	}

	/* detect changes to HEBLNK and HSBLNK and force an update before they change */
	if ((offset == REG_HEBLNK || offset == REG_HSBLNK) && data != tms34010_io_register_r(space, offset, 0xffff))
		space->machine->primary_screen->update_partial(space->machine->primary_screen->vpos() - 1);

	tms34010_io_register_w(space, offset, data, mem_mask);
}



/*************************************
 *
 *  GSP protection workarounds
 *
 *************************************/

WRITE16_HANDLER( hdgsp_protection_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	/* this memory address is incremented whenever a protection check fails */
	/* after it reaches a certain value, the GSP will randomly trash a */
	/* register; we just prevent it from ever going above 0 */
	*state->gsp_protection = 0;
}



/*************************************
 *
 *  MSP synchronization helpers
 *
 *************************************/

static TIMER_CALLBACK( stmsp_sync_update )
{
	harddriv_state *state = machine->driver_data<harddriv_state>();
	int which = param >> 28;
	offs_t offset = (param >> 16) & 0xfff;
	UINT16 data = param;
	state->stmsp_sync[which][offset] = data;
	cpu_triggerint(state->msp);
}


INLINE void stmsp_sync_w(const address_space *space, offs_t offset, UINT16 data, UINT16 mem_mask, int which)
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT16 newdata = state->stmsp_sync[which][offset];
	COMBINE_DATA(&newdata);

	/* if being written from the 68000, synchronize on it */
	if (state->hd34010_host_access)
		timer_call_after_resynch(space->machine, NULL, newdata | (offset << 16) | (which << 28), stmsp_sync_update);

	/* otherwise, just update */
	else
		state->stmsp_sync[which][offset] = newdata;
}


WRITE16_HANDLER( stmsp_sync0_w )
{
	stmsp_sync_w(space, offset, data, mem_mask, 0);
}


WRITE16_HANDLER( stmsp_sync1_w )
{
	stmsp_sync_w(space, offset, data, mem_mask, 1);
}


WRITE16_HANDLER( stmsp_sync2_w )
{
	stmsp_sync_w(space, offset, data, mem_mask, 2);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT32 word = state->adsp_pgm_memory[offset/2];
	return (!(offset & 1)) ? (word >> 16) : (word & 0xffff);
}


WRITE16_HANDLER( hd68k_adsp_program_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT32 *base = &state->adsp_pgm_memory[offset/2];
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->adsp_data_memory[offset];
}


WRITE16_HANDLER( hd68k_adsp_data_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	COMBINE_DATA(&state->adsp_data_memory[offset]);

	/* any write to $1FFF is taken to be a trigger; synchronize the CPUs */
	if (offset == 0x1fff)
	{
		logerror("%06X:ADSP sync address written (%04X)\n", cpu_get_previouspc(space->cpu), data);
		timer_call_after_resynch(space->machine, NULL, 0, 0);
		cpu_triggerint(state->adsp);
	}
	else
		logerror("%06X:ADSP W@%04X (%04X)\n", cpu_get_previouspc(space->cpu), offset, data);
}



/*************************************
 *
 *  68000 access to ADSP output memory
 *
 *************************************/

READ16_HANDLER( hd68k_adsp_buffer_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
/*  logerror("hd68k_adsp_buffer_r(%04X)\n", offset);*/
	return state->som_memory[state->m68k_adsp_buffer_bank * 0x2000 + offset];
}


WRITE16_HANDLER( hd68k_adsp_buffer_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	COMBINE_DATA(&state->som_memory[state->m68k_adsp_buffer_bank * 0x2000 + offset]);
}



/*************************************
 *
 *  68000 access to ADSP control regs
 *
 *************************************/

static TIMER_CALLBACK( deferred_adsp_bank_switch )
{
	harddriv_state *state = machine->driver_data<harddriv_state>();
	if (LOG_COMMANDS && state->m68k_adsp_buffer_bank != param && input_code_pressed(machine, KEYCODE_L))
	{
		static FILE *commands;
		if (!commands) commands = fopen("commands.log", "w");
		if (commands)
		{
			INT16 *base = (INT16 *)&state->som_memory[param * 0x2000];
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

	state->m68k_adsp_buffer_bank = param;
	logerror("ADSP bank = %d\n", param);
}


WRITE16_HANDLER( hd68k_adsp_control_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

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
			timer_call_after_resynch(space->machine, NULL, val, deferred_adsp_bank_switch);
			break;

		case 5:
			/* connected to the /BR (bus request) line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			state->adsp_br = !val;
			logerror("ADSP /BR = %d\n", !state->adsp_br);
			if (state->adsp_br || state->adsp_halt)
				cpu_set_input_line(state->adsp, INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				cpu_set_input_line(state->adsp, INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				cpu_spin(space->cpu);
			}
			break;

		case 6:
			/* connected to the /HALT line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			state->adsp_halt = !val;
			logerror("ADSP /HALT = %d\n", !state->adsp_halt);
			if (state->adsp_br || state->adsp_halt)
				cpu_set_input_line(state->adsp, INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				cpu_set_input_line(state->adsp, INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				cpu_spin(space->cpu);
			}
			break;

		case 7:
			logerror("ADSP reset = %d\n", val);
			cpu_set_input_line(state->adsp, INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			cpu_yield(space->cpu);
			break;

		default:
			logerror("ADSP control %02X = %04X\n", offset, data);
			break;
	}
}


WRITE16_HANDLER( hd68k_adsp_irq_clear_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	logerror("%06X:68k clears ADSP interrupt\n", cpu_get_previouspc(space->cpu));
	state->adsp_irq_state = 0;
	atarigen_update_interrupts(space->machine);
}


READ16_HANDLER( hd68k_adsp_irq_state_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int result = 0xfffd;
	if (state->adsp_xflag) result ^= 2;
	if (state->adsp_irq_state) result ^= 1;
	logerror("%06X:68k reads ADSP interrupt state = %04x\n", cpu_get_previouspc(space->cpu), result);
	return result;
}



/*************************************
 *
 *  ADSP memory-mapped I/O
 *
 *************************************/

READ16_HANDLER( hdadsp_special_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	switch (offset & 7)
	{
		case 0:	/* /SIMBUF */
			if (state->adsp_eprom_base + state->adsp_sim_address < state->sim_memory_size)
				return state->sim_memory[state->adsp_eprom_base + state->adsp_sim_address++];
			else
				return 0xff;

		case 1:	/* /SIMLD */
			break;

		case 2:	/* /SOMO */
			break;

		case 3:	/* /SOMLD */
			break;

		default:
			logerror("%04X:hdadsp_special_r(%04X)\n", cpu_get_previouspc(space->cpu), offset);
			break;
	}
	return 0;
}


WRITE16_HANDLER( hdadsp_special_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	switch (offset & 7)
	{
		case 1:	/* /SIMCLK */
			state->adsp_sim_address = data;
			break;

		case 2:	/* SOMLATCH */
			state->som_memory[(state->m68k_adsp_buffer_bank ^ 1) * 0x2000 + (state->adsp_som_address++ & 0x1fff)] = data;
			break;

		case 3:	/* /SOMCLK */
			state->adsp_som_address = data;
			break;

		case 5:	/* /XOUT */
			state->adsp_xflag = data & 1;
			break;

		case 6:	/* /GINT */
			logerror("%04X:ADSP signals interrupt\n", cpu_get_previouspc(space->cpu));
			state->adsp_irq_state = 1;
			atarigen_update_interrupts(space->machine);
			break;

		case 7:	/* /MP */
			state->adsp_eprom_base = 0x10000 * data;
			break;

		default:
			logerror("%04X:hdadsp_special_w(%04X)=%04X\n", cpu_get_previouspc(space->cpu), offset, data);
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
	if (!(!state->ds3_g68flag && state->ds3_g68irqs) && !(state->ds3_gflag && state->ds3_gfirqs))
		cpu_set_input_line(state->adsp, ADSP2100_IRQ2, ASSERT_LINE);
	else
		cpu_set_input_line(state->adsp, ADSP2100_IRQ2, CLEAR_LINE);
}


WRITE16_HANDLER( hd68k_ds3_control_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
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
			state->adsp_br = !val;
			if (state->adsp_br)
				cpu_set_input_line(state->adsp, INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				cpu_set_input_line(state->adsp, INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				cpu_spin(space->cpu);
			}
			break;

		case 3:
			cpu_set_input_line(state->adsp, INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			if (val && !state->ds3_reset)
			{
				state->ds3_gflag = 0;
				state->ds3_gcmd = 0;
				state->ds3_gfirqs = 0;
				state->ds3_g68irqs = !state->ds3_gfirqs;
				state->ds3_send = 0;
				update_ds3_irq(state);
			}
			state->ds3_reset = val;
			cpu_yield(space->cpu);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int result = 0x0fff;
	if (state->ds3_g68flag) result ^= 0x8000;
	if (state->ds3_gflag) result ^= 0x4000;
	if (state->ds3_g68irqs) result ^= 0x2000;
	if (!state->adsp_irq_state) result ^= 0x1000;
	return result;
}


READ16_HANDLER( hd68k_ds3_gdata_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	offs_t pc = cpu_get_pc(space->cpu);

	state->ds3_gflag = 0;
	update_ds3_irq(state);

	logerror("%06X:hd68k_ds3_gdata_r(%04X)\n", cpu_get_previouspc(space->cpu), state->ds3_gdata);

	/* attempt to optimize the transfer if conditions are right */
	if (space->cpu == state->maincpu && pc == state->ds3_transfer_pc &&
		!(!state->ds3_g68flag && state->ds3_g68irqs) && !(state->ds3_gflag && state->ds3_gfirqs))
	{
		UINT32 destaddr = state->maincpu->state(M68K_A1);
		UINT16 count68k = state->maincpu->state(M68K_D1);
		UINT16 mstat = state->adsp->state(ADSP2100_MSTAT);
		UINT16 i6 = state->adsp->state((mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC);
		UINT16 l6 = state->adsp->state(ADSP2100_L6) - 1;
		UINT16 m7 = state->adsp->state(ADSP2100_M7);

		logerror("%06X:optimizing 68k transfer, %d words\n", state->maincpu->pcbase(), count68k);

		while (count68k > 0 && state->adsp_data_memory[0x16e6] > 0)
		{
			memory_write_word(space, destaddr, state->ds3_gdata);
			{
				state->adsp_data_memory[0x16e6]--;
				state->ds3_gdata = state->adsp_pgm_memory[i6] >> 8;
				i6 = (i6 & ~l6) | ((i6 + m7) & l6);
			}
			count68k--;
		}
		state->maincpu->set_state(M68K_D1, count68k);
		state->adsp->set_state((mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC, i6);
		state->adsp_speedup_count[1]++;
	}

	/* if we just cleared the IRQ, we are going to do some VERY timing critical reads */
	/* it is important that all the CPUs be in sync before we continue, so spin a little */
	/* while to let everyone else catch up */
	cpu_spinuntil_trigger(space->cpu, DS3_TRIGGER);
	cpuexec_triggertime(space->machine, DS3_TRIGGER, ATTOTIME_IN_USEC(5));

	return state->ds3_gdata;
}


WRITE16_HANDLER( hd68k_ds3_gdata_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	logerror("%06X:hd68k_ds3_gdata_w(%04X)\n", cpu_get_previouspc(space->cpu), state->ds3_gdata);

	COMBINE_DATA(&state->ds3_g68data);
	state->ds3_g68flag = 1;
	state->ds3_gcmd = offset & 1;
	cpu_triggerint(state->adsp);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int result;

	switch (offset & 7)
	{
		case 0:
			state->ds3_g68flag = 0;
			update_ds3_irq(state);
			return state->ds3_g68data;

		case 1:
			result = 0x0fff;
			if (state->ds3_gcmd) result ^= 0x8000;
			if (state->ds3_g68flag) result ^= 0x4000;
			if (state->ds3_gflag) result ^= 0x2000;
			return result;

		case 6:
			logerror("ADSP r @ %04x\n", state->ds3_sim_address);
			if (state->ds3_sim_address < state->sim_memory_size)
				return state->sim_memory[state->ds3_sim_address];
			else
				return 0xff;
	}
	return 0;
}


WRITE16_HANDLER( hdds3_special_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	/* IMPORTANT! these data values also write through to the underlying RAM */
	state->adsp_data_memory[offset] = data;

	switch (offset & 7)
	{
		case 0:
			logerror("%04X:ADSP sets gdata to %04X\n", cpu_get_previouspc(space->cpu), data);
			state->ds3_gdata = data;
			state->ds3_gflag = 1;
			update_ds3_irq(state);

			/* once we've written data, trigger the main CPU to wake up again */
			cpuexec_trigger(space->machine, DS3_TRIGGER);
			break;

		case 1:
			logerror("%04X:ADSP sets interrupt = %d\n", cpu_get_previouspc(space->cpu), (data >> 1) & 1);
			state->adsp_irq_state = (data >> 1) & 1;
			hd68k_update_interrupts(space->machine);
			break;

		case 2:
			state->ds3_send = (data >> 0) & 1;
			break;

		case 3:
			state->ds3_gfirqs = (data >> 1) & 1;
			state->ds3_g68irqs = !state->ds3_gfirqs;
			update_ds3_irq(state);
			break;

		case 4:
			state->ds3_sim_address = (state->ds3_sim_address & 0xffff0000) | (data & 0xffff);
			break;

		case 5:
			state->ds3_sim_address = (state->ds3_sim_address & 0xffff) | ((data << 16) & 0x00070000);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT32 *base = &state->adsp_pgm_memory[offset & 0x1fff];
	UINT32 word = *base;
	return (!(offset & 0x2000)) ? (word >> 8) : (word & 0xff);
}


WRITE16_HANDLER( hd68k_ds3_program_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT32 *base = &state->adsp_pgm_memory[offset & 0x1fff];
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

void hddsk_update_pif(running_device *device, UINT32 pins)
{
	atarigen_state *atarigen = device->machine->driver_data<atarigen_state>();
	atarigen->sound_int_state = ((pins & DSP32_OUTPUT_PIF) != 0);
	hd68k_update_interrupts(device->machine);
}



/*************************************
 *
 *  DSK board control handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_dsk_control_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 0:	/* DSPRESTN */
			cpu_set_input_line(state->dsp32, INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 1:	/* DSPZN */
			cpu_set_input_line(state->dsp32, INPUT_LINE_HALT, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 2:	/* ZW1 */
			break;

		case 3:	/* ZW2 */
			break;

		case 4:	/* ASIC65 reset */
			asic65_reset(space->machine, !val);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->dsk_ram[offset];
}


WRITE16_HANDLER( hd68k_dsk_ram_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	COMBINE_DATA(&state->dsk_ram[offset]);
}


READ16_HANDLER( hd68k_dsk_zram_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->dsk_zram[offset];
}


WRITE16_HANDLER( hd68k_dsk_zram_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	COMBINE_DATA(&state->dsk_zram[offset]);
}


READ16_HANDLER( hd68k_dsk_small_rom_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->dsk_rom[offset & 0x1ffff];
}


READ16_HANDLER( hd68k_dsk_rom_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	return state->dsk_rom[offset];
}



/*************************************
 *
 *  DSK board DSP32C I/O handlers
 *
 *************************************/

WRITE16_HANDLER( hd68k_dsk_dsp32_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	state->dsk_pio_access = TRUE;
	dsp32c_pio_w(state->dsp32, offset, data);
	state->dsk_pio_access = FALSE;
}


READ16_HANDLER( hd68k_dsk_dsp32_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	UINT16 result;
	state->dsk_pio_access = TRUE;
	result = dsp32c_pio_r(state->dsp32, offset);
	state->dsk_pio_access = FALSE;
	return result;
}


/*************************************
 *
 *  DSP32C synchronization
 *
 *************************************/

static TIMER_CALLBACK( rddsp32_sync_cb )
{
	harddriv_state *state = machine->driver_data<harddriv_state>();
	*state->dataptr[param] = state->dataval[param];
}


WRITE32_HANDLER( rddsp32_sync0_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	if (state->dsk_pio_access)
	{
		UINT32 *dptr = &state->rddsp32_sync[0][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		state->dataptr[state->next_msp_sync % MAX_MSP_SYNC] = dptr;
		state->dataval[state->next_msp_sync % MAX_MSP_SYNC] = newdata;
		timer_call_after_resynch(space->machine, NULL, state->next_msp_sync++ % MAX_MSP_SYNC, rddsp32_sync_cb);
	}
	else
		COMBINE_DATA(&state->rddsp32_sync[0][offset]);
}


WRITE32_HANDLER( rddsp32_sync1_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	if (state->dsk_pio_access)
	{
		UINT32 *dptr = &state->rddsp32_sync[1][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		state->dataptr[state->next_msp_sync % MAX_MSP_SYNC] = dptr;
		state->dataval[state->next_msp_sync % MAX_MSP_SYNC] = newdata;
		timer_call_after_resynch(space->machine, NULL, state->next_msp_sync++ % MAX_MSP_SYNC, rddsp32_sync_cb);
	}
	else
		COMBINE_DATA(&state->rddsp32_sync[1][offset]);
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
			asic65_reset(space->machine, !val);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int bank = slapstic_tweak(space, offset & 0x3fff) * 0x4000;
	return state->m68k_slapstic_base[bank + (offset & 0x3fff)];
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
				state->st68k_sloop_bank = 0;
				break;
			case 0x6ca4:
				state->st68k_sloop_bank = 1;
				break;
			case 0x15ea:
				state->st68k_sloop_bank = 2;
				break;
			case 0x6b28:
				state->st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return state->st68k_sloop_bank;
}


WRITE16_HANDLER( st68k_sloop_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	st68k_sloop_tweak(state, offset & 0x3fff);
}


READ16_HANDLER( st68k_sloop_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int bank = st68k_sloop_tweak(state, offset) * 0x4000;
	return state->m68k_slapstic_base[bank + (offset & 0x3fff)];
}


READ16_HANDLER( st68k_sloop_alt_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	if (state->st68k_last_alt_sloop_offset == 0x00fe)
	{
		switch (offset*2)
		{
			case 0x22c:
				state->st68k_sloop_bank = 0;
				break;
			case 0x1e2:
				state->st68k_sloop_bank = 1;
				break;
			case 0x1fa:
				state->st68k_sloop_bank = 2;
				break;
			case 0x206:
				state->st68k_sloop_bank = 3;
				break;
		}
	}
	state->st68k_last_alt_sloop_offset = offset*2;
	return state->m68k_sloop_alt_base[offset];
}


static int st68k_protosloop_tweak(harddriv_state *state, offs_t offset)
{
	static int last_offset;

	if (last_offset == 0)
	{
		switch (offset)
		{
			case 0x0001:
				state->st68k_sloop_bank = 0;
				break;
			case 0x0002:
				state->st68k_sloop_bank = 1;
				break;
			case 0x0003:
				state->st68k_sloop_bank = 2;
				break;
			case 0x0004:
				state->st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return state->st68k_sloop_bank;
}


WRITE16_HANDLER( st68k_protosloop_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	st68k_protosloop_tweak(state, offset & 0x3fff);
}


READ16_HANDLER( st68k_protosloop_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int bank = st68k_protosloop_tweak(state, offset) * 0x4000;
	return state->m68k_slapstic_base[bank + (offset & 0x3fff)];
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int result = state->gsp_speedup_addr[0][offset];

	/* if both this address and the other important address are not $ffff */
	/* then we can spin until something gets written */
	if (result != 0xffff && state->gsp_speedup_addr[1][0] != 0xffff &&
		space->cpu == state->gsp && cpu_get_pc(space->cpu) == state->gsp_speedup_pc)
	{
		state->gsp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return result;
}


WRITE16_HANDLER( hdgsp_speedup1_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	COMBINE_DATA(&state->gsp_speedup_addr[0][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (state->gsp_speedup_addr[0][offset] == 0xffff)
		cpu_triggerint(state->gsp);
}


WRITE16_HANDLER( hdgsp_speedup2_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	COMBINE_DATA(&state->gsp_speedup_addr[1][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (state->gsp_speedup_addr[1][offset] == 0xffff)
		cpu_triggerint(state->gsp);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int result = state->gsp_speedup_addr[0][offset];

	/* if this address is equal to $f000, spin until something gets written */
	if (space->cpu == state->gsp && cpu_get_pc(space->cpu) == state->gsp_speedup_pc &&
		(result & 0xff) < cpu_get_reg(space->cpu, TMS34010_A1))
	{
		state->gsp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return result;
}


WRITE16_HANDLER( rdgsp_speedup1_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	COMBINE_DATA(&state->gsp_speedup_addr[0][offset]);
	if (space->cpu != state->gsp)
		cpu_triggerint(state->gsp);
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int data = state->msp_speedup_addr[offset];

	if (data == 0 && space->cpu == state->msp && cpu_get_pc(space->cpu) == state->msp_speedup_pc)
	{
		state->msp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return data;
}


WRITE16_HANDLER( hdmsp_speedup_w )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	COMBINE_DATA(&state->msp_speedup_addr[offset]);
	if (offset == 0 && state->msp_speedup_addr[offset] != 0)
		cpu_triggerint(state->msp);
}


READ16_HANDLER( stmsp_speedup_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();

	/* assumes: stmsp_sync[0] -> $80010, stmsp_sync[1] -> $99680, stmsp_sync[2] -> $99d30 */
	if (state->stmsp_sync[0][0] == 0 &&		/* 80010 */
		state->stmsp_sync[0][1] == 0 &&		/* 80020 */
		state->stmsp_sync[0][2] == 0 &&		/* 80030 */
		state->stmsp_sync[0][3] == 0 &&		/* 80040 */
		state->stmsp_sync[0][4] == 0 &&		/* 80050 */
		state->stmsp_sync[0][5] == 0 && 		/* 80060 */
		state->stmsp_sync[0][6] == 0 && 		/* 80070 */
		state->stmsp_sync[1][0] == 0 && 		/* 99680 */
		state->stmsp_sync[2][0] == 0xffff &&	/* 99d30 */
		state->stmsp_sync[2][1] == 0xffff &&	/* 99d40 */
		state->stmsp_sync[2][2] == 0 &&		/* 99d50 */
		cpu_get_pc(space->cpu) == 0x3c0)
	{
		state->msp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}
	return state->stmsp_sync[0][1];
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
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int data = state->adsp_data_memory[0x1fff];

	if (data == 0xffff && space->cpu == state->adsp && cpu_get_pc(space->cpu) <= 0x3b)
	{
		state->adsp_speedup_count[0]++;
		cpu_spinuntil_int(space->cpu);
	}

	return data;
}


READ16_HANDLER( hdds3_speedup_r )
{
	harddriv_state *state = space->machine->driver_data<harddriv_state>();
	int data = *state->ds3_speedup_addr;

	if (data != 0 && space->cpu == state->adsp && cpu_get_pc(space->cpu) == state->ds3_speedup_pc)
	{
		state->adsp_speedup_count[2]++;
		cpu_spinuntil_int(space->cpu);
	}

	return data;
}
