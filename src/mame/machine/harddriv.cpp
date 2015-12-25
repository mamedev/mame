// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Hard Drivin' machine hardware

****************************************************************************/

#include "emu.h"
#include "sound/dac.h"
#include "includes/slapstic.h"
#include "includes/harddriv.h"


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
	//atarigen_state::machine_start();

	/* predetermine memory regions */
	m_sim_memory = (UINT16 *)memregion("user1")->base();
	m_sim_memory_size = memregion("user1")->bytes() / 2;
	m_adsp_pgm_memory_word = (UINT16 *)(reinterpret_cast<UINT8 *>(m_adsp_pgm_memory.target()) + 1);

	init_video();

}


void  harddriv_state::device_reset()
{
	/* generic reset */
	//atarigen_state::machine_reset();
	m_slapstic_device->slapstic_reset();

	/* halt several of the DSPs to start */
	if (m_adsp != nullptr) m_adsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	if (m_dsp32 != nullptr) m_dsp32->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_last_gsp_shiftreg = 0;

	m_m68k_adsp_buffer_bank = 0;

	/* reset IRQ states */
	m_irq_state = m_gsp_irq_state = m_msp_irq_state = m_adsp_irq_state = m_duart_irq_state = 0;

	/* reset the ADSP/DSIII/DSIV boards */
	m_adsp_halt = 1;
	m_adsp_br = 0;
	m_adsp_xflag = 0;

	if (m_ds3sdsp != nullptr)
	{
		m_ds3sdsp->load_boot_data(m_ds3sdsp->region()->base(), m_ds3sdsp_pgm_memory);
		m_ds3sdsp_timer_en = 0;
		m_ds3sdsp_internal_timer->adjust(attotime::never);
	}

	if (m_ds3xdsp != nullptr)
	{
		m_ds3xdsp->load_boot_data(m_ds3xdsp->region()->base(), m_ds3xdsp_pgm_memory);
		m_ds3xdsp_timer_en = 0;
		m_ds3xdsp_internal_timer->adjust(attotime::never);
	}
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


WRITE16_MEMBER( harddriv_state::hd68k_irq_ack_w )
{
	m_irq_state = 0;
	update_interrupts();
}


WRITE_LINE_MEMBER(harddriv_state::hdgsp_irq_gen)
{
	m_gsp_irq_state = state;
	update_interrupts();
}


WRITE_LINE_MEMBER(harddriv_state::hdmsp_irq_gen)
{
	m_msp_irq_state = state;
	update_interrupts();
}



/*************************************
 *
 *  68000 access to GSP
 *
 *************************************/

READ16_MEMBER( harddriv_state::hd68k_gsp_io_r )
{
	UINT16 result;
	offset = (offset / 2) ^ 1;
	m_hd34010_host_access = TRUE;
	result = m_gsp->host_r(space, offset, 0xffff);
	m_hd34010_host_access = FALSE;
	return result;
}


WRITE16_MEMBER( harddriv_state::hd68k_gsp_io_w )
{
	offset = (offset / 2) ^ 1;
	m_hd34010_host_access = TRUE;
	m_gsp->host_w(space, offset, data, 0xffff);
	m_hd34010_host_access = FALSE;
}



/*************************************
 *
 *  68000 access to MSP
 *
 *************************************/

READ16_MEMBER( harddriv_state::hd68k_msp_io_r )
{
	UINT16 result;
	offset = (offset / 2) ^ 1;
	m_hd34010_host_access = TRUE;
	result = (m_msp != nullptr) ? m_msp->host_r(space, offset, 0xffff) : 0xffff;
	m_hd34010_host_access = FALSE;
	return result;
}


WRITE16_MEMBER( harddriv_state::hd68k_msp_io_w )
{
	offset = (offset / 2) ^ 1;
	if (m_msp != nullptr)
	{
		m_hd34010_host_access = TRUE;
		m_msp->host_w(space, offset, data, 0xffff);
		m_hd34010_host_access = FALSE;
	}
}



/*************************************
 *
 *  68000 input handlers
 *
 *************************************/

READ16_MEMBER( harddriv_state::hd68k_a80000_r )
{
	return m_a80000->read();
}

READ16_MEMBER( harddriv_state::hd68k_port0_r )
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

	int temp = ((m_sw1 ? m_sw1->read() : 0xff) << 8) | m_in0->read();
	if (get_hblank(scr)) temp ^= 0x0002;
	temp ^= 0x0018;     /* both EOCs always high for now */
	return temp;
}


