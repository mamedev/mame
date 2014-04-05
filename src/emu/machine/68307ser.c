/* 68307 SERIAL Module */
/* all ports on this are 8-bit? */

/* this is a 68681 'compatible' chip but with only a single channel implemented
  (writes to the other channel have no effects)

  for now at least we piggyback on the existing 68307 emulation rather than having
  a custom verson here, that may change later if subtle differences exist.

*/

#include "emu.h"
#include "68307.h"


READ8_MEMBER( m68307cpu_device::m68307_internal_serial_r )
{
	m68307cpu_device *m68k = this;
	m68307_serial* serial = m68k->m68307SERIAL;
	assert(serial != NULL);

	if (serial)
	{
		// if we're piggybacking on the existing 68681 implementation...
		if (serial->m_duart68681)
		{
			if (offset&1) return serial->m_duart68681->read(*m68k->program, offset>>1);
		}
		else
		{
			int pc = space.device().safe_pc();

			switch (offset)
			{
				case m68307SER_UMR1_UMR2:
					logerror("%08x m68307_internal_serial_r %08x (UMR1, UMR2 - UART Mode Register)\n", pc, offset);
					return space.machine().rand();

				case m68307SER_USR_UCSR:
					logerror("%08x m68307_internal_serial_r %08x (USR, UCSR - UART Status/Clock Select Register)\n", pc, offset);
					return space.machine().rand();

				case m68307SER_UCR:
					logerror("%08x m68307_internal_serial_r %08x (UCR - UART Command Register)\n", pc, offset);
					return space.machine().rand();

				case m68307SER_URB_UTB:
					logerror("%08x m68307_internal_serial_r %08x (URB, UTB - UART Recieve/Transmit Buffer)\n", pc, offset);
					return 0xff;//space.machine().rand();

				case m68307SER_UIPCR_UACR:
					logerror("%08x m68307_internal_serial_r %08x (UIPCR, UACR - UART Input Port Change Register / UART Control Register)\n", pc, offset);
					return 0xff;//space.machine().rand();

				case m68307SER_UISR_UIMR:
					logerror("%08x m68307_internal_serial_r %08x (UISR, UIMR - UART Interrupt Status Register / UART Interrupt Mask Register)\n", pc, offset);
					return space.machine().rand() & 0x87;

				case m68307SER_UBG1:
					logerror("%08x m68307_internal_serial_r %08x (UBG1 - UART Baud Rate Gen. Precaler MSB)\n", pc, offset);
					return space.machine().rand() & 0x87;

				case m68307SER_UBG2:
					logerror("%08x m68307_internal_serial_r %08x (UBG1 - UART Baud Rate Gen. Precaler LSB)\n", pc, offset);
					return space.machine().rand() & 0x87;

				case m68307SER_UIVR:
					logerror("%08x m68307_internal_serial_r %08x (UIVR - UART Interrupt Vector Register)\n", pc, offset);
					return space.machine().rand() & 0x87;

				case m68307SER_UIP:
					logerror("%08x m68307_internal_serial_r %08x (UIP - UART Register Input Port)\n", pc, offset);
					return space.machine().rand() & 0x87;

				case m68307SER_UOP1:
					logerror("%08x m68307_internal_serial_r %08x (UOP1 - UART Output Port Bit Set Cmd)\n", pc, offset);
					return space.machine().rand() & 0x87;

				case m68307SER_UOP0:
					logerror("%08x m68307_internal_serial_r %08x (UOP0 - UART Output Port Bit Reset Cmd)\n", pc, offset);
					return space.machine().rand() & 0x87;

				default:
					logerror("%08x m68307_internal_serial_r %08x (UNKNOWN / ILLEGAL)\n", pc, offset);
					break;
			}
		}
	}

	return 0x0000;
}

WRITE8_MEMBER( m68307cpu_device::m68307_internal_serial_w )
{
	m68307cpu_device *m68k = this;
	m68307_serial* serial = m68k->m68307SERIAL;
	assert(serial != NULL);

	int pc = space.device().safe_pc();

	if (serial)
	{
		// if we're piggybacking on the existing 68681 implementation...
		if (serial->m_duart68681)
		{
			if (offset&1) serial->m_duart68681->write(*m68k->program, offset>>1, data);
		}
		else
		{
			switch (offset)
			{
				case m68307SER_UMR1_UMR2:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UMR1, UMR2 - UART Mode Register)\n", pc, offset,data);
					break;

				case m68307SER_USR_UCSR:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UCSR - Clock Select Register)\n", pc, offset,data);
					break;

				case m68307SER_UCR:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UCR - UART Command Register)\n", pc, offset,data);
					break;

				case m68307SER_URB_UTB:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UTB - Transmit Buffer)\n", pc, offset,data);
					break;

				case m68307SER_UIPCR_UACR:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UIPCR, UACR - UART Input Port Change Register / UART Control Register)\n", pc, offset,data);
					break;

				case m68307SER_UISR_UIMR:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UIMR - Interrupt Mask Register)\n", pc, offset,data);
					break;

				case m68307SER_UBG1:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UBG1 - UART Baud Rate Gen. Precaler MSB)\n", pc, offset,data);
					break;

				case m68307SER_UBG2:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UBG1 - UART Baud Rate Gen. Precaler LSB)\n", pc, offset,data);
					break;

				case m68307SER_UIVR:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UIVR - Interrupt Vector Register)\n", pc, offset,data);
					serial->m_uivr = data;
					break;

				case m68307SER_UIP:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UIP - UART Register Input Port)\n", pc, offset,data);
					break;

				case m68307SER_UOP1:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UOP1 - UART Output Port Bit Set Cmd)\n", pc, offset,data);
					break;

				case m68307SER_UOP0:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UOP0 - UART Output Port Bit Reset Cmd)\n", pc, offset,data);
					break;


				default:
					logerror("%08x m68307_internal_serial_w %08x, %02x (UNKNOWN / ILLEGAL)\n", pc, offset,data);
					break;
			}
		}
	}
}

void m68307_serial::reset(void)
{
}
