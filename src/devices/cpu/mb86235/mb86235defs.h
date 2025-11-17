#ifndef MAME_CPU_MB86235_MB86235DEFS_H
#define MAME_CPU_MB86235_MB86235DEFS_H

#pragma once


#define OP_USERFLAG_FIFOIN              0x1
#define OP_USERFLAG_FIFOOUT             0x2
#define OP_USERFLAG_REPEAT              0x8
#define OP_USERFLAG_REPEATED_OP         0x10
#define OP_USERFLAG_PR_MASK             0x300
#define OP_USERFLAG_PR_INC              0x100
#define OP_USERFLAG_PR_DEC              0x200
#define OP_USERFLAG_PR_ZERO             0x300
#define OP_USERFLAG_PW_MASK             0xc00
#define OP_USERFLAG_PW_INC              0x400
#define OP_USERFLAG_PW_DEC              0x800
#define OP_USERFLAG_PW_ZERO             0xc00

#endif // MAME_CPU_MB86235_MB86235DEFS_H
