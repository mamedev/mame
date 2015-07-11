// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edström
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
        : device_t(mconfig, PIT68230, "Motorola 68230 PI/T", tag, owner, clock, "pit68230", __FILE__),
	m_internal_clock(0.0)
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
  m_pacr = 0;
  m_pbcr = 0;
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
  default:
    printf("unhandled register %02x", offset);
  }
  printf("\n");
}

READ8_MEMBER( pit68230_device::data_r )
{
  printf("data_r: %04x & %04x\n", offset, mem_mask);
  return (UINT8) 0;
}


