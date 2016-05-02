// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * s11a.h
 *
 *  Created on: 1/01/2013
 */

#ifndef S11A_H_
#define S11A_H_

#include "includes/s11.h"

class s11a_state : public s11_state
{
public:
	s11a_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11_state(mconfig, type, tag)
	{ }

	DECLARE_WRITE8_MEMBER(bgbank_w);
	DECLARE_WRITE8_MEMBER(dig0_w);
	DECLARE_MACHINE_RESET(s11a);
	DECLARE_DRIVER_INIT(s11a);

protected:

private:

};

#endif /* S11A_H_ */
