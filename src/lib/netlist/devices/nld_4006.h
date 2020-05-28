// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_4006.h
 *
 *  CD4006: CMOS 18-Stage Static Register
 *
 * Description
 *
 * CD4006BMS types are composed of 4 separate shift register sections: two
 * sections of four stages and two sections of five stages with an output tap
 * at the fourth stage. Each section has an independent single-rail data path.
 *
 * A common clock signal is used for all stages. Data are shifted to the next
 * stages on negative-going transitions of the clock. Through appropriate
 * connections of inputs and outputs, multiple register sections of 4, 5, 8,
 * and 9 stages or single register sections of 10, 12, 13, 14, 16, 17 and 18
 * stages can be implemented using one CD4006BMS package. Longer shift register
 * sections can be assembled by using more than one CD4006BMS.
 *
 * To facilitate cascading stages when clock rise and fall times are slow,
 * an optional output (D1 + 4’) that is delayed one-half clockcycle, is
 * provided.
 *
 *            +--------------+
 *       D1   |1     ++    14| VDD
 *      D1+4' |2           13| D1+4
 *      CLOCK |3           12| D2+5
 *         D2 |4    4006   11| D2+4
 *         D3 |5           10| D3+4
 *         D4 |6            9| D4+5
 *        VSS |7            8| D4+4
 *            +--------------+
 *
 *
 *  Naming conventions follow SYC datasheet
 *
 *  FIXME: Timing depends on VDD-VSS
 *
 */

#ifndef NLD_4006_H_
#define NLD_4006_H_

#include "netlist/nl_setup.h"

#define CD4006(name)                                                            \
		NET_REGISTER_DEV(CD4006, name)

#endif /* NLD_4006_H_ */
