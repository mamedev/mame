// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  KB1013VK1-2

*/

#ifndef _KB1013VK12_H_
#define _KB1013VK12_H_

#include "sm500.h"


// I/O ports setup

// ..





class kb1013vk12_device : public sm500_device
{
public:
	kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void execute_one() override;

	// opcode handlers
};


extern const device_type KB1013VK12;


#endif /* _KB1013VK12_H_ */
