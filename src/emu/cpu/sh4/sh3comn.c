/* Handlers for SH3 internals */

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"
/* High internal area (ffffxxxx) */

WRITE32_HANDLER( sh3_internal_high_w )
{
	sh4_state *sh4 = get_safe_token(&space->device());
	COMBINE_DATA(&sh4->m_sh3internal_upper[offset]);	

	switch (offset)
	{

		case SH3_ICR0_IPRA_ADDR:
			if (mem_mask & 0xffff0000)
			{
				logerror("'%s' (%08x): INTC internal write to %08x = %08x & %08x (SH3_ICR0_IPRA_ADDR - ICR0)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
			}

			if (mem_mask & 0x0000ffff)
			{
				logerror("'%s' (%08x): INTC internal write to %08x = %08x & %08x (SH3_ICR0_IPRA_ADDR - IPRA)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
				sh4_handler_ipra_w(sh4,data&0xffff,mem_mask&0xffff);
			}
				
			break;

		case SH3_IPRB_ADDR:
			logerror("'%s' (%08x): INTC internal write to %08x = %08x & %08x (SH3_IPRB_ADDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
		break;
	
		case SH3_TOCR_TSTR_ADDR:
			logerror("'%s' (%08x): TMU internal write to %08x = %08x & %08x (SH3_TOCR_TSTR_ADDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
			if (mem_mask&0xff000000)
			{
				sh4_handle_tocr_addr_w(sh4, (data>>24)&0xffff, (mem_mask>>24)&0xff);
			}
			if (mem_mask&0x0000ff00)
			{
				sh4_handle_tstr_addr_w(sh4, (data>>8)&0xff, (mem_mask>>8)&0xff);
			}
			if (mem_mask&0x00ff00ff)
			{
				fatalerror("SH3_TOCR_TSTR_ADDR unused bits accessed (write)\n");
			}
			break;
		case SH3_TCOR0_ADDR:  sh4_handle_tcor0_addr_w(sh4, data, mem_mask);break;
		case SH3_TCOR1_ADDR:  sh4_handle_tcor1_addr_w(sh4, data, mem_mask);break;
		case SH3_TCOR2_ADDR:  sh4_handle_tcor2_addr_w(sh4, data, mem_mask);break;
		case SH3_TCNT0_ADDR:  sh4_handle_tcnt0_addr_w(sh4, data, mem_mask);break;
		case SH3_TCNT1_ADDR:  sh4_handle_tcnt1_addr_w(sh4, data, mem_mask);break;
		case SH3_TCNT2_ADDR:  sh4_handle_tcnt2_addr_w(sh4, data, mem_mask);break;
		case SH3_TCR0_ADDR:   sh4_handle_tcr0_addr_w(sh4, data>>16, mem_mask>>16);break;
		case SH3_TCR1_ADDR:   sh4_handle_tcr1_addr_w(sh4, data>>16, mem_mask>>16);break;
		case SH3_TCR2_ADDR:   sh4_handle_tcr2_addr_w(sh4, data>>16, mem_mask>>16);break;
		case SH3_TCPR2_ADDR:  sh4_handle_tcpr2_addr_w(sh4,data,  mem_mask);break;

		default:
			logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (unk)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,data,mem_mask);
			break;

	}

	

	
}

READ32_HANDLER( sh3_internal_high_r )
{
	sh4_state *sh4 = get_safe_token(&space->device());

	UINT32 ret = 0;

	switch (offset)
	{

		case SH3_ICR0_IPRA_ADDR:
			logerror("'%s' (%08x): INTC internal read from %08x mask %08x (SH3_ICR0_IPRA_ADDR - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
			return (sh4->m_sh3internal_upper[offset] & 0xffff0000) | (sh4->SH4_IPRA & 0xffff);

		case SH3_IPRB_ADDR:
			logerror("'%s' (%08x): INTC internal read from %08x mask %08x (SH3_IPRB_ADDR - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
			return sh4->m_sh3internal_upper[offset];

		case SH3_TOCR_TSTR_ADDR:

			if (mem_mask&0xff00000)
			{
				ret |= (sh4_handle_tocr_addr_r(sh4, mem_mask)&0xff)<<24;
			}
			if (mem_mask&0x0000ff00)
			{
				ret |= (sh4_handle_tstr_addr_r(sh4, mem_mask)&0xff)<<8;
			}
			if (mem_mask&0x00ff00ff)
			{
				fatalerror("SH3_TOCR_TSTR_ADDR unused bits accessed (read)\n");
			}
			return ret;
		case SH3_TCOR0_ADDR:  return sh4_handle_tcor0_addr_r(sh4, mem_mask);
		case SH3_TCOR1_ADDR:  return sh4_handle_tcor1_addr_r(sh4, mem_mask);
		case SH3_TCOR2_ADDR:  return sh4_handle_tcor2_addr_r(sh4, mem_mask);
		case SH3_TCNT0_ADDR:  return sh4_handle_tcnt0_addr_r(sh4, mem_mask);
		case SH3_TCNT1_ADDR:  return sh4_handle_tcnt1_addr_r(sh4, mem_mask);
		case SH3_TCNT2_ADDR:  return sh4_handle_tcnt2_addr_r(sh4, mem_mask);
		case SH3_TCR0_ADDR:   return sh4_handle_tcr0_addr_r(sh4, mem_mask)<<16;
		case SH3_TCR1_ADDR:   return sh4_handle_tcr1_addr_r(sh4, mem_mask)<<16;
		case SH3_TCR2_ADDR:   return sh4_handle_tcr2_addr_r(sh4, mem_mask)<<16;
		case SH3_TCPR2_ADDR:  return sh4_handle_tcpr2_addr_r(sh4, mem_mask);


		case SH3_TRA_ADDR:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 TRA - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
			return sh4->m_sh3internal_upper[offset];

		case SH3_EXPEVT_ADDR:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 EXPEVT - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
			return sh4->m_sh3internal_upper[offset];

		case SH3_INTEVT_ADDR:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 INTEVT - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
			fatalerror("INTEVT unsupported on SH3\n");
			return sh4->m_sh3internal_upper[offset];


		default:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask);
			return sh4->m_sh3internal_upper[offset];
	}	
}


READ32_HANDLER( sh3_internal_r )
{
	sh4_state *sh4 = get_safe_token(&space->device());

	if (offset<0x1000)
	{

		switch (offset)
		{
			case INTEVT2:
				{
				//	logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (INTEVT2)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,mem_mask);
					return sh4->m_sh3internal_lower[offset];
				}
				break;
			

			case PEDR_PFDR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PEDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,mem_mask);
						return (0x20)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PFDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,mem_mask);
					}
				}
				break;


			case PJDR_PKDR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PJDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,mem_mask);
						return (0x40)<<24;
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (PKDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,mem_mask);
					}
				}
				break;

			default:
				{
					logerror("'%s' (%08x): unmapped internal read from %08x mask %08x\n",
						sh4->device->tag(), sh4->pc & AM,
						(offset *4)+0x4000000,
						mem_mask);
				}
				break;

		}

	}
	else
	{

		logerror("'%s' (%08x): unmapped internal read from %08x mask %08x\n",
			sh4->device->tag(), sh4->pc & AM,
			(offset *4)+0x4000000,
			mem_mask);
	}

	return 0;
}

