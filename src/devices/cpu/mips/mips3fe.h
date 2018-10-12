// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3fe.h

    Front-end for MIPS3 recompiler

***************************************************************************/
#ifndef MAME_CPU_MIPS_MIPS3FE_H
#define MAME_CPU_MIPS_MIPS3FE_H

#pragma once


//**************************************************************************
//  MACROS
//**************************************************************************

// register flags 0
#define REGFLAG_R(n)                    (((n) == 0) ? 0 : (1 << (n)))

// register flags 1
#define REGFLAG_CPR1(n)                 (1 << (n))

// register flags 2
#define REGFLAG_LO                      (1 << 0)
#define REGFLAG_HI                      (1 << 1)
#define REGFLAG_FCC                     (1 << 2)


#endif // MAME_CPU_MIPS_MIPS3FE_H
