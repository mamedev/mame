// license:BSD-3-Clause
// copyright-holders:Robert Justice
/***************************************************************************
    swtpc09 machine file
    Robert Justice ,2009-2014

****************************************************************************/

#include "emu.h"
#include "includes/swtpc09.h"

#define DMAC_IRQ 0x01             // interrupt handler IDs
#define ACIA_IRQ 0x02
#define PTM_IRQ 0x04
#define PIA_IRQ 0x08
#define FDC_IRQ 0x10
#define VIA_IRQ 0x20

#define FLEX_DMF2 1               // system type flags
#define UNIFLEX_DMF2 2
#define UNIFLEX_DMF3 3
#define FLEX_DC4_PIAIDE 4

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


/******* MC6840 PTM on MPID Board *******/

/* 6840 PTM handlers */
WRITE_LINE_MEMBER( swtpc09_state::ptm_o1_callback )
{
	m_pia_counter++;
	//pia_counter = pia_counter && 0xff;
	if (m_pia_counter & 0x80) m_pia->ca1_w(1);
}

WRITE_LINE_MEMBER( swtpc09_state::ptm_o3_callback )
{
	/* the output from timer3 is the input clock for timer2 */
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

READ8_MEMBER( swtpc09_state::pia0_a_r )
{
	return m_pia_counter;
}

READ8_MEMBER( swtpc09_state::pia0_ca1_r )
{
	return 0;
}

WRITE_LINE_MEMBER( swtpc09_state::pia0_irq_a )
{
	if ( m_pia->irq_a_state())
		swtpc09_irq_handler(PIA_IRQ, ASSERT_LINE);
	else
		swtpc09_irq_handler(PIA_IRQ, CLEAR_LINE);
}


/******* MC6850 ACIA on MPS2 *******/

WRITE_LINE_MEMBER( swtpc09_state::acia_interrupt )
{
	if (state)
	{
		swtpc09_irq_handler(ACIA_IRQ, ASSERT_LINE);
		logerror("swtpc09_acia_irq_assert\n");
	}
	else
	{
		swtpc09_irq_handler(ACIA_IRQ, CLEAR_LINE);
		logerror("swtpc09_acia_irq_clear\n");
	}
}

/*********************************************************************/
/*   DMF2 Floppy Controller Board                                    */
/*********************************************************************/

/* DMF2 dma extended address register */
READ8_MEMBER ( swtpc09_state::dmf2_dma_address_reg_r )
{
	return m_fdc_dma_address_reg;
}

WRITE8_MEMBER ( swtpc09_state::dmf2_dma_address_reg_w )
{
	m_fdc_dma_address_reg = data;

	// bit 4 controls a gate enable/disable for DMF2 fdc irq line
	if ((m_fdc_dma_address_reg & 0x10) && (m_system_type == UNIFLEX_DMF2 || m_system_type == FLEX_DMF2))
		swtpc09_irq_handler(FDC_IRQ, CLEAR_LINE); //then clear the irq to cpu

	logerror("swtpc09_dmf2_dma_address_reg_w %02X\n", data);
}

/* DMF2 fdc control register */
READ8_MEMBER ( swtpc09_state::dmf2_control_reg_r )
{
	//logerror("swtpc09_dmf2_control_reg_r $%02X\n", m_fdc_status);
	return m_fdc_status;
}

WRITE8_MEMBER ( swtpc09_state::dmf2_control_reg_w )
{
	logerror("swtpc09_dmf2_control_reg_w $%02X\n", data);

	floppy_image_device *floppy = nullptr;

	if (!BIT(data, 0)) floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) floppy = m_floppy1->get_device();
	if (!BIT(data, 2)) floppy = m_floppy2->get_device();
	if (!BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(!BIT(data, 4));
	}

	m_fdc->dden_w(!BIT(data, 5));
}

/* FDC controller dma transfer */
void swtpc09_state::swtpc09_fdc_dma_transfer()
{
	uint32_t offset;
	address_space &space = *m_banked_space;

	offset = (m_fdc_dma_address_reg & 0x0f)<<16;

	if (m_m6844_channel[0].active == 1)  //active dma transfer
	{
		if (!(m_m6844_channel[0].control & 0x01))  // dma write to memory
		{
			uint8_t data = m_fdc->data_r();

			logerror("swtpc09_dma_write_mem %05X %02X\n", m_m6844_channel[0].address + offset, data);
			space.write_byte(m_m6844_channel[0].address + offset, data);
		}
		else
		{
			uint8_t data = space.read_byte(m_m6844_channel[0].address + offset);

			m_fdc->data_w(data);
			//logerror("swtpc09_dma_read_mem %04X %02X\n", m_m6844_channel[0].address, data);
		}

		m_m6844_channel[0].address++;
		m_m6844_channel[0].counter--;

		if (m_m6844_channel[0].counter == 0)    // dma transfer has finished
		{
			m_m6844_channel[0].control |= 0x80; // set dend flag
			if (m_m6844_interrupt & 0x01)       // interrupt for channel 0 is enabled?
			{
				m_m6844_interrupt   |= 0x80;      // set bit 7 to indicate active interrupt
				swtpc09_irq_handler(DMAC_IRQ, ASSERT_LINE);
			}
		}
	}

}

