// license:BSD-3-Clause
// copyright-holders:Robert Justice,68bit
/***************************************************************************
    swtpc09 machine file
    Robert Justice ,2009-2014

****************************************************************************/

#include "emu.h"
#include "swtpc09.h"

#define DMAC_IRQ 0x01             // interrupt handler IDs
#define PTM_IRQ 0x04
#define PIA_IRQ 0x08
#define FDC_IRQ 0x10
#define VIA_IRQ 0x20
#define IO_IRQ 0x40

#define FLEX_DMAF2 1               // system type flags
#define UNIFLEX_DMAF2 2
#define UNIFLEX_DMAF3 3
#define FLEX_DC5_PIAIDE 4
#define OS9_DC5 5


uint8_t swtpc09_state::unmapped_r(offs_t offset)
{
	if (!machine().side_effects_disabled()) {
		logerror("%s Unmapped read from addr %04x\n", machine().describe_context(), offset);
	}
	return 0;
}

void swtpc09_state::unmapped_w(offs_t offset, uint8_t data)
{
	logerror("%s Unmapped write to addr %04x with data %02x\n", machine().describe_context(), offset, data);
}

WRITE_LINE_MEMBER(swtpc09_state::io_irq_w)
{
	if (state)
		swtpc09_irq_handler(IO_IRQ, ASSERT_LINE);
	else
		swtpc09_irq_handler(IO_IRQ, CLEAR_LINE);
}

/******* MC6840 PTM on MPID Board *******/

// 6840 PTM handlers
WRITE_LINE_MEMBER( swtpc09_state::ptm_o1_callback )
{
	m_pia_counter++;
	//pia_counter = pia_counter && 0xff;
	if (m_pia_counter & 0x80) m_pia->ca1_w(1);
}

WRITE_LINE_MEMBER( swtpc09_state::ptm_o3_callback )
{
	// the output from timer3 is the input clock for timer2
	//m_ptm->set_c2(state);
}

WRITE_LINE_MEMBER( swtpc09_state::ptm_irq )
{
	if (state)
		swtpc09_irq_handler(PTM_IRQ, ASSERT_LINE);
	else
		swtpc09_irq_handler(PTM_IRQ, CLEAR_LINE);
}

/******* MC6821 PIA on MPID Board *******/
/* Read/Write handlers for pia */

uint8_t swtpc09_state::pia0_a_r()
{
	return m_pia_counter;
}

WRITE_LINE_MEMBER( swtpc09_state::pia0_irq_a )
{
	if ( m_pia->irq_a_state())
		swtpc09_irq_handler(PIA_IRQ, ASSERT_LINE);
	else
		swtpc09_irq_handler(PIA_IRQ, CLEAR_LINE);
}


/* Shared floppy support. */

void swtpc09_state::floppy_motor_trigger()
{
	m_floppy0->get_device()->mon_w(CLEAR_LINE);
	m_floppy1->get_device()->mon_w(CLEAR_LINE);
	m_floppy2->get_device()->mon_w(CLEAR_LINE);
	m_floppy3->get_device()->mon_w(CLEAR_LINE);
	m_floppy_motor_timer->adjust(attotime::from_msec(30000));
	m_floppy_motor_on = 1;
}

TIMER_CALLBACK_MEMBER(swtpc09_state::floppy_motor_callback)
{
	m_floppy0->get_device()->mon_w(ASSERT_LINE);
	m_floppy1->get_device()->mon_w(ASSERT_LINE);
	m_floppy2->get_device()->mon_w(ASSERT_LINE);
	m_floppy3->get_device()->mon_w(ASSERT_LINE);
	m_floppy_motor_on = 0;
}

