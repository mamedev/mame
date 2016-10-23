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

	void bgbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dig0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_reset_s11a();
	void init_s11a();

protected:

private:

};

#endif /* S11A_H_ */
