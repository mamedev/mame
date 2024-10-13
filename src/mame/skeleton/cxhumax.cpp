// license:BSD-3-Clause
// copyright-holders:Lukasz Markowski
/***************************************************************************
    HUMAX HDCI-2000 ( Conexant CX2417x )

    http://www.humaxdigital.com/global/products/product_stb_satellite_hdci2000.asp

    Running on Nucleus PLUS - ARM7TDMI ADS v. 1.14
    some Conexant/Nucleus goodies may be found at http://code.google.com/p/cherices/

    runs up to frame 280 or so...

****************************************************************************/

#include "emu.h"
#include "cxhumax.h"

#include "emupal.h"
#include "screen.h"


#define VERBOSE ( 0 )
#include "logmacro.h"

uint32_t cxhumax_state::cx_gxa_r(offs_t offset)
{
	uint32_t res = m_gxa_cmd_regs[offset];
	LOG("%s: (GXA) %08X -> %08X\n", machine().describe_context(), 0xE0600000 + (offset << 2), res);
/*  uint8_t gxa_command_number = (offset >> 9) & 0x7F;
    LOG("      Command: %08X\n", gxa_command_number);
    switch (gxa_command_number) {
        case GXA_CMD_RW_REGISTER:
            switch(offset) {
                case GXA_CFG2_REG:
                    break;
                default:
                    LOG("      Unimplemented register - TODO?\n");
                    break;
            }
            break;
        default:
            // do we need it?
            LOG("      Unimplemented read command - TODO?\n");
            break;
    }*/
	return res;
}

void cxhumax_state::cx_gxa_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (GXA) %08X <- %08X\n", machine().describe_context(), 0xE0600000 + (offset << 2), data);
	uint8_t gxa_command_number = (offset >> 9) & 0x7F;
	LOG("      Command: %08X\n", gxa_command_number);

	/* Clear non persistent data */
	m_gxa_cmd_regs[GXA_CMD_REG] &= 0xfffc0000;

	if (gxa_command_number == GXA_CMD_RW_REGISTER) {
		LOG("      Register Number: %08X\n", offset & 0xff);
	} else {
		m_gxa_cmd_regs[GXA_CMD_REG] |= (offset << 2) & 0x3ffff;
		LOG("      Source Bitmap Selector: %08X\n", (offset >> 6) & 0x7);
		LOG("      Destination Bitmap Selector: %08X\n", (offset >> 3) & 0x7);
		LOG("      Parameter Count: %08X\n", offset & 0x7);
	}
	switch (gxa_command_number) {
		case GXA_CMD_RW_REGISTER:
			switch(offset) {
				case GXA_CFG2_REG:
					// clear IRQ_STAT bits if requested
					m_gxa_cmd_regs[GXA_CFG2_REG] = (m_gxa_cmd_regs[GXA_CFG2_REG]&(0xfff00000 & ~(data&0x00300000))) | (data & 0x000fffff);
					break;
				default:
					LOG("      Unimplemented register - TODO?\n");
					COMBINE_DATA(&m_gxa_cmd_regs[offset]);
					break;
			}
			break;
		case GXA_CMD_QMARK:
			LOG("      QMARK - TODO?\n");

			/* Set value and copy of WAIT4_VERTICAL bit written by QMARK */
			m_gxa_cmd_regs[GXA_CMD_REG] = (m_gxa_cmd_regs[GXA_CMD_REG] & 0x3ffff) | (data<<24) | ((data&0x10)?1<<23:0);

			/* QMARK command has completed */
			m_gxa_cmd_regs[GXA_CFG2_REG] |= (1<<IRQ_STAT_QMARK);

			// Interrupt
			if (m_gxa_cmd_regs[GXA_CFG2_REG] & (1<<IRQ_EN_QMARK)) {
				m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] |= 1<<18;
				m_intctrl_regs[INTREG(INTGROUP2, INTSTATCLR)] |= 1<<18;
				m_intctrl_regs[INTREG(INTGROUP2, INTSTATSET)] |= 1<<18;
				LOG("      QMARK INT - TODO?\n");
			}

			if((m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] & m_intctrl_regs[INTREG(INTGROUP2, INTENABLE)])
				|| (m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] & m_intctrl_regs[INTREG(INTGROUP1, INTENABLE)]))
					m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);

			break;
		default:
			LOG("      Unimplemented command - TODO?\n");
			break;
	}
}

void cxhumax_state::flash_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	offset *= 2;
	if(ACCESSING_BITS_0_15)
		m_flash->write(offset, data);
	if(ACCESSING_BITS_16_31)
		m_flash->write(offset+1, data >> 16);
	LOG("%s: (FLASH) %08X <- %08X\n", machine().describe_context(), 0xF0000000 + (offset << 2), data);
}

uint32_t cxhumax_state::flash_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t res = 0;
	offset *= 2;
	if(ACCESSING_BITS_0_15)
		res |= m_flash->read(offset);
	if(ACCESSING_BITS_16_31)
		res |= m_flash->read(offset+1) << 16;
	//if(m_flash->m_flash_mode!=FM_NORMAL) LOG("%s: (FLASH) %08X -> %08X\n", machine().describe_context(), 0xF0000000 + (offset << 2), res);
	return res;
}

uint32_t cxhumax_state::dummy_flash_r()
{
	return 0xFFFFFFFF;
}

void cxhumax_state::cx_remap_w(offs_t offset, uint32_t data)
{
	if(!(data&1)) {
		LOG("%s: (REMAP) %08X -> %08X\n", machine().describe_context(), 0xE0400014 + (offset << 2), data);
		memset(m_ram, 0, 0x400000); // workaround :P
	}
}

