/* 68307 SERIAL Module */

#include "emu.h"
#include "m68kcpu.h"


READ16_HANDLER( m68307_internal_serial_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_serial* serial = m68k->m68307SERIAL;
	assert(serial != NULL);

	if (serial)
	{
		int pc = cpu_get_pc(&space->device());

		switch (offset<<1)
		{
			case m68307SER_USR_UCSR:
				logerror("%08x m68307_internal_serial_r %08x, (%04x) (USR - Status Register)\n", pc, offset*2,mem_mask);
				return space->machine().rand();

			case m68307SER_URB_UTB:
				logerror("%08x m68307_internal_serial_r %08x, (%04x) (URB - Recieve Buffer)\n", pc, offset*2,mem_mask);
				return 0xff;//space->machine().rand();

			case m68307SER_UISR_UIMR:
				logerror("%08x m68307_internal_serial_r %08x, (%04x) (UISR - Interrupt Status Register)\n", pc, offset*2,mem_mask);
				return space->machine().rand() & 0x87;

			default:
				logerror("%08x m68307_internal_serial_r %08x, (%04x)\n", pc, offset*2,mem_mask);
				break;
		}
	}

	return 0x0000;
}

WRITE16_HANDLER( m68307_internal_serial_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_serial* serial = m68k->m68307SERIAL;
	assert(serial != NULL);

	int pc = cpu_get_pc(&space->device());

	if (serial)
	{
		switch (offset<<1)
		{
			case m68307SER_USR_UCSR:
				logerror("%08x m68307_internal_serial_r %08x, (%04x) (UCSR - Clock Select Register)\n", pc, offset*2,mem_mask);
				break;

			case m68307SER_URB_UTB:
				logerror("%08x m68307_internal_serial_w %08x, %04x (%04x) (UTB - Transmit Buffer)\n", pc, offset*2,data,mem_mask);
				break;

			case m68307SER_UISR_UIMR:
				logerror("%08x m68307_internal_serial_w %08x, %04x (%04x) (UIMR - Interrupt Mask Register)\n", pc, offset*2,data,mem_mask);
				break;

			case m68307SER_UIVR:
				logerror("%08x m68307_internal_serial_w %08x, %04x (%04x) (UIVR - Interrupt Vector Register)\n", pc, offset*2,data,mem_mask);
				COMBINE_DATA(&serial->m_uivr);
				break;

			default:
				logerror("%08x m68307_internal_serial_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
		}

	}
}

void m68307_serial::reset(void)
{

}