// Hack On a FDC command write, check that the floppy side is as expected
// given the track and sector. This check is performed for the type II and III
// commands. The floppy side is modified if necessary.
void swtpc09_state::validate_floppy_side(uint8_t cmd)
{
	if ((cmd & 0xe1) == 0x80 || (cmd & 0xe0) == 0xa0 ||
		(cmd & 0xf9) == 0xc0 || (cmd & 0xf9) == 0xe0 ||
		(cmd & 0xf9) == 0xf0)
	{
		uint32_t expected_sectors = m_floppy_expected_sectors->read();
		uint32_t track_zero_expected_sectors = m_floppy_track_zero_expected_sectors->read();
		uint8_t sector = m_fdc->sector_r();
		uint8_t track = m_fdc->track_r();

		if (track_zero_expected_sectors && track == 0)
		{
			uint8_t expected_side = sector > track_zero_expected_sectors ? 1 : 0;

			if (m_fdc_side != expected_side)
			{
				logerror("%s Unexpected size %d for track %d sector %d expected side %d\n", machine().describe_context(), m_fdc_side, track, sector, expected_side);
				if (m_fdc_floppy)
				{
					m_fdc_floppy->ss_w(expected_side);
					m_fdc_side = expected_side;
				}
			}
		}
		else if (expected_sectors)
		{
			uint8_t expected_side = sector > expected_sectors ? 1 : 0;

			if (m_fdc_side != expected_side)
			{
				logerror("%s Unexpected side %d for track %d sector %d expected side %d\n", machine().describe_context(), m_fdc_side, track, sector, expected_side);
				if (m_fdc_floppy)
				{
					m_fdc_floppy->ss_w(expected_side);
					m_fdc_side = expected_side;
				}
			}
		}
	}
}

// Note the dden line is low for double density.
uint8_t swtpc09_state::validate_fdc_dden(uint8_t dden)
{
	uint8_t expected_density = m_floppy_expected_density->read();
	switch (expected_density)
	{
		case 1:
			// Single density.
			if (!dden)
				logerror("%s Unexpected DDEN %d for single density\n", machine().describe_context(), dden);
			return 1;
		case 2:
		{
			// Double density with track zero head zero single density.
			uint8_t track = m_fdc->track_r();

			if (track == 0 && m_fdc_side == 0)
			{
				if (!dden)
					logerror("%s Unexpected DDEN %d for single density track 0 head 0\n", machine().describe_context(), dden);
				return 1;
			}
			if (dden)
				logerror("%s Unexpected DDEN %d for double density\n", machine().describe_context(), dden);
			return 0;
		}
		case 3:
		{
			// Double density with track zero all heads single density.
			uint8_t track = m_fdc->track_r();

			if (track == 0)
			{
				if (!dden)
					logerror("%s Unexpected DDEN %d for single density track 0\n", machine().describe_context(), dden);
				return 1;
			}
			if (dden)
				logerror("%s Unexpected DDEN %d for double density\n", machine().describe_context(), dden);
			return 0;
		}
		case 4:
			// Pure double density.
			if (dden)
				logerror("%s Unexpected DDEN %d for double density\n", machine().describe_context(), dden);
			return 0;
		default:
			return dden;
	}
}

// The WD2797 supports an alternate interpretation of the sector size. Check
// that the flag is as expected and return the corrected command if necessary.
uint8_t swtpc09_state::validate_fdc_sector_size(uint8_t cmd)
{
	if ((cmd & 0xe1) == 0x80 || (cmd & 0xe0) == 0xa0)
	{
		// Check that the sector length flag is set as expected.
		uint8_t sector_length_default = cmd & 0x08;
		if (sector_length_default != 0x08)
		{
			logerror("%s Unexpected sector length default %02x\n", machine().describe_context(), sector_length_default);
			// Patch the sector length flag.
			cmd |= 0x08;
		}
	}
	return cmd;
}

/*********************************************************************/
/*   DMAF2 Floppy Controller Board                                    */
/*********************************************************************/

uint8_t swtpc09_state::dmaf2_fdc_r(offs_t offset)
{
	// TODO Does access to the dmaf2 fdc also trigger the motor timer?
	if (!machine().side_effects_disabled())
		floppy_motor_trigger();
	return m_fdc->fd1797_device::read(offset);
}

void swtpc09_state::dmaf2_fdc_w(offs_t offset, uint8_t data)
{
	// TODO Does access to the dmaf2 fdc also trigger the motor timer.
	floppy_motor_trigger();

	if (offset == 0) {
		validate_floppy_side(data);
		m_fdc->dden_w(validate_fdc_dden(m_fdc_dden));
		data = validate_fdc_sector_size(data);
	}

	m_fdc->fd1797_device::write(offset, data);
}

