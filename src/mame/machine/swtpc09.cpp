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
WRITE8_MEMBER( swtpc09_state::ptm_o1_callback )
{
	pia6821_device *pia = machine().device<pia6821_device>("pia");

	m_pia_counter++;
	//pia_counter = pia_counter && 0xff;
	if (m_pia_counter & 0x80) pia->ca1_w(1);
}

WRITE8_MEMBER( swtpc09_state::ptm_o3_callback )
{
	//ptm6840_device *ptm = machine().device<ptm6840_device>("ptm");
	/* the output from timer3 is the input clock for timer2 */
	//m_ptm->set_c2(data);
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
		pia6821_device *pia = machine().device<pia6821_device>("pia");

		if ( pia->irq_a_state())
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
		LOG(("swtpc09_acia_irq_assert\n"));
	}
	else
	{
		swtpc09_irq_handler(ACIA_IRQ, CLEAR_LINE);
		LOG(("swtpc09_acia_irq_clear\n"));
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

	LOG(("swtpc09_dmf2_dma_address_reg_w %02X\n", data));
}

/* DMF2 fdc control register */
READ8_MEMBER ( swtpc09_state::dmf2_control_reg_r )
{
	//LOG(("swtpc09_dmf2_control_reg_r $%02X\n", m_fdc_status));
	return m_fdc_status;
}

WRITE8_MEMBER ( swtpc09_state::dmf2_control_reg_w )
{
	LOG(("swtpc09_dmf2_control_reg_w $%02X\n", data));

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
	UINT8 *RAM = memregion("maincpu")->base();
	UINT32 offset;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	offset = (m_fdc_dma_address_reg & 0x0f)<<16;

	if (m_m6844_channel[0].active == 1)  //active dma transfer
	{
		if (!(m_m6844_channel[0].control & 0x01))  // dma write to memory
		{
			UINT8 data = m_fdc->data_r(space, 0);

			LOG(("swtpc09_dma_write_mem %05X %02X\n", m_m6844_channel[0].address + offset, data));
			RAM[m_m6844_channel[0].address + offset] = data;
		}
		else
		{
			UINT8 data = RAM[m_m6844_channel[0].address + offset];

			m_fdc->data_w(space, 0, data);
			//LOG(("swtpc09_dma_read_mem %04X %02X\n", m_m6844_channel[0].address, data));
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
void swtpc09_state::swtpc09_irq_handler(UINT8 peripheral, UINT8 state)
{
	LOG(("swtpc09_irq_handler peripheral:%02X state:%02X\n", peripheral, state));

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
		m_active_interrupt=TRUE;
		LOG(("swtpc09_irq_assert %02X\n", peripheral));
	}
	else if (m_active_interrupt && !m_interrupt)  //active interrupt and it needs to be cleared
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
		LOG(("swtpc09_irq_clear %02X\n", peripheral));
		m_active_interrupt=FALSE;
	}
}

/* handlers for fdc */
WRITE_LINE_MEMBER( swtpc09_state::fdc_intrq_w )
{
	LOG(("swtpc09_fdc_intrq_w %02X\n", state));
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
				LOG(("swtpc09_fdc_int ** assert\n"));
				swtpc09_irq_handler(FDC_IRQ, ASSERT_LINE);
			}
		}
		else
		{
			m_fdc_status &= ~0x40;
			if (!(m_fdc_dma_address_reg & 0x10)) // is dmf2 fdc irq enabled
			{
				LOG(("swtpc09_fdc_int ** clear\n"));
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
//    LOG(("swtpc09_dmf3_via_write_ca1 %02X\n", state));

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
	LOG(("swtpc09_dmf3_dma_address_reg_w %02X\n", data));
}

/* DMF3 fdc control register */
READ8_MEMBER ( swtpc09_state::dmf3_control_reg_r )
{
	//LOG(("swtpc09_dmf3_control_reg_r $%02X\n", m_fdc_status));
	return m_fdc_status;
}

WRITE8_MEMBER ( swtpc09_state::dmf3_control_reg_w )
{
	LOG(("swtpc09_dmf3_control_reg_w $%02X\n", data));

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
	LOG(("swtpc09_dc4_control_reg_w $%02X\n", data));

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
			tempidedata = m_ide->read_cs0(space, (data&0x1c)>>2, 0xffff);
			LOG(("swtpc09_ide_bus_r: offset $%02X data %04X\n", (data&0x1c)>>2, tempidedata));
			m_piaide_porta = tempidedata & 0x00ff;
		}
		else if (!(data & 0x01))  //wr line bit 0
		{
			m_ide->write_cs0(space, (data&0x1c)>>2, m_piaide_porta, 0xffff);
			LOG(("swtpc09_ide_bus_w: offset $%02X data %04X\n", (data&0x1c)>>2, m_piaide_porta));
		}
	}
	else if ((data & 0x20)&&(!(data&0x40)))  //cs0=1 cs1=0 bit 5&6
	{
		if (!(data & 0x02))  //rd line bit 1
		{
			tempidedata = m_ide->read_cs1(space, (data&0x1c)>>2, 0xffff);
			LOG(("swtpc09_ide_bus_r: offset $%02X data %04X\n", (data&0x1c)>>2, tempidedata));
			m_piaide_porta = tempidedata & 0x00ff;
		}
		else if (!(data & 0x01))  //wr line bit 0
		{
			m_ide->write_cs1(space, (data&0x1c)>>2, m_piaide_porta, 0xffff);
			LOG(("swtpc09_ide_bus_w: offset $%02X data %04X\n", (data&0x1c)>>2, m_piaide_porta));
		}
	}
}


