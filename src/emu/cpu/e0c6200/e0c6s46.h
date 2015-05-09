// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6S46 MCU

*/

#ifndef _E0C6S46_H_
#define _E0C6S46_H_

#include "e0c6200.h"


class e0c6s46_device : public e0c6200_cpu_device
{
public:
	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device_execute_interface overrides
	virtual void execute_one();
};



extern const device_type E0C6S46;

#endif /* _E0C6S46_H_ */