/* DMAF2 dma extended address register latch. */

uint8_t swtpc09_state::dmaf2_dma_address_reg_r()
{
	// This does not appear to be readable.
	logerror("%s Unexpected read of DMAF2 DMA address reg\n", machine().describe_context());
	return 0x00;
}

void swtpc09_state::dmaf2_dma_address_reg_w(uint8_t data)
{
	// This latch appears to be write-only, there is no reader function.
	// The DMAF2 has a single latch for the high address bits, so it would
	// appear to apply for all DMA channels. These outputs are inverted
	// which cancels the inversion of the data lines.
	m_dmaf_high_address[0] = data & 0x0f;
	m_dmaf_high_address[1] = data & 0x0f;
	m_dmaf_high_address[2] = data & 0x0f;
	m_dmaf_high_address[3] = data & 0x0f;

	// bit 4 controls a gate enable/disable for DMAF2 fdc irq line
	m_dmaf2_interrupt_enable = !BIT(data, 4);
	if (!m_dmaf2_interrupt_enable)
		swtpc09_irq_handler(FDC_IRQ, CLEAR_LINE); //then clear the irq to cpu
}

/* DMAF2 fdc control register */
uint8_t swtpc09_state::dmaf2_control_reg_r()
{
	// TODO is this readable?
	logerror("%s Unexpected read from DMAF2 control reg\n", machine().describe_context());
	return m_fdc_status;
}

void swtpc09_state::dmaf2_control_reg_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// The DMAF2 data lines are inverted.
	data = ~data & 0xff;

	// TODO what to do if multiple drives are selected?
	if (BIT(data, 0) + BIT(data, 1) + BIT(data, 2) + BIT(data, 3) > 1)
		logerror("%s Unexpected DMAF2 has multiple drives selected: %d %d %d %d\n", machine().describe_context(), BIT(data, 0), BIT(data, 1), BIT(data, 2), BIT(data, 3));

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);
	m_fdc_floppy = floppy;

	if (floppy)
	{
		uint8_t side = BIT(data, 4);
		floppy->ss_w(side);
		m_fdc_side = side;
	}

	uint8_t dden = BIT(data, 5);
	dden = validate_fdc_dden(dden);
	m_fdc->dden_w(dden);
	m_fdc_dden = dden;
}

/* common interrupt handler */
void swtpc09_state::swtpc09_irq_handler(uint8_t peripheral, uint8_t state)
{
	switch (state)
	{
		case ASSERT_LINE:
			m_interrupt |= peripheral;
			break;

		case CLEAR_LINE:
			m_interrupt &= (~peripheral & 0x7f);
			break;
	}

	if (!m_active_interrupt && m_interrupt)    //no active interrupt and it needs to be asserted
	{
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
		m_active_interrupt=true;
	}
	else if (m_active_interrupt && !m_interrupt)  //active interrupt and it needs to be cleared
	{
		m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
		m_active_interrupt=false;
	}
}

/* handlers for fdc */
WRITE_LINE_MEMBER( swtpc09_state::fdc_intrq_w )
{
	if ( m_system_type == UNIFLEX_DMAF3 )
	{
		// IRQ from 1791 is connected into VIA CB2 inverted, and
		// connected to VIA port B bit 2 without inversion.
		if (state)
		{
			m_fdc_status |= 0x40;
			//m_via->write_cb2(0);
			m_via_cb2->in_w<0>(0);
			m_dmaf3_via_portb |= 0x04;
			//m_via->write_portb(m_dmaf3_via_portb);
			//swtpc09_irq_handler(FDC_IRQ, ASSERT_LINE);
		}
		else
		{
			m_fdc_status &= ~0x40;
			//m_via->write_cb2(1);
			m_via_cb2->in_w<0>(1);
			m_dmaf3_via_portb &= 0xfb;
			//m_via->write_portb(m_dmaf3_via_portb);
			//swtpc09_irq_handler(FDC_IRQ, CLEAR_LINE);
		}
	}
	else   //for dmaf2 it is connected directly to cpu via a gate
	{
		if (state)
		{
			m_fdc_status |= 0x40;
			if (m_dmaf2_interrupt_enable)
			{
				swtpc09_irq_handler(FDC_IRQ, ASSERT_LINE);
			}
		}
		else
		{
			m_fdc_status &= ~0x40;
			if (m_dmaf2_interrupt_enable)
			{
				swtpc09_irq_handler(FDC_IRQ, CLEAR_LINE);
			}
		}
	}
}

