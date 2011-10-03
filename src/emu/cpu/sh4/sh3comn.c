/* Handlers for SH3 internals */

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4comn.h"
#include "sh3comn.h"

/* High internal area (ffffxxxx) */

WRITE32_HANDLER( sh3_internal_high_w )
{
	sh4_state *sh4 = get_safe_token(&space->device());

	COMBINE_DATA(&sh4->m_sh3internal_upper[offset]);
}

READ32_HANDLER( sh3_internal_high_r )
{
	sh4_state *sh4 = get_safe_token(&space->device());

	switch (offset)
	{
		case SH3_TRA:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 TRA - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
			return sh4->m_sh3internal_upper[offset];

		case SH3_EXPEVT:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 EXPEVT - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
			return sh4->m_sh3internal_upper[offset];

		case SH3_INTEVT:
			logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (SH3 INTEVT - %08x)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+SH3_UPPER_REGBASE,mem_mask, sh4->m_sh3internal_upper[offset]);
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
					logerror("'%s' (%08x): unmapped internal read from %08x mask %08x (INTEVT2)\n",sh4->device->tag(), sh4->pc & AM,(offset *4)+0x4000000,mem_mask);
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

