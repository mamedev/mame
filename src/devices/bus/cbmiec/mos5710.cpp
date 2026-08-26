// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS5710 Custom Floppy Controller and Gate Array

**********************************************************************/

/*

Address Decode

   A 15 14 13 12 10  4  3
RAM   0  0  0  0  x  x  x
VIA1  0  0  0  1  0  x  x
VIA2  0  0  0  1  1  x  x
FDC   0  0  1  0  x  0  0
CIA   0  1  0  0  x  0  x
FDC2  0  1  0  0  0  1  x
RAM   0  1  1  x  x  x  x


Registers

2000  FDC Status Register
2001  FDC Track Register
2002  FDC Sector Register
2003  FDC Data Register
2004  FDC Control for 2001, 2002
2005  FDC Control/Counter for 2001, 2002

400C  CIA Serial Data Register
400D  CIA Interrupt Control Register
400D  CIA Control Register A

4010  FDC2 used in 2002, 2004
4011  FDC2 used in 2002
4012  FDC2 used in 2001
4013  FDC2 used in 2001
4014  FDC2 used in 2002
4015  FDC2 used in 2002
4016  FDC2 used in 2003
4017  FDC2

*/

#include "emu.h"
#include "mos5710.h"

DEFINE_DEVICE_TYPE(MOS5710, mos5710_device, "mos5710", "MOS5710 Custom Floppy Controller and Gate Array")

mos5710_device::mos5710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
    device_t(mconfig, MOS5710, tag, owner, clock)
{
}

void mos5710_device::device_start()
{
}
