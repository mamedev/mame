// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* Handlers for SH3 internals */

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"
#include "sh4dmac.h"

/* High internal area (ffffxxxx) */

WRITE32_MEMBER( sh3_base_device::sh3_internal_high_w )
{
	COMBINE_DATA(&m_sh3internal_upper[offset]);

	switch (offset)
	{
		case SH3_ICR0_IPRA_ADDR:
			if (mem_mask & 0xffff0000)
			{
				logerror("'%s' (%08x): INTC internal write to %08x = %08x & %08x (SH3_ICR0_IPRA_ADDR - ICR0)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
			}

			if (mem_mask & 0x0000ffff)
			{
				logerror("'%s' (%08x): INTC internal write to %08x = %08x & %08x (SH3_ICR0_IPRA_ADDR - IPRA)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
				sh4_handler_ipra_w(data&0xffff,mem_mask&0xffff);
			}

			break;

		case SH3_IPRB_ADDR:
			logerror("'%s' (%08x): INTC internal write to %08x = %08x & %08x (SH3_IPRB_ADDR)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
		break;

		case SH3_TOCR_TSTR_ADDR:
			logerror("'%s' (%08x): TMU internal write to %08x = %08x & %08x (SH3_TOCR_TSTR_ADDR)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
			if (mem_mask&0xff000000)
			{
				sh4_handle_tocr_addr_w((data>>24)&0xffff, (mem_mask>>24)&0xff);
			}
			if (mem_mask&0x0000ff00)
			{
				sh4_handle_tstr_addr_w((data>>8)&0xff, (mem_mask>>8)&0xff);
			}
			if (mem_mask&0x00ff00ff)
			{
				fatalerror("SH3_TOCR_TSTR_ADDR unused bits accessed (write)\n");
			}
			break;
		case SH3_TCOR0_ADDR:  sh4_handle_tcor0_addr_w(data, mem_mask);break;
		case SH3_TCOR1_ADDR:  sh4_handle_tcor1_addr_w(data, mem_mask);break;
		case SH3_TCOR2_ADDR:  sh4_handle_tcor2_addr_w(data, mem_mask);break;
		case SH3_TCNT0_ADDR:  sh4_handle_tcnt0_addr_w(data, mem_mask);break;
		case SH3_TCNT1_ADDR:  sh4_handle_tcnt1_addr_w(data, mem_mask);break;
		case SH3_TCNT2_ADDR:  sh4_handle_tcnt2_addr_w(data, mem_mask);break;
		case SH3_TCR0_ADDR:   sh4_handle_tcr0_addr_w(data>>16, mem_mask>>16);break;
		case SH3_TCR1_ADDR:   sh4_handle_tcr1_addr_w(data>>16, mem_mask>>16);break;
		case SH3_TCR2_ADDR:   sh4_handle_tcr2_addr_w(data>>16, mem_mask>>16);break;
		case SH3_TCPR2_ADDR:  sh4_handle_tcpr2_addr_w(data,  mem_mask);break;

		default:
			logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (unk)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
			break;

	}




}

READ32_MEMBER( sh3_base_device::sh3_internal_high_r )
{
	UINT32 ret = 0;

	switch (offset)
	{
		case SH3_ICR0_IPRA_ADDR:
			logerror("'%s' (%08x): INTC internal read from %08x mask %08x (SH3_ICR0_IPRA_ADDR - %08x)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, m_sh3internal_upper[offset]);
			return (m_sh3internal_upper[offset] & 0xffff0000) | (m_SH4_IPRA & 0xffff);

		case SH3_IPRB_ADDR:
			logerror("'%s' (%08x): INTC internal read from %08x mask %08x (SH3_IPRB_ADDR - %08x)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, m_sh3internal_upper[offset]);
			return m_sh3internal_upper[offset];

		case SH3_TOCR_TSTR_ADDR:

			if (mem_mask&0xff00000)
			{
				ret |= (sh4_handle_tocr_addr_r(mem_mask)&0xff)<<24;
			}
			if (mem_mask&0x0000ff00)
			{
				ret |= (sh4_handle_tstr_addr_r(mem_mask)&0xff)<<8;
			}
			if (mem_mask&0x00ff00ff)
			{
				fatalerror("SH3_TOCR_TSTR_ADDR unused bits accessed (read)\n");
			}
			return ret;
		case SH3_TCOR0_ADDR:  return sh4_handle_tcor0_addr_r(mem_mask);
		case SH3_TCOR1_ADDR:  return sh4_handle_tcor1_addr_r(mem_mask);
		case SH3_TCOR2_ADDR:  return sh4_handle_tcor2_addr_r(mem_mask);
		case SH3_TCNT0_ADDR:  return sh4_handle_tcnt0_addr_r(mem_mask);
		case SH3_TCNT1_ADDR:  return sh4_handle_tcnt1_addr_r(mem_mask);
		case SH3_TCNT2_ADDR:  return sh4_handle_tcnt2_addr_r(mem_mask);
		case SH3_TCR0_ADDR:   return sh4_handle_tcr0_addr_r(mem_mask)<<16;
		case SH3_TCR1_ADDR:   return sh4_handle_tcr1_addr_r(mem_mask)<<16;
		case SH3_TCR2_ADDR:   return sh4_handle_tcr2_addr_r(mem_mask)<<16;
		case SH3_TCPR2_ADDR:  return sh4_handle_tcpr2_addr_r(mem_mask);


		case SH3_TRA_ADDR:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 TRA - %08x)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, m_sh3internal_upper[offset]);
			return m_sh3internal_upper[offset];

		case SH3_EXPEVT_ADDR:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 EXPEVT - %08x)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, m_sh3internal_upper[offset]);
			return m_sh3internal_upper[offset];

		case SH3_INTEVT_ADDR:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 INTEVT - %08x)\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, m_sh3internal_upper[offset]);
			fatalerror("INTEVT unsupported on SH3\n");
			// never executed
			//return m_sh3internal_upper[offset];


		default:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x\n", tag().c_str(), m_pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask);
			return m_sh3internal_upper[offset];
	}
}