WRITE_LINE_MEMBER( swtpc09_state::fdc_drq_w )
{
	if (state)
	{
		m_fdc_status |= 0x80;
		// The DMAF2 schematic shows an input to two pins on the 6844,
		// it might also trigger channel 1.
		m6844_fdc_dma_transfer(0);
	}
	else
		m_fdc_status &= 0x7f;
}

WRITE_LINE_MEMBER( swtpc09_state::fdc_sso_w )
{
	// The DMAF2 and DMAF3 do not appear to use a SSO output?
}

/*********************************************************************/
/*   DMAF3 Board                                                      */
/*********************************************************************/

uint8_t swtpc09_state::dmaf3_fdc_r(offs_t offset)
{
	// TODO Does access to the fdc also trigger the motor timer.
	if (!machine().side_effects_disabled())
		floppy_motor_trigger();
	return m_fdc->fd1797_device::read(offset);
}

void swtpc09_state::dmaf3_fdc_w(offs_t offset, uint8_t data)
{
	// TODO Does access to the fdc also trigger the motor timer.
	floppy_motor_trigger();

	if (offset == 0) {
		validate_floppy_side(data);
		m_fdc->dden_w(validate_fdc_dden(m_fdc_dden));
		data = validate_fdc_sector_size(data);
	}

	m_fdc->fd1797_device::write(offset, data);
}

/* via on dmaf3 board */
uint8_t swtpc09_state::dmaf3_via_read_porta()
{
	return m_dmaf3_via_porta;
}

uint8_t swtpc09_state::dmaf3_via_read_portb()
{
	// Bit 0 - output ?
	// Bit 1 - output, tape drive request strobe.
	// Bit 2 - input, WD1791 FDC interrupt.
	// Bit 3 - input ?
	// Bit 4 - input, tape drive ready
	// Bit 5 - input, tape drive exception, timer 2.
	// Bit 6 - input ?
	// Bit 7 - output, UniFLEX configures a timer to toggle this output.

	// Set the tape drive exception bit to avoid detection, otherwise
	// UniFLEX gets stuck in a loop trying to open the archive driver.
	return m_dmaf3_via_portb | 0x20;
}

void swtpc09_state::dmaf3_via_write_porta(uint8_t data)
{
	m_dmaf3_via_porta &= data;
}

void swtpc09_state::dmaf3_via_write_portb(uint8_t data)
{
	m_dmaf3_via_portb &= data;
}

//WRITE_LINE_MEMBER( swtpc09_state::dmaf3_via_write_ca1 )
//{
//  return m_via_ca1_input;
//    logerror("swtpc09_dmaf3_via_write_ca1 %02X\n", state);

//}

WRITE_LINE_MEMBER( swtpc09_state::dmaf3_via_irq )
{
	if (state)
		swtpc09_irq_handler(VIA_IRQ, ASSERT_LINE);
	else
		swtpc09_irq_handler(VIA_IRQ, CLEAR_LINE);
}

/* DMAF3 dma extended address register */
uint8_t swtpc09_state::dmaf3_dma_address_reg_r()
{
	// TODO is this readable?
	logerror("%s Unexpected read of DMAF3 DMA address reg\n", machine().describe_context());
	return 0x00;
}

void swtpc09_state::dmaf3_dma_address_reg_w(uint8_t data)
{
	// Based on source code comments it appears that there are four high
	// address latches, one for each DMA channel. TODO check hardware or a
	// schematic.
	uint8_t channel = (data & 0x30) >> 4;
	m_dmaf_high_address[channel] = data & 0x0f;

	// Bit 6 controls the 'archive edge select'.
	// Bit 7 controls the DMA halt versus bus-req mode.
}

