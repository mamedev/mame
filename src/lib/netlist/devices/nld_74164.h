// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*****************************************************************************

    5/74164 8-bit parallel-out serial shift registers

***********************************************************************

    Connection Diagram:
              ___ ___
        A  1 |*  u   | 14  Vcc
        B  2 |       | 13  QH
       QA  3 |       | 12  QG
       QB  4 |       | 11  QF
       QC  5 |       | 10  QE
       QD  6 |       |  9  *Clear
      GND  7 |_______|  8  Clock

***********************************************************************
    Function Table:
    +-------------------------+----------------+
    |       Inputs            |  Qutputs*      |
    +-------+-------+---------+----------------+
    | Clear | Clock |  A   B  | QA  QB ... QH  |
    +-------+-------+---------+----------------+
    |   L   |   X   |  X   X  |  L   L      L  |
    |   H   |   L   |  X   X  | QA0 QB0    QH0 |
    |   H   |   ^   |  H   H  |  H  QAn    QGn |
    |   H   |   ^   |  L   X  |  L  QAn    QGn |
    |   H   |   ^   |  X   L  |  L  QAn    QGn |
    +-------+-------+---------+----------------+

    H = High Level (steady state)
    L = Low Level (steady state)
    X = Don't Care
    ^ = Transition from low to high level
    QA0, QB0 ... QH0 = The level of QA, QB ... QH before the indicated steady-state input conditions were established.
    QAn, QGn = The level of QA or QG before the most recent ^ transition of the clock; indicates a 1 bit shift.

**********************************************************************/

#ifndef NLD_74164_H_
#define NLD_74164_H_

#include "netlist/nl_setup.h"

#define TTL_74164(name, cA, cB, cCLRQ, cCLK)                                   \
		NET_REGISTER_DEV(TTL_74164, name)                                      \
		NET_CONNECT(name, GND, GND)                                            \
		NET_CONNECT(name, VCC, VCC)                                            \
		NET_CONNECT(name, A,       cA)                                         \
		NET_CONNECT(name, B,       cB)                                         \
		NET_CONNECT(name, CLRQ,   cCLRQ)                                       \
		NET_CONNECT(name, CLK,     cCLK)

#define TTL_74164_DIP(name)                                                    \
		NET_REGISTER_DEV(TTL_74164_DIP, name)

#endif /* NLD_74164_H_ */
