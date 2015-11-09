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
#include "includes/cxhumax.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( device_t &device, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		device.logerror( "%s: %s", device.machine().describe_context( ), buf);
	}
}

READ32_MEMBER ( cxhumax_state::cx_gxa_r )
{
	UINT32 res = m_gxa_cmd_regs[offset];
	verboselog(*this, 9, "(GXA) %08X -> %08X\n", 0xE0600000 + (offset << 2), res);
/*  UINT8 gxa_command_number = (offset >> 9) & 0x7F;
    verboselog(*this, 9, "      Command: %08X\n", gxa_command_number);
    switch (gxa_command_number) {
        case GXA_CMD_RW_REGISTER:
            switch(offset) {
                case GXA_CFG2_REG:
                    break;
                default:
                    verboselog(*this, 9, "      Unimplemented register - TODO?\n");
                    break;
            }
            break;
        default:
            // do we need it?
            verboselog(*this, 9, "      Unimplemented read command - TODO?\n");
            break;
    }*/
	return res;
}

WRITE32_MEMBER( cxhumax_state::cx_gxa_w )
{
	verboselog(*this, 9, "(GXA) %08X <- %08X\n", 0xE0600000 + (offset << 2), data);
	UINT8 gxa_command_number = (offset >> 9) & 0x7F;
	verboselog(*this, 9, "      Command: %08X\n", gxa_command_number);

	/* Clear non persistent data */
	m_gxa_cmd_regs[GXA_CMD_REG] &= 0xfffc0000;

	if (gxa_command_number == GXA_CMD_RW_REGISTER) {
		verboselog(*this, 9, "      Register Number: %08X\n", offset & 0xff);
	} else {
		m_gxa_cmd_regs[GXA_CMD_REG] |= (offset << 2) & 0x3ffff;
		verboselog(*this, 9, "      Source Bitmap Selector: %08X\n", (offset >> 6) & 0x7);
		verboselog(*this, 9, "      Destination Bitmap Selector: %08X\n", (offset >> 3) & 0x7);
		verboselog(*this, 9, "      Parameter Count: %08X\n", offset & 0x7);
	}
	switch (gxa_command_number) {
		case GXA_CMD_RW_REGISTER:
			switch(offset) {
				case GXA_CFG2_REG:
					// clear IRQ_STAT bits if requested
					m_gxa_cmd_regs[GXA_CFG2_REG] = (m_gxa_cmd_regs[GXA_CFG2_REG]&(0xfff00000 & ~(data&0x00300000))) | (data & 0x000fffff);
					break;
				default:
					verboselog(*this, 9, "      Unimplemented register - TODO?\n");
					COMBINE_DATA(&m_gxa_cmd_regs[offset]);
					break;
			}
			break;
		case GXA_CMD_QMARK:
			verboselog(*this, 9, "      QMARK - TODO?\n");

			/* Set value and copy of WAIT4_VERTICAL bit written by QMARK */
			m_gxa_cmd_regs[GXA_CMD_REG] = (m_gxa_cmd_regs[GXA_CMD_REG] & 0x3ffff) | (data<<24) | ((data&0x10)?1<<23:0);

			/* QMARK command has completed */
			m_gxa_cmd_regs[GXA_CFG2_REG] |= (1<<IRQ_STAT_QMARK);

			// Interrupt
			if (m_gxa_cmd_regs[GXA_CFG2_REG] & (1<<IRQ_EN_QMARK)) {
				m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] |= 1<<18;
				m_intctrl_regs[INTREG(INTGROUP2, INTSTATCLR)] |= 1<<18;
				m_intctrl_regs[INTREG(INTGROUP2, INTSTATSET)] |= 1<<18;
				verboselog(*this, 9, "      QMARK INT - TODO?\n");
			}

			if((m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] & m_intctrl_regs[INTREG(INTGROUP2, INTENABLE)])
				|| (m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] & m_intctrl_regs[INTREG(INTGROUP1, INTENABLE)]))
					m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);

			break;
		default:
			verboselog(*this, 9, "      Unimplemented command - TODO?\n");
			break;
	}
}

