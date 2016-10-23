// license:BSD-3-Clause
// copyright-holders:Tim Lindner
/*********************************************************************

    ds1315.h

    Dallas Semiconductor's Phantom Time Chip DS1315.

    by tim lindner, November 2001.

*********************************************************************/

#ifndef __DS1315_H__
#define __DS1315_H__

#include "emu.h"


/***************************************************************************
    MACROS
***************************************************************************/

enum ds1315_mode_t
{
	DS_SEEK_MATCHING,
	DS_CALENDAR_IO
};

ALLOW_SAVE_TYPE(ds1315_mode_t);

class ds1315_device : public device_t
{
public:
	ds1315_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~ds1315_device() {}

	uint8_t read_0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t read_1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t read_data(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t write_data(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	bool chip_enable();
	void chip_reset();

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	ds1315_mode_t m_mode;

	void fill_raw_data();
	void input_raw_data();

	int m_count;
	uint8_t m_raw_data[8*8];
};

extern const device_type DS1315;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_DS1315_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DS1315, 0)


#endif /* __DS1315_H__ */
