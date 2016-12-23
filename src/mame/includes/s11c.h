// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
 * s11c.h
 *
 *  Created on: 1/01/2013
 */

#ifndef S11C_H_
#define S11C_H_

#include "includes/s11b.h"

class s11c_state : public s11b_state
{
public:
	s11c_state(const machine_config &mconfig, device_type type, const char *tag)
		: s11b_state(mconfig, type, tag)
	{ }

	DECLARE_MACHINE_RESET(s11c);
	DECLARE_DRIVER_INIT(s11c);

protected:

private:


};


#endif /* S11C_H_ */
