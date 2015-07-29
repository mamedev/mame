// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstr??m
/**********************************************************************

    Motorola MC68230 PI/T Parallell Interface and Timer

Revisions
 2015-07-15 JLE initial

Todo
 - Add clock and timers
 - Add all missing registers
 - Add configuration
**********************************************************************/

/*
Force CPU-1 init sequence
0801EA 0E0000 W 0000 PGCR  data_w: 0000 -> 0000 & 00ff
0801EA 0E0002 W 0000 PSRR  data_w: 0000 -> 0001 & 00ff
0801EA 0E0004 W FFFF PADDR data_w: 00ff -> 0002 & 00ff
0801EA 0E0006 W 0000 PBDDR data_w: 0000 -> 0003 & 00ff
0801F0 0E000C W 6060 PACR  data_w: 0060 -> 0006 & 00ff
0801F6 0E000E W A0A0 PBCR  data_w: 00a0 -> 0007 & 00ff
0801FC 0E0000 W 3030 PGCR  data_w: 0030 -> 0000 & 00ff
080202 0E000E W A8A8 PBCR  data_w: 00a8 -> 0007 & 00ff
080210 0E000E W A0A0 PBCR  data_w: 00a0 -> 0007 & 00ff

Force CPU-1 after one keypress in terminal
081DC0 0E000C W 6868 PACR
081DC8 0E000C W 6060 PACR
*/


#include "emu.h"
#include "68230pit.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// device type definition
const device_type PIT68230 = &device_creator<pit68230_device>;

//-------------------------------------------------
//  pit68230_device - constructor
//-------------------------------------------------

pit68230_device::pit68230_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, PIT68230, "Motorola 68230 PI/T", tag, owner, clock, "pit68230", __FILE__)
{
}

void pit68230_device::device_start()
{
	printf("PIT68230 device started\n");
}

void pit68230_device::device_reset()
{
	printf("PIT68230 device reseted\n");
	m_pgcr = 0;
	m_psrr = 0;
	m_paddr = 0;
	m_pbddr = 0;
	m_pcddr = 0;
	m_pacr = 0;
	m_pbcr = 0;
	m_padr = 0;
	m_pbdr = 0;
	m_psr = 0;
}

WRITE8_MEMBER( pit68230_device::data_w )
{
	printf("data_w: %04x -> ", data);
	switch (offset)
	{
	case PIT_68230_PGCR:
	printf("PGCR");
	m_pgcr = data;
	break;
	case PIT_68230_PSRR:
	printf("PSRR");
	m_psrr = data;
	break;
	case PIT_68230_PADDR:
	printf("PADDR");
	m_paddr = data;
	break;
	case PIT_68230_PBDDR:
	printf("PBDDR");
	m_pbddr = data;
	break;
	case PIT_68230_PACR:
	printf("PACR");
	m_pacr = data;
	break;
	case PIT_68230_PBCR:
	printf("PBCR");
	m_pbcr = data;
	break;
	case PIT_68230_PADR:
	printf("PADR");
	m_padr = data;
	break;
	case PIT_68230_PSR:
	printf("PSR");
	m_padr = data;
	break;
	default:
	printf("unhandled register %02x", offset);
	}
	printf("\n");
}

READ8_MEMBER( pit68230_device::data_r )
{
	UINT8 data = 0;

	printf("data_r: ");
	switch (offset)
	{
	case PIT_68230_PGCR:
	printf("PGCR");
	data = m_pgcr;
	break;
	case PIT_68230_PSRR:
	printf("PSRR");
	data = m_psrr;
	break;
	case PIT_68230_PADDR:
	printf("PADDR");
	data = m_paddr;
	break;
	case PIT_68230_PBDDR:
	printf("PBDDR");
	data = m_pbddr;
	break;
	case PIT_68230_PACR:
	printf("PACR");
	data = m_pacr;
	break;
	case PIT_68230_PBCR:
	printf("PBCR");
	data = m_pbcr;
	break;
	case PIT_68230_PADR:
	printf("PADR");
	data = m_padr;
	break;
	case PIT_68230_PBDR:
	/* 4.6.2. PORT B DATA REGISTER (PBDR). The port B data register is a holding register for moving data
to and from port B pins. The port B data direction register determines whether each pin is an input (zero)
or an output (one). This register is readable and writable at all times. Depending on the chosen mode/submode,
reading or writing may affect the double-buffered handshake mechanism. The port B data register is not affected
by the assertion of the RESET pin. PB0-PB7 sits on pins 17-24 on a 48 pin DIP package */
	printf("PBDR");
	data = m_pbdr;
	//    data = (m_pbdr & 0xfc) | 1; // CPU-1 centronics interface expects to see 2 lowest bits equal 1 for printer
	break;
	case PIT_68230_PSR:
	printf("PSR");
	data = m_psr;
	//    data = m_psr | 1; // CPU-1 centronics interface expects status to be non zero
	break;
	default:
	printf("unhandled register %02x", offset);
	data = 0;
	}
	printf("\n");

	return data;
}
