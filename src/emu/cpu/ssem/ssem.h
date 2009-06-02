/*
    Manchester Small-Scale Experimental Machine (SSEM) emulator

    Written by MooglyGuy
*/

#pragma once

#ifndef __SSEM_H__
#define __SSEM_H__

enum
{
    SSEM_PC = 1,
    SSEM_A,
    SSEM_HALT,
};

CPU_GET_INFO( ssem );
#define CPU_SSEM CPU_GET_INFO_NAME( ssem )

extern offs_t ssem_dasm_one(char *buffer, offs_t pc, UINT32 op);

#endif /* __SSEM_H__ */