READ32_MEMBER( sh3_base_device::sh3_internal_r )
{
	if (offset<0x1000)
	{
		switch (offset)
		{
			case SH3_SAR0_ADDR: return sh4_handle_sar0_addr_r(mem_mask);
			case SH3_SAR1_ADDR: return sh4_handle_sar1_addr_r(mem_mask);
			case SH3_SAR2_ADDR: return sh4_handle_sar2_addr_r(mem_mask);
			case SH3_SAR3_ADDR: return sh4_handle_sar3_addr_r(mem_mask);
			case SH3_DAR0_ADDR: return sh4_handle_dar0_addr_r(mem_mask);
			case SH3_DAR1_ADDR: return sh4_handle_dar1_addr_r(mem_mask);
			case SH3_DAR2_ADDR: return sh4_handle_dar2_addr_r(mem_mask);
			case SH3_DAR3_ADDR: return sh4_handle_dar3_addr_r(mem_mask);
			case SH3_DMATCR0_ADDR: return sh4_handle_dmatcr0_addr_r(mem_mask);
			case SH3_DMATCR1_ADDR: return sh4_handle_dmatcr1_addr_r(mem_mask);
			case SH3_DMATCR2_ADDR: return sh4_handle_dmatcr2_addr_r(mem_mask);
			case SH3_DMATCR3_ADDR: return sh4_handle_dmatcr3_addr_r(mem_mask);
			case SH3_CHCR0_ADDR: return sh4_handle_chcr0_addr_r(mem_mask);
			case SH3_CHCR1_ADDR: return sh4_handle_chcr1_addr_r(mem_mask);
			case SH3_CHCR2_ADDR: return sh4_handle_chcr2_addr_r(mem_mask);
			case SH3_CHCR3_ADDR: return sh4_handle_chcr3_addr_r(mem_mask);
			case SH3_DMAOR_ADDR: return sh4_handle_dmaor_addr_r(mem_mask)<<16;


			case INTEVT2:
				{
				//  logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (INTEVT2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
					return m_sh3internal_lower[offset];
				}


			case IRR0_IRR1:
				{
					{
						if (mem_mask & 0xff000000)
						{
							logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (IRR0)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
							return m_sh3internal_lower[offset];
						}

						if (mem_mask & 0x0000ff00)
						{
							logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (IRR1)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
							return m_sh3internal_lower[offset];
						}

						fatalerror("'%s' (%08x): unmapped internal read from %08x mask %08x (IRR0/1 unused bits)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
					}
				}

			case PADR_PBDR:
				{
					if (mem_mask & 0xffff0000)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PADR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_A)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PBDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_B)<<8;
					}
				}
				break;

			case PCDR_PDDR:
				{
					if (mem_mask & 0xffff0000)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PCDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_C)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PDDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_D)<<8;
					}
				}
				break;

			case PEDR_PFDR:
				{
					if (mem_mask & 0xffff0000)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PEDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_E)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PFDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_F)<<8;
					}
				}
				break;

			case PGDR_PHDR:
				{
					if (mem_mask & 0xffff0000)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PGDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_G)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PHDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_H)<<8;
					}
				}
				break;

			case PJDR_PKDR:
				{
					if (mem_mask & 0xffff0000)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PJDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_J)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PKDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_K)<<8;
					}
				}
				break;

			case PLDR_SCPDR:
				{
					if (mem_mask & 0xffff0000)
					{
						//logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PLDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_io->read_qword(SH3_PORT_L)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SCPDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						//return m_io->read_qword(SH3_PORT_K)<<8;
					}
				}
				break;


			case SCSMR2_SCBRR2:
				{
					if (mem_mask & 0xff000000)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCSMR2 - Serial Mode Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}

					if (mem_mask & 0x0000ff00)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCBRR2 - Bit Rate Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}
				}
				break;

			case SCSCR2_SCFTDR2:
				{
					if (mem_mask & 0xff000000)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCSCR2 - Serial Control Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}

					if (mem_mask & 0x0000ff00)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCFTDR2 - Transmit FIFO Data Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}
				}
				break;

			case SCSSR2_SCFRDR2:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCSSR2 - Serial Status Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}

					if (mem_mask & 0x0000ff00)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCFRDR2 - Receive FIFO Data Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}
				}
				break;

			case SCFCR2_SCFDR2:
				{
					if (mem_mask & 0xff000000)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCFCR2 - Fifo Control Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): SCIF internal read from %08x mask %08x (SCFDR2 - Fifo Data Count Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,mem_mask);
						return m_sh3internal_lower[offset];
					}
				}
				break;


			default:
				{
					logerror("'%s' (%08x): unmapped internal read from %08x mask %08x\n",
						tag().c_str(), m_pc & AM,
						(offset *4)+0x4000000,
						mem_mask);
				}
				break;

		}

	}
	else
	{
		logerror("'%s' (%08x): unmapped internal read from %08x mask %08x\n",
			tag().c_str(), m_pc & AM,
			(offset *4)+0x4000000,
			mem_mask);
	}

	return 0;
}

