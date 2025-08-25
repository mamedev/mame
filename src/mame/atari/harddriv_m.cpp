// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Hard Drivin' machine hardware

****************************************************************************/

#include "emu.h"
#include "harddriv.h"


/*************************************
 *
 *  Constants and macros
 *
 *************************************/

#define DS3_TRIGGER         7777
#define DS3_STRIGGER        5555

/* debugging tools */
#define LOG_COMMANDS        0


#if 0
#pragma mark * DRIVER/MULTISYNC BOARD
#endif


/*************************************
 *
 *  Initialization
 *
 *************************************/

void harddriv_state::device_start()
{
	m_lamps.resolve();
	m_sel.resolve();
	m_wheel.resolve();

	/* predetermine memory regions */
	m_adsp_pgm_memory_word = (uint16_t *)(reinterpret_cast<uint8_t *>(m_adsp_pgm_memory.target()) + 1);

	init_video();

	m_xdsp_serial_irq_off_timer = timer_alloc(FUNC(harddriv_state::xdsp_sport1_irq_off_callback), this);
}


void harddriv_state::device_reset()
{
	/* halt several of the DSPs to start */
	m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	if (m_dsp32.found()) m_dsp32->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_last_gsp_shiftreg = 0;

	m_m68k_adsp_buffer_bank = 0;

	/* reset IRQ states */
	m_irq_state = m_gsp_irq_state = m_msp_irq_state = m_adsp_irq_state = m_duart_irq_state = 0;

	/* reset the ADSP/DSIII/DSIV boards */
	m_adsp_halt = 1;
	m_adsp_br = 0;
	m_adsp_xflag = 0;

	if (m_ds3sdsp.found())
	{
		m_ds3sdsp->load_boot_data(m_ds3sdsp_region->base(), m_ds3sdsp_pgm_memory);
		m_ds3sdsp_timer_en = 0;
		m_ds3sdsp_internal_timer->adjust(attotime::never);
	}

	if (m_ds3xdsp.found())
	{
		m_ds3xdsp->load_boot_data(m_ds3xdsp_region->base(), m_ds3xdsp_pgm_memory);
		m_ds3xdsp_timer_en = 0;
		m_ds3xdsp_internal_timer->adjust(attotime::never);
	}

	m_xdsp_serial_irq_off_timer->adjust(attotime::never);
}



/*************************************
 *
 *  68000 interrupt handling
 *
 *************************************/

void harddriv_state::update_interrupts()
{
	m_maincpu->set_input_line(1, m_msp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(2, m_adsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(3, m_gsp_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(4, m_sound_int_state ? ASSERT_LINE : CLEAR_LINE); /* /LINKIRQ on STUN Runner */
	m_maincpu->set_input_line(5, m_irq_state ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(6, m_duart_irq_state ? ASSERT_LINE : CLEAR_LINE);
}


INTERRUPT_GEN_MEMBER(harddriv_state::hd68k_irq_gen)
{
	m_irq_state = 1;
	update_interrupts();
}


void harddriv_state::hd68k_irq_ack_w(uint16_t data)
{
	m_irq_state = 0;
	update_interrupts();
}


void harddriv_state::hdgsp_irq_gen(int state)
{
	m_gsp_irq_state = state;
	update_interrupts();
}


void harddriv_state::hdmsp_irq_gen(int state)
{
	m_msp_irq_state = state;
	update_interrupts();
}



/*************************************
 *
 *  68000 access to GSP
 *
 *************************************/

uint16_t harddriv_state::hd68k_gsp_io_r(offs_t offset)
{
	uint16_t result;
	offset = (offset / 2) ^ 1;
	m_hd34010_host_access = true;
	result = m_gsp->host_r(offset);
	m_hd34010_host_access = false;
	return result;
}


void harddriv_state::hd68k_gsp_io_w(offs_t offset, uint16_t data)
{
	offset = (offset / 2) ^ 1;
	m_hd34010_host_access = true;
	m_gsp->host_w(offset, data);
	m_hd34010_host_access = false;
}



/*************************************
 *
 *  68000 access to MSP
 *
 *************************************/

uint16_t harddriv_state::hd68k_msp_io_r(offs_t offset)
{
	uint16_t result;
	offset = (offset / 2) ^ 1;
	m_hd34010_host_access = true;
	result = m_msp.found() ? m_msp->host_r(offset) : 0xffff;
	m_hd34010_host_access = false;
	return result;
}


void harddriv_state::hd68k_msp_io_w(offs_t offset, uint16_t data)
{
	offset = (offset / 2) ^ 1;
	if (m_msp.found())
	{
		m_hd34010_host_access = true;
		m_msp->host_w(offset, data);
		m_hd34010_host_access = false;
	}
}



/*************************************
 *
 *  68000 input handlers
 *
 *************************************/

uint16_t harddriv_state::hd68k_a80000_r()
{
	return m_a80000->read();
}

uint16_t harddriv_state::hd68k_port0_r()
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
	screen_device &scr = m_gsp->screen();

	int temp = (m_sw1.read_safe(0xff) << 8) | m_in0->read();
	if (get_hblank(scr)) temp ^= 0x0002;
	temp ^= 0x0008;     /* 12-bit EOC always high for now */
	return temp;
}


uint16_t harddriv_state::hdc68k_port1_r()
{
	uint16_t result = m_a80000->read();
	uint16_t diff = result ^ m_hdc68k_last_port1;

	/* if a new shifter position is selected, use it */
	/* if it's the same shifter position as last time, go back to neutral */
	if ((diff & 0x0100) && !(result & 0x0100))
		m_hdc68k_shifter_state = (m_hdc68k_shifter_state == 1) ? 0 : 1;
	if ((diff & 0x0200) && !(result & 0x0200))
		m_hdc68k_shifter_state = (m_hdc68k_shifter_state == 2) ? 0 : 2;
	if ((diff & 0x0400) && !(result & 0x0400))
		m_hdc68k_shifter_state = (m_hdc68k_shifter_state == 4) ? 0 : 4;
	if ((diff & 0x0800) && !(result & 0x0800))
		m_hdc68k_shifter_state = (m_hdc68k_shifter_state == 8) ? 0 : 8;

	/* merge in the new shifter value */
	result = (result | 0x0f00) ^ (m_hdc68k_shifter_state << 8);

	/* merge in the wheel edge latch bit */
	if (m_hdc68k_wheel_edge)
		result ^= 0x4000;

	m_hdc68k_last_port1 = result;
	return result;
}


uint16_t harddriv_state::hda68k_port1_r()
{
	uint16_t result = m_a80000->read();

	/* merge in the wheel edge latch bit */
	if (m_hdc68k_wheel_edge)
		result ^= 0x4000;

	return result;
}


uint16_t harddriv_state::hdc68k_wheel_r()
{
	/* grab the new wheel value */
	uint16_t new_wheel = m_12badc[0].read_safe(0xffff);

	/* hack to display the wheel position */
	if (machine().input().code_pressed(KEYCODE_LSHIFT))
		popmessage("%04X", new_wheel);

	/* if we crossed the center line, latch the edge bit */
	if ((m_hdc68k_last_wheel / 0xf00) != (new_wheel / 0xf00))
		m_hdc68k_wheel_edge = 1;

	/* remember the last value and return the low 8 bits */
	m_hdc68k_last_wheel = new_wheel;
	return (new_wheel << 8) | 0xff;
}


uint16_t harddriv_state::hd68k_adc12_r()
{
	return m_adc12_byte ? ((m_adc12_data >> 8) & 0x0f) : (m_adc12_data & 0xff);
}


uint16_t harddriv_state::hd68k_sound_reset_r()
{
	if (m_jsa.found())
		m_jsa->reset();
	return ~0;
}



/*************************************
 *
 *  68000 output handlers
 *
 *************************************/

void harddriv_state::hd68k_adc_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_adc_control);

	/* handle a write to the 8-bit ADC address select */
	m_adc8->address_w(m_adc_control & 0x07);
	m_adc8->start_w(BIT(m_adc_control, 3));

	/* handle a write to the 12-bit ADC address select */
	if (m_adc_control & 0x40)
	{
		m_adc12_select = (m_adc_control >> 4) & 0x03;
		m_adc12_data = m_12badc[m_adc12_select].read_safe(0xffff);
	}

	/* bit 7 selects which byte of the 12 bit data to read */
	m_adc12_byte = (m_adc_control >> 7) & 1;
}


void harddriv_state::hd68k_wr0_write(offs_t offset, uint16_t data)
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;

	m_sel_select = 0;

	switch (offset)
	{
		case 1: /* SEL1 */
		case 2: /* SEL2 */
		case 3: /* SEL3 */
		case 4: /* SEL4 */
			m_sel_select = offset;
			break;

		case 6: /* CC1 */
		case 7: /* CC2 */
			machine().bookkeeping().coin_counter_w(offset - 6, data);
			break;

		default:
			/* just ignore */
			break;
	}
}


