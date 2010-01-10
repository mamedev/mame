#pragma once

#ifndef __MINX_H__
#define __MINX_H__


enum
{
        MINX_PC=1, MINX_SP, MINX_BA, MINX_HL, MINX_X, MINX_Y,
        MINX_U, MINX_V, MINX_F, MINX_E, MINX_N, MINX_I,
        MINX_XI, MINX_YI,
};

CPU_GET_INFO( minx );
#define CPU_MINX CPU_GET_INFO_NAME( minx )

extern CPU_DISASSEMBLE( minx );

#endif /* __MINX_H__ */