/* DAT ram write handler (Dynamic Address Translator)    */
/* This creates the address map when a page is mapped in */
/* memory map is created based on system_type flag       */
/* this is accommodate the different cards installed     */

WRITE8_MEMBER(swtpc09_state::dat_w)
{
	UINT8 a16_to_a19, a12_to_a15;
	UINT8 *RAM = memregion("maincpu")->base();
	UINT32 physical_address, logical_address;

	address_space &mem = m_maincpu->space(AS_PROGRAM);

	fd1793_t *fdc = machine().device<fd1793_t>("fdc");
	pia6821_device *pia = machine().device<pia6821_device>("pia");
	ptm6840_device *ptm = machine().device<ptm6840_device>("ptm");
	acia6850_device *acia = machine().device<acia6850_device>("acia");
	via6522_device *via = machine().device<via6522_device>("via");
	pia6821_device *piaide = machine().device<pia6821_device>("piaide");

	a16_to_a19 = data & 0xf0;
	a12_to_a15 = ~data & 0x0f; //lower 4 bits are inverted
	physical_address = ((a16_to_a19 + a12_to_a15) << 12);
	logical_address = offset << 12;
	LOG(("swtpc09_dat_bank_unmap Logical address:%04X\n", offset << 12 ));
	LOG(("swtpc09_dat_bank_set dat:%02X Logical address:%04X Physical address:%05X\n", data, offset << 12,  physical_address ));


	// unmap page to be changed
	mem.unmap_readwrite(offset << 12, (offset << 12)+0x0fff);

	// map in new page
	if (a12_to_a15 == 0x0e)  // 0xE000 address range to be mapped in at this page
	{
		if (m_system_type == FLEX_DMF2)   // if flex/sbug, map in acia at 0xE004
		{
			mem.nop_readwrite(logical_address+0x000, logical_address+0x003);
			mem.install_readwrite_handler(logical_address+0x004, logical_address+0x004, read8_delegate(FUNC(acia6850_device::status_r), acia), write8_delegate(FUNC(acia6850_device::control_w),acia));
			mem.install_readwrite_handler(logical_address+0x005, logical_address+0x005, read8_delegate(FUNC(acia6850_device::data_r), acia), write8_delegate(FUNC(acia6850_device::data_w),acia));
			mem.nop_readwrite(logical_address+0x006, logical_address+0x07f);
			mem.install_readwrite_handler(logical_address+0x080, logical_address+0x08f, read8_delegate(FUNC(pia6821_device::read), pia), write8_delegate(FUNC(pia6821_device::write), pia));
			mem.install_readwrite_handler(logical_address+0x090, logical_address+0x09f, read8_delegate(FUNC(ptm6840_device::read), ptm), write8_delegate(FUNC(ptm6840_device::write), ptm));
			mem.nop_readwrite(logical_address+0x0a0, logical_address+0xfff);
		}
		else if (m_system_type == FLEX_DC4_PIAIDE)   // if flex/sbug and dc4 and piaide
		{
			mem.nop_readwrite(logical_address+0x000, logical_address+0x003);
			mem.install_readwrite_handler(logical_address+0x004, logical_address+0x004, read8_delegate(FUNC(acia6850_device::status_r), acia), write8_delegate(FUNC(acia6850_device::control_w),acia));
			mem.install_readwrite_handler(logical_address+0x005, logical_address+0x005, read8_delegate(FUNC(acia6850_device::data_r), acia), write8_delegate(FUNC(acia6850_device::data_w),acia));
			mem.install_write_handler(logical_address+0x014, logical_address+0x014, write8_delegate(FUNC(swtpc09_state::dc4_control_reg_w),this));
			mem.install_readwrite_handler(logical_address+0x018, logical_address+0x01b, read8_delegate(FUNC(fd1793_t::read), fdc), write8_delegate(FUNC(fd1793_t::write),fdc));
			//mem.nop_readwrite(logical_address+0x01c, logical_address+0x05f);
			mem.install_readwrite_handler(logical_address+0x060, logical_address+0x06f, read8_delegate(FUNC(pia6821_device::read), piaide), write8_delegate(FUNC(pia6821_device::write), piaide));
			//mem.nop_readwrite(logical_address+0x070, logical_address+0x07f);
			mem.install_readwrite_handler(logical_address+0x080, logical_address+0x08f, read8_delegate(FUNC(pia6821_device::read), pia), write8_delegate(FUNC(pia6821_device::write), pia));
			mem.install_readwrite_handler(logical_address+0x090, logical_address+0x09f, read8_delegate(FUNC(ptm6840_device::read), ptm), write8_delegate(FUNC(ptm6840_device::write), ptm));
			//mem.nop_readwrite(logical_address+0x0a0, logical_address+0x7ff);
			mem.install_rom(logical_address+0x800, logical_address+0xfff, &RAM[0xe800]); //piaide rom
		}
		else        // assume unibug, map in acia at 0xE000
		{
			mem.install_readwrite_handler(logical_address+0x000, logical_address+0x000, read8_delegate(FUNC(acia6850_device::status_r), acia), write8_delegate(FUNC(acia6850_device::control_w), acia));
			mem.install_readwrite_handler(logical_address+0x001, logical_address+0x001, read8_delegate(FUNC(acia6850_device::data_r), acia), write8_delegate(FUNC(acia6850_device::data_w), acia));
			mem.nop_readwrite(logical_address+0x002, logical_address+0x07f);
			mem.install_readwrite_handler(logical_address+0x080, logical_address+0x08f, read8_delegate(FUNC(pia6821_device::read), pia), write8_delegate(FUNC(pia6821_device::write), pia));
			mem.install_readwrite_handler(logical_address+0x090, logical_address+0x09f, read8_delegate(FUNC(ptm6840_device::read), ptm), write8_delegate(FUNC(ptm6840_device::write), ptm));
			mem.nop_readwrite(logical_address+0x0a0, logical_address+0xfff);
		}
	}
	else if (a12_to_a15 == 0x0f)   // 0xF000 address range to be mapped in at this page
	{
		if (m_system_type == UNIFLEX_DMF2 || m_system_type == FLEX_DMF2)   // if DMF2 conroller this is the map
		{
			mem.install_readwrite_handler(logical_address+0x000, logical_address+0x01f, read8_delegate(FUNC(swtpc09_state::m6844_r),this), write8_delegate(FUNC(swtpc09_state::m6844_w),this));
			mem.install_readwrite_handler(logical_address+0x020, logical_address+0x023, read8_delegate(FUNC(fd1793_t::read), fdc), write8_delegate(FUNC(fd1793_t::write),fdc));
			mem.install_readwrite_handler(logical_address+0x024, logical_address+0x03f, read8_delegate(FUNC(swtpc09_state::dmf2_control_reg_r),this), write8_delegate(FUNC(swtpc09_state::dmf2_control_reg_w),this));
			mem.install_readwrite_handler(logical_address+0x040, logical_address+0x041, read8_delegate(FUNC(swtpc09_state::dmf2_dma_address_reg_r),this), write8_delegate(FUNC(swtpc09_state::dmf2_dma_address_reg_w),this));
			//mem.nop_readwrite(logical_address+0x042, logical_address+0x7ff);
			mem.install_rom(logical_address+0x800, logical_address+0xfff, &RAM[0xf800]);
			mem.install_write_handler(logical_address+0xff0, logical_address+0xfff, write8_delegate(FUNC(swtpc09_state::dat_w),this));
		}
		else if (m_system_type == FLEX_DC4_PIAIDE)   // 2k ram for piaide on s09 board
		{
			//mem.install_readwrite_handler(logical_address+0x000, logical_address+0x01f, read8_delegate(FUNC(swtpc09_state::m6844_r),this), write8_delegate(FUNC(swtpc09_state::m6844_w),this));
			//mem.install_readwrite_handler(logical_address+0x020, logical_address+0x023, read8_delegate(FUNC(fd1793_t::read), fdc), write8_delegate(FUNC(fd1793_t::write),fdc));
			//mem.install_readwrite_handler(logical_address+0x024, logical_address+0x03f, read8_delegate(FUNC(swtpc09_state::dmf2_control_reg_r),this), write8_delegate(FUNC(swtpc09_state::dmf2_control_reg_w),this));
			//mem.install_readwrite_handler(logical_address+0x040, logical_address+0x041, read8_delegate(FUNC(swtpc09_state::dmf2_dma_address_reg_r),this), write8_delegate(FUNC(swtpc09_state::dmf2_dma_address_reg_w),this));
			mem.install_ram(logical_address+0x000, logical_address+0x7ff, &RAM[0xf000]);
			mem.install_rom(logical_address+0x800, logical_address+0xfff, &RAM[0xf800]);
			mem.install_write_handler(logical_address+0xff0, logical_address+0xfff, write8_delegate(FUNC(swtpc09_state::dat_w),this));
		}
		else    // assume DMF3 controller
		{
			mem.install_readwrite_handler(logical_address+0x000, logical_address+0x01f, read8_delegate(FUNC(swtpc09_state::m6844_r),this), write8_delegate(FUNC(swtpc09_state::m6844_w),this));
			mem.install_readwrite_handler(logical_address+0x020, logical_address+0x023, read8_delegate(FUNC(fd1793_t::read), fdc), write8_delegate(FUNC(fd1793_t::write),fdc));
			mem.install_readwrite_handler(logical_address+0x024, logical_address+0x024, read8_delegate(FUNC(swtpc09_state::dmf3_control_reg_r),this), write8_delegate(FUNC(swtpc09_state::dmf3_control_reg_w),this));
			mem.install_readwrite_handler(logical_address+0x025, logical_address+0x025, read8_delegate(FUNC(swtpc09_state::dmf3_dma_address_reg_r),this), write8_delegate(FUNC(swtpc09_state::dmf3_dma_address_reg_w),this));
			//mem.nop_readwrite(logical_address+0x030, logical_address+0x03f);
			mem.install_readwrite_handler(logical_address+0x040, logical_address+0x04f, read8_delegate(FUNC(via6522_device::read), via), write8_delegate(FUNC(via6522_device::write), via));
			//mem.nop_readwrite(logical_address+0x050, logical_address+0x7ff);
			mem.install_rom(logical_address+0x800, logical_address+0xfff, &RAM[0xf800]);
			mem.install_write_handler(logical_address+0xff0, logical_address+0xfff, write8_delegate(FUNC(swtpc09_state::dat_w),this));
		}
	}
	else if (offset==0x0f)  // then we need to leave in top part of ram and dat write
	{
		mem.install_ram(logical_address, logical_address+0x0eff, 0, 0, &RAM[physical_address]);
		mem.install_rom(logical_address+0xf00, logical_address+0xfff, 0, 0, &RAM[0xff00]);
		mem.install_write_handler(logical_address+0xff0, logical_address+0xfff, write8_delegate(FUNC(swtpc09_state::dat_w),this));

	}
	else   // all the rest is treated as ram, 1MB ram emulated
	{
		mem.install_ram(logical_address, logical_address+0x0fff, &RAM[physical_address]);
	}

// unused code to limit to 256k ram
//    else if (!(a12_to_a15 & 0x0c) )  // limit ram to 256k || a12_to_a15 == 0x02
//    {
//        memory_install_ram(space, logical_address, logical_address+0x0fff, 0, 0, &RAM[physical_address]);
//    }
//
//    else   // all the rest is treated as unallocated
//    {
//        memory_nop_readwrite(space, logical_address, logical_address+0x0fff, 0, 0);
//    }

}