/* Lower internal area */

WRITE32_MEMBER( sh3_base_device::sh3_internal_w )
{
	if (offset<0x1000)
	{
		//UINT32 old = m_sh3internal_lower[offset];
		COMBINE_DATA(&m_sh3internal_lower[offset]);

		switch (offset)
		{
			case SH3_SAR0_ADDR: sh4_handle_sar0_addr_w(data,mem_mask);   break;
			case SH3_SAR1_ADDR: sh4_handle_sar1_addr_w(data,mem_mask);   break;
			case SH3_SAR2_ADDR: sh4_handle_sar2_addr_w(data,mem_mask);   break;
			case SH3_SAR3_ADDR: sh4_handle_sar3_addr_w(data,mem_mask);   break;
			case SH3_DAR0_ADDR: sh4_handle_dar0_addr_w(data,mem_mask);   break;
			case SH3_DAR1_ADDR: sh4_handle_dar1_addr_w(data,mem_mask);   break;
			case SH3_DAR2_ADDR: sh4_handle_dar2_addr_w(data,mem_mask);   break;
			case SH3_DAR3_ADDR: sh4_handle_dar3_addr_w(data,mem_mask);   break;
			case SH3_DMATCR0_ADDR: sh4_handle_dmatcr0_addr_w(data,mem_mask);   break;
			case SH3_DMATCR1_ADDR: sh4_handle_dmatcr1_addr_w(data,mem_mask);   break;
			case SH3_DMATCR2_ADDR: sh4_handle_dmatcr2_addr_w(data,mem_mask);   break;
			case SH3_DMATCR3_ADDR: sh4_handle_dmatcr3_addr_w(data,mem_mask);   break;
			case SH3_CHCR0_ADDR: sh4_handle_chcr0_addr_w(data,mem_mask);   break;
			case SH3_CHCR1_ADDR: sh4_handle_chcr1_addr_w(data,mem_mask);   break;
			case SH3_CHCR2_ADDR: sh4_handle_chcr2_addr_w(data,mem_mask);   break;
			case SH3_CHCR3_ADDR: sh4_handle_chcr3_addr_w(data,mem_mask);   break;
			case SH3_DMAOR_ADDR: sh4_handle_dmaor_addr_w(data>>16,mem_mask>>16);   break;


			case IRR0_IRR1:
				{
					{
						if (mem_mask & 0xff000000)
						{
							logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (IRR0)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
							// not sure if this is how we should clear lines in this core...
							if (!(data & 0x01000000)) execute_set_input(0, CLEAR_LINE);
							if (!(data & 0x02000000)) execute_set_input(1, CLEAR_LINE);
							if (!(data & 0x04000000)) execute_set_input(2, CLEAR_LINE);
							if (!(data & 0x08000000)) execute_set_input(3, CLEAR_LINE);

						}
						if (mem_mask & 0x0000ff00)
						{
							logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (IRR1)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
						}
						if (mem_mask & 0x00ff00ff)
						{
							fatalerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (IRR0/1 unused bits)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
						}
					}
				}
				break;

			case PINTER_IPRC:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PINTER)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						data &= 0xffff; mem_mask &= 0xffff;
						COMBINE_DATA(&m_SH4_IPRC);
						logerror("'%s' (%08x): INTC internal write to %08x = %08x & %08x (IPRC)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
						m_exception_priority[SH4_INTC_IRL0]     = INTPRI((m_SH4_IPRC & 0x000f)>>0, SH4_INTC_IRL0);
						m_exception_priority[SH4_INTC_IRL1]     = INTPRI((m_SH4_IPRC & 0x00f0)>>4, SH4_INTC_IRL1);
						m_exception_priority[SH4_INTC_IRL2]     = INTPRI((m_SH4_IPRC & 0x0f00)>>8, SH4_INTC_IRL2);
						m_exception_priority[SH4_INTC_IRL3]     = INTPRI((m_SH4_IPRC & 0xf000)>>12,SH4_INTC_IRL3);
						sh4_exception_recompute();
					}
				}
				break;

			case PCCR_PDCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PCCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PDCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case PECR_PFCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PECR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PFCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;


			case PGCR_PHCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PGCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PHCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;


			case PJCR_PKCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PJCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PKCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;


			case PLCR_SCPCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PLCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (SCPCR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case PADR_PBDR:
				{
					if (mem_mask & 0xffff0000)
					{
						m_io->write_qword(SH3_PORT_A, (data>>24)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PADR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						m_io->write_qword(SH3_PORT_B, (data>>8)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PBDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case PCDR_PDDR:
				{
					if (mem_mask & 0xffff0000)
					{
						m_io->write_qword(SH3_PORT_C, (data>>24)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PADR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						m_io->write_qword(SH3_PORT_D, (data>>8)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PBDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;
			case PEDR_PFDR:
				{
					if (mem_mask & 0xffff0000)
					{
						m_io->write_qword(SH3_PORT_E, (data>>24)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PEDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						m_io->write_qword(SH3_PORT_F, (data>>8)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PFDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case PGDR_PHDR:
				{
					if (mem_mask & 0xffff0000)
					{
						m_io->write_qword(SH3_PORT_G, (data>>24)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PGDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						m_io->write_qword(SH3_PORT_H, (data>>8)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PHDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;


			case PJDR_PKDR:
				{
					if (mem_mask & 0xffff0000)
					{
						m_io->write_qword(SH3_PORT_J, (data>>24)&0xff);
					//  logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PJDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						m_io->write_qword(SH3_PORT_K, (data>>8)&0xff);
						//logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PKDR)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case SCSMR2_SCBRR2:
				{
					if (mem_mask & 0xff000000)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCSMR2 - Serial Mode Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ff00)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCBRR2 - Bit Rate Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case SCSCR2_SCFTDR2:
				{
					if (mem_mask & 0xff000000)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCSCR2 - Serial Control Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ff00)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCFTDR2 - Transmit FIFO Data Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case SCSSR2_SCFRDR2:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCSSR2 - Serial Status Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ff00)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCFRDR2 - Receive FIFO Data Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case SCFCR2_SCFDR2:
				{
					if (mem_mask & 0xff000000)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCFCR2 - Fifo Control Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): SCIF internal write to %08x = %08x & %08x (SCFDR2 - Fifo Data Count Register 2)\n", tag().c_str(), m_pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			default:
				{
					logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x\n",
							tag().c_str(), m_pc & AM,
							(offset *4)+0x4000000,
							data,
							mem_mask);
				}
				break;
		}

	}
	else
	{
		logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x\n",
				tag().c_str(), m_pc & AM,
				(offset *4)+0x4000000,
				data,
				mem_mask);
	}

}