READ16_MEMBER( harddriv_state::hdc68k_port1_r )
{
	UINT16 result = m_a80000->read();
	UINT16 diff = result ^ m_hdc68k_last_port1;

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


READ16_MEMBER( harddriv_state::hda68k_port1_r )
{
	UINT16 result = m_a80000->read();

	/* merge in the wheel edge latch bit */
	if (m_hdc68k_wheel_edge)
		result ^= 0x4000;

	return result;
}


READ16_MEMBER( harddriv_state::hdc68k_wheel_r )
{
	/* grab the new wheel value and upconvert to 12 bits */
	UINT16 new_wheel = (m_12badc[0] ? m_12badc[0]->read() : 0xffff) << 4;

	/* hack to display the wheel position */
	if (space.machine().input().code_pressed(KEYCODE_LSHIFT))
		popmessage("%04X", new_wheel);

	/* if we crossed the center line, latch the edge bit */
	if ((m_hdc68k_last_wheel / 0xf0) != (new_wheel / 0xf0))
		m_hdc68k_wheel_edge = 1;

	/* remember the last value and return the low 8 bits */
	m_hdc68k_last_wheel = new_wheel;
	return (new_wheel << 8) | 0xff;
}


READ16_MEMBER( harddriv_state::hd68k_adc8_r )
{
	return m_adc8_data;
}


READ16_MEMBER( harddriv_state::hd68k_adc12_r )
{
	return m_adc12_byte ? ((m_adc12_data >> 8) & 0x0f) : (m_adc12_data & 0xff);
}


READ16_MEMBER( harddriv_state::hd68k_sound_reset_r )
{
	if (m_jsa != nullptr)
		m_jsa->reset();
	return ~0;
}



/*************************************
 *
 *  68000 output handlers
 *
 *************************************/

WRITE16_MEMBER( harddriv_state::hd68k_adc_control_w )
{
	COMBINE_DATA(&m_adc_control);

	/* handle a write to the 8-bit ADC address select */
	if (m_adc_control & 0x08)
	{
		m_adc8_select = m_adc_control & 0x07;
		m_adc8_data = m_8badc[m_adc8_select] ? m_8badc[m_adc8_select]->read() : 0xffff;
	}

	/* handle a write to the 12-bit ADC address select */
	if (m_adc_control & 0x40)
	{
		m_adc12_select = (m_adc_control >> 4) & 0x03;
		m_adc12_data = (m_12badc[m_adc12_select] ? m_12badc[m_adc12_select]->read() : 0xffff) << 4;
	}

	/* bit 7 selects which byte of the 12 bit data to read */
	m_adc12_byte = (m_adc_control >> 7) & 1;
}


WRITE16_MEMBER( harddriv_state::hd68k_wr0_write )
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 1: /* SEL1 */
		case 2: /* SEL2 */
		case 3: /* SEL3 */
		case 4: /* SEL4 */
		default:
			/* just ignore */
			break;

		case 6: /* CC1 */
		case 7: /* CC2 */
			coin_counter_w(space.machine(), offset - 6, data);
			break;
	}
}


WRITE16_MEMBER( harddriv_state::hd68k_wr1_write )
{
	if (offset == 0) { //   logerror("Shifter Interface Latch = %02X\n", data);
	} else {                logerror("/WR1(%04X)=%02X\n", offset, data);
	}
}


WRITE16_MEMBER( harddriv_state::hd68k_wr2_write )
{
	if (offset == 0) { //   logerror("Steering Wheel Latch = %02X\n", data);
	} else {                logerror("/WR2(%04X)=%02X\n", offset, data);
	}
}


WRITE16_MEMBER( harddriv_state::hd68k_nwr_w )
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0: /* CR2 */
		case 1: /* CR1 */
			set_led_status(space.machine(), offset, data);
			break;
		case 2: /* LC1 */
			break;
		case 3: /* LC2 */
			break;
		case 4: /* ZP1 */
			m_m68k_zp1 = data;
			break;
		case 5: /* ZP2 */
			m_m68k_zp2 = data;
			break;
		case 6: /* /GSPRES */
			logerror("Write to /GSPRES(%d)\n", data);
			if (m_gsp != nullptr)
				m_gsp->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
		case 7: /* /MSPRES */
			logerror("Write to /MSPRES(%d)\n", data);
			if (m_msp != nullptr)
				m_msp->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
			break;
	}
}