void harddriv_state::hd68k_wr1_write(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		// logerror("Shifter Interface Latch = %02X\n", data);
		data = data >> 8;
		switch (m_sel_select)
		{
			case 1: /* SEL1 */
				m_sel1_data = data;
				m_sel[0] = m_sel1_data;
				break;

			case 2: /* SEL2 */
				m_sel2_data = data;
				m_sel[1] = m_sel2_data;
				break;

			case 3: /* SEL3 */
				m_sel3_data = data;
				m_sel[2] = m_sel3_data;
				break;

			case 4: /* SEL4 */
				m_sel4_data = data;
				m_sel[3] = m_sel4_data;
				break;
		}
	}
	else
		logerror("/WR1(%04X)=%02X\n", offset, data);
}


void harddriv_state::hd68k_wr2_write(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		// logerror("Steering Wheel Latch = %02X\n", data);
		m_wheel = data >> 8;
	}
	else
		logerror("/WR2(%04X)=%02X\n", offset, data);
}


void harddriv_state::hd68k_nwr_w(offs_t offset, uint16_t data)
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0: /* CR2 */
		case 1: /* CR1 */
			break;
		case 2: /* LC1 */
			// used for seat locking on harddriv
			m_lamps[0] = data;
			break;
		case 3: /* LC2 */
			// used for "abort" button lamp
			m_lamps[1] = data;
			break;
		case 4: /* ZP1 */
			m_m68k_zp1 = data;
			break;
		case 5: /* ZP2 */
			m_m68k_zp2 = data;
			break;
		case 6: /* /GSPRES */
			logerror("Write to /GSPRES(%d)\n", data);
			m_gsp->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
		case 7: /* /MSPRES */
			logerror("Write to /MSPRES(%d)\n", data);
			if (m_msp.found())
				m_msp->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


void harddriv_state::hdc68k_wheel_edge_reset_w(uint16_t data)
{
	/* reset the edge latch */
	m_hdc68k_wheel_edge = 0;
}



/*************************************
 *
 *  68000 ZRAM access
 *
 *************************************/

uint16_t harddriv_state::hd68k_zram_r(address_space &space, offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	if (ACCESSING_BITS_0_7)
		data |= m_210e->read(space, offset);

	if (ACCESSING_BITS_8_15)
		data |= m_200e->read(offset) << 8;

	return data;
}


void harddriv_state::hd68k_zram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (m_m68k_zp1 == 0 && m_m68k_zp2 == 1)
	{
		if (ACCESSING_BITS_0_7)
			m_210e->write(offset, data);

		if (ACCESSING_BITS_8_15)
			m_200e->write(offset, data >> 8);
	}
}