WRITE32_MEMBER ( cxhumax_state::flash_w )
{
	offset *= 2;
	if(ACCESSING_BITS_0_15)
		m_flash->write(offset, data);
	if(ACCESSING_BITS_16_31)
		m_flash->write(offset+1, data >> 16);
	verboselog(*this, 9, "(FLASH) %08X <- %08X\n", 0xF0000000 + (offset << 2), data);
}

READ32_MEMBER ( cxhumax_state::flash_r )
{
	UINT32 res = 0;
	offset *= 2;
	if(ACCESSING_BITS_0_15)
		res |= m_flash->read(offset);
	if(ACCESSING_BITS_16_31)
		res |= m_flash->read(offset+1) << 16;
	//if(m_flash->m_flash_mode!=FM_NORMAL) verboselog(*this, 9, "(FLASH) %08X -> %08X\n", 0xF0000000 + (offset << 2), res);
	return res;
}

READ32_MEMBER ( cxhumax_state::dummy_flash_r )
{
	return 0xFFFFFFFF;
}

WRITE32_MEMBER ( cxhumax_state::cx_remap_w )
{
	if(!(data&1)) {
		verboselog(*this, 9, "(REMAP) %08X -> %08X\n", 0xE0400014 + (offset << 2), data);
		memset(m_ram, 0, 0x400000); // workaround :P
	}
}

READ32_MEMBER( cxhumax_state::cx_scratch_r )
{
	UINT32 data = m_scratch_reg;
	verboselog(*this, 9, "(SCRATCH) %08X -> %08X\n", 0xE0400024 + (offset << 2), data);

	if((m_maincpu->pc()==0xF0003BB8) || (m_maincpu->pc()==0x01003724) || (m_maincpu->pc()==0x00005d8c)) { // HDCI-2000
		//we're in disabled debug_printf
		unsigned char* buf = (unsigned char *)alloca(200);
		unsigned char temp;
		address_space &program = m_maincpu->space(AS_PROGRAM);

		memset(buf,0,200);

		int i = 0;
		while ((temp=program.read_byte(m_maincpu->state_int(ARM7_R0)+i))) {
			buf[i++]=temp;
			//m_terminal->write(space, 0, temp);
		}
		osd_printf_debug("%s", buf);
		verboselog(*this, 9, "(DEBUG) %s", buf);
	}
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_scratch_w )
{
	verboselog(*this, 9, "(SCRATCH) %08X <- %08X\n", 0xE0400024 + (offset << 2), data);
	COMBINE_DATA(&m_scratch_reg);
}

