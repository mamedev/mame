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
	msx_s1985_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t get_id() override;

	virtual uint8_t io_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void io_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_6_1;
	uint8_t m_6_2;
	uint8_t m_7;
};

#endif