/*************************************
 *
 *  68681 DUART
 *
 *************************************/

void harddriv_state::harddriv_duart_irq_handler(int state)
{
	m_duart_irq_state = state;
	update_interrupts();
}


/*************************************
 *
 *  GSP I/O register writes
 *
 *************************************/

void harddriv_state::hdgsp_io_w(offs_t offset, u16 data, u16 mem_mask)
{
	/* detect an enabling of the shift register and force yielding */
	if (offset == REG_DPYCTL)
	{
		uint8_t new_shiftreg = (data >> 11) & 1;
		if (new_shiftreg != m_last_gsp_shiftreg)
		{
			m_last_gsp_shiftreg = new_shiftreg;
			if (new_shiftreg)
				m_gsp->yield();
		}
	}

	screen_device &scr = m_gsp->screen();

	/* detect changes to HEBLNK and HSBLNK and force an update before they change */
	if ((offset == REG_HEBLNK || offset == REG_HSBLNK) && data != m_gsp->io_register_r(offset))
		scr.update_partial(scr.vpos() - 1);
}



/*************************************
 *
 *  GSP protection workarounds
 *
 *************************************/

void harddriv_state::hdgsp_protection_w(uint16_t data)
{
	/* this memory address is incremented whenever a protection check fails */
	/* after it reaches a certain value, the GSP will randomly trash a */
	/* register; we just prevent it from ever going above 0 */
	*m_gsp_protection = 0;
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

uint16_t harddriv_state::hd68k_adsp_program_r(offs_t offset)
{
	uint32_t word = m_adsp_pgm_memory[offset/2];
	return (!(offset & 1)) ? (word >> 16) : (word & 0xffff);
}


void harddriv_state::hd68k_adsp_program_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t *base = &m_adsp_pgm_memory[offset/2];
	uint32_t oldword = *base;
	uint16_t temp;

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

uint16_t harddriv_state::hd68k_adsp_data_r(offs_t offset)
{
	return m_adsp_data_memory[offset];
}


void harddriv_state::hd68k_adsp_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_adsp_data_memory[offset]);

	/* any write to $1FFF is taken to be a trigger; synchronize the CPUs */
	if (offset == 0x1fff)
	{
		logerror("%06X:ADSP sync address written (%04X)\n", m_maincpu->pcbase(), data);
		machine().scheduler().synchronize();
		m_adsp->signal_interrupt_trigger();
	}
	else
		logerror("%06X:ADSP W@%04X (%04X)\n", m_maincpu->pcbase(), offset, data);
}



/*************************************
 *
 *  68000 access to ADSP output memory
 *
 *************************************/

uint16_t harddriv_state::hd68k_adsp_buffer_r(offs_t offset)
{
	/*  logerror("hd68k_adsp_buffer_r(%04X)\n", offset);*/
	return m_som_memory[m_m68k_adsp_buffer_bank * 0x2000 + offset];
}


void harddriv_state::hd68k_adsp_buffer_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_som_memory[m_m68k_adsp_buffer_bank * 0x2000 + offset]);
}



/*************************************
 *
 *  68000 access to ADSP control regs
 *
 *************************************/

TIMER_CALLBACK_MEMBER(harddriv_state::deferred_adsp_bank_switch)
{
	if (LOG_COMMANDS && m_m68k_adsp_buffer_bank != param && machine().input().code_pressed(KEYCODE_L))
	{
		static FILE *commands;
		if (!commands) commands = fopen("commands.log", "w");
		if (commands)
		{
			int16_t *base = (int16_t *)&m_som_memory[param * 0x2000];
			int16_t *end = base + (uint16_t)*base;
			int16_t *current = base + 1;
			int16_t *table = base + 1 + (uint16_t)*current++;

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
					uint32_t rslope, lslope;
					rslope = (uint16_t)*current++,
					rslope |= *current++ << 16;
					if (rslope == 0xffffffff)
					{
						fprintf(commands, "  (end)\n");
						break;
					}
					lslope = (uint16_t)*current++,
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

	m_m68k_adsp_buffer_bank = param;
	logerror("ADSP bank = %d\n", param);
}


void harddriv_state::hd68k_adsp_control_w(offs_t offset, uint16_t data)
{
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
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_state::deferred_adsp_bank_switch),this), val);
			break;

		case 5:
			/* connected to the /BR (bus request) line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			m_adsp_br = !val;
			logerror("ADSP /BR = %d\n", !m_adsp_br);
			if (m_adsp_br || m_adsp_halt)
				m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				m_adsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				m_maincpu->spin();
			}
			break;

		case 6:
			/* connected to the /HALT line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			m_adsp_halt = !val;
			logerror("ADSP /HALT = %d\n", !m_adsp_halt);
			if (m_adsp_br || m_adsp_halt)
				m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				m_adsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				m_maincpu->spin();
			}
			break;

		case 7:
			logerror("ADSP reset = %d\n", val);
			m_adsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			m_maincpu->yield();
			break;

		default:
			logerror("ADSP control %02X = %04X\n", offset, data);
			break;
	}
}


void harddriv_state::hd68k_adsp_irq_clear_w(uint16_t data)
{
	logerror("%06X:68k clears ADSP interrupt\n", m_maincpu->pcbase());
	m_adsp_irq_state = 0;
	update_interrupts();
}


uint16_t harddriv_state::hd68k_adsp_irq_state_r()
{
	int result = 0xfffd;
	if (m_adsp_xflag) result ^= 2;
	if (m_adsp_irq_state) result ^= 1;
	logerror("%06X:68k reads ADSP interrupt state = %04x\n", m_maincpu->pcbase(), result);
	return result;
}



/*************************************
 *
 *  ADSP memory-mapped I/O
 *
 *************************************/

