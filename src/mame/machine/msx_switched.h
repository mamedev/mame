// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SWITCHED_H
#define __MSX_SWITCHED_H


class msx_switched_device : public device_t
{
public:
	msx_switched_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual UINT8 get_id() = 0;

	virtual DECLARE_READ8_MEMBER(io_read) = 0;
	virtual DECLARE_WRITE8_MEMBER(io_write) = 0;

protected:
	virtual void device_start() override;
};

#endif
