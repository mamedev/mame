#pragma once

#ifndef __M68KMAME_H__
#define __M68KMAME_H__

/* ======================================================================== */
/* ============================== MAME STUFF ============================== */
/* ======================================================================== */

#include "debugger.h"
#include "deprecat.h"
#include "m68000.h"

#define OPT_ON 1
#define OPT_OFF 0

/* Configuration switches (see m68kconf.h for explanation) */
#define M68K_EMULATE_TRACE          OPT_OFF

#define M68K_EMULATE_FC             OPT_OFF
#define M68K_SET_FC_CALLBACK(A)

#define M68K_LOG_ENABLE             OPT_OFF
#define M68K_LOG_1010_1111          OPT_OFF
#define M68K_LOG_FILEHANDLE         errorlog

#define M68K_USE_64_BIT             OPT_OFF


void m68k_set_encrypted_opcode_range(const device_config *device, offs_t start, offs_t end);


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __M68KMAME_H__ */
