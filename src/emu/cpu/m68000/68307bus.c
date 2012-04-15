/* 68307 MBUS module */
/* all ports on this are 8-bit? */

#include "emu.h"
#include "m68kcpu.h"


READ8_HANDLER( m68307_internal_mbus_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_mbus* mbus = m68k->m68307MBUS;
	assert(mbus != NULL);

	if (mbus)
	{
		int pc = cpu_get_pc(&space->device());


		switch (offset)
		{
			case m68307BUS_MADR:
				logerror("%08x m68307_internal_mbus_r %08x (MADR - M-Bus Address Register)\n", pc, offset);
				return space->machine().rand();

			case m68307BUS_MFDR:
				logerror("%08x m68307_internal_mbus_r %08x (MFDR - M-Bus Frequency Divider Register)\n", pc, offset);
				return space->machine().rand();

			case m68307BUS_MBCR:
				logerror("%08x m68307_internal_mbus_r %08x (MFDR - M-Bus Control Register)\n", pc, offset);
				return space->machine().rand();

			case m68307BUS_MBSR:
				logerror("%08x m68307_internal_mbus_r %08x (MBSR - M-Bus Status Register)\n", pc, offset);
				return space->machine().rand();

			case m68307BUS_MBDR:
				logerror("%08x m68307_internal_mbus_r %08x (MBDR - M-Bus Data I/O Register)\n", pc, offset);
				return space->machine().rand();

			default:
				logerror("%08x m68307_internal_mbus_r %08x (UNKNOWN / ILLEGAL)\n", pc, offset);
				return 0x00;
		}
	}

	return 0xff;
}

WRITE8_HANDLER( m68307_internal_mbus_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_mbus* mbus = m68k->m68307MBUS;
	assert(mbus != NULL);

	if (mbus)
	{
		int pc = cpu_get_pc(&space->device());

		switch (offset)
		{
			case m68307BUS_MADR:
				logerror("%08x m68307_internal_mbus_w %08x, %02x (MADR - M-Bus Address Register)\n", pc, offset,data);
				break;

			case m68307BUS_MFDR:
				logerror("%08x m68307_internal_mbus_w %08x, %02x (MFDR - M-Bus Frequency Divider Register)\n", pc, offset,data);
				break;

			case m68307BUS_MBCR:
				logerror("%08x m68307_internal_mbus_w %08x, %02x (MFDR - M-Bus Control Register)\n", pc, offset,data);
				break;

			case m68307BUS_MBSR:
				logerror("%08x m68307_internal_mbus_w %08x, %02x (MBSR - M-Bus Status Register)\n", pc, offset,data);
				break;
			
			case m68307BUS_MBDR:
				logerror("%08x m68307_internal_mbus_w %08x, %02x (MBDR - M-Bus Data I/O Register)\n", pc, offset,data);
				break;

			default:
				logerror("%08x m68307_internal_mbus_w %08x, %02x (UNKNOWN / ILLEGAL)\n", pc, offset,data);
				break;
		}
	}
}

void m68307_mbus::reset(void)
{

}
