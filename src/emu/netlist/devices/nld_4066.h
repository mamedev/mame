// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4066.h
 *
 *  CD4066: Quad Bilateral Switch
 *
 *          +--------------+
 *   INOUTA |1     ++    14| VDD
 *   OUTINA |2           13| CONTROLA
 *   OUTINB |3           12| CONTROLD
 *   INOUTB |4    4066   11| INOUTD
 * CONTROLB |5           10| OUTIND
 * CONTROLC |6            9| OUTINC
 *      VSS |7            8| INOUTC
 *          +--------------+
 *
 *  FIXME: These devices are slow (~125 ns). THis is currently not reflected
 *
 *  Naming conventions follow National semiconductor datasheet
 *
 */

#ifndef NLD_4066_H_
#define NLD_4066_H_

#include "../nl_base.h"
#include "nld_cmos.h"

#define CD_4066_DIP(_name)                                                         \
		NET_REGISTER_DEV(4066_dip, _name)

NETLIB_SUBDEVICE(4066,
    NETLIB_LOGIC_FAMILY(CD4000)
public:

	netlist_analog_input_t m_control;
	NETLIB_NAME(R) m_R;

	netlist_state_t<NETLIB_NAME(vdd_vss) *>m_supply;
);

NETLIB_DEVICE(4066_dip,
    NETLIB_LOGIC_FAMILY(CD4000)

	NETLIB_NAME(4066) m_A;
	NETLIB_NAME(4066) m_B;
	NETLIB_NAME(4066) m_C;
	NETLIB_NAME(4066) m_D;
	NETLIB_NAME(vdd_vss) m_supply;
);

#endif /* NLD_4066_H_ */
