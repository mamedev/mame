// license:BSD-3-Clause
// copyright-holders:Ville Linde, Ryan Holtz
#ifndef MAME_CPU_RSP_RSPDEFS_H
#define MAME_CPU_RSP_RSPDEFS_H

/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define REG_LO          32
#define REG_HI          33

#define RSREG           ((op >> 21) & 31)
#define RTREG           ((op >> 16) & 31)
#define RDREG           ((op >> 11) & 31)
#define SHIFT           ((op >> 6) & 31)

#define FRREG           ((op >> 21) & 31)
#define FTREG           ((op >> 16) & 31)
#define FSREG           ((op >> 11) & 31)
#define FDREG           ((op >> 6) & 31)

#define IS_SINGLE(o)    (((o) & (1 << 21)) == 0)
#define IS_DOUBLE(o)    (((o) & (1 << 21)) != 0)
#define IS_FLOAT(o)     (((o) & (1 << 23)) == 0)
#define IS_INTEGRAL(o)  (((o) & (1 << 23)) != 0)

#define SIMMVAL         ((int16_t)op)
#define UIMMVAL         ((uint16_t)op)
#define LIMMVAL         (op & 0x03ffffff)

#endif // MAME_CPU_RSP_RSPDEFS_H