/*  MC6844 DMA controller I/O */

READ8_MEMBER( swtpc09_state::m6844_r )
{
	UINT8 result = 0;


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
				LOG(("swtpc09_6844_r interrupt cleared \n"));
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
	//LOG(("swtpc09_6844_r %02X %02X\n", offset, result & 0xff));

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

	LOG(("swtpc09_6844_w %02X %02X\n", offset, data));
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
					LOG(("swtpc09_dma_channel active %02X\n", i));

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
			LOG(("swtpc09_m_m6844_interrupt_w %02X\n", m_m6844_interrupt));
			break;

		/* chaining control */
		case 0x16:
			m_m6844_chain = data;
			break;

		/* 0x17-0x1f not used */
		default: break;
	}
}


DRIVER_INIT_MEMBER( swtpc09_state, swtpc09 )
{
	int i;
	m_pia_counter = 0;  // init ptm/pia counter to 0
	m_term_data = 0;    // terminal keyboard input
	m_fdc_status = 0;    // for floppy controller
	m_system_type = FLEX_DMF2;
	m_interrupt = 0;
	m_active_interrupt = FALSE;

	/* reset the 6844 */
	for (i = 0; i < 4; i++)
	{
		m_m6844_channel[i].active = 0;
		m_m6844_channel[i].control = 0x00;
	}
	m_m6844_priority = 0x00;
	m_m6844_interrupt = 0x00;
	m_m6844_chain = 0x00;

}