WRITE16_MEMBER( harddriv_state::hdc68k_wheel_edge_reset_w )
{
	/* reset the edge latch */
	m_hdc68k_wheel_edge = 0;
}



/*************************************
 *
 *  68000 ZRAM access
 *
 *************************************/

READ16_MEMBER( harddriv_state::hd68k_zram_r )
{
	UINT16 data = 0;

	if (ACCESSING_BITS_0_7)
		data |= m_210e->read(space, offset, mem_mask);

	if (ACCESSING_BITS_8_15)
		data |= m_200e->read(space, offset, mem_mask >> 8) << 8;

	return data;
}


WRITE16_MEMBER( harddriv_state::hd68k_zram_w )
{
	if (m_m68k_zp1 == 0 && m_m68k_zp2 == 1)
	{
		if (ACCESSING_BITS_0_7)
			m_210e->write(space, offset, data, mem_mask);

		if (ACCESSING_BITS_8_15)
			m_200e->write(space, offset, data >> 8, mem_mask >> 8);
	}
}



/*************************************
 *
 *  68681 DUART
 *
 *************************************/

WRITE_LINE_MEMBER(harddriv_state::harddriv_duart_irq_handler)
{
	m_duart_irq_state = state;
	update_interrupts();
}


/*************************************
 *
 *  GSP I/O register writes
 *
 *************************************/

WRITE16_MEMBER( harddriv_state::hdgsp_io_w )
{
	/* detect an enabling of the shift register and force yielding */
	if (offset == REG_DPYCTL)
	{
		UINT8 new_shiftreg = (data >> 11) & 1;
		if (new_shiftreg != m_last_gsp_shiftreg)
		{
			m_last_gsp_shiftreg = new_shiftreg;
			if (new_shiftreg)
				space.device().execute().yield();
		}
	}

	screen_device &scr = m_gsp->screen();

	/* detect changes to HEBLNK and HSBLNK and force an update before they change */
	if ((offset == REG_HEBLNK || offset == REG_HSBLNK) && data != m_gsp->io_register_r(space, offset, 0xffff))
		scr.update_partial(scr.vpos() - 1);

	m_gsp->io_register_w(space, offset, data, mem_mask);
}



/*************************************
 *
 *  GSP protection workarounds
 *
 *************************************/

WRITE16_MEMBER( harddriv_state::hdgsp_protection_w )
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

READ16_MEMBER( harddriv_state::hd68k_adsp_program_r )
{
	UINT32 word = m_adsp_pgm_memory[offset/2];
	return (!(offset & 1)) ? (word >> 16) : (word & 0xffff);
}


WRITE16_MEMBER( harddriv_state::hd68k_adsp_program_w )
{
	UINT32 *base = &m_adsp_pgm_memory[offset/2];
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

READ16_MEMBER( harddriv_state::hd68k_adsp_data_r )
{
	return m_adsp_data_memory[offset];
}


WRITE16_MEMBER( harddriv_state::hd68k_adsp_data_w )
{
	COMBINE_DATA(&m_adsp_data_memory[offset]);

	/* any write to $1FFF is taken to be a trigger; synchronize the CPUs */
	if (offset == 0x1fff)
	{
		logerror("%06X:ADSP sync address written (%04X)\n", space.device().safe_pcbase(), data);
		space.machine().scheduler().synchronize();
		m_adsp->signal_interrupt_trigger();
	}
	else
		logerror("%06X:ADSP W@%04X (%04X)\n", space.device().safe_pcbase(), offset, data);
}



/*************************************
 *
 *  68000 access to ADSP output memory
 *
 *************************************/

READ16_MEMBER( harddriv_state::hd68k_adsp_buffer_r )
{
	/*  logerror("hd68k_adsp_buffer_r(%04X)\n", offset);*/
	return m_som_memory[m_m68k_adsp_buffer_bank * 0x2000 + offset];
}


WRITE16_MEMBER( harddriv_state::hd68k_adsp_buffer_w )
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
			INT16 *base = (INT16 *)&m_som_memory[param * 0x2000];
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

	m_m68k_adsp_buffer_bank = param;
	logerror("ADSP bank = %d\n", param);
}


