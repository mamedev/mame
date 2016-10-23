// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __MSX_SWITCHED_H
#define __MSX_SWITCHED_H


class msx_switched_device : public device_t
{
public:
	msx_switched_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual uint8_t get_id() = 0;

	virtual uint8_t io_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) = 0;
	virtual void io_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) = 0;

protected:
	virtual void device_start() override;
};

#endif
