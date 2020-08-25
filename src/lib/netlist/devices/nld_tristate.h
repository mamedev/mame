// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef NLD_TRISTATE_H_
#define NLD_TRISTATE_H_

#include "../nl_setup.h"

#define TTL_TRISTATE(name, cCEQ1, cD1, cCEQ2, cD2)  \
		NET_REGISTER_DEV(TTL_TRISTATE, name)    \
		NET_CONNECT(name, CEQ1, cCEQ1)  \
		NET_CONNECT(name, D1,  cD1)     \
		NET_CONNECT(name, CEQ2, cCEQ2)  \
		NET_CONNECT(name, D2,  cD2)

#define TTL_TRISTATE3(name, cCEQ1, cD1, cCEQ2, cD2, cCEQ3, cD3) \
		NET_REGISTER_DEV(TTL_TRISTATE3, name)   \
		NET_CONNECT(name, CEQ1, cCEQ1)  \
		NET_CONNECT(name, D1,  cD1)     \
		NET_CONNECT(name, CEQ2, cCEQ2)  \
		NET_CONNECT(name, D2,  cD2)     \
		NET_CONNECT(name, CEQ3, cCEQ3)  \
		NET_CONNECT(name, D3, cD3)

#endif /* NLD_TRISTATE_H_ */