WRITE16_MEMBER( harddriv_state::hd68k_adsp_control_w )
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
			space.machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_state::deferred_adsp_bank_switch),this), val);
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
				space.device().execute().spin();
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
				space.device().execute().spin();
			}
			break;

		case 7:
			logerror("ADSP reset = %d\n", val);
			m_adsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			space.device().execute().yield();
			break;

		default:
			logerror("ADSP control %02X = %04X\n", offset, data);
			break;
	}
}


WRITE16_MEMBER( harddriv_state::hd68k_adsp_irq_clear_w )
{
	logerror("%06X:68k clears ADSP interrupt\n", space.device().safe_pcbase());
	m_adsp_irq_state = 0;
	update_interrupts();
}


READ16_MEMBER( harddriv_state::hd68k_adsp_irq_state_r )
{
	int result = 0xfffd;
	if (m_adsp_xflag) result ^= 2;
	if (m_adsp_irq_state) result ^= 1;
	logerror("%06X:68k reads ADSP interrupt state = %04x\n", space.device().safe_pcbase(), result);
	return result;
}



/*************************************
 *
 *  ADSP memory-mapped I/O
 *
 *************************************/

READ16_MEMBER( harddriv_state::hdadsp_special_r )
{
	switch (offset & 7)
	{
		case 0: /* /SIMBUF */
			if (m_adsp_eprom_base + m_adsp_sim_address < m_sim_memory_size)
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
			logerror("%04X:hdadsp_special_r(%04X)\n", space.device().safe_pcbase(), offset);
			break;
	}
	return 0;
}


WRITE16_MEMBER( harddriv_state::hdadsp_special_w )
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
			logerror("%04X:ADSP signals interrupt\n", space.device().safe_pcbase());
			m_adsp_irq_state = 1;
			update_interrupts();
			break;

		case 7: /* /MP */
			m_adsp_eprom_base = 0x10000 * data;
			break;

		default:
			logerror("%04X:hdadsp_special_w(%04X)=%04X\n", space.device().safe_pcbase(), offset, data);
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