/* common interrupt handler */
void swtpc09_state::swtpc09_irq_handler(uint8_t peripheral, uint8_t state)
{
	logerror("swtpc09_irq_handler peripheral:%02X state:%02X\n", peripheral, state);

	switch (state)
	{
		case ASSERT_LINE:
			m_interrupt |= peripheral;
			break;

		case CLEAR_LINE:
			m_interrupt &= (~peripheral & 0x3f);
			break;
	}

	if (!m_active_interrupt && m_interrupt)    //no active interrupt and it needs to be asserted
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_active_interrupt=true;
		logerror("swtpc09_irq_assert %02X\n", peripheral);
	}
	else if (m_active_interrupt && !m_interrupt)  //active interrupt and it needs to be cleared
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		logerror("swtpc09_irq_clear %02X\n", peripheral);
		m_active_interrupt=false;
	}
}

/* handlers for fdc */
WRITE_LINE_MEMBER( swtpc09_state::fdc_intrq_w )
{
	logerror("swtpc09_fdc_intrq_w %02X\n", state);
	if ( m_system_type == UNIFLEX_DMF3 )  //IRQ from 1791 is connect into VIA ca2
	{
		if (state)
		{
			m_fdc_status |= 0x40;
			m_via->write_cb2(0);     //fdc interrupt is connected to CA1
			m_dmf3_via_porta &= 0xfb; //clear pa3
			//m_via->write_porta(m_dmf3_via_porta);     //and connected to PA3
			//swtpc09_irq_handler(FDC_IRQ, ASSERT_LINE);
		}
		else
		{
			m_fdc_status &= ~0x40;
			m_via->write_cb2(1);
			m_dmf3_via_porta |= 0x04;  //and connected to PA3
			//m_via->write_porta(m_dmf3_via_porta);     //and connected to PA3
			//swtpc09_irq_handler(FDC_IRQ, CLEAR_LINE);
		}
	}
	else if ( m_system_type == FLEX_DC4_PIAIDE )  //for dc4 emulate irq jumper out
	{
		if (state)
		{
			m_fdc_status |= 0x40;
		}
		else
		{
			m_fdc_status &= ~0x40;
		}
	}
	else   //for dmf2 it is connected directly to cpu via a gate
	{
		if (state)
		{
			m_fdc_status |= 0x40;
			if (!(m_fdc_dma_address_reg & 0x10))  // is dmf2 fdc irq enabled
			{
				logerror("swtpc09_fdc_int ** assert\n");
				swtpc09_irq_handler(FDC_IRQ, ASSERT_LINE);
			}
		}
		else
		{
			m_fdc_status &= ~0x40;
			if (!(m_fdc_dma_address_reg & 0x10)) // is dmf2 fdc irq enabled
			{
				logerror("swtpc09_fdc_int ** clear\n");
				swtpc09_irq_handler(FDC_IRQ, CLEAR_LINE);
			}
		}
	}
}

WRITE_LINE_MEMBER( swtpc09_state::fdc_drq_w )
{
	if (m_system_type == FLEX_DC4_PIAIDE)  //for dc4 no dma
	{
		if (state)
		{
			m_fdc_status |= 0x80;
		}
		else
			m_fdc_status &= 0x7f;
	}
	else
	{
		if (state)
		{
			m_fdc_status |= 0x80;
			swtpc09_fdc_dma_transfer();
		}
		else
			m_fdc_status &= 0x7f;
	}
}

/*********************************************************************/
/*   DMF3 Board                                                      */
/*********************************************************************/

/* via on dmf3 board */
READ8_MEMBER( swtpc09_state::dmf3_via_read_porta )
{
	return m_dmf3_via_porta;
}

READ8_MEMBER( swtpc09_state::dmf3_via_read_portb )
{
	return 0xff;
}

WRITE8_MEMBER( swtpc09_state::dmf3_via_write_porta )
{
	m_dmf3_via_porta &= data;
}

//WRITE_LINE_MEMBER( swtpc09_state::dmf3_via_write_ca1 )
//{
//  return m_via_ca1_input;
//    logerror("swtpc09_dmf3_via_write_ca1 %02X\n", state);

//}

