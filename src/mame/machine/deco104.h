// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef __DECO104_H__
#define __DECO104_H__

#include "deco146.h"



/* Data East 104 protection chip */

class deco104_device : public deco_146_base_device
{
public:
	deco104_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);




protected:
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;





private:


};

extern const device_type DECO104PROT;


#define MCFG_DECO104_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECO104PROT, 0)




#endif
