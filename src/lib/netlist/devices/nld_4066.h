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

#include "nl_base.h"
#include "nld_cmos.h"

#define CD4066_GATE(name)                                                     \
		NET_REGISTER_DEV(CD4066_GATE, name)

NETLIB_NAMESPACE_DEVICES_START()

NETLIB_OBJECT(CD4066_GATE)
{
	NETLIB_CONSTRUCTOR(CD4066_GATE)
	NETLIB_FAMILY("CD4XXX")
	, m_supply(*this, "PS")
	, m_R(*this, "R")
	, m_base_r(*this, "BASER", 270.0)
	{
		enregister("CTL", m_control);
	}

	NETLIB_RESETI() { }
	NETLIB_UPDATEI();

public:
	NETLIB_SUB(vdd_vss) m_supply;
	NETLIB_SUB(R) m_R;

	analog_input_t m_control;
	param_double_t m_base_r;
};

NETLIB_NAMESPACE_DEVICES_END()

#endif /* NLD_4066_H_ */