WRITE_LINE_MEMBER( swtpc09_state::dmf3_via_irq )
{
	if (state)
		swtpc09_irq_handler(VIA_IRQ, ASSERT_LINE);
	else
		swtpc09_irq_handler(VIA_IRQ, CLEAR_LINE);
}

/* DMF3 dma extended address register */
READ8_MEMBER ( swtpc09_state::dmf3_dma_address_reg_r )
{
	return m_fdc_dma_address_reg;
}

WRITE8_MEMBER ( swtpc09_state::dmf3_dma_address_reg_w )
{
	m_fdc_dma_address_reg = data;
	logerror("swtpc09_dmf3_dma_address_reg_w %02X\n", data);
}

/* DMF3 fdc control register */
READ8_MEMBER ( swtpc09_state::dmf3_control_reg_r )
{
	//logerror("swtpc09_dmf3_control_reg_r $%02X\n", m_fdc_status);
	return m_fdc_status;
}

WRITE8_MEMBER ( swtpc09_state::dmf3_control_reg_w )
{
	logerror("swtpc09_dmf3_control_reg_w $%02X\n", data);

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 4));
	}

	m_fdc->dden_w(BIT(data, 5));
}

// DC4 drive select

WRITE8_MEMBER ( swtpc09_state::dc4_control_reg_w )
{
	logerror("swtpc09_dc4_control_reg_w $%02X\n", data);

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();
	if (BIT(data, 2)) floppy = m_floppy2->get_device();
	if (BIT(data, 3)) floppy = m_floppy3->get_device();

	m_fdc->set_floppy(floppy);
}


/******* MC6821 PIA on IDE Board *******/
/* Read/Write handlers for pia ide */
/* TODO: update and finish this off */

READ8_MEMBER( swtpc09_state::piaide_a_r )
{
	return m_piaide_porta;
}

READ8_MEMBER( swtpc09_state::piaide_b_r )
{
	return m_piaide_portb;
}

WRITE8_MEMBER( swtpc09_state::piaide_a_w )
{
	m_piaide_porta = data;
}