/* DMAF3 fdc control register */
uint8_t swtpc09_state::dmaf3_control_reg_r()
{
	// TODO is this readable?
	logerror("%s Unexpected read from DMAF3 control reg\n", machine().describe_context());
	return m_fdc_status;
}

void swtpc09_state::dmaf3_control_reg_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// TODO multiple selected?
	if (BIT(data, 0) + BIT(data, 1) + BIT(data, 2) + BIT(data, 3) > 1)
		logerror("%s Unexpected DMAF3 has multiple drives selected: %d %d %d %d\n", machine().describe_context(), BIT(data, 0), BIT(data, 1), BIT(data, 2), BIT(data, 3));

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);
	m_fdc_floppy = floppy;

	if (floppy)
	{
		uint8_t side = BIT(data, 4);
		floppy->ss_w(side);
		m_fdc_side = side;
	}

	uint8_t dden = BIT(data, 5);
	dden = validate_fdc_dden(dden);
	m_fdc->dden_w(dden);
	m_fdc_dden = dden;
}

// DMAF3 WD1000 hard disk controller.

WRITE_LINE_MEMBER( swtpc09_state::dmaf3_hdc_intrq_w )
{
	// The IRQ from WD1000 is connected into VIA CB2 inverted, and perhaps
	// connected to a VIA port B bit 3?
	if (state)
	{
		m_via_cb2->in_w<1>(0);
		//m_dmaf3_via_portb &= 0xf7;
		//m_via->write_portb(m_dmaf3_via_portb);
	}
	else
	{
		m_via_cb2->in_w<1>(1);
		//m_dmaf3_via_portb |= 0x08;
		//m_via->write_portb(m_dmaf3_via_portb);
	}
}

WRITE_LINE_MEMBER( swtpc09_state::dmaf3_hdc_drq_w )
{
	if (state)
		m6844_hdc_dma_transfer(1);
}

uint8_t swtpc09_state::dmaf3_hdc_control_r()
{
	// TODO head load toggle?
	return 0;
}

void swtpc09_state::dmaf3_hdc_control_w(uint8_t data)
{
	// TODO head load toggle?
}

uint8_t swtpc09_state::dmaf3_hdc_reset_r()
{
	// TODO reset?
	return 0;
}

void swtpc09_state::dmaf3_hdc_reset_w(uint8_t data)
{
	// TODO reset
}

uint8_t swtpc09_state::dmaf3_archive_reset_r()
{
	// TODO
	return 0;
}

void swtpc09_state::dmaf3_archive_reset_w(uint8_t data)
{
	// TODO
}

uint8_t swtpc09_state::dmaf3_archive_clear_r()
{
	// TODO
	return 0;
}

void swtpc09_state::dmaf3_archive_clear_w(uint8_t data)
{
	// TODO
}


/* DAT ram write handler (Dynamic Address Translator)    */
/* This creates the address map when a page is mapped in */
/* memory map is created based on system_type flag       */
/* this is accommodate the different cards installed     */

offs_t swtpc09_state::dat_translate(offs_t offset) const
{
	// lower 4 bits are inverted
	return offs_t(m_dat[offset >> 12] ^ 0x0f) << 12 | (offset & 0x0fff);
}

uint8_t swtpc09_state::main_r(offs_t offset)
{
	if (offset < 0xff00)
		return m_banked_space->read_byte(dat_translate(offset));
	else if (m_system_type == OS9_DC5)
		return m_banked_space->read_byte(offset | 0x0ff00);
	else
		return m_banked_space->read_byte(offset | 0xfff00);
}

void swtpc09_state::main_w(offs_t offset, uint8_t data)
{
	if (offset < 0xff00)
		m_banked_space->write_byte(dat_translate(offset), data);
	else if (m_system_type == OS9_DC5)
		m_banked_space->write_byte(offset | 0x0ff00, data);
	else
		m_banked_space->write_byte(offset | 0xfff00, data);
}

/*  MC6844 DMA controller I/O */

