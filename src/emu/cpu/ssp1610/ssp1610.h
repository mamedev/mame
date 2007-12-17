#ifndef SSP1610_H
#define SSP1610_H

#include "cpuintrf.h"

/* Functions */

#if (HAS_SSP1610)
void e116t_get_info(UINT32 state, cpuinfo *info);
#endif

#ifdef MAME_DEBUG
extern unsigned dasm_ssp1610(char *buffer, unsigned pc, const UINT8 *oprom);
#endif

#endif /* SSP1610_H */