WRITE8_MEMBER( swtpc09_state::piaide_b_w )
{
	int tempidedata;

	m_piaide_portb = data;

	if ((data & 0x40)&&(!(data&0x20)))  //cs0=0 cs1=1 bit 5&6
	{
		if (!(data & 0x02))  //rd line bit 1
		{
			tempidedata = m_ide->read_cs0((data&0x1c)>>2);
			logerror("swtpc09_ide_bus_r: offset $%02X data %04X\n", (data&0x1c)>>2, tempidedata);
			m_piaide_porta = tempidedata & 0x00ff;
		}
		else if (!(data & 0x01))  //wr line bit 0
		{
			m_ide->write_cs0((data&0x1c)>>2, m_piaide_porta);
			logerror("swtpc09_ide_bus_w: offset $%02X data %04X\n", (data&0x1c)>>2, m_piaide_porta);
		}
	}
	else if ((data & 0x20)&&(!(data&0x40)))  //cs0=1 cs1=0 bit 5&6
	{
		if (!(data & 0x02))  //rd line bit 1
		{
			tempidedata = m_ide->read_cs1((data&0x1c)>>2);
			logerror("swtpc09_ide_bus_r: offset $%02X data %04X\n", (data&0x1c)>>2, tempidedata);
			m_piaide_porta = tempidedata & 0x00ff;
		}
		else if (!(data & 0x01))  //wr line bit 0
		{
			m_ide->write_cs1((data&0x1c)>>2, m_piaide_porta);
			logerror("swtpc09_ide_bus_w: offset $%02X data %04X\n", (data&0x1c)>>2, m_piaide_porta);
		}
	}
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

READ8_MEMBER(swtpc09_state::main_r)
{
	return m_banked_space->read_byte(dat_translate(offset));
}

WRITE8_MEMBER(swtpc09_state::main_w)
{
	m_banked_space->write_byte(dat_translate(offset), data);
}

/*  MC6844 DMA controller I/O */

READ8_MEMBER( swtpc09_state::m6844_r )
{
	uint8_t result = 0;


	/* switch off the offset we were given */
	switch (offset)
	{
		/* upper byte of address */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			result = m_m6844_channel[offset / 4].address >> 8;
			break;

		/* lower byte of address */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			result = m_m6844_channel[offset / 4].address & 0xff;
			break;

		/* upper byte of counter */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			result = m_m6844_channel[offset / 4].counter >> 8;
			break;

		/* lower byte of counter */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			result = m_m6844_channel[offset / 4].counter & 0xff;
			break;

		/* channel control */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			result = m_m6844_channel[offset - 0x10].control;

			/* a read here clears the DMA end flag */
			m_m6844_channel[offset - 0x10].control &= ~0x80;
			if (m_m6844_interrupt & 0x80) // if interrupt is active, then clear
			{
				swtpc09_irq_handler(0x01, CLEAR_LINE);
				m_m6844_interrupt &= 0x7f;  // clear interrupt indication bit 7
				logerror("swtpc09_6844_r interrupt cleared \n");
			}
			break;

		/* priority control */
		case 0x14:
			result = m_m6844_priority;
			break;

		/* interrupt control */
		case 0x15:
			result = m_m6844_interrupt;
			break;

		/* chaining control */
		case 0x16:
			result = m_m6844_chain;
			break;

		/* 0x17-0x1f not used */
		default: break;
	}
	//logerror("swtpc09_6844_r %02X %02X\n", offset, result & 0xff);

	if (m_system_type == UNIFLEX_DMF2 || m_system_type == FLEX_DMF2)   // if DMF2 controller data bus is inverted to 6844
	{
		return ~result & 0xff;
	}
	else
	{
		return result & 0xff;
	}
}


WRITE8_MEMBER( swtpc09_state::m6844_w )
{
	int i;

	if (m_system_type == UNIFLEX_DMF2 || m_system_type == FLEX_DMF2)   // if DMF2 controller data bus is inverted to 6844
		data = ~data & 0xff;

	logerror("swtpc09_6844_w %02X %02X\n", offset, data);
	/* switch off the offset we were given */
	switch (offset)
	{
		/* upper byte of address */
		case 0x00:
		case 0x04:
		case 0x08:
		case 0x0c:
			m_m6844_channel[offset / 4].address = (m_m6844_channel[offset / 4].address & 0xff) | (data << 8);
			break;

		/* lower byte of address */
		case 0x01:
		case 0x05:
		case 0x09:
		case 0x0d:
			m_m6844_channel[offset / 4].address = (m_m6844_channel[offset / 4].address & 0xff00) | (data & 0xff);
			break;

		/* upper byte of counter */
		case 0x02:
		case 0x06:
		case 0x0a:
		case 0x0e:
			m_m6844_channel[offset / 4].counter = (m_m6844_channel[offset / 4].counter & 0xff) | (data << 8);
			break;

		/* lower byte of counter */
		case 0x03:
		case 0x07:
		case 0x0b:
		case 0x0f:
			m_m6844_channel[offset / 4].counter = (m_m6844_channel[offset / 4].counter & 0xff00) | (data & 0xff);
			break;

		/* channel control */
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			m_m6844_channel[offset - 0x10].control = (m_m6844_channel[offset - 0x10].control & 0xc0) | (data & 0x3f);
			break;

		/* priority control */
		case 0x14:
			m_m6844_priority = data;

			/* update each channel */
			for (i = 0; i < 4; i++)
			{
				/* if we're going active... */
				if (!m_m6844_channel[i].active && (data & (1 << i)))
				{
					/* mark us active */
					m_m6844_channel[i].active = 1;
					logerror("swtpc09_dma_channel active %02X\n", i);

					/* set the DMA busy bit and clear the DMA end bit */
					m_m6844_channel[i].control |= 0x40;
					m_m6844_channel[i].control &= ~0x80;

					/* set the starting address, counter, and time */
					m_m6844_channel[i].start_address = m_m6844_channel[i].address;
					m_m6844_channel[i].start_counter = m_m6844_channel[i].counter;


					/* generate and play the sample */
					//play_cvsd(space->machine, i);
				}

				/* if we're going inactive... */
				else if (m_m6844_channel[i].active && !(data & (1 << i)))
				{
					/* mark us inactive */
					m_m6844_channel[i].active = 0;
				}
			}
			break;

		/* interrupt control */
		case 0x15:
			m_m6844_interrupt = (m_m6844_interrupt & 0x80) | (data & 0x7f);
			logerror("swtpc09_m_m6844_interrupt_w %02X\n", m_m6844_interrupt);
			break;

		/* chaining control */
		case 0x16:
			m_m6844_chain = data;
			break;

		/* 0x17-0x1f not used */
		default: break;
	}
}


void swtpc09_state::machine_start()
{
	m_pia_counter = 0;  // init ptm/pia counter to 0
	m_fdc_status = 0;    // for floppy controller
	m_interrupt = 0;
	m_active_interrupt = false;

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

	m_brg->rsa_w(0);
	m_brg->rsb_w(1);
}

void swtpc09_state::init_swtpc09()
{
	m_system_type = FLEX_DMF2;
}

void swtpc09_state::init_swtpc09i()
{
	m_system_type = FLEX_DC4_PIAIDE;
}

void swtpc09_state::init_swtpc09u()
{
	m_system_type = UNIFLEX_DMF2;
}

void swtpc09_state::init_swtpc09d3()
{
	m_via_ca1_input = 0;
	m_system_type = UNIFLEX_DMF3;
}