WRITE16_MEMBER( harddriv_state::hd68k_ds3_control_w )
{
	int val = (offset >> 3) & 1;

	switch (offset & 7)
	{
		case 0:
			/* SRES - reset sound CPU */
			if (m_ds3sdsp)
			{
				m_ds3sdsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
				m_ds3sdsp->load_boot_data(m_ds3sdsp->region()->base(), m_ds3sdsp_pgm_memory);

				if (val && !m_ds3_sreset)
				{
					m_ds3_sflag = 0;
					m_ds3_scmd = 0;
					m_ds3_sfirqs = 0;
					m_ds3_s68irqs = !m_ds3_sfirqs;
					update_ds3_sirq();
				}
				m_ds3_sreset = val;
				space.device().execute().yield();
			}
			break;

		case 1:
			/* XRES - reset sound helper CPU */
			if (m_ds3xdsp)
			{
				m_ds3xdsp->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
				m_ds3xdsp->load_boot_data(m_ds3xdsp->region()->base(), m_ds3xdsp_pgm_memory);
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
				space.device().execute().spin();
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
			space.device().execute().yield();
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

READ16_MEMBER( harddriv_state::hd68k_ds3_girq_state_r )
{
	int result = 0x0fff;
	if (m_ds3_g68flag) result ^= 0x8000;
	if (m_ds3_gflag) result ^= 0x4000;
	if (m_ds3_g68irqs) result ^= 0x2000;
	if (!m_adsp_irq_state) result ^= 0x1000;
	return result;
}


READ16_MEMBER( harddriv_state::hd68k_ds3_gdata_r )
{
	offs_t pc = space.device().safe_pc();

	m_ds3_gflag = 0;
	update_ds3_irq();

	logerror("%06X:hd68k_ds3_gdata_r(%04X)\n", space.device().safe_pcbase(), m_ds3_gdata);

	/* attempt to optimize the transfer if conditions are right */
	if (&space.device() == m_maincpu && pc == m_ds3_transfer_pc &&
		!(!m_ds3_g68flag && m_ds3_g68irqs) && !(m_ds3_gflag && m_ds3_gfirqs))
	{
		UINT32 destaddr = m_maincpu->state_int(M68K_A1);
		UINT16 count68k = m_maincpu->state_int(M68K_D1);
		UINT16 mstat = m_adsp->state_int(ADSP2100_MSTAT);
		UINT16 i6 = m_adsp->state_int((mstat & 1) ? ADSP2100_MR0 : ADSP2100_MR0_SEC);
		UINT16 l6 = m_adsp->state_int(ADSP2100_L6) - 1;
		UINT16 m7 = m_adsp->state_int(ADSP2100_M7);

		logerror("%06X:optimizing 68k transfer, %d words\n", m_maincpu->pcbase(), count68k);

		while (count68k > 0 && m_adsp_data_memory[0x16e6] > 0)
		{
			space.write_word(destaddr, m_ds3_gdata);
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
	space.device().execute().spin_until_trigger(DS3_TRIGGER);
	space.machine().scheduler().trigger(DS3_TRIGGER, attotime::from_usec(5));

	return m_ds3_gdata;
}


WRITE16_MEMBER( harddriv_state::hd68k_ds3_gdata_w )
{
	logerror("%06X:hd68k_ds3_gdata_w(%04X)\n", space.device().safe_pcbase(), m_ds3_gdata);

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

WRITE16_MEMBER( harddriv_state::hd68k_ds3_sirq_clear_w )
{
	logerror("%06X:68k clears ADSP interrupt\n", space.device().safe_pcbase());
	m_sound_int_state = 0;
	update_interrupts();
}


READ16_MEMBER( harddriv_state::hd68k_ds3_sirq_state_r )
{
	int result = 0x0fff;
	if (m_ds3_s68flag) result ^= 0x8000;
	if (m_ds3_sflag) result ^= 0x4000;
	if (m_ds3_s68irqs) result ^= 0x2000;
	if (!m_sound_int_state) result ^= 0x1000;
	return result;
}


READ16_MEMBER( harddriv_state::hd68k_ds3_sdata_r )
{
	m_ds3_sflag = 0;
	update_ds3_sirq();

	/* if we just cleared the IRQ, we are going to do some VERY timing critical reads */
	/* it is important that all the CPUs be in sync before we continue, so spin a little */
	/* while to let everyone else catch up */
	space.device().execute().spin_until_trigger(DS3_STRIGGER);
	space.machine().scheduler().trigger(DS3_STRIGGER, attotime::from_usec(5));

	return m_ds3_sdata;
}


WRITE16_MEMBER( harddriv_state::hd68k_ds3_sdata_w )
{
	COMBINE_DATA(&m_ds3_s68data);
	m_ds3_s68flag = 1;
	m_ds3_scmd = offset & 1;
	m_ds3sdsp->signal_interrupt_trigger();

	update_ds3_sirq();
}


READ16_MEMBER( harddriv_state::hdds3_sdsp_special_r )
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


WRITE16_MEMBER( harddriv_state::hdds3_sdsp_special_w )
{
	/* Note: DS IV is slightly different */
	switch (offset & 7)
	{
		case 0:
			m_ds3_sdata = data;
			m_ds3_sflag = 1;
			update_ds3_sirq();

			/* once we've written data, trigger the main CPU to wake up again */
			space.machine().scheduler().trigger(DS3_STRIGGER);
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
			m_ds3dac1->write_signed16(data);
			break;

		case 5:
			m_ds3dac2->write_signed16(data);
			break;

		case 6:
			m_ds3_sdata_address = (m_ds3_sdata_address & 0xffff0000) | (data & 0xffff);
			break;

		case 7:
			m_ds3_sdata_address = (m_ds3_sdata_address & 0x0000ffff) | (data << 16);
			break;
	}
}

READ16_MEMBER( harddriv_state::hdds3_sdsp_control_r )
{
	switch (offset)
	{
		default:
			return m_ds3sdsp_regs[offset];
	}
}


WRITE16_MEMBER( harddriv_state::hdds3_sdsp_control_w )
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
				UINT32 page = (data >> 6) & 7;
				m_ds3sdsp->load_boot_data(m_ds3sdsp->region()->base() + (0x2000 * page), m_ds3sdsp_pgm_memory);
				m_ds3sdsp->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
				data &= ~0x200;
			}

			m_ds3sdsp_regs[0x1f] = data;
			break;

		default:
			m_ds3sdsp_regs[offset] = data;
			break;
	}
}


READ16_MEMBER( harddriv_state::hdds3_xdsp_control_r )
{
	switch (offset)
	{
		default:
			return m_ds3xdsp_regs[offset];
	}

	// never executed
	//return 0xff;
}


WRITE16_MEMBER( harddriv_state::hdds3_xdsp_control_w )
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
	UINT16 period = m_ds3sdsp_regs[0x1d];
	UINT16 scale = m_ds3sdsp_regs[0x1b] + 1;

	m_ds3sdsp_internal_timer->adjust(m_ds3sdsp->cycles_to_attotime(period * scale));

	/* the IRQ line is edge triggered */
	m_ds3sdsp->set_input_line(ADSP2105_TIMER, ASSERT_LINE);
	m_ds3sdsp->set_input_line(ADSP2105_TIMER, CLEAR_LINE);
}


void harddriv_state::hdds3sdsp_reset_timer()
{
	if (!m_ds3sdsp_timer_en)
		return;

	UINT16 count = m_ds3sdsp_regs[0x1c];
	UINT16 scale = m_ds3sdsp_regs[0x1b] + 1;

	m_ds3sdsp_internal_timer->adjust(m_ds3sdsp->cycles_to_attotime(count * scale));
}

WRITE_LINE_MEMBER(harddriv_state::hdds3sdsp_timer_enable_callback)
{
	m_ds3sdsp_timer_en = state;

	if (state)
		hdds3sdsp_reset_timer();
	else
		m_ds3sdsp_internal_timer->adjust(attotime::never);
}


TIMER_DEVICE_CALLBACK_MEMBER( harddriv_state::ds3xdsp_internal_timer_callback )
{
	UINT16 period = m_ds3xdsp_regs[0x1d];
	UINT16 scale = m_ds3xdsp_regs[0x1b] + 1;

	m_ds3xdsp_internal_timer->adjust(m_ds3xdsp->cycles_to_attotime(period * scale));

	/* the IRQ line is edge triggered */
	m_ds3xdsp->set_input_line(ADSP2105_TIMER, ASSERT_LINE);
	m_ds3xdsp->set_input_line(ADSP2105_TIMER, CLEAR_LINE);
}


void harddriv_state::hdds3xdsp_reset_timer()
{
	if (!m_ds3xdsp_timer_en)
		return;

	UINT16 count = m_ds3xdsp_regs[0x1c];
	UINT16 scale = m_ds3xdsp_regs[0x1b] + 1;

	m_ds3xdsp_internal_timer->adjust(m_ds3xdsp->cycles_to_attotime(count * scale));
}


WRITE_LINE_MEMBER(harddriv_state::hdds3xdsp_timer_enable_callback)
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
TIMER_CALLBACK_MEMBER(harddriv_state::xsdp_sport1_irq_off_callback)
{
	m_ds3xdsp->set_input_line(ADSP2105_SPORT1_RX, CLEAR_LINE);
}


WRITE32_MEMBER(harddriv_state::hdds3sdsp_serial_tx_callback)
{
	if ((m_ds3sdsp_regs[0x1f] & 0xc00) != 0xc00)
		return;

	m_ds3sdsp_sdata = data;

	m_ds3xdsp->set_input_line(ADSP2105_SPORT1_RX, ASSERT_LINE);
	machine().scheduler().timer_set(attotime::from_nsec(200), timer_expired_delegate(FUNC(harddriv_state::xsdp_sport1_irq_off_callback), this));
}


READ32_MEMBER(harddriv_state::hdds3sdsp_serial_rx_callback)
{
	if ((m_ds3sdsp_regs[0x1f] & 0xc00) != 0xc00)
		return 0xff;

	return m_ds3xdsp_sdata;
}


WRITE32_MEMBER(harddriv_state::hdds3xdsp_serial_tx_callback)
{
	if ((m_ds3xdsp_regs[0x1f] & 0xc00) != 0xc00)
		return;

	m_ds3xdsp_sdata = data;
}


READ32_MEMBER(harddriv_state::hdds3xdsp_serial_rx_callback)
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

READ16_MEMBER( harddriv_state::hdds3_special_r )
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
			if (m_ds3_sim_address < m_sim_memory_size)
				return m_sim_memory[m_ds3_sim_address];
			else
				return 0xff;
	}
	return 0;
}