uint32_t cxhumax_state::cx_scratch_r(offs_t offset)
{
	uint32_t data = m_scratch_reg;
	LOG("%s: (SCRATCH) %08X -> %08X\n", machine().describe_context(), 0xE0400024 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_scratch_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (SCRATCH) %08X <- %08X\n", machine().describe_context(), 0xE0400024 + (offset << 2), data);
	COMBINE_DATA(&m_scratch_reg);
}

uint32_t cxhumax_state::cx_hsx_r(offs_t offset)
{
	uint32_t data = 0; // dummy
	LOG("%s: (HSX) %08X -> %08X\n", machine().describe_context(), 0xE0000000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_hsx_w(offs_t offset, uint32_t data)
{
	LOG("%s: (HSX) %08X <- %08X\n", machine().describe_context(), 0xE0000000 + (offset << 2), data);
}

uint32_t cxhumax_state::cx_romdescr_r(offs_t offset)
{
	uint32_t data = m_romdescr_reg;
	LOG("%s: (ROMDESC0) %08X -> %08X\n", machine().describe_context(), 0xE0010000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_romdescr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (ROMDESC0) %08X <- %08X\n", machine().describe_context(), 0xE0010000 + (offset << 2), data);
	COMBINE_DATA(&m_romdescr_reg);
}

uint32_t cxhumax_state::cx_isaromdescr_r(offs_t offset)
{
	uint32_t data = m_isaromdescr_regs[offset];
	LOG("%s: (ISAROMDESC%d) %08X -> %08X\n", machine().describe_context(), offset+1, 0xE0010004 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_isaromdescr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (ISAROMDESC%d) %08X <- %08X\n", machine().describe_context(), offset+1, 0xE0010004 + (offset << 2), data);
	COMBINE_DATA(&m_isaromdescr_regs[offset]);
}

uint32_t cxhumax_state::cx_isadescr_r(offs_t offset)
{
	uint32_t data = m_isaromdescr_regs[offset];
	LOG("%s: (ISA_DESC%d) %08X -> %08X\n", machine().describe_context(), offset+4, 0xE0010010 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_isadescr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (ISA_DESC%d) %08X <- %08X\n", machine().describe_context(), offset+4, 0xE0010010 + (offset << 2), data);
	COMBINE_DATA(&m_isaromdescr_regs[offset]);
}

uint32_t cxhumax_state::cx_rommap_r(offs_t offset)
{
	uint32_t data = 0;
	LOG("%s: (ROM%d_MAP) %08X -> %08X\n", machine().describe_context(), offset, 0xE0010020 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_rommap_w(offs_t offset, uint32_t data)
{
	LOG("%s: (ROM%d_MAP) %08X <- %08X\n", machine().describe_context(), offset, 0xE0010020 + (offset << 2), data);
}

uint32_t cxhumax_state::cx_rommode_r(offs_t offset)
{
	uint32_t data = m_rommode_reg;
	LOG("%s: (ROMMODE) %08X -> %08X\n", machine().describe_context(), 0xE0010034 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_rommode_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (ROMMODE) %08X <- %08X\n", machine().describe_context(), 0xE0010034 + (offset << 2), data);
	COMBINE_DATA(&m_rommode_reg);
}

uint32_t cxhumax_state::cx_xoemask_r(offs_t offset)
{
	uint32_t data = m_xoemask_reg;
	LOG("%s: (XOEMASK) %08X -> %08X\n", machine().describe_context(), 0xE0010034 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_xoemask_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (XOEMASK) %08X <- %08X\n", machine().describe_context(), 0xE0010034 + (offset << 2), data);
	COMBINE_DATA(&m_xoemask_reg);
}

uint32_t cxhumax_state::cx_pci_r(offs_t offset)
{
	uint32_t data = 0;
	switch (offset) {
		case PCI_CFG_ADDR_REG:
			data = m_pci_regs[offset]; break;
		case PCI_CFG_DATA_REG:
			{
				switch (m_pci_regs[PCI_CFG_ADDR_REG]) {
					case 0: data = (0x4170<<16) /*Device ID*/ | 0x14f1 /* Vendor ID */; break;
					case 8: data = (0x060000 << 8) /* Class Code */ | 0x1f /* Revision ID */; break;
				}
			} break;
	}
	LOG("%s: (PCI) %08X -> %08X\n", machine().describe_context(), 0xE0010040 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_pci_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (PCI) %08X <- %08X\n", machine().describe_context(), 0xE0010040 + (offset << 2), data);
	COMBINE_DATA(&m_pci_regs[offset]);
}

uint32_t cxhumax_state::cx_extdesc_r(offs_t offset)
{
	uint32_t data = m_extdesc_regs[offset];
	LOG("%s: (EXTDESC) %08X -> %08X\n", machine().describe_context(), 0xE0010080 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_extdesc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (EXTDESC) %08X <- %08X\n", machine().describe_context(), 0xE0010080 + (offset << 2), data);
	COMBINE_DATA(&m_extdesc_regs[offset]);
}

TIMER_CALLBACK_MEMBER(cxhumax_state::timer_tick)
{
	m_timer_regs.timer[param].value++;
	if(m_timer_regs.timer[param].value==m_timer_regs.timer[param].limit) {
		/* Reset counter when reaching limit and RESET_CNTR bit is cleared */
		if(!(m_timer_regs.timer[param].mode & 2))
			m_timer_regs.timer[param].value=0;

		/* Indicate interrupt request if EN_INT bit is set */
		if (m_timer_regs.timer[param].mode & 8) {
			//printf( "IRQ on Timer %d\n", param );
			LOG("%s: (TIMER%d) Interrupt\n", machine().time().to_string(), param);
			m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] |= INT_TIMER_BIT;     /* Timer interrupt */
			m_intctrl_regs[INTREG(INTGROUP2, INTSTATCLR)] |= INT_TIMER_BIT; /* Timer interrupt */
			m_intctrl_regs[INTREG(INTGROUP2, INTSTATSET)] |= INT_TIMER_BIT; /* Timer interrupt */

			m_timer_regs.timer_irq |= 1<<param; /* Indicate which timer interrupted */

			/* Interrupt if Timer interrupt is not masked in ITC_INTENABLE_REG */
			if (m_intctrl_regs[INTREG(INTGROUP2, INTENABLE)] & INT_TIMER_BIT)
				m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
		}
	}
	attotime period = attotime::from_hz(XTAL(54'000'000))*m_timer_regs.timer[param].timebase;
	m_timer_regs.timer[param].timer->adjust(period,param);
}

uint32_t cxhumax_state::cx_timers_r(offs_t offset)
{
	uint32_t data = 0;
	uint8_t index = offset>>2;
	if(index==16) {
		data = m_timer_regs.timer_irq;
		//m_timer_regs.timer_irq=0;
		LOG("%s: (TIMERIRQ) %08X -> %08X\n", machine().describe_context(), 0xE0430000 + (offset << 2), data);
	}
	else {
		switch (offset&3) {
			case TIMER_VALUE:
				data = m_timer_regs.timer[index].value; break;
			case TIMER_LIMIT:
				data = m_timer_regs.timer[index].limit; break;
			case TIMER_MODE:
				data = m_timer_regs.timer[index].mode; break;
			case TIMER_TIMEBASE:
				data = m_timer_regs.timer[index].timebase; break;
		}
		LOG("%s: (TIMER%d) %08X -> %08X\n", machine().describe_context(), offset>>2, 0xE0430000 + (offset << 2), data);
	}
	return data;
}

void cxhumax_state::cx_timers_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint8_t index = offset>>2;
	if(index==16) {
		LOG("%s: (TIMERIRQ) %08X <- %08X\n", machine().describe_context(), 0xE0430000 + (offset << 2), data);
		COMBINE_DATA(&m_timer_regs.timer_irq);
	}
	else {
		LOG("%s: (TIMER%d) %08X <- %08X\n", machine().describe_context(), index, 0xE0430000 + (offset << 2), data);
		switch(offset&3) {
			case TIMER_VALUE:
				COMBINE_DATA(&m_timer_regs.timer[index].value); break;
			case TIMER_LIMIT:
				COMBINE_DATA(&m_timer_regs.timer[index].limit); break;
			case TIMER_MODE:
				COMBINE_DATA(&m_timer_regs.timer[index].mode);
				if(data&1) {
					attotime period = attotime::from_hz(XTAL(54'000'000))*m_timer_regs.timer[index].timebase;
					m_timer_regs.timer[index].timer->adjust(period,index);
				} else {
					m_timer_regs.timer[index].timer->adjust(attotime::never,index);
				} break;
			case TIMER_TIMEBASE:
				COMBINE_DATA(&m_timer_regs.timer[index].timebase); break;
		}
		/* A timer will hold an interrupt active until any one of that timer?s registers is written. */
		if(m_timer_regs.timer_irq & (1<<index)) {
			m_timer_regs.timer_irq &= ~(1<<index);
		}
	}
}

uint32_t cxhumax_state::cx_uart2_r(offs_t offset)
{
	uint32_t data;
	switch (offset) {
		case UART_STAT_REG:
			/* Transmitter Idle */
			data = UART_STAT_TID_BIT | UART_STAT_TSR_BIT; break;
		default:
			data = m_uart2_regs[offset]; break;
	}
	LOG("%s: (UART2) %08X -> %08X\n", machine().describe_context(), 0xE0411000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_uart2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (UART2) %08X <- %08X\n", machine().describe_context(), 0xE0411000 + (offset << 2), data);
	switch (offset) {
		case UART_FIFO_REG:
			if(!(m_uart2_regs[UART_FRMC_REG]&UART_FRMC_BDS_BIT)) {
				/* Sending byte... add logging */
				m_terminal->write(data);

				/* Transmitter Idle Interrupt Enable */
				if(m_uart2_regs[UART_IRQE_REG]&UART_IRQE_TIDE_BIT) {
					/* Signal pending INT */
					m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] |= INT_UART2_BIT;
					m_intctrl_regs[INTREG(INTGROUP1, INTSTATCLR)] |= INT_UART2_BIT;
					m_intctrl_regs[INTREG(INTGROUP1, INTSTATSET)] |= INT_UART2_BIT;

					/* If INT is enabled at INT Ctrl raise it */
					if(m_intctrl_regs[INTREG(INTGROUP1, INTENABLE)]&INT_UART2_BIT) {
						m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
					}
				}
			}
			[[fallthrough]];
		default:
			COMBINE_DATA(&m_uart2_regs[offset]); break;
	}
}

uint32_t cxhumax_state::cx_pll_r(offs_t offset)
{
	uint32_t data = m_pll_regs[offset];
	LOG("%s: (PLL) %08X -> %08X\n", machine().describe_context(), 0xE0440000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_pll_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (PLL) %08X <- %08X\n", machine().describe_context(), 0xE0440000 + (offset << 2), data);
	COMBINE_DATA(&m_pll_regs[offset]);
}

uint32_t cxhumax_state::cx_pllprescale_r(offs_t offset)
{
	uint32_t data = m_pllprescale_reg;
	LOG("%s: (PLLPRESCALE) %08X -> %08X\n", machine().describe_context(), 0xE0440094 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_pllprescale_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (PLLPRESCALE) %08X <- %08X\n", machine().describe_context(), 0xE0440094 + (offset << 2), data);
	COMBINE_DATA(&m_pllprescale_reg);
}

uint32_t cxhumax_state::cx_clkdiv_r(offs_t offset)
{
	uint32_t data = m_clkdiv_regs[offset];
	LOG("%s: (CLKDIV) %08X -> %08X\n", machine().describe_context(), 0xE0440020 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_clkdiv_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (CLKDIV) %08X <- %08X\n", machine().describe_context(), 0xE0440020 + (offset << 2), data);
	COMBINE_DATA(&m_clkdiv_regs[offset]);
}

uint32_t cxhumax_state::cx_chipcontrol_r(offs_t offset)
{
	uint32_t data = m_chipcontrol_regs[offset];
	LOG("%s: (CHIPCONTROL) %08X -> %08X\n", machine().describe_context(), 0xE0440100 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_chipcontrol_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (CHIPCONTROL) %08X <- %08X\n", machine().describe_context(), 0xE0440100 + (offset << 2), data);
	COMBINE_DATA(&m_chipcontrol_regs[offset]);
}

uint32_t cxhumax_state::cx_intctrl_r(offs_t offset)
{
	uint32_t data = m_intctrl_regs[offset];
	LOG("%s: (INTCTRL) %08X -> %08X\n", machine().describe_context(), 0xE0450000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_intctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (INTCTRL) %08X <- %08X\n", machine().describe_context(), 0xE0450000 + (offset << 2), data);
	switch (offset >> 3) { // Decode the group
		case 0: // Group 1
			switch(offset & 7) {
				case INTSTATCLR:    // ITC_INTSTATCLR_REG Group 1
					/*
					    Bits 15 (PWM), 14 (PIO103) of Group 1 are the logical OR of their lower level interrupt
					    status bits down within the interrupting module and are not registered.

					    The source registers must be cleared to clear these interrupt bits.
					*/
					data &= ~(INT_PWM_BIT|INT_PIO103_BIT);

					m_intctrl_regs[INTREG(INTGROUP1, INTSTATCLR)] &= ~data;
					m_intctrl_regs[INTREG(INTGROUP1, INTSTATSET)] &= ~data;
					m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] &= ~data;
					break;
				default:
					COMBINE_DATA(&m_intctrl_regs[offset]);
					break;
			}
			break;
		case 1: // Group 2
			switch(offset & 7) {
				case INTSTATCLR:    // ITC_INTSTATCLR_REG Group 2
					/*
					    The timer interrupt service routine must write to one of the timer
					    registers before clearing the corresponding Interrupt Controller ISR timer
					    interrupt bit.

					    Bit 7 (Timers) of Group 2 is the logical OR of its lower level interrupt
					    status bits down within the interrupting module and are not registered.

					    The source registers must be cleared to clear these interrupt bits.
					*/
					if(m_timer_regs.timer_irq) data &= ~INT_TIMER_BIT;

					m_intctrl_regs[INTREG(INTGROUP2, INTSTATCLR)] &= ~data;
					m_intctrl_regs[INTREG(INTGROUP2, INTSTATSET)] &= ~data;
					m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] &= ~data;
					break;
				default:
					COMBINE_DATA(&m_intctrl_regs[offset]);
					break;
			}
			break;
		default:
			break;
	}

	if(m_i2c1_regs[I2C_STAT_REG]&I2C_INT_BIT)
	{
		m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] |= 1<<7;
		m_intctrl_regs[INTREG(INTGROUP1, INTSTATCLR)] |= 1<<7;
		m_intctrl_regs[INTREG(INTGROUP1, INTSTATSET)] |= 1<<7;
	}

	/* check if */
	if((m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] & m_intctrl_regs[INTREG(INTGROUP2, INTENABLE)])
		|| (m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] & m_intctrl_regs[INTREG(INTGROUP1, INTENABLE)]))
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, CLEAR_LINE);

}

uint32_t cxhumax_state::cx_ss_r(offs_t offset)
{
	uint32_t data = 0;
	switch(offset) {
		case SS_FIFC_REG:
			data = m_ss_regs[offset] & 0xFFF0;
			break;
		default:
			data = m_ss_regs[offset];
			break;
	}
	LOG("%s: (SS) %08X -> %08X\n", machine().describe_context(), 0xE0490000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_ss_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (SS) %08X <- %08X\n", machine().describe_context(), 0xE0490000 + (offset << 2), data);
	switch(offset) {
		case SS_CNTL_REG:
			if (data&1) {
				// "Send" pending data
				uint8_t tfd = (m_ss_regs[SS_STAT_REG]>>4) & 0xF;
				if ((tfd>1) && (m_ss_tx_fifo[0] == 0) && (m_ss_tx_fifo[1] != 0xFF)) {
					// ASCII
					printf("%s\n", &m_ss_tx_fifo[1]);
				} else {
					// UNKNOWN
					for (int i=0; i<tfd; i++) {
						printf("%02X ", m_ss_tx_fifo[i]);
					}
					printf("\n");
				}
				// Clear TX FIFO
				memset(m_ss_tx_fifo,0,sizeof(m_ss_tx_fifo));
				m_ss_regs[SS_STAT_REG] &= 0xFF0F;
			}
			COMBINE_DATA(&m_ss_regs[offset]);
			break;
		case SS_FIFO_REG:
			{
				// Push data into TX FIFO (if it's not full) and adjust transmit FIFO depth
				uint8_t tfd = (m_ss_regs[SS_STAT_REG]>>4) & 0xF;
				if (tfd<8) {
					m_ss_tx_fifo[tfd++] = data;
					m_ss_regs[SS_STAT_REG] = (m_ss_regs[SS_STAT_REG] & 0xFF0F) | (tfd<<4);
				}
			}
			break;
		case SS_STAT_REG:
			// read-only
			break;
		default:
			COMBINE_DATA(&m_ss_regs[offset]);
			break;
	};
}

uint32_t cxhumax_state::cx_i2c0_r(offs_t offset)
{
	uint32_t data = m_i2c0_regs[offset];
	LOG("%s: (I2C0) %08X -> %08X\n", machine().describe_context(), 0xE04E0000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_i2c0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (I2C0) %08X <- %08X\n", machine().describe_context(), 0xE04E0000 + (offset << 2), data);
	COMBINE_DATA(&m_i2c0_regs[offset]);
}

uint8_t cxhumax_state::i2cmem_read_byte(int last)
{
	uint8_t data = 0;
	int i;
	m_i2cmem->write_sda(1);
	for (i = 0; i < 8; i++)
	{
		m_i2cmem->write_scl(1);
		data = (data << 1) + (m_i2cmem->read_sda() ? 1 : 0);
		m_i2cmem->write_scl(0);
	}
	m_i2cmem->write_sda(last);
	m_i2cmem->write_scl(1);
	m_i2cmem->write_scl(0);
	return data;
}

void cxhumax_state::i2cmem_write_byte(uint8_t data)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		m_i2cmem->write_sda((data & 0x80) ? 1 : 0);
		data = data << 1;
		m_i2cmem->write_scl(1);
		m_i2cmem->write_scl(0);
	}
	m_i2cmem->write_sda(1); // ack bit
	m_i2cmem->write_scl(1);
	m_i2cmem->write_scl(0);
}

void cxhumax_state::i2cmem_start()
{
	m_i2cmem->write_sda(1);
	m_i2cmem->write_scl(1);
	m_i2cmem->write_sda(0);
	m_i2cmem->write_scl(0);
}

void cxhumax_state::i2cmem_stop()
{
	m_i2cmem->write_sda(0);
	m_i2cmem->write_scl(1);
	m_i2cmem->write_sda(1);
	m_i2cmem->write_scl(0);
}

uint32_t cxhumax_state::cx_i2c1_r(offs_t offset)
{
	uint32_t data=0;
	switch(offset) {
		case I2C_STAT_REG:
			data |= m_i2cmem->read_sda()<<3;
			[[fallthrough]];
		default:
			data |= m_i2c1_regs[offset]; break;
	}
	LOG("%s: (I2C1) %08X -> %08X\n", machine().describe_context(), 0xE04E1000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_i2c1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (I2C1) %08X <- %08X\n", machine().describe_context(), 0xE04E1000 + (offset << 2), data);
	switch(offset) {
		case I2C_CTRL_REG:
			if(data&0x10) {// START
				i2cmem_start();
			}
			if((data&0x4) || ((data&3)==3)) // I2C READ
			{
				m_i2c1_regs[I2C_RDATA_REG] = 0;
				if(data&0x10) i2cmem_write_byte((data>>24)&0xFF);
				if(m_i2c1_regs[I2C_MODE_REG]&(1<<5)) // BYTE_ORDER
				{
					for(int i=0; i<(data&3); i++) {
						m_i2c1_regs[I2C_RDATA_REG] |= i2cmem_read_byte(0) << (i*8);
					}
					m_i2c1_regs[I2C_RDATA_REG] |= i2cmem_read_byte((data&0x20)?1:0) << ((data&3)*8);
				}
				else
				{
					for(int i=0; i<(data&3); i++) {
						m_i2c1_regs[I2C_RDATA_REG] |= i2cmem_read_byte(0);
						m_i2c1_regs[I2C_RDATA_REG] <<= 8;
					}
					m_i2c1_regs[I2C_RDATA_REG] |= i2cmem_read_byte((data&0x20)?1:0);
				}
			}
			else
			{
				for(int i=0; i<=(data&3); i++) {
					i2cmem_write_byte((data>>(24-(i*8))&0xFF));
				}
			}
			if(data&0x20) {// STOP
				i2cmem_stop();
			}

			/* The interrupt status bit is set at the end of an I2C read or write operation. */
			m_i2c1_regs[I2C_STAT_REG] |= I2C_INT_BIT;
			m_i2c1_regs[I2C_STAT_REG] |= I2C_WACK_BIT;

			m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] |= 1<<7;
			m_intctrl_regs[INTREG(INTGROUP1, INTSTATCLR)] |= 1<<7;
			m_intctrl_regs[INTREG(INTGROUP1, INTSTATSET)] |= 1<<7;
			if (m_intctrl_regs[INTREG(INTGROUP1, INTENABLE)] & (1<<7)) {
					LOG("%s: (I2C1) Int\n",  machine().describe_context());
					m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, ASSERT_LINE);
			}
			break;
		case I2C_STAT_REG:
			/* The interrupt status bit may be cleared by writing (anything) to the status register, which also clears the acknowledge status. */
			data&=~(I2C_WACK_BIT|I2C_INT_BIT);
			[[fallthrough]];
		default:
			COMBINE_DATA(&m_i2c1_regs[offset]);
	}
}

uint32_t cxhumax_state::cx_i2c2_r(offs_t offset)
{
	uint32_t data = m_i2c2_regs[offset];
	LOG("%s: (I2C2) %08X -> %08X\n", machine().describe_context(), 0xE04E2000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_i2c2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (I2C2) %08X <- %08X\n", machine().describe_context(), 0xE04E2000 + (offset << 2), data);
	COMBINE_DATA(&m_i2c2_regs[offset]);
}

uint32_t cxhumax_state::cx_mc_cfg_r(offs_t offset)
{
	uint32_t data = m_mccfg_regs[offset];
	LOG("%s: (MC_CFG) %08X -> %08X\n", machine().describe_context(), 0xE0500300 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_mc_cfg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (MC_CFG) %08X <- %08X\n", machine().describe_context(), 0xE0500300 + (offset << 2), data);
	COMBINE_DATA(&m_mccfg_regs[offset]);
}

uint32_t cxhumax_state::cx_drm0_r(offs_t offset)
{
	uint32_t data = m_drm0_regs[offset];
	LOG("%s: (DRM0) %08X -> %08X\n", machine().describe_context(), 0xE0560000 + (offset << 2), data);
	switch(offset) {
		case 0x14/4: // DRM_STATUS_REG
			data |= 1<<21;
			data |= 1<<20;
	}
	return data;
}

void cxhumax_state::cx_drm0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (DRM0) %08X <- %08X\n", machine().describe_context(), 0xE0560000 + (offset << 2), data);
	COMBINE_DATA(&m_drm0_regs[offset]);
}

uint32_t cxhumax_state::cx_drm1_r(offs_t offset)
{
	uint32_t data = m_drm1_regs[offset];
	LOG("%s: (DRM1) %08X -> %08X\n", machine().describe_context(), 0xE0570000 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_drm1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (DRM1) %08X <- %08X\n", machine().describe_context(), 0xE0570000 + (offset << 2), data);
	COMBINE_DATA(&m_drm1_regs[offset]);
}

uint32_t cxhumax_state::cx_hdmi_r(offs_t offset)
{
	uint32_t data = m_hdmi_regs[offset];
	LOG("%s: (HDMI) %08X -> %08X\n", machine().describe_context(), 0xE05D0800 + (offset << 2), data);
	return data;
}

void cxhumax_state::cx_hdmi_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s: (HDMI) %08X <- %08X\n", machine().describe_context(), 0xE05D0800 + (offset << 2), data);
	switch(offset) {
		case 0x40/4: // HDMI_CONFIG_REG
			if(data&8) m_hdmi_regs[0xc0/4] |= 0x80;
	}
	COMBINE_DATA(&m_hdmi_regs[offset]);
}

void cxhumax_state::video_start()
{
}

/* copy from emu/rendersw.inc */
/*------------------------------------------------------------------------
    ycc_to_rgb - convert YCC to RGB; the YCC pixel
    contains Y in the LSB, Cb << 8, and Cr << 16
    This actually a YCbCr conversion,
    details my be found in chapter 6.4 ff of
    http://softwarecommunity.intel.com/isn/downloads/softwareproducts/pdfs/346495.pdf
    The document also contains the constants below as floats.
--------------------------------------------------------------------------*/

static inline uint8_t clamp16_shift8(uint32_t x)
{
	return (((int32_t) x < 0) ? 0 : (x > 65535 ? 255: x >> 8));
}

static inline uint32_t ycc_to_rgb(uint32_t ycc)
{
	/* original equations:

	    C = Y - 16
	    D = Cb - 128
	    E = Cr - 128

	    R = clip(( 298 * C           + 409 * E + 128) >> 8)
	    G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8)
	    B = clip(( 298 * C + 516 * D           + 128) >> 8)

	    R = clip(( 298 * (Y - 16)                    + 409 * (Cr - 128) + 128) >> 8)
	    G = clip(( 298 * (Y - 16) - 100 * (Cb - 128) - 208 * (Cr - 128) + 128) >> 8)
	    B = clip(( 298 * (Y - 16) + 516 * (Cb - 128)                    + 128) >> 8)

	    R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
	    G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
	    B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)

	    R = clip(( 298 * Y - 298 * 16                        + 409 * Cr - 409 * 128 + 128) >> 8)
	    G = clip(( 298 * Y - 298 * 16 - 100 * Cb + 100 * 128 - 208 * Cr + 208 * 128 + 128) >> 8)
	    B = clip(( 298 * Y - 298 * 16 + 516 * Cb - 516 * 128                        + 128) >> 8)

	    Now combine constants:

	    R = clip(( 298 * Y            + 409 * Cr - 56992) >> 8)
	    G = clip(( 298 * Y - 100 * Cb - 208 * Cr + 34784) >> 8)
	    B = clip(( 298 * Y + 516 * Cb            - 70688) >> 8)

	    Define common = 298 * y - 56992. This will save one addition

	    R = clip(( common            + 409 * Cr -     0) >> 8)
	    G = clip(( common - 100 * Cb - 208 * Cr + 91776) >> 8)
	    B = clip(( common + 516 * Cb            - 13696) >> 8)

	*/
	uint8_t y = ycc;
	uint8_t cb = ycc >> 8;
	uint8_t cr = ycc >> 16;
	uint32_t r, g, b, common;

	common = 298 * y - 56992;
	r = (common +            409 * cr);
	g = (common - 100 * cb - 208 * cr + 91776);
	b = (common + 516 * cb - 13696);

	/* Now clamp and shift back */
	return rgb_t(clamp16_shift8(r), clamp16_shift8(g), clamp16_shift8(b));
}

uint32_t cxhumax_state::screen_update_cxhumax(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t osd_pointer = m_drm1_regs[DRM_OSD_PTR_REG];

	if(osd_pointer)
	{
		uint32_t const *ram = m_ram;
		uint32_t const *osd_header = &ram[osd_pointer/4];
		uint8_t  const *vbuf = (uint8_t*)(&ram[osd_header[3]/4]);
		uint32_t const *palette = &ram[osd_header[7]/4];

		uint32_t x_disp_start_and_width = osd_header[1];
		uint32_t xdisp_width = (x_disp_start_and_width >> 16) & 0x1fff;
		uint32_t xdisp_start = x_disp_start_and_width & 0xfff;

		uint32_t image_height_and_width = osd_header[2];
		uint32_t yimg_height = (image_height_and_width >> 16) & 0x7ff;
		uint32_t ximg_width = image_height_and_width & 0x7ff;

		uint32_t y_position_and_region_alpha = osd_header[5];
		uint32_t ydisp_last = (y_position_and_region_alpha >> 12) & 0x7ff;
		uint32_t ydisp_start = y_position_and_region_alpha & 0x7ff;

	/*  uint32_t first_x = m_drm0_regs[DRM_ACTIVE_X_REG] & 0xffff;
	    uint32_t last_x = (m_drm0_regs[DRM_ACTIVE_X_REG] >> 16) & 0xffff;

	    uint32_t first_y = m_drm0_regs[DRM_ACTIVE_Y_REG] & 0xfff;
	    uint32_t last_y = (m_drm0_regs[DRM_ACTIVE_Y_REG] >> 16) & 0xfff;*/

		for (int j=ydisp_start; j <= ydisp_last; j++)
		{
			uint32_t *const bmp = &bitmap.pix(j);

			for (int i=xdisp_start; i <= (xdisp_start + xdisp_width); i++)
			{
				if ((i <= (xdisp_start + ximg_width)) && (j <= (ydisp_start + yimg_height))) {
					bmp[i] = palette[vbuf[i+((j-ydisp_start)*ximg_width)]]; // FIXME: need BYTE4_XOR_?E for endianness
				} else {
					bmp[i] = ycc_to_rgb(m_drm1_regs[DRM_BCKGND_REG]);
				}
			}
		}
	}
	return 0;
}

void cxhumax_state::cxhumax_map(address_map &map)
{
	map(0x00000000, 0x03ffffff).ram().share("ram").mirror(0x40000000);           // 64?MB RAM
	map(0xe0000000, 0xe000ffff).rw(FUNC(cxhumax_state::cx_hsx_r), FUNC(cxhumax_state::cx_hsx_w));                       // HSX
	map(0xe0010000, 0xe0010003).rw(FUNC(cxhumax_state::cx_romdescr_r), FUNC(cxhumax_state::cx_romdescr_w));             // ROM Descriptor
	map(0xe0010004, 0xe001000f).rw(FUNC(cxhumax_state::cx_isaromdescr_r), FUNC(cxhumax_state::cx_isaromdescr_w));       // ISA/ROM Descriptors
	map(0xe0010010, 0xe001001f).rw(FUNC(cxhumax_state::cx_isadescr_r), FUNC(cxhumax_state::cx_isadescr_w));             // ISA Descriptors
	map(0xe0010020, 0xe001002f).rw(FUNC(cxhumax_state::cx_rommap_r), FUNC(cxhumax_state::cx_rommap_w));                 // ROM Mapping
	map(0xe0010030, 0xe0010033).rw(FUNC(cxhumax_state::cx_rommode_r), FUNC(cxhumax_state::cx_rommode_w));               // ISA Mode
	map(0xe0010034, 0xe0010037).rw(FUNC(cxhumax_state::cx_xoemask_r), FUNC(cxhumax_state::cx_xoemask_w));               // XOE Mask
	map(0xe0010040, 0xe0010047).rw(FUNC(cxhumax_state::cx_pci_r), FUNC(cxhumax_state::cx_pci_w));                       // PCI
	map(0xe0010080, 0xe00100ff).rw(FUNC(cxhumax_state::cx_extdesc_r), FUNC(cxhumax_state::cx_extdesc_w));               // Extended Control
	map(0xe0400014, 0xe0400017).w(FUNC(cxhumax_state::cx_remap_w));                                   // RST_REMAP_REG
	map(0xe0400024, 0xe0400027).rw(FUNC(cxhumax_state::cx_scratch_r), FUNC(cxhumax_state::cx_scratch_w));               // RST_SCRATCH_REG - System Scratch Register
	map(0xe0430000, 0xe0430103).rw(FUNC(cxhumax_state::cx_timers_r), FUNC(cxhumax_state::cx_timers_w));                 // Timers
	map(0xe0411000, 0xe0411033).rw(FUNC(cxhumax_state::cx_uart2_r), FUNC(cxhumax_state::cx_uart2_w));                   // UART2
	map(0xe0440000, 0xe0440013).rw(FUNC(cxhumax_state::cx_pll_r), FUNC(cxhumax_state::cx_pll_w));                       // PLL Registers
	map(0xe0440020, 0xe0440037).rw(FUNC(cxhumax_state::cx_clkdiv_r), FUNC(cxhumax_state::cx_clkdiv_w));                 // Clock Divider Registers
	map(0xe0440094, 0xe0440097).rw(FUNC(cxhumax_state::cx_pllprescale_r), FUNC(cxhumax_state::cx_pllprescale_w));       // PLL Prescale
	map(0xe0440100, 0xe0440173).rw(FUNC(cxhumax_state::cx_chipcontrol_r), FUNC(cxhumax_state::cx_chipcontrol_w));       // Chip Control Registers
	map(0xe0450000, 0xe0450037).rw(FUNC(cxhumax_state::cx_intctrl_r), FUNC(cxhumax_state::cx_intctrl_w));               // Interrupt Controller Registers
	map(0xe0490000, 0xe0490017).rw(FUNC(cxhumax_state::cx_ss_r), FUNC(cxhumax_state::cx_ss_w));                         // Synchronous Serial Port
	map(0xe04e0000, 0xe04e001f).rw(FUNC(cxhumax_state::cx_i2c0_r), FUNC(cxhumax_state::cx_i2c0_w));                     // I2C0
	map(0xe04e1000, 0xe04e101f).rw(FUNC(cxhumax_state::cx_i2c1_r), FUNC(cxhumax_state::cx_i2c1_w));                     // I2C1
	map(0xe04e2000, 0xe04e201f).rw(FUNC(cxhumax_state::cx_i2c2_r), FUNC(cxhumax_state::cx_i2c2_w));                     // I2C2
	map(0xe0500300, 0xe050030b).rw(FUNC(cxhumax_state::cx_mc_cfg_r), FUNC(cxhumax_state::cx_mc_cfg_w));                 // Memory Controller configuration
	map(0xe0560000, 0xe05600fb).rw(FUNC(cxhumax_state::cx_drm0_r), FUNC(cxhumax_state::cx_drm0_w));                     // DRM0
	map(0xe0570000, 0xe05700fb).rw(FUNC(cxhumax_state::cx_drm1_r), FUNC(cxhumax_state::cx_drm1_w));                     // DRM1
	map(0xe05d0800, 0xe05d0bff).rw(FUNC(cxhumax_state::cx_hdmi_r), FUNC(cxhumax_state::cx_hdmi_w));                     // HDMI
	map(0xe0600000, 0xe063ffff).rw(FUNC(cxhumax_state::cx_gxa_r), FUNC(cxhumax_state::cx_gxa_w));                       // GXA
	map(0xe4017000, 0xe40173ff).ram();                                                 // HSX - BSP - 1K Video Shared Dual Port RAM (shared with MVP)
	map(0xe4080000, 0xe4083fff).ram();                                                 // HSX - TSP 0 - 16K Private Instructions/Data and Host-Shared Data
	map(0xf0000000, 0xf03fffff).rw(FUNC(cxhumax_state::flash_r), FUNC(cxhumax_state::flash_w)).mirror(0x08000000);   // 4MB FLASH (INTEL 28F320J3D)
	map(0xf4000000, 0xf43fffff).r(FUNC(cxhumax_state::dummy_flash_r));                                 // do we need it?
}

static INPUT_PORTS_START( cxhumax )
INPUT_PORTS_END

void cxhumax_state::machine_start()
{
	int index = 0;
	for(index = 0; index < MAX_CX_TIMERS; index++)
	{
		m_timer_regs.timer[index].timer = timer_alloc(FUNC(cxhumax_state::timer_tick), this);
		m_timer_regs.timer[index].timer->adjust(attotime::never, index);
	}
}

void cxhumax_state::machine_reset()
{
	m_i2c0_regs[0x08/4] = 0x08; // SDA high
	m_i2c2_regs[0x08/4] = 0x08; // SDA high

	uint8_t* FLASH = memregion("flash")->base();
	memcpy(m_ram,FLASH,0x400000);

	m_chipcontrol_regs[PIN_CONFIG_0_REG] =
		1 << 0  | /* Short Reset: 0=200ms delay ; 1=1ms delay */
		1 << 1  | /* Software config bit. OK */
		1 << 4  | /* SDRAM memory controller data width. 0=16bit 1=32bit */
		1 << 11 | /* I/O Addr bus width 11=23 bit 10=22bit 01=21bit 00=20bit / PCImode: 0=held in reset 1=normal reset OK? */
		0 << 16 | /* 0=PCI mode 1=Standard I/O mode */
		1 << 23 | /* 0=PCI device 1=PCI host bridge OK */
		1 << 26 | /* 0=SC1 used for NDS 1=SC1 not used for NDS */
		1 << 27 | /* 0=8bit ROM 1=16bit ROM */
		1 << 28 | /* 0=SC0 used for NDS 1=SC0 not used for NDS */
		0 << 29 | /* 0=using SC2 1=not using SC2 */
		0 << 30 | /* 0=using SC1 (TDA8004) 1=not using SC1 */
		1 << 31;  /* 0=Ext clk for boot 1=Int PLL for boot OK */
	m_chipcontrol_regs[SREG_MODE_REG] = 0x0000020F;

	memset(m_isaromdescr_regs,0,sizeof(m_isaromdescr_regs));
	memset(m_isadescr_regs,0,sizeof(m_isadescr_regs));
	m_rommode_reg=0;
	m_xoemask_reg=0;
	memset(m_extdesc_regs,0,sizeof(m_extdesc_regs));
	memset(m_drm1_regs,0,sizeof(m_drm1_regs));

	m_pll_regs[SREG_MPG_0_INTFRAC_REG] = (0x1A << 25) /* integer */ | 0x5D1764 /* fraction */;
	m_pll_regs[SREG_MPG_1_INTFRAC_REG] = (0x1A << 25) /* integer */ | 0x5D1764 /* fraction */;
	m_pll_regs[SREG_ARM_INTFRAC_REG] = (0x28 << 25) /* integer */ | 0xCEDE62 /* fraction */;
	m_pll_regs[SREG_MEM_INTFRAC_REG] = (0x13 << 25) /* integer */ | 0xC9B26D /* fraction */;
	m_pll_regs[SREG_USB_INTFRAC_REG] = (0x08 << 25) /* integer */ | 0x52BF5B /* fraction */;

	m_clkdiv_regs[SREG_DIV_0_REG] = (2<<0)|(1<<6)|(2<<8)|(2<<14)|(10<<16)|(1<<22)|(10<<24)|(1<<30);
	m_clkdiv_regs[SREG_DIV_1_REG] = (5<<0)|(0<<6)|(12<<8)|(0<<14)|(4<<16)|(1<<22)|(5<<24)|(1<<30);
	m_clkdiv_regs[SREG_DIV_2_REG] = (22<<0)|(0<<6)|(12<<8)|(1<<14)|(4<<16)|(3<<22); //|(5<<24)|(1<<30);???
	m_clkdiv_regs[SREG_DIV_3_REG] = (5<<0)|(0<<6)|(5<<8)|(0<<14)|(5<<16)|(0<<22)|(5<<24)|(0<<30);
	m_clkdiv_regs[SREG_DIV_4_REG] = (8<<0)|(0<<6)|(5<<8)|(0<<14)|(5<<16)|(0<<22)|(5<<24)|(0<<30);
	m_clkdiv_regs[SREG_DIV_5_REG] = (8<<0)|(0<<6)|(5<<8)|(0<<14)|(5<<16)|(0<<22);

	m_pllprescale_reg=0xFFF;

	m_mccfg_regs[MC_CFG0] = ((m_chipcontrol_regs[PIN_CONFIG_0_REG]>>4)&1)<<16;
	m_mccfg_regs[MC_CFG1] = 0;
	m_mccfg_regs[MC_CFG2] = (7<<8)|(7<<0);

	// UART2
	m_uart2_regs[UART_FIFC_REG] = 0x30;

	// Clear SS TX FIFO
	memset(m_ss_tx_fifo,0,sizeof(m_ss_tx_fifo));
	m_ss_regs[SS_BAUD_REG] = 1; // Default SS clock = 13,5MHz

	memset(m_intctrl_regs,0,sizeof(m_intctrl_regs));

	memset(m_hdmi_regs,0,sizeof(m_hdmi_regs));

	memset(m_gxa_cmd_regs,0,sizeof(m_gxa_cmd_regs));
}

void cxhumax_state::cxhumax(machine_config &config)
{
	ARM920T(config, m_maincpu, 180000000); // CX24175 (RevC up?)
	m_maincpu->set_addrmap(AS_PROGRAM, &cxhumax_state::cxhumax_map);


	INTEL_28F320J3D(config, "flash");
	I2C_24C64(config, "eeprom", 0); // 24LC64

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(1920, 1080);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(cxhumax_state::screen_update_cxhumax));

	PALETTE(config, "palette", palette_device::MONOCHROME);

	GENERIC_TERMINAL(config, m_terminal, 0);
}

ROM_START( hxhdci2k )
	ROM_REGION( 0x400000, "flash", 0 )
	ROM_SYSTEM_BIOS( 0, "fw10005", "HDCI REV 1.0 RHDXSCI 1.00.05" ) /* 19 AUG 2008 */
	ROM_LOAD16_WORD_SWAP( "28f320j3d.bin", 0x000000, 0x400000, BAD_DUMP CRC(63d98942) SHA1(c5b8d701677a3edc25f203854f44953b19c9158d) )

	ROM_REGION( 0x2000, "eeprom", 0 )
	ROM_LOAD( "24lc64.bin", 0x0000, 0x2000, NO_DUMP)
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME           FLAGS
SYST( 2008, hxhdci2k, 0,      0,      cxhumax, cxhumax, cxhumax_state, empty_init, "HUMAX", "HUMAX HDCI-2000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