uint16_t harddriv_state::hdadsp_special_r(offs_t offset)
{
	switch (offset & 7)
	{
		case 0: /* /SIMBUF */
			if (m_adsp_eprom_base + m_adsp_sim_address < m_sim_memory.length())
				return m_sim_memory[m_adsp_eprom_base + m_adsp_sim_address++];
			else
				return 0xff;

		case 1: /* /SIMLD */
			break;

		case 2: /* /SOMO */
			break;

		case 3: /* /SOMLD */
			break;

		default:
			logerror("%04X:hdadsp_special_r(%04X)\n", m_adsp->pcbase(), offset);
			break;
	}
	return 0;
}


void harddriv_state::hdadsp_special_w(offs_t offset, uint16_t data)
{
	switch (offset & 7)
	{
		case 1: /* /SIMCLK */
			m_adsp_sim_address = data;
			break;

		case 2: /* SOMLATCH */
			m_som_memory[(m_m68k_adsp_buffer_bank ^ 1) * 0x2000 + (m_adsp_som_address++ & 0x1fff)] = data;
			break;

		case 3: /* /SOMCLK */
			m_adsp_som_address = data;
			break;

		case 5: /* /XOUT */
			m_adsp_xflag = data & 1;
			break;

		case 6: /* /GINT */
			logerror("%04X:ADSP signals interrupt\n", m_adsp->pcbase());
			m_adsp_irq_state = 1;
			update_interrupts();
			break;

		case 7: /* /MP */
			m_adsp_eprom_base = 0x10000 * data;
			break;

		default:
			logerror("%04X:hdadsp_special_w(%04X)=%04X\n", m_adsp->pcbase(), offset, data);
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

void harddriv_state::update_ds3_irq()
{
	/* update the IRQ2 signal to the ADSP2101 */
	if (!(!m_ds3_g68flag && m_ds3_g68irqs) && !(m_ds3_gflag && m_ds3_gfirqs))
		m_adsp->set_input_line(ADSP2100_IRQ2, ASSERT_LINE);
	else
		m_adsp->set_input_line(ADSP2100_IRQ2, CLEAR_LINE);
}


void harddriv_state::update_ds3_sirq()
{
	/* update the IRQ2 signal to the ADSP2105 */
	if (!(!m_ds3_s68flag && m_ds3_s68irqs) && !(m_ds3_sflag && m_ds3_sfirqs))
		m_ds3sdsp->set_input_line(ADSP2105_IRQ2, ASSERT_LINE);
	else
		m_ds3sdsp->set_input_line(ADSP2105_IRQ2, CLEAR_LINE);
}


void harddriv_state::hd68k_ds3_control_w(offs_t offset, uint16_t data)
{
	int val = (offset >> 3) & 1;

	switch (offset & 7)
	{
		case 0:
			/* SRES - reset sound CPU */
			if (m_ds3sdsp.found())
			{
				m_ds3sdsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
				m_ds3sdsp->load_boot_data(m_ds3sdsp_region->base(), m_ds3sdsp_pgm_memory);

				if (val && !m_ds3_sreset)
				{
					m_ds3_sflag = 0;
					m_ds3_scmd = 0;
					m_ds3_sfirqs = 0;
					m_ds3_s68irqs = !m_ds3_sfirqs;
					update_ds3_sirq();
				}
				m_ds3_sreset = val;
				m_maincpu->yield();
			}
			break;

		case 1:
			/* XRES - reset sound helper CPU */
			if (m_ds3xdsp.found())
			{
				m_ds3xdsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
				m_ds3xdsp->load_boot_data(m_ds3xdsp_region->base(), m_ds3xdsp_pgm_memory);
			}
			break;

		case 2:
			/* connected to the /BR (bus request) line; this effectively halts */
			/* the ADSP at the next instruction boundary */
			m_adsp_br = !val;
			if (m_adsp_br)
				m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			else
			{
				m_adsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				/* a yield in this case is not enough */
				/* we would need to increase the interleaving otherwise */
				/* note that this only affects the test mode */
				m_maincpu->spin();
			}
			break;

		case 3:
			m_adsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			if (val && !m_ds3_reset)
			{
				m_ds3_gflag = 0;
				m_ds3_gcmd = 0;
				m_ds3_gfirqs = 0;
				m_ds3_g68irqs = !m_ds3_gfirqs;
				m_ds3_send = 0;
				update_ds3_irq();
			}
			m_ds3_reset = val;
			m_maincpu->yield();
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

uint16_t harddriv_state::hd68k_ds3_girq_state_r()
{
	int result = 0x0fff;
	if (m_ds3_g68flag) result ^= 0x8000;
	if (m_ds3_gflag) result ^= 0x4000;
	if (m_ds3_g68irqs) result ^= 0x2000;
	if (!m_adsp_irq_state) result ^= 0x1000;
	return result;
}


uint16_t harddriv_state::hd68k_ds3_gdata_r()
{
	offs_t pc = m_maincpu->pc();

	m_ds3_gflag = 0;
	update_ds3_irq();

	logerror("%06X:hd68k_ds3_gdata_r(%04X)\n", m_maincpu->pcbase(), m_ds3_gdata);

	/* attempt to optimize the transfer if conditions are right */
	if (pc == m_ds3_transfer_pc &&
		!(!m_ds3_g68flag && m_ds3_g68irqs) && !(m_ds3_gflag && m_ds3_gfirqs))
	{
		uint32_t destaddr = m_maincpu->state_int(M68K_A1);
		uint16_t count68k = m_maincpu->state_int(M68K_D1);
		uint16_t mstat = m_adsp->state_int(ADSP2100_MSTAT);
		uint16_t i6 = m_adsp->state_int((mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC);
		uint16_t l6 = m_adsp->state_int(ADSP2100_L6) - 1;
		uint16_t m7 = m_adsp->state_int(ADSP2100_M7);
		auto &mspace = m_maincpu->space(AS_PROGRAM);

		logerror("%06X:optimizing 68k transfer, %d words\n", m_maincpu->pcbase(), count68k);

		while (count68k > 0 && m_adsp_data_memory[0x16e6] > 0)
		{
			mspace.write_word(destaddr, m_ds3_gdata);
			{
				m_adsp_data_memory[0x16e6]--;
				m_ds3_gdata = m_adsp_pgm_memory[i6] >> 8;
				i6 = (i6 & ~l6) | ((i6 + m7) & l6);
			}
			count68k--;
		}
		m_maincpu->set_state_int(M68K_D1, count68k);
		m_adsp->set_state_int((mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC, i6);
		m_adsp_speedup_count[1]++;
	}

	/* if we just cleared the IRQ, we are going to do some VERY timing critical reads */
	/* it is important that all the CPUs be in sync before we continue, so spin a little */
	/* while to let everyone else catch up */
	m_maincpu->spin_until_trigger(DS3_TRIGGER);
	machine().scheduler().trigger(DS3_TRIGGER, attotime::from_usec(5));

	return m_ds3_gdata;
}


void harddriv_state::hd68k_ds3_gdata_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%06X:hd68k_ds3_gdata_w(%04X)\n", m_maincpu->pcbase(), m_ds3_gdata);

	COMBINE_DATA(&m_ds3_g68data);
	m_ds3_g68flag = 1;
	m_ds3_gcmd = offset & 1;
	m_adsp->signal_interrupt_trigger();
	update_ds3_irq();
}



/*************************************
 *
 *  DS III sound I/O
 *
 *************************************/

void harddriv_state::hd68k_ds3_sirq_clear_w(uint16_t data)
{
	logerror("%06X:68k clears ADSP interrupt\n", m_maincpu->pcbase());
	m_sound_int_state = 0;
	update_interrupts();
}


uint16_t harddriv_state::hd68k_ds3_sirq_state_r()
{
	int result = 0x0fff;
	if (m_ds3_s68flag) result ^= 0x8000;
	if (m_ds3_sflag) result ^= 0x4000;
	if (m_ds3_s68irqs) result ^= 0x2000;
	if (!m_sound_int_state) result ^= 0x1000;
	return result;
}


uint16_t harddriv_state::hd68k_ds3_sdata_r()
{
	m_ds3_sflag = 0;
	update_ds3_sirq();

	/* if we just cleared the IRQ, we are going to do some VERY timing critical reads */
	/* it is important that all the CPUs be in sync before we continue, so spin a little */
	/* while to let everyone else catch up */
	m_maincpu->spin_until_trigger(DS3_STRIGGER);
	machine().scheduler().trigger(DS3_STRIGGER, attotime::from_usec(5));

	return m_ds3_sdata;
}


void harddriv_state::hd68k_ds3_sdata_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ds3_s68data);
	m_ds3_s68flag = 1;
	m_ds3_scmd = offset & 1;
	m_ds3sdsp->signal_interrupt_trigger();

	update_ds3_sirq();
}


uint16_t harddriv_state::hdds3_sdsp_special_r(offs_t offset)
{
	int result;

	switch (offset & 7)
	{
		case 0:
			m_ds3_s68flag = 0;
			update_ds3_sirq();
			return m_ds3_s68data;

		case 1:
			result = 0x0fff;
			if (m_ds3_scmd) result ^= 0x8000;
			if (m_ds3_s68flag) result ^= 0x4000;
			if (m_ds3_sflag) result ^= 0x2000;
			return result;

		case 4:
			if (m_ds3_sdata_address < m_ds3_sdata_memory_size)
				return m_ds3_sdata_memory[m_ds3_sdata_address];
			else
				return 0xff;

		case 5: /* DS IV: sound ROM configuration */
			return 1;

		case 7: /* SFWCLR */
			break;

		default:
			return 0xff;
	}

	return 0;
}


void harddriv_state::hdds3_sdsp_special_w(offs_t offset, uint16_t data)
{
	/* Note: DS IV is slightly different */
	switch (offset & 7)
	{
		case 0:
			m_ds3_sdata = data;
			m_ds3_sflag = 1;
			update_ds3_sirq();

			/* once we've written data, trigger the main CPU to wake up again */
			machine().scheduler().trigger(DS3_STRIGGER);
			break;

		case 1:
			m_sound_int_state = (data >> 1) & 1;
			update_interrupts();
			break;

		case 2: /* bit 0 = T1 (unused) */
			break;

		case 3:
			m_ds3_sfirqs = (data >> 1) & 1;
			m_ds3_s68irqs = !m_ds3_sfirqs;
			update_ds3_sirq();
			break;

		case 4:
			m_ldac->write(data);
			break;

		case 5:
			m_rdac->write(data);
			break;

		case 6:
			m_ds3_sdata_address = (m_ds3_sdata_address & 0xffff0000) | (data & 0xffff);
			break;

		case 7:
			m_ds3_sdata_address = (m_ds3_sdata_address & 0x0000ffff) | (data << 16);
			break;
	}
}

uint16_t harddriv_state::hdds3_sdsp_control_r(offs_t offset)
{
	switch (offset)
	{
		default:
			return m_ds3sdsp_regs[offset];
	}
}


void harddriv_state::hdds3_sdsp_control_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0x1b:
			// Scale
			data &= 0xff;

			if (m_ds3sdsp_regs[0x1b] != data)
			{
				m_ds3sdsp_regs[0x1b] = data;
				hdds3sdsp_reset_timer();
			}
			break;

		case 0x1c:
			// Count
			if (m_ds3sdsp_regs[0x1c] != data)
			{
				m_ds3sdsp_regs[0x1c] = data;
				hdds3sdsp_reset_timer();
			}
			break;

		case 0x1d:
			// Period
			m_ds3sdsp_regs[0x1d] = data;
			break;

		case 0x1e:
			m_ds3sdsp_regs[0x1e] = data;
			break;

		case 0x1f:
			/* are we asserting BFORCE? */
			if (data & 0x200)
			{
				uint32_t page = (data >> 6) & 7;
				m_ds3sdsp->load_boot_data(m_ds3sdsp_region->base() + (0x2000 * page), m_ds3sdsp_pgm_memory);
				m_ds3sdsp->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
				data &= ~0x200;
			}

			m_ds3sdsp_regs[0x1f] = data;
			break;

		default:
			m_ds3sdsp_regs[offset] = data;
			break;
	}
}


uint16_t harddriv_state::hdds3_xdsp_control_r(offs_t offset)
{
	switch (offset)
	{
		default:
			return m_ds3xdsp_regs[offset];
	}

	// never executed
	//return 0xff;
}


void harddriv_state::hdds3_xdsp_control_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		default:
			m_ds3xdsp_regs[offset] = data;
			break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( harddriv_state::ds3sdsp_internal_timer_callback )
{
	uint16_t period = m_ds3sdsp_regs[0x1d];
	uint16_t scale = m_ds3sdsp_regs[0x1b] + 1;

	m_ds3sdsp_internal_timer->adjust(m_ds3sdsp->cycles_to_attotime(period * scale));

	/* the IRQ line is edge triggered */
	m_ds3sdsp->set_input_line(ADSP2105_TIMER, ASSERT_LINE);
	m_ds3sdsp->set_input_line(ADSP2105_TIMER, CLEAR_LINE);
}


void harddriv_state::hdds3sdsp_reset_timer()
{
	if (!m_ds3sdsp_timer_en)
		return;

	uint16_t count = m_ds3sdsp_regs[0x1c];
	uint16_t scale = m_ds3sdsp_regs[0x1b] + 1;

	m_ds3sdsp_internal_timer->adjust(m_ds3sdsp->cycles_to_attotime(count * scale));
}

void harddriv_state::hdds3sdsp_timer_enable_callback(int state)
{
	m_ds3sdsp_timer_en = state;

	if (state)
		hdds3sdsp_reset_timer();
	else
		m_ds3sdsp_internal_timer->adjust(attotime::never);
}


TIMER_DEVICE_CALLBACK_MEMBER( harddriv_state::ds3xdsp_internal_timer_callback )
{
	uint16_t period = m_ds3xdsp_regs[0x1d];
	uint16_t scale = m_ds3xdsp_regs[0x1b] + 1;

	m_ds3xdsp_internal_timer->adjust(m_ds3xdsp->cycles_to_attotime(period * scale));

	/* the IRQ line is edge triggered */
	m_ds3xdsp->set_input_line(ADSP2105_TIMER, ASSERT_LINE);
	m_ds3xdsp->set_input_line(ADSP2105_TIMER, CLEAR_LINE);
}


void harddriv_state::hdds3xdsp_reset_timer()
{
	if (!m_ds3xdsp_timer_en)
		return;

	uint16_t count = m_ds3xdsp_regs[0x1c];
	uint16_t scale = m_ds3xdsp_regs[0x1b] + 1;

	m_ds3xdsp_internal_timer->adjust(m_ds3xdsp->cycles_to_attotime(count * scale));
}


void harddriv_state::hdds3xdsp_timer_enable_callback(int state)
{
	m_ds3xdsp_timer_en = state;

	if (state)
		hdds3xdsp_reset_timer();
	else
		m_ds3xdsp_internal_timer->adjust(attotime::never);
}


/*
    TODO: The following does not work correctly
*/
TIMER_CALLBACK_MEMBER(harddriv_state::xdsp_sport1_irq_off_callback)
{
	m_ds3xdsp->set_input_line(ADSP2105_SPORT1_RX, CLEAR_LINE);
}


void harddriv_state::hdds3sdsp_serial_tx_callback(uint32_t data)
{
	if ((m_ds3sdsp_regs[0x1f] & 0xc00) != 0xc00)
		return;

	m_ds3sdsp_sdata = data;

	m_ds3xdsp->set_input_line(ADSP2105_SPORT1_RX, ASSERT_LINE);
	m_xdsp_serial_irq_off_timer->adjust(attotime::from_nsec(200));
}


uint32_t harddriv_state::hdds3sdsp_serial_rx_callback()
{
	if ((m_ds3sdsp_regs[0x1f] & 0xc00) != 0xc00)
		return 0xff;

	return m_ds3xdsp_sdata;
}


void harddriv_state::hdds3xdsp_serial_tx_callback(uint32_t data)
{
	if ((m_ds3xdsp_regs[0x1f] & 0xc00) != 0xc00)
		return;

	m_ds3xdsp_sdata = data;
}


uint32_t harddriv_state::hdds3xdsp_serial_rx_callback()
{
	m_ds3xdsp->set_input_line(ADSP2105_SPORT1_RX, ASSERT_LINE);
	m_ds3xdsp->set_input_line(ADSP2105_SPORT1_RX, CLEAR_LINE);
	m_ds3xdsp->signal_interrupt_trigger();
	return m_ds3sdsp_sdata;
}



/*************************************
 *
 *  DS III internal I/O
 *
 *************************************/

uint16_t harddriv_state::hdds3_special_r(offs_t offset)
{
	int result;

	switch (offset & 7)
	{
		case 0:
			m_ds3_g68flag = 0;
			update_ds3_irq();
			return m_ds3_g68data;

		case 1:
			result = 0x0fff;
			if (m_ds3_gcmd) result ^= 0x8000;
			if (m_ds3_g68flag) result ^= 0x4000;
			if (m_ds3_gflag) result ^= 0x2000;
			return result;

		case 6:
			logerror("ADSP r @ %04x\n", m_ds3_sim_address);
			if (m_ds3_sim_address < m_sim_memory.length())
				return m_sim_memory[m_ds3_sim_address];
			else
				return 0xff;
	}
	return 0;
}


void harddriv_state::hdds3_special_w(offs_t offset, uint16_t data)
{
	/* IMPORTANT! these data values also write through to the underlying RAM */
	m_adsp_data_memory[offset] = data;

	switch (offset & 7)
	{
		case 0:
			logerror("%s:ADSP sets gdata to %04X\n", machine().describe_context(), data);
			m_ds3_gdata = data;
			m_ds3_gflag = 1;
			update_ds3_irq();

			/* once we've written data, trigger the main CPU to wake up again */
			machine().scheduler().trigger(DS3_TRIGGER);
			break;

		case 1:
			logerror("%s:ADSP sets interrupt = %d\n", machine().describe_context(), (data >> 1) & 1);
			m_adsp_irq_state = (data >> 1) & 1;
			update_interrupts();
			break;

		case 2:
			m_ds3_send = (data >> 0) & 1;
			break;

		case 3:
			m_ds3_gfirqs = (data >> 1) & 1;
			m_ds3_g68irqs = !m_ds3_gfirqs;
			update_ds3_irq();
			break;

		case 4:
			m_ds3_sim_address = (m_ds3_sim_address & 0xffff0000) | (data & 0xffff);
			break;

		case 5:
			m_ds3_sim_address = (m_ds3_sim_address & 0xffff) | ((data << 16) & 0x00070000);
			break;
	}
}


uint16_t harddriv_state::hdds3_control_r(offs_t offset)
{
	logerror("adsp2101 control r @ %04X\n", 0x3fe0 + offset);
	return 0;
}


void harddriv_state::hdds3_control_w(offs_t offset, uint16_t data)
{
	if (offset != 0x1e && offset != 0x1f)
		logerror("adsp2101 control w @ %04X = %04X\n", 0x3fe0 + offset, data);
}



/*************************************
 *
 *  DS III program memory handlers
 *
 *************************************/

uint16_t harddriv_state::hd68k_ds3_program_r(offs_t offset)
{
	uint32_t *base = &m_adsp_pgm_memory[offset & 0x1fff];
	uint32_t word = *base;
	return (!(offset & 0x2000)) ? (word >> 8) : (word & 0xff);
}


void harddriv_state::hd68k_ds3_program_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t *base = &m_adsp_pgm_memory[offset & 0x1fff];
	uint32_t oldword = *base;
	uint16_t temp;

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

void harddriv_state::hddsk_update_pif(uint32_t data)
{
	m_sound_int_state = ((data & DSP32_OUTPUT_PIF) != 0);
	update_interrupts();
}



/*************************************
 *
 *  DSK board control handlers
 *
 *************************************/

void harddriv_state::hd68k_dsk_control_w(offs_t offset, uint16_t data)
{
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 0: /* DSPRESTN */
			if (m_dsp32.found()) m_dsp32->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 1: /* DSPZN */
			if (m_dsp32.found()) m_dsp32->set_input_line(INPUT_LINE_HALT, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 2: /* ZW1 */
			break;

		case 3: /* ZW2 */
			break;

		case 4: /* ASIC65 reset */
			m_asic65->reset_line(!val);
			break;

		case 7: /* LED */
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

uint16_t harddriv_state::hd68k_dsk_ram_r(offs_t offset)
{
	return m_dsk_ram[offset];
}


void harddriv_state::hd68k_dsk_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dsk_ram[offset]);
}


uint16_t harddriv_state::hd68k_dsk_small_rom_r(offs_t offset)
{
	return m_dsk_rom[offset & 0x1ffff];
}


uint16_t harddriv_state::hd68k_dsk_rom_r(offs_t offset)
{
	return m_dsk_rom[offset];
}



/*************************************
 *
 *  DSK board DSP32C I/O handlers
 *
 *************************************/

void harddriv_state::hd68k_dsk_dsp32_w(offs_t offset, uint16_t data)
{
	m_dsk_pio_access = true;
	if (m_dsp32.found()) m_dsp32->pio_w(offset, data);
	m_dsk_pio_access = false;
}


uint16_t harddriv_state::hd68k_dsk_dsp32_r(offs_t offset)
{
	uint16_t result;
	m_dsk_pio_access = true;
	if (m_dsp32.found()) result = m_dsp32->pio_r(offset);
	else result = 0x00;

	m_dsk_pio_access = false;
	return result;
}


/*************************************
 *
 *  DSP32C synchronization
 *
 *************************************/

TIMER_CALLBACK_MEMBER(harddriv_state::rddsp32_sync_cb)
{
	*m_dataptr[param] = m_dataval[param];
}


void harddriv_state::rddsp32_sync0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_dsk_pio_access)
	{
		uint32_t *dptr = &m_rddsp32_sync[0][offset];
		uint32_t newdata = *dptr;
		COMBINE_DATA(&newdata);
		m_dataptr[m_next_msp_sync % MAX_MSP_SYNC] = dptr;
		m_dataval[m_next_msp_sync % MAX_MSP_SYNC] = newdata;
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_state::rddsp32_sync_cb),this), m_next_msp_sync++ % MAX_MSP_SYNC);
	}
	else
		COMBINE_DATA(&m_rddsp32_sync[0][offset]);
}


