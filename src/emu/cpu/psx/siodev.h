#pragma once

#include "emu.h"

#ifndef _PSXSIODEV_H_
#define _PSXSIODEV_H_

#define PSX_SIO_OUT_DATA ( 1 )	/* COMMAND */
#define PSX_SIO_OUT_DTR ( 2 )	/* ATT */
#define PSX_SIO_OUT_RTS ( 4 )
#define PSX_SIO_OUT_CLOCK ( 8 )	/* CLOCK */
#define PSX_SIO_IN_DATA ( 1 )	/* DATA */
#define PSX_SIO_IN_DSR ( 2 )	/* ACK */
#define PSX_SIO_IN_CTS ( 4 )

class psxsio_device;

class psxsiodev_device : public device_t
{
	friend class psxsio_device;

public:
	// construction/destruction
	psxsiodev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();

	void data_out( int data, int mask );

private:
	psxsio_device *m_psxsio;

	virtual void data_in( int data, int mask ) = 0;
	int m_dataout;
};

#endif