DRIVER_INIT_MEMBER( swtpc09_state, swtpc09i )
{
	int i;
	m_pia_counter = 0;  // init ptm/pia counter to 0
	m_term_data = 0;    // terminal keyboard input
	m_fdc_status = 0;    // for floppy controller
	m_system_type = FLEX_DC4_PIAIDE;
	m_interrupt = 0;
	m_active_interrupt = FALSE;

	/* reset the 6844 */
	for (i = 0; i < 4; i++)
	{
		m_m6844_channel[i].active = 0;
		m_m6844_channel[i].control = 0x00;
	}
	m_m6844_priority = 0x00;
	m_m6844_interrupt = 0x00;
	m_m6844_chain = 0x00;

}

DRIVER_INIT_MEMBER( swtpc09_state, swtpc09u )
{
	int i;
	m_pia_counter = 0;  //init ptm/pia counter to 0
	m_term_data = 0;  //terminal keyboard input
	m_fdc_status = 0;    // for floppy controller
	m_system_type = UNIFLEX_DMF2;
	m_interrupt = 0;
	m_active_interrupt = FALSE;

	/* reset the 6844 */
	for (i = 0; i < 4; i++)
	{
		m_m6844_channel[i].active = 0;
		m_m6844_channel[i].control = 0x00;
	}
	m_m6844_priority = 0x00;
	m_m6844_interrupt = 0x00;
	m_m6844_chain = 0x00;

}

DRIVER_INIT_MEMBER( swtpc09_state, swtpc09d3 )
{
	int i;
	m_pia_counter = 0;  //init ptm/pia counter to 0
	m_term_data = 0;  //terminal keyboard input
	m_fdc_status = 0;    // for floppy controller
	m_via_ca1_input = 0;
	m_system_type = UNIFLEX_DMF3;
	m_interrupt = 0;
	m_active_interrupt = FALSE;


	/* reset the 6844 */
	for (i = 0; i < 4; i++)
	{
		m_m6844_channel[i].active = 0;
		m_m6844_channel[i].control = 0x00;
	}
	m_m6844_priority = 0x00;
	m_m6844_interrupt = 0x00;
	m_m6844_chain = 0x00;

}