WRITE16_MEMBER( harddriv_state::hdds3_special_w )
{
	/* IMPORTANT! these data values also write through to the underlying RAM */
	m_adsp_data_memory[offset] = data;

	switch (offset & 7)
	{
		case 0:
			logerror("%04X:ADSP sets gdata to %04X\n", space.device().safe_pcbase(), data);
			m_ds3_gdata = data;
			m_ds3_gflag = 1;
			update_ds3_irq();

			/* once we've written data, trigger the main CPU to wake up again */
			space.machine().scheduler().trigger(DS3_TRIGGER);
			break;

		case 1:
			logerror("%04X:ADSP sets interrupt = %d\n", space.device().safe_pcbase(), (data >> 1) & 1);
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


READ16_MEMBER( harddriv_state::hdds3_control_r )
{
	logerror("adsp2101 control r @ %04X\n", 0x3fe0 + offset);
	return 0;
}


WRITE16_MEMBER( harddriv_state::hdds3_control_w )
{
	if (offset != 0x1e && offset != 0x1f)
		logerror("adsp2101 control w @ %04X = %04X\n", 0x3fe0 + offset, data);
}



/*************************************
 *
 *  DS III program memory handlers
 *
 *************************************/

READ16_MEMBER( harddriv_state::hd68k_ds3_program_r )
{
	UINT32 *base = &m_adsp_pgm_memory[offset & 0x1fff];
	UINT32 word = *base;
	return (!(offset & 0x2000)) ? (word >> 8) : (word & 0xff);
}


WRITE16_MEMBER( harddriv_state::hd68k_ds3_program_w )
{
	UINT32 *base = &m_adsp_pgm_memory[offset & 0x1fff];
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

WRITE32_MEMBER(harddriv_state::hddsk_update_pif)
{
	m_sound_int_state = ((data & DSP32_OUTPUT_PIF) != 0);
	update_interrupts();
}



/*************************************
 *
 *  DSK board control handlers
 *
 *************************************/

WRITE16_MEMBER( harddriv_state::hd68k_dsk_control_w )
{
	int val = (offset >> 3) & 1;
	switch (offset & 7)
	{
		case 0: /* DSPRESTN */
			if (m_dsp32) m_dsp32->set_input_line(INPUT_LINE_RESET, val ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 1: /* DSPZN */
			if (m_dsp32) m_dsp32->set_input_line(INPUT_LINE_HALT, val ? CLEAR_LINE : ASSERT_LINE);
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

READ16_MEMBER( harddriv_state::hd68k_dsk_ram_r )
{
	return m_dsk_ram[offset];
}


WRITE16_MEMBER( harddriv_state::hd68k_dsk_ram_w )
{
	COMBINE_DATA(&m_dsk_ram[offset]);
}


READ16_MEMBER( harddriv_state::hd68k_dsk_small_rom_r )
{
	return m_dsk_rom[offset & 0x1ffff];
}


READ16_MEMBER( harddriv_state::hd68k_dsk_rom_r )
{
	return m_dsk_rom[offset];
}



/*************************************
 *
 *  DSK board DSP32C I/O handlers
 *
 *************************************/

WRITE16_MEMBER( harddriv_state::hd68k_dsk_dsp32_w )
{
	m_dsk_pio_access = TRUE;
	if (m_dsp32) m_dsp32->pio_w(offset, data);
	m_dsk_pio_access = FALSE;
}


READ16_MEMBER( harddriv_state::hd68k_dsk_dsp32_r )
{
	UINT16 result;
	m_dsk_pio_access = TRUE;
	if (m_dsp32) result = m_dsp32->pio_r(offset);
	else result = 0x00;

	m_dsk_pio_access = FALSE;
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


WRITE32_MEMBER( harddriv_state::rddsp32_sync0_w )
{
	if (m_dsk_pio_access)
	{
		UINT32 *dptr = &m_rddsp32_sync[0][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		m_dataptr[m_next_msp_sync % MAX_MSP_SYNC] = dptr;
		m_dataval[m_next_msp_sync % MAX_MSP_SYNC] = newdata;
		space.machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_state::rddsp32_sync_cb),this), m_next_msp_sync++ % MAX_MSP_SYNC);
	}
	else
		COMBINE_DATA(&m_rddsp32_sync[0][offset]);
}


WRITE32_MEMBER( harddriv_state::rddsp32_sync1_w )
{
	if (m_dsk_pio_access)
	{
		UINT32 *dptr = &m_rddsp32_sync[1][offset];
		UINT32 newdata = *dptr;
		COMBINE_DATA(&newdata);
		m_dataptr[m_next_msp_sync % MAX_MSP_SYNC] = dptr;
		m_dataval[m_next_msp_sync % MAX_MSP_SYNC] = newdata;
		space.machine().scheduler().synchronize(timer_expired_delegate(FUNC(harddriv_state::rddsp32_sync_cb),this), m_next_msp_sync++ % MAX_MSP_SYNC);
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

WRITE16_MEMBER( harddriv_state::hddspcom_control_w )
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
 *  Race Drivin' slapstic handling
 *
 *************************************/

WRITE16_MEMBER( harddriv_state::rd68k_slapstic_w )
{
	m_slapstic_device->slapstic_tweak(space, offset & 0x3fff);
}


READ16_MEMBER( harddriv_state::rd68k_slapstic_r )
{
	int bank = m_slapstic_device->slapstic_tweak(space, offset & 0x3fff) * 0x4000;
	return m_m68k_slapstic_base[bank + (offset & 0x3fff)];
}



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


WRITE16_MEMBER( harddriv_state::st68k_sloop_w )
{
	st68k_sloop_tweak(offset & 0x3fff);
}


READ16_MEMBER( harddriv_state::st68k_sloop_r )
{
	int bank = st68k_sloop_tweak(offset) * 0x4000;
	return m_m68k_slapstic_base[bank + (offset & 0x3fff)];
}


READ16_MEMBER( harddriv_state::st68k_sloop_alt_r )
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


WRITE16_MEMBER( harddriv_state::st68k_protosloop_w )
{
	st68k_protosloop_tweak(offset & 0x3fff);
}


READ16_MEMBER( harddriv_state::st68k_protosloop_r )
{
	int bank = st68k_protosloop_tweak(offset) * 0x4000;
	return m_m68k_slapstic_base[bank + (offset & 0x3fff)];
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

READ16_MEMBER( harddriv_state::hdgsp_speedup_r )
{
	int result = m_gsp_speedup_addr[0][offset];

	/* if both this address and the other important address are not $ffff */
	/* then we can spin until something gets written */
	if (result != 0xffff && m_gsp_speedup_addr[1][0] != 0xffff &&
		&space.device() == m_gsp && space.device().safe_pc() == m_gsp_speedup_pc)
	{
		m_gsp_speedup_count[0]++;
		space.device().execute().spin_until_interrupt();
	}

	return result;
}


WRITE16_MEMBER( harddriv_state::hdgsp_speedup1_w )
{
	COMBINE_DATA(&m_gsp_speedup_addr[0][offset]);

	/* if $ffff is written, send an "interrupt" trigger to break us out of the spin loop */
	if (m_gsp_speedup_addr[0][offset] == 0xffff)
		m_gsp->signal_interrupt_trigger();
}


WRITE16_MEMBER( harddriv_state::hdgsp_speedup2_w )
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

READ16_MEMBER( harddriv_state::rdgsp_speedup1_r )
{
	UINT16 result = m_gsp_speedup_addr[0][offset];

	/* if this address is equal to $f000, spin until something gets written */
	if (&space.device() == m_gsp && space.device().safe_pc() == m_gsp_speedup_pc &&
		(UINT8)result < space.device().state().state_int(TMS34010_A1))
	{
		m_gsp_speedup_count[0]++;
		space.device().execute().spin_until_interrupt();
	}

	return result;
}


WRITE16_MEMBER( harddriv_state::rdgsp_speedup1_w )
{
	COMBINE_DATA(&m_gsp_speedup_addr[0][offset]);
	if (&space.device() != m_gsp)
		m_gsp->signal_interrupt_trigger();
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

READ16_MEMBER( harddriv_state::hdmsp_speedup_r )
{
	int data = m_msp_speedup_addr[offset];

	if (data == 0 && &space.device() == m_msp && space.device().safe_pc() == m_msp_speedup_pc)
	{
		m_msp_speedup_count[0]++;
		space.device().execute().spin_until_interrupt();
	}

	return data;
}


WRITE16_MEMBER( harddriv_state::hdmsp_speedup_w )
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

READ16_MEMBER( harddriv_state::hdadsp_speedup_r )
{
	int data = m_adsp_data_memory[0x1fff];

	if (data == 0xffff && &space.device() == m_adsp && space.device().safe_pc() <= 0x3b)
	{
		m_adsp_speedup_count[0]++;
		space.device().execute().spin_until_interrupt();
	}

	return data;
}


READ16_MEMBER( harddriv_state::hdds3_speedup_r )
{
	int data = *m_ds3_speedup_addr;

	if (data != 0 && &space.device() == m_adsp && space.device().safe_pc() == m_ds3_speedup_pc)
	{
		m_adsp_speedup_count[2]++;
		space.device().execute().spin_until_interrupt();
	}

	return data;
}