void swtpc09_state::m6844_update_interrupt()
{
	uint8_t interrupt = 0;

	interrupt |= BIT(m_m6844_channel[0].control, 7) & BIT(m_m6844_interrupt, 0);
	interrupt |= BIT(m_m6844_channel[1].control, 7) & BIT(m_m6844_interrupt, 1);
	interrupt |= BIT(m_m6844_channel[2].control, 7) & BIT(m_m6844_interrupt, 2);
	interrupt |= BIT(m_m6844_channel[3].control, 7) & BIT(m_m6844_interrupt, 3);

	if (interrupt)
	{
		if (!(m_m6844_interrupt & 0x80))
		{
			// Set interrupt indication bit 7.
			m_m6844_interrupt |= 0x80;
			swtpc09_irq_handler(DMAC_IRQ, ASSERT_LINE);
		}
	}
	else
	{
		if (m_m6844_interrupt & 0x80)
		{
			// Clear interrupt indication bit 7.
			m_m6844_interrupt &= 0x7f;
			swtpc09_irq_handler(DMAC_IRQ, CLEAR_LINE);
		}
	}
}

void swtpc09_state::m6844_fdc_dma_transfer(uint8_t channel)
{
	uint32_t offset;
	address_space &space = *m_banked_space;

	offset = m_dmaf_high_address[channel] << 16;

	if (m_m6844_channel[channel].active == 1)  //active dma transfer
	{
		if (!(m_m6844_channel[channel].control & 0x01))  // dma write to memory
		{
			uint8_t data = m_fdc->data_r();

			space.write_byte(m_m6844_channel[channel].address + offset, data);
		}
		else
		{
			uint8_t data = space.read_byte(m_m6844_channel[channel].address + offset);

			m_fdc->data_w(data);
		}

		if (m_m6844_channel[channel].control & 0x08)
			m_m6844_channel[channel].address--;
		else
			m_m6844_channel[channel].address++;

		m_m6844_channel[channel].counter--;

		if (m_m6844_channel[channel].counter == 0)    // dma transfer has finished
		{
			m_m6844_channel[channel].control |= 0x80; // set dend flag
			m6844_update_interrupt();
		}
	}
}

void swtpc09_state::m6844_hdc_dma_transfer(uint8_t channel)
{
	uint32_t offset;
	address_space &space = *m_banked_space;

	offset = m_dmaf_high_address[channel] << 16;

	if (m_m6844_channel[channel].active == 1)  //active dma transfer
	{
		if (!(m_m6844_channel[channel].control & 0x01))  // dma write to memory
		{
			uint8_t data = m_hdc->data_r();

			space.write_byte(m_m6844_channel[channel].address + offset, data);
		}
		else
		{
			uint8_t data = space.read_byte(m_m6844_channel[channel].address + offset);

			m_hdc->data_w(data);
		}

		if (m_m6844_channel[channel].control & 0x08)
			m_m6844_channel[channel].address--;
		else
			m_m6844_channel[channel].address++;

		m_m6844_channel[channel].counter--;

		if (m_m6844_channel[channel].counter == 0)    // dma transfer has finished
		{
			m_m6844_channel[channel].control |= 0x80; // set dend flag
			m6844_update_interrupt();
		}
	}
}

uint8_t swtpc09_state::m6844_r(offs_t offset)
{
	uint8_t result = 0;

	// switch off the offset we were given
	switch (offset)
	{
		// upper byte of address
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			result = m_m6844_channel[offset / 4].address >> 8;
			break;

		// lower byte of address
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			result = m_m6844_channel[offset / 4].address & 0xff;
			break;

		// upper byte of counter
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			result = m_m6844_channel[offset / 4].counter >> 8;
			break;

		// lower byte of counter
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			result = m_m6844_channel[offset / 4].counter & 0xff;
			break;

		// channel control
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			result = m_m6844_channel[offset - 0x10].control;

			// A read here clears the 'DMA end' flag of the
			// associated channel.
			if (!machine().side_effects_disabled())
			{
				m_m6844_channel[offset - 0x10].control &= ~0x80;
				if (m_m6844_interrupt & 0x80)
					m6844_update_interrupt();
			}
			break;

		// priority control
		case 0x14:
			result = m_m6844_priority;
			break;

		// interrupt control
		case 0x15:
			result = m_m6844_interrupt;
			break;

		// chaining control
		case 0x16:
			result = m_m6844_chain;
			break;

		// 0x17-0x1f not used
		default: break;
	}

	// if DMAF2 controller data bus is inverted to 6844
	if (m_system_type == UNIFLEX_DMAF2 || m_system_type == FLEX_DMAF2)
		return ~result & 0xff;
	else
		return result & 0xff;
}


