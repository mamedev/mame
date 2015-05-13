// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_S1985_H
#define __MSX_S1985_H


#include "msx_switched.h"


extern const device_type MSX_S1985;


#define MCFG_MSX_S1985_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MSX_S1985, 0)


class msx_s1985_device : public msx_switched_device
{
public:
	msx_s1985_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual UINT8 get_id();

	virtual DECLARE_READ8_MEMBER(io_read);
	virtual DECLARE_WRITE8_MEMBER(io_write);

private:
	UINT8 m_6_1;
	UINT8 m_6_2;
	UINT8 m_7;
};

#endif