void harddriv_state::rddsp32_sync1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_dsk_pio_access)
	{
		uint32_t *dptr = &m_rddsp32_sync[1][offset];
		uint32_t newdata = *dptr;
		COMBINE_DATA(&newdata);
		m_dataptr[m_next_msp_sync % MAX_MSP_SYNC] = dptr;
		m_dataval[m_next_msp_sync % MAX_MSP_SYNC] = newdata;
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_state::rddsp32_sync_cb),this), m_next_msp_sync++ % MAX_MSP_SYNC);
	}
	else
		COMBINE_DATA(&m_rddsp32_sync[1][offset]);
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

void harddriv_state::hddspcom_control_w(offs_t offset, uint16_t data)
{
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 2: /* ASIC65 reset */
			m_asic65->reset_line(!val);
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
 *  Steel Talons SLOOP handling
 *
 *************************************/

int harddriv_state::st68k_sloop_tweak(offs_t offset)
{
	static int last_offset;

	if (last_offset == 0)
	{
		switch (offset)
		{
			case 0x78e8:
				m_st68k_sloop_bank = 0;
				break;
			case 0x6ca4:
				m_st68k_sloop_bank = 1;
				break;
			case 0x15ea:
				m_st68k_sloop_bank = 2;
				break;
			case 0x6b28:
				m_st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return m_st68k_sloop_bank;
}


void harddriv_state::st68k_sloop_w(offs_t offset, uint16_t data)
{
	st68k_sloop_tweak(offset & 0x3fff);
}


uint16_t harddriv_state::st68k_sloop_r(offs_t offset)
{
	int bank = st68k_sloop_tweak(offset) * 0x4000;
	return m_m68k_sloop_base[bank + (offset & 0x3fff)];
}


uint16_t harddriv_state::st68k_sloop_alt_r(offs_t offset)
{
	if (m_st68k_last_alt_sloop_offset == 0x00fe)
	{
		switch (offset*2)
		{
			case 0x22c:
				m_st68k_sloop_bank = 0;
				break;
			case 0x1e2:
				m_st68k_sloop_bank = 1;
				break;
			case 0x1fa:
				m_st68k_sloop_bank = 2;
				break;
			case 0x206:
				m_st68k_sloop_bank = 3;
				break;
		}
	}
	m_st68k_last_alt_sloop_offset = offset*2;
	return m_m68k_sloop_alt_base[offset];
}


int harddriv_state::st68k_protosloop_tweak(offs_t offset)
{
	static int last_offset;

	if (last_offset == 0)
	{
		switch (offset)
		{
			case 0x0001:
				m_st68k_sloop_bank = 0;
				break;
			case 0x0002:
				m_st68k_sloop_bank = 1;
				break;
			case 0x0003:
				m_st68k_sloop_bank = 2;
				break;
			case 0x0004:
				m_st68k_sloop_bank = 3;
				break;
		}
	}
	last_offset = offset;
	return m_st68k_sloop_bank;
}


void harddriv_state::st68k_protosloop_w(offs_t offset, uint16_t data)
{
	st68k_protosloop_tweak(offset & 0x3fff);
}


uint16_t harddriv_state::st68k_protosloop_r(offs_t offset)
{
	int bank = st68k_protosloop_tweak(offset) * 0x4000;
	return m_m68k_sloop_base[bank + (offset & 0x3fff)];
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

uint16_t harddriv_state::hdgsp_speedup_r(offs_t offset)
{
	int result = m_gsp_speedup_addr[0][offset];

	/* if both this address and the other important address are not $ffff */
	/* then we can spin until something gets written */
	if (result != 0xffff && m_gsp_speedup_addr[1][0] != 0xffff &&
		m_gsp->pc() == m_gsp_speedup_pc)
	{
		m_gsp_speedup_count[0]++;
		m_gsp->spin_until_interrupt();
	}

	return result;
}


void harddriv_state::hdgsp_speedup1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gsp_speedup_addr[0][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (m_gsp_speedup_addr[0][offset] == 0xffff)
		m_gsp->signal_interrupt_trigger();
}


void harddriv_state::hdgsp_speedup2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gsp_speedup_addr[1][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (m_gsp_speedup_addr[1][offset] == 0xffff)
		m_gsp->signal_interrupt_trigger();
}



/*************************************
 *
 *  GSP Optimizations - case 2
 *  Works for:
 *      Race Drivin'
 *
 *************************************/

uint16_t harddriv_state::rdgsp_speedup1_r(offs_t offset)
{
	uint16_t result = m_gsp_speedup_addr[0][offset];

	/* if this address is equal to $f000, spin until something gets written */
	if (m_gsp->pc() == m_gsp_speedup_pc &&
		(uint8_t)result < m_gsp->state_int(TMS34010_A1))
	{
		m_gsp_speedup_count[0]++;
		m_gsp->spin_until_interrupt();
	}

	return result;
}


void harddriv_state::rdgsp_speedup1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gsp_speedup_addr[0][offset]);
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

uint16_t harddriv_state::hdmsp_speedup_r(offs_t offset)
{
	int data = m_msp_speedup_addr[offset];

	if (data == 0 && m_msp->pc() == m_msp_speedup_pc)
	{
		m_msp_speedup_count[0]++;
		m_msp->spin_until_interrupt();
	}

	return data;
}


void harddriv_state::hdmsp_speedup_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_msp_speedup_addr[offset]);
	if (offset == 0 && m_msp_speedup_addr[offset] != 0)
		m_msp->signal_interrupt_trigger();
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

uint16_t harddriv_state::hdadsp_speedup_r()
{
	int data = m_adsp_data_memory[0x1fff];

	if (data == 0xffff && m_adsp->pc() <= 0x3b)
	{
		m_adsp_speedup_count[0]++;
		m_adsp->spin_until_interrupt();
	}

	return data;
}


uint16_t harddriv_state::hdds3_speedup_r()
{
	int data = *m_ds3_speedup_addr;

	if (data != 0 && m_adsp->pc() == m_ds3_speedup_pc)
	{
		m_adsp_speedup_count[2]++;
		m_adsp->spin_until_interrupt();
	}

	return data;
}