READ32_MEMBER( cxhumax_state::cx_hsx_r )
{
	UINT32 data = 0; // dummy
	verboselog(*this, 9, "(HSX) %08X -> %08X\n", 0xE0000000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_hsx_w )
{
	verboselog(*this, 9, "(HSX) %08X <- %08X\n", 0xE0000000 + (offset << 2), data);
}

READ32_MEMBER( cxhumax_state::cx_romdescr_r )
{
	UINT32 data = m_romdescr_reg;
	verboselog(*this, 9, "(ROMDESC0) %08X -> %08X\n", 0xE0010000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_romdescr_w )
{
	verboselog(*this, 9, "(ROMDESC0) %08X <- %08X\n", 0xE0010000 + (offset << 2), data);
	COMBINE_DATA(&m_romdescr_reg);
}

READ32_MEMBER( cxhumax_state::cx_isaromdescr_r )
{
	UINT32 data = m_isaromdescr_regs[offset];
	verboselog(*this, 9, "(ISAROMDESC%d) %08X -> %08X\n", offset+1, 0xE0010004 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_isaromdescr_w )
{
	verboselog(*this, 9, "(ISAROMDESC%d) %08X <- %08X\n", offset+1, 0xE0010004 + (offset << 2), data);
	COMBINE_DATA(&m_isaromdescr_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_isadescr_r )
{
	UINT32 data = m_isaromdescr_regs[offset];
	verboselog(*this, 9, "(ISA_DESC%d) %08X -> %08X\n", offset+4, 0xE0010010 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_isadescr_w )
{
	verboselog(*this, 9, "(ISA_DESC%d) %08X <- %08X\n", offset+4, 0xE0010010 + (offset << 2), data);
	COMBINE_DATA(&m_isaromdescr_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_rommap_r )
{
	UINT32 data = 0;
	verboselog(*this, 9, "(ROM%d_MAP) %08X -> %08X\n", offset, 0xE0010020 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_rommap_w )
{
	verboselog(*this, 9, "(ROM%d_MAP) %08X <- %08X\n", offset, 0xE0010020 + (offset << 2), data);
}

READ32_MEMBER( cxhumax_state::cx_rommode_r )
{
	UINT32 data = m_rommode_reg;
	verboselog(*this, 9, "(ROMMODE) %08X -> %08X\n", 0xE0010034 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_rommode_w )
{
	verboselog(*this, 9, "(ROMMODE) %08X <- %08X\n", 0xE0010034 + (offset << 2), data);
	COMBINE_DATA(&m_rommode_reg);
}

READ32_MEMBER( cxhumax_state::cx_xoemask_r )
{
	UINT32 data = m_xoemask_reg;
	verboselog(*this, 9, "(XOEMASK) %08X -> %08X\n", 0xE0010034 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_xoemask_w )
{
	verboselog(*this, 9, "(XOEMASK) %08X <- %08X\n", 0xE0010034 + (offset << 2), data);
	COMBINE_DATA(&m_xoemask_reg);
}

READ32_MEMBER( cxhumax_state::cx_pci_r )
{
	UINT32 data = 0;
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
	verboselog(*this, 9, "(PCI) %08X -> %08X\n", 0xE0010040 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_pci_w )
{
	verboselog(*this, 9, "(PCI) %08X <- %08X\n", 0xE0010040 + (offset << 2), data);
	COMBINE_DATA(&m_pci_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_extdesc_r )
{
	UINT32 data = m_extdesc_regs[offset];
	verboselog(*this, 9, "(EXTDESC) %08X -> %08X\n", 0xE0010080 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_extdesc_w )
{
	verboselog(*this, 9, "(EXTDESC) %08X <- %08X\n", 0xE0010080 + (offset << 2), data);
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
			verboselog(*this, 9, "(TIMER%d) Interrupt\n", param);
			m_intctrl_regs[INTREG(INTGROUP2, INTIRQ)] |= INT_TIMER_BIT;     /* Timer interrupt */
			m_intctrl_regs[INTREG(INTGROUP2, INTSTATCLR)] |= INT_TIMER_BIT; /* Timer interrupt */
			m_intctrl_regs[INTREG(INTGROUP2, INTSTATSET)] |= INT_TIMER_BIT; /* Timer interrupt */

			m_timer_regs.timer_irq |= 1<<param; /* Indicate which timer interrupted */

			/* Interrupt if Timer interrupt is not masked in ITC_INTENABLE_REG */
			if (m_intctrl_regs[INTREG(INTGROUP2, INTENABLE)] & INT_TIMER_BIT)
				m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
		}
	}
	attotime period = attotime::from_hz(XTAL_54MHz)*m_timer_regs.timer[param].timebase;
	m_timer_regs.timer[param].timer->adjust(period,param);
}

READ32_MEMBER( cxhumax_state::cx_timers_r )
{
	UINT32 data = 0;
	UINT8 index = offset>>2;
	if(index==16) {
		data = m_timer_regs.timer_irq;
		//m_timer_regs.timer_irq=0;
		verboselog(*this, 9, "(TIMERIRQ) %08X -> %08X\n", 0xE0430000 + (offset << 2), data);
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
		verboselog(*this, 9, "(TIMER%d) %08X -> %08X\n", offset>>2, 0xE0430000 + (offset << 2), data);
	}
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_timers_w )
{
	UINT8 index = offset>>2;
	if(index==16) {
		verboselog(*this, 9, "(TIMERIRQ) %08X <- %08X\n", 0xE0430000 + (offset << 2), data);
		COMBINE_DATA(&m_timer_regs.timer_irq);
	}
	else {
		verboselog(*this, 9, "(TIMER%d) %08X <- %08X\n", index, 0xE0430000 + (offset << 2), data);
		switch(offset&3) {
			case TIMER_VALUE:
				COMBINE_DATA(&m_timer_regs.timer[index].value); break;
			case TIMER_LIMIT:
				COMBINE_DATA(&m_timer_regs.timer[index].limit); break;
			case TIMER_MODE:
				COMBINE_DATA(&m_timer_regs.timer[index].mode);
				if(data&1) {
					attotime period = attotime::from_hz(XTAL_54MHz)*m_timer_regs.timer[index].timebase;
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

READ32_MEMBER( cxhumax_state::cx_uart2_r )
{
	UINT32 data;
	switch (offset) {
		case UART_STAT_REG:
			/* Transmitter Idle */
			data = UART_STAT_TID_BIT | UART_STAT_TSR_BIT; break;
		default:
			data = m_uart2_regs[offset]; break;
	}
	verboselog(*this, 9, "(UART2) %08X -> %08X\n", 0xE0411000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_uart2_w )
{
	verboselog(*this, 9, "(UART2) %08X <- %08X\n", 0xE0411000 + (offset << 2), data);
	switch (offset) {
		case UART_FIFO_REG:
			if(!(m_uart2_regs[UART_FRMC_REG]&UART_FRMC_BDS_BIT)) {
				/* Sending byte... add logging */
				m_terminal->write(space, 0, data);

				/* Transmitter Idle Interrupt Enable */
				if(m_uart2_regs[UART_IRQE_REG]&UART_IRQE_TIDE_BIT) {
					/* Signal pending INT */
					m_intctrl_regs[INTREG(INTGROUP1, INTIRQ)] |= INT_UART2_BIT;
					m_intctrl_regs[INTREG(INTGROUP1, INTSTATCLR)] |= INT_UART2_BIT;
					m_intctrl_regs[INTREG(INTGROUP1, INTSTATSET)] |= INT_UART2_BIT;

					/* If INT is enabled at INT Ctrl raise it */
					if(m_intctrl_regs[INTREG(INTGROUP1, INTENABLE)]&INT_UART2_BIT) {
						m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
					}
				}
			}
		default:
			COMBINE_DATA(&m_uart2_regs[offset]); break;
	}
}

READ32_MEMBER( cxhumax_state::cx_pll_r )
{
	UINT32 data = m_pll_regs[offset];
	verboselog(*this, 9, "(PLL) %08X -> %08X\n", 0xE0440000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_pll_w )
{
	verboselog(*this, 9, "(PLL) %08X <- %08X\n", 0xE0440000 + (offset << 2), data);
	COMBINE_DATA(&m_pll_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_pllprescale_r )
{
	UINT32 data = m_pllprescale_reg;
	verboselog(*this, 9, "(PLLPRESCALE) %08X -> %08X\n", 0xE0440094 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_pllprescale_w )
{
	verboselog(*this, 9, "(PLLPRESCALE) %08X <- %08X\n", 0xE0440094 + (offset << 2), data);
	COMBINE_DATA(&m_pllprescale_reg);
}

READ32_MEMBER( cxhumax_state::cx_clkdiv_r )
{
	UINT32 data = m_clkdiv_regs[offset];
	verboselog(*this, 9, "(CLKDIV) %08X -> %08X\n", 0xE0440020 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_clkdiv_w )
{
	verboselog(*this, 9, "(CLKDIV) %08X <- %08X\n", 0xE0440020 + (offset << 2), data);
	COMBINE_DATA(&m_clkdiv_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_chipcontrol_r )
{
	UINT32 data = m_chipcontrol_regs[offset];
	verboselog(*this, 9, "(CHIPCONTROL) %08X -> %08X\n", 0xE0440100 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_chipcontrol_w )
{
	verboselog(*this, 9, "(CHIPCONTROL) %08X <- %08X\n", 0xE0440100 + (offset << 2), data);
	COMBINE_DATA(&m_chipcontrol_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_intctrl_r )
{
	UINT32 data = m_intctrl_regs[offset];
	verboselog(*this, 9, "(INTCTRL) %08X -> %08X\n", 0xE0450000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_intctrl_w )
{
	verboselog(*this, 9, "(INTCTRL) %08X <- %08X\n", 0xE0450000 + (offset << 2), data);
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
		m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(ARM7_IRQ_LINE, CLEAR_LINE);

}

READ32_MEMBER( cxhumax_state::cx_ss_r )
{
	UINT32 data = 0;
	switch(offset) {
		case SS_FIFC_REG:
			data = m_ss_regs[offset] & 0xFFF0;
			break;
		default:
			data = m_ss_regs[offset];
			break;
	}
	verboselog(*this, 9, "(SS) %08X -> %08X\n", 0xE0490000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_ss_w )
{
	verboselog(*this, 9, "(SS) %08X <- %08X\n", 0xE0490000 + (offset << 2), data);
	switch(offset) {
		case SS_CNTL_REG:
			if (data&1) {
				// "Send" pending data
				UINT8 tfd = (m_ss_regs[SS_STAT_REG]>>4) & 0xF;
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
				UINT8 tfd = (m_ss_regs[SS_STAT_REG]>>4) & 0xF;
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

READ32_MEMBER( cxhumax_state::cx_i2c0_r )
{
	UINT32 data = m_i2c0_regs[offset];
	verboselog(*this, 9, "(I2C0) %08X -> %08X\n", 0xE04E0000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_i2c0_w )
{
	verboselog(*this, 9, "(I2C0) %08X <- %08X\n", 0xE04E0000 + (offset << 2), data);
	COMBINE_DATA(&m_i2c0_regs[offset]);
}

UINT8 cxhumax_state::i2cmem_read_byte(int last)
{
	UINT8 data = 0;
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

void cxhumax_state::i2cmem_write_byte(UINT8 data)
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

READ32_MEMBER( cxhumax_state::cx_i2c1_r )
{
	UINT32 data=0;
	switch(offset) {
		case I2C_STAT_REG:
			data |= m_i2cmem->read_sda()<<3;
			// fall
		default:
			data |= m_i2c1_regs[offset]; break;
	}
	verboselog(*this, 9, "(I2C1) %08X -> %08X\n", 0xE04E1000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_i2c1_w )
{
	verboselog(*this, 9, "(I2C1) %08X <- %08X\n", 0xE04E1000 + (offset << 2), data);
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
					verboselog(*this, 9, "(I2C1) Int\n" );
					m_maincpu->set_input_line(ARM7_IRQ_LINE, ASSERT_LINE);
			}
			break;
		case I2C_STAT_REG:
			/* The interrupt status bit may be cleared by writing (anything) to the status register, which also clears the acknowledge status. */
			data&=~(I2C_WACK_BIT|I2C_INT_BIT);
			// fall
		default:
			COMBINE_DATA(&m_i2c1_regs[offset]);
	}
}

READ32_MEMBER( cxhumax_state::cx_i2c2_r )
{
	UINT32 data = m_i2c2_regs[offset];
	verboselog(*this, 9, "(I2C2) %08X -> %08X\n", 0xE04E2000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_i2c2_w )
{
	verboselog(*this, 9, "(I2C2) %08X <- %08X\n", 0xE04E2000 + (offset << 2), data);
	COMBINE_DATA(&m_i2c2_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_mc_cfg_r )
{
	UINT32 data = m_mccfg_regs[offset];
	verboselog(*this, 9, "(MC_CFG) %08X -> %08X\n", 0xE0500300 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_mc_cfg_w )
{
	verboselog(*this, 9, "(MC_CFG) %08X <- %08X\n", 0xE0500300 + (offset << 2), data);
	COMBINE_DATA(&m_mccfg_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_drm0_r )
{
	UINT32 data = m_drm0_regs[offset];
	verboselog(*this, 9, "(DRM0) %08X -> %08X\n", 0xE0560000 + (offset << 2), data);
	switch(offset) {
		case 0x14/4: // DRM_STATUS_REG
			data |= 1<<21;
			data |= 1<<20;
	}
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_drm0_w )
{
	verboselog(*this, 9, "(DRM0) %08X <- %08X\n", 0xE0560000 + (offset << 2), data);
	COMBINE_DATA(&m_drm0_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_drm1_r )
{
	UINT32 data = m_drm1_regs[offset];
	verboselog(*this, 9, "(DRM1) %08X -> %08X\n", 0xE0570000 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_drm1_w )
{
	verboselog(*this, 9, "(DRM1) %08X <- %08X\n", 0xE0570000 + (offset << 2), data);
	COMBINE_DATA(&m_drm1_regs[offset]);
}

READ32_MEMBER( cxhumax_state::cx_hdmi_r )
{
	UINT32 data = m_hdmi_regs[offset];
	verboselog(*this, 9, "(HDMI) %08X -> %08X\n", 0xE05D0800 + (offset << 2), data);
	return data;
}

WRITE32_MEMBER( cxhumax_state::cx_hdmi_w )
{
	verboselog(*this, 9, "(HDMI) %08X <- %08X\n", 0xE05D0800 + (offset << 2), data);
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

INLINE UINT8 clamp16_shift8(UINT32 x)
{
	return (((INT32) x < 0) ? 0 : (x > 65535 ? 255: x >> 8));
}

INLINE UINT32 ycc_to_rgb(UINT32 ycc)
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
	UINT8 y = ycc;
	UINT8 cb = ycc >> 8;
	UINT8 cr = ycc >> 16;
	UINT32 r, g, b, common;

	common = 298 * y - 56992;
	r = (common +            409 * cr);
	g = (common - 100 * cb - 208 * cr + 91776);
	b = (common + 516 * cb - 13696);

	/* Now clamp and shift back */
	return rgb_t(clamp16_shift8(r), clamp16_shift8(g), clamp16_shift8(b));
}

UINT32 cxhumax_state::screen_update_cxhumax(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int i, j;


	UINT32 osd_pointer = m_drm1_regs[DRM_OSD_PTR_REG];

	if(osd_pointer)
	{
		UINT32 *ram = m_ram;
		UINT32 *osd_header = &ram[osd_pointer/4];
		UINT8  *vbuf = (UINT8*)(&ram[osd_header[3]/4]);
		UINT32 *palette = &ram[osd_header[7]/4];

		UINT32 x_disp_start_and_width = osd_header[1];
		UINT32 xdisp_width = (x_disp_start_and_width >> 16) & 0x1fff;
		UINT32 xdisp_start = x_disp_start_and_width & 0xfff;

		UINT32 image_height_and_width = osd_header[2];
		UINT32 yimg_height = (image_height_and_width >> 16) & 0x7ff;
		UINT32 ximg_width = image_height_and_width & 0x7ff;

		UINT32 y_position_and_region_alpha = osd_header[5];
		UINT32 ydisp_last = (y_position_and_region_alpha >> 12) & 0x7ff;
		UINT32 ydisp_start = y_position_and_region_alpha & 0x7ff;

	/*  UINT32 first_x = m_drm0_regs[DRM_ACTIVE_X_REG] & 0xffff;
	    UINT32 last_x = (m_drm0_regs[DRM_ACTIVE_X_REG] >> 16) & 0xffff;

	    UINT32 first_y = m_drm0_regs[DRM_ACTIVE_Y_REG] & 0xfff;
	    UINT32 last_y = (m_drm0_regs[DRM_ACTIVE_Y_REG] >> 16) & 0xfff;*/

		for (j=ydisp_start; j <= ydisp_last; j++)
		{
			UINT32 *bmp = &bitmap.pix32(j);

			for (i=xdisp_start; i <= (xdisp_start + xdisp_width); i++)
			{
				if ((i <= (xdisp_start + ximg_width)) && (j <= (ydisp_start + yimg_height))) {
					bmp[i] = palette[vbuf[i+((j-ydisp_start)*ximg_width)]];
				} else {
					bmp[i] = ycc_to_rgb(m_drm1_regs[DRM_BCKGND_REG]);
				}
			}
		}
	}
	return 0;
}

static ADDRESS_MAP_START(cxhumax_map, AS_PROGRAM, 32, cxhumax_state)
	AM_RANGE(0x00000000, 0x03ffffff) AM_RAM AM_SHARE("ram") AM_MIRROR(0x40000000)           // 64?MB RAM
	AM_RANGE(0xe0000000, 0xe000ffff) AM_READWRITE(cx_hsx_r, cx_hsx_w)                       // HSX
	AM_RANGE(0xe0010000, 0xe0010003) AM_READWRITE(cx_romdescr_r, cx_romdescr_w)             // ROM Descriptor
	AM_RANGE(0xe0010004, 0xe001000f) AM_READWRITE(cx_isaromdescr_r, cx_isaromdescr_w)       // ISA/ROM Descriptors
	AM_RANGE(0xe0010010, 0xe001001f) AM_READWRITE(cx_isadescr_r, cx_isadescr_w)             // ISA Descriptors
	AM_RANGE(0xe0010020, 0xe001002f) AM_READWRITE(cx_rommap_r, cx_rommap_w)                 // ROM Mapping
	AM_RANGE(0xe0010030, 0xe0010033) AM_READWRITE(cx_rommode_r, cx_rommode_w)               // ISA Mode
	AM_RANGE(0xe0010034, 0xe0010037) AM_READWRITE(cx_xoemask_r, cx_xoemask_w)               // XOE Mask
	AM_RANGE(0xe0010040, 0xe0010047) AM_READWRITE(cx_pci_r, cx_pci_w)                       // PCI
	AM_RANGE(0xe0010080, 0xe00100ff) AM_READWRITE(cx_extdesc_r, cx_extdesc_w)               // Extended Control
	AM_RANGE(0xe0400014, 0xe0400017) AM_WRITE(cx_remap_w)                                   // RST_REMAP_REG
	AM_RANGE(0xe0400024, 0xe0400027) AM_READWRITE(cx_scratch_r, cx_scratch_w)               // RST_SCRATCH_REG - System Scratch Register
	AM_RANGE(0xe0430000, 0xe0430103) AM_READWRITE(cx_timers_r, cx_timers_w)                 // Timers
	AM_RANGE(0xe0411000, 0xe0411033) AM_READWRITE(cx_uart2_r, cx_uart2_w)                   // UART2
	AM_RANGE(0xe0440000, 0xe0440013) AM_READWRITE(cx_pll_r, cx_pll_w)                       // PLL Registers
	AM_RANGE(0xe0440020, 0xe0440037) AM_READWRITE(cx_clkdiv_r, cx_clkdiv_w)                 // Clock Divider Registers
	AM_RANGE(0xe0440094, 0xe0440097) AM_READWRITE(cx_pllprescale_r, cx_pllprescale_w)       // PLL Prescale
	AM_RANGE(0xe0440100, 0xe0440173) AM_READWRITE(cx_chipcontrol_r, cx_chipcontrol_w)       // Chip Control Registers
	AM_RANGE(0xe0450000, 0xe0450037) AM_READWRITE(cx_intctrl_r, cx_intctrl_w)               // Interrupt Controller Registers
	AM_RANGE(0xe0490000, 0xe0490017) AM_READWRITE(cx_ss_r, cx_ss_w)                         // Synchronous Serial Port
	AM_RANGE(0xe04e0000, 0xe04e001f) AM_READWRITE(cx_i2c0_r, cx_i2c0_w)                     // I2C0
	AM_RANGE(0xe04e1000, 0xe04e101f) AM_READWRITE(cx_i2c1_r, cx_i2c1_w)                     // I2C1
	AM_RANGE(0xe04e2000, 0xe04e201f) AM_READWRITE(cx_i2c2_r, cx_i2c2_w)                     // I2C2
	AM_RANGE(0xe0500300, 0xe050030b) AM_READWRITE(cx_mc_cfg_r, cx_mc_cfg_w)                 // Memory Controller configuration
	AM_RANGE(0xe0560000, 0xe05600fb) AM_READWRITE(cx_drm0_r, cx_drm0_w)                     // DRM0
	AM_RANGE(0xe0570000, 0xe05700fb) AM_READWRITE(cx_drm1_r, cx_drm1_w)                     // DRM1
	AM_RANGE(0xe05d0800, 0xe05d0bff) AM_READWRITE(cx_hdmi_r, cx_hdmi_w)                     // HDMI
	AM_RANGE(0xe0600000, 0xe063ffff) AM_READWRITE(cx_gxa_r, cx_gxa_w)                       // GXA
	AM_RANGE(0xe4017000, 0xe40173ff) AM_RAM                                                 // HSX - BSP - 1K Video Shared Dual Port RAM (shared with MVP)
	AM_RANGE(0xe4080000, 0xe4083fff) AM_RAM                                                 // HSX - TSP 0 - 16K Private Instructions/Data and Host-Shared Data
	AM_RANGE(0xf0000000, 0xf03fffff) AM_READWRITE(flash_r, flash_w) AM_MIRROR(0xf8000000)   // 4MB FLASH (INTEL 28F320J3D)
	AM_RANGE(0xf4000000, 0xf43fffff) AM_READ(dummy_flash_r)                                 // do we need it?
ADDRESS_MAP_END

static INPUT_PORTS_START( cxhumax )
INPUT_PORTS_END

void cxhumax_state::machine_start()
{
	int index = 0;
	for(index = 0; index < MAX_CX_TIMERS; index++)
	{
		m_timer_regs.timer[index].timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cxhumax_state::timer_tick),this));
		m_timer_regs.timer[index].timer->adjust(attotime::never, index);
	}
}

void cxhumax_state::machine_reset()
{
	m_i2c0_regs[0x08/4] = 0x08; // SDA high
	m_i2c2_regs[0x08/4] = 0x08; // SDA high

	UINT8* FLASH = memregion("flash")->base();
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

static MACHINE_CONFIG_START( cxhumax, cxhumax_state )
	MCFG_CPU_ADD("maincpu", ARM920T, 180000000) // CX24175 (RevC up?)
	MCFG_CPU_PROGRAM_MAP(cxhumax_map)


	MCFG_INTEL_28F320J3D_ADD("flash")
	MCFG_I2CMEM_ADD("eeprom")
	MCFG_I2CMEM_DATA_SIZE(0x2000)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(1920, 1080)
	MCFG_SCREEN_VISIBLE_AREA(0, 1920-1, 0, 1080-1)
	MCFG_SCREEN_UPDATE_DRIVER(cxhumax_state, screen_update_cxhumax)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
MACHINE_CONFIG_END

ROM_START( hxhdci2k )
	ROM_REGION( 0x400000, "flash", 0 )
	ROM_SYSTEM_BIOS( 0, "FW10005", "HDCI REV 1.0 RHDXSCI 1.00.05" ) /* 19 AUG 2008 */
	ROM_LOAD16_WORD_SWAP( "28f320j3d.bin", 0x000000, 0x400000, BAD_DUMP CRC(63d98942) SHA1(c5b8d701677a3edc25f203854f44953b19c9158d) )

	ROM_REGION16_BE( 0x2000, "eeprom", 0 )
	ROM_LOAD( "24lc64.bin", 0x0000, 0x2000, NO_DUMP)
ROM_END

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
SYST( 2008, hxhdci2k, 0,       0,   cxhumax,    cxhumax, driver_device,  0,   "HUMAX",   "HUMAX HDCI-2000",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
