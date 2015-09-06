#ifndef NLD_TTL74XX_H_
#define NLD_TTL74XX_H_

#include "nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#define TTL_7416_GATE(_name)                                                   \
		NET_REGISTER_DEV(TTL_7416_GATE, _name)

#define TTL_7416_DIP(_name)                                                    \
		NET_REGISTER_DEV(TTL7416_DIP, _name)

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(TTL74XX_lib)

#endif

#endif
