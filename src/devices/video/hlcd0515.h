// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hughes HLCD 0515/0569 LCD Driver

*/

#ifndef _HLCD0515_H_
#define _HLCD0515_H_

#include "emu.h"


class hlcd0515_device : public device_t
{
public:
	hlcd0515_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	hlcd0515_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source);

	// static configuration helpers
	//template<class _Object> static devcb_base &set_write_x_callback(device_t &device, _Object object) { return downcast<hlcd0515_device &>(device).m_write_x.set_callback(object); }

	//DECLARE_WRITE_LINE_MEMBER(write_cs);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_cs;

	// callbacks
	//devcb_write32 m_write_x;
};


class hlcd0569_device : public hlcd0515_device
{
public:
	hlcd0569_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};



extern const device_type HLCD0515;
extern const device_type HLCD0569;


#endif /* _HLCD0515_H_ */
