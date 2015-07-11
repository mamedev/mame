#ifndef NLD_CD4XXX_H_
#define NLD_CD4XXX_H_

#include "../nl_setup.h"

#ifndef __PLIB_PREPROCESSOR__

/* ----------------------------------------------------------------------------
 *  Netlist Macros
 * ---------------------------------------------------------------------------*/

#define CD4001_NOR(_name)                                                      \
		NET_REGISTER_DEV_X(CD4001_NOR, _name)

#define CD4001_DIP(_name)                                                      \
		NET_REGISTER_DEV_X(CD4001_DIP, _name)

/* ----------------------------------------------------------------------------
 *  External declarations
 * ---------------------------------------------------------------------------*/

NETLIST_EXTERNAL(CD4XXX_lib)

#endif

#endif