/* Lower internal area */

WRITE32_HANDLER( sh3_internal_w )
{
	sh4_state *sh4 = get_safe_token(&space->device());



	if (offset<0x1000)
	{
		//UINT32 old = sh4->m_sh3internal_lower[offset];
		COMBINE_DATA(&sh4->m_sh3internal_lower[offset]);

		switch (offset)
		{

			case PINTER_IPRC:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PINTER)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (IPRC)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case PCCR_PDCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PCCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PDCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case PECR_PFCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PECR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PFCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;


			case PGCR_PHCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PGCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PHCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;


			case PJCR_PKCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PJCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PKCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;


			case PLCR_SCPCR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PLCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (SCPCR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			case PEDR_PFDR:
				{
					if (mem_mask & 0xffff0000)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PEDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PFDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;
				
			case PJDR_PKDR:
				{
					if (mem_mask & 0xffff0000)
					{
					//	logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PJDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}

					if (mem_mask & 0x0000ffff)
					{
						logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x (PKDR)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,data,mem_mask);
					}
				}
				break;

			default:
				{
					logerror("'%s' (%08x): unmapped internal write to %08x = %08x & %08x\n",
							sh4->device->tag(), sh4->pc & AM,
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
				sh4->device->tag(), sh4->pc & AM,
				(offset *4)+0x4000000,
				data,
				mem_mask);
	}

}