void swtpc09_state::m6844_w(offs_t offset, uint8_t data)
{
	int i;

	// if DMAF2 controller data bus is inverted to 6844
	if (m_system_type == UNIFLEX_DMAF2 || m_system_type == FLEX_DMAF2)
		data = ~data & 0xff;

	// switch off the offset we were given
	switch (offset)
	{
		// upper byte of address
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			m_m6844_channel[offset / 4].address = (m_m6844_channel[offset / 4].address & 0xff) | (data << 8);
			break;

		// lower byte of address
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			m_m6844_channel[offset / 4].address = (m_m6844_channel[offset / 4].address & 0xff00) | (data & 0xff);
			break;

		// upper byte of counter
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			m_m6844_channel[offset / 4].counter = (m_m6844_channel[offset / 4].counter & 0xff) | (data << 8);
			break;

		// lower byte of counter
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			m_m6844_channel[offset / 4].counter = (m_m6844_channel[offset / 4].counter & 0xff00) | (data & 0xff);
			break;

		// channel control
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			m_m6844_channel[offset - 0x10].control = (m_m6844_channel[offset - 0x10].control & 0xc0) | (data & 0x3f);
			break;

		// priority control
		case 0x14:
			m_m6844_priority = data;

			// update each channel
			for (i = 0; i < 4; i++)
			{
				// if we're going active...
				if (!m_m6844_channel[i].active && (data & (1 << i)))
				{
					// mark us active
					m_m6844_channel[i].active = 1;

					// set the DMA busy bit and clear the DMA end bit
					m_m6844_channel[i].control |= 0x40;
					m_m6844_channel[i].control &= ~0x80;

					// set the starting address, counter, and time
					m_m6844_channel[i].start_address = m_m6844_channel[i].address;
					m_m6844_channel[i].start_counter = m_m6844_channel[i].counter;
				}

				// if we're going inactive...
				else if (m_m6844_channel[i].active && !(data & (1 << i)))
				{
					//mark us inactive
					m_m6844_channel[i].active = 0;
				}
			}
			break;

		// interrupt control
		case 0x15:
			m_m6844_interrupt = (m_m6844_interrupt & 0x80) | (data & 0x7f);
			m6844_update_interrupt();
			break;

		// chaining control
		case 0x16:
			m_m6844_chain = data;
			break;

		// 0x17-0x1f not used
		default: break;
	}
}

INPUT_CHANGED_MEMBER(swtpc09_state::maincpu_clock_change)
{
	m_maincpu->set_clock(newval * 4);
}

INPUT_CHANGED_MEMBER(swtpc09_state::fdc_clock_change)
{
	if (m_system_type == FLEX_DMAF2 ||
		m_system_type == UNIFLEX_DMAF2 ||
		m_system_type == UNIFLEX_DMAF3)
	{
		m_fdc->set_unscaled_clock(newval);
	}
}

INPUT_CHANGED_MEMBER(swtpc09_state::baud_rate_high_change)
{
	m_brg->rsa_w(newval);
}

