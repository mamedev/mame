// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3fe.h

    Front-end for MIPS3 recompiler

***************************************************************************/

#pragma once

#ifndef __MIPS3FE_H__
#define __MIPS3FE_H__


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


#endif /* __MIPS3FE_H__ */
