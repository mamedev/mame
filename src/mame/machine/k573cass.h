#pragma once

#ifndef __K573CASS_H__
#define __K573CASS_H__

#include "cpu/psx/siodev.h"

extern const device_type KONAMI573CASSETTE;

class konami573cassette_device : public psxsiodev_device
{
public:
	konami573cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	void device_start();

private:
	virtual void data_in( int data, int mask );
};

#endif