void swtpc09_state::machine_reset()
{
	uint32_t maincpu_clock = m_maincpu_clock->read();
	m_maincpu->set_clock(maincpu_clock * 4);

	if (m_system_type == FLEX_DMAF2 ||
		m_system_type == UNIFLEX_DMAF2 ||
		m_system_type == UNIFLEX_DMAF3)
	{
		uint32_t fdc_clock = m_fdc_clock->read();
		m_fdc->set_unscaled_clock(fdc_clock);
	}

	// Divider select X64 is the default Low baud rate setting. A High
	// baud rate setting is also available that selects a X16 divider, so
	// gives a rate four times as high. Note the schematic appears to have
	// mislabeled this setting.
	uint8_t baud_rate_high = m_baud_rate_high->read();
	m_brg->rsa_w(baud_rate_high);
	m_brg->rsb_w(1);

	m_pia->portb_w(0);
	m_pia->cb1_w(0);
	m_pia->ca2_w(0);
	m_pia->cb2_w(0);

	// Note UNIBUG has a smarter boot loader in ROM and will toggle the
	// density on failure so this is not necessary for UniFLEX.
	if ((m_system_type == FLEX_DMAF2 ||
		m_system_type == FLEX_DC5_PIAIDE) &&
		m_sbug_double_density->read())
	{
		// Patch the boot ROM to load the boot sector in double density.
		uint8_t* sbug = memregion("bankdev")->base();
		sbug[0xffaf8] = 0xfe; // 'D' DMAF2 boot path
		sbug[0xffb78] = 0xfe;
		sbug[0xffbe1] = 0x8e; // 'U' mini boot path
	}

	if (m_system_type == FLEX_DC5_PIAIDE &&
		m_piaide_flex_boot_cd00->read())
	{
		// Patch the PIA-IDE boot rom to use IO1
		uint8_t* rom = memregion("bankdev")->base();

		// Patch the FLEX entry point.
		rom[0xfe979] = 0xcd;
		rom[0xfe97a] = 0x00;
	}
}

void swtpc09_state::machine_start()
{
	m_pia_counter = 0;   // init ptm/pia counter to 0
	m_fdc_status = 0;    // for floppy controller
	m_interrupt = 0;
	m_active_interrupt = false;

	m_fdc_side = 0;
	m_fdc_dden = 0;

	// Start with the IRQ disabled?
	m_dmaf2_interrupt_enable = 0;

	m_dmaf_high_address[0] = 0;
	m_dmaf_high_address[1] = 0;
	m_dmaf_high_address[2] = 0;
	m_dmaf_high_address[3] = 0;

	m_floppy_motor_timer = timer_alloc(FUNC(swtpc09_state::floppy_motor_callback), this);
	m_floppy_motor_on = 0;

	// reset the 6844
	for (int i = 0; i < 4; i++)
	{
		m_m6844_channel[i].active = 0;
		m_m6844_channel[i].control = 0x00;
	}
	m_m6844_priority = 0x00;
	m_m6844_interrupt = 0x00;
	m_m6844_chain = 0x00;

	m_banked_space = &subdevice<address_map_bank_device>("bankdev")->space(AS_PROGRAM);

	save_item(NAME(m_pia_counter));
	save_item(NAME(m_dmaf_high_address));
	save_item(NAME(m_dmaf2_interrupt_enable));
	save_item(NAME(m_system_type));
	save_item(NAME(m_fdc_status));
	save_item(NAME(m_floppy_motor_on));
	save_item(NAME(m_fdc_side));
	save_item(NAME(m_fdc_dden));
	save_item(NAME(m_dmaf3_via_porta));
	save_item(NAME(m_dmaf3_via_portb));
	save_item(NAME(m_active_interrupt));
	save_item(NAME(m_interrupt));
	for (int i = 0; i < 4; i++)
	{
		save_item(NAME(m_m6844_channel[i].active), i);
		save_item(NAME(m_m6844_channel[i].address), i);
		save_item(NAME(m_m6844_channel[i].counter), i);
		save_item(NAME(m_m6844_channel[i].control), i);
		save_item(NAME(m_m6844_channel[i].start_address), i);
		save_item(NAME(m_m6844_channel[i].start_counter), i);
	}
	save_item(NAME(m_m6844_priority));
	save_item(NAME(m_m6844_interrupt));
	save_item(NAME(m_m6844_chain));
}

void swtpc09_state::init_swtpc09()
{
	m_system_type = FLEX_DMAF2;
}

void swtpc09_state::init_swtpc09i()
{
	m_system_type = FLEX_DC5_PIAIDE;
}

void swtpc09_state::init_swtpc09u()
{
	m_system_type = UNIFLEX_DMAF2;
}

void swtpc09_state::init_swtpc09d3()
{
	m_system_type = UNIFLEX_DMAF3;
	// UniFLEX numbers sectors from 1.
	m_hdc->set_sector_base(1);
}

void swtpc09_state::init_swtpc09o()
{
	m_system_type = OS9_DC5;
}
