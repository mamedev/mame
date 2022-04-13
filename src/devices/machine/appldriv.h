// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    appldriv.h

    Apple 5.25" floppy drive emulation (to be interfaced with applefdc.c)

*********************************************************************/

#ifndef MAME_MACHINE_APPLDRIV_H
#define MAME_MACHINE_APPLDRIV_H

#pragma once

#include "imagedev/flopdrv.h"
#include "formats/ap2_dsk.h"

#define FLOPPY_0 "floppy0"
#define FLOPPY_1 "floppy1"
#define FLOPPY_2 "floppy2"
#define FLOPPY_3 "floppy3"

void apple525_set_lines(device_t *device, uint8_t lines);
void apple525_set_enable_lines(device_t *device, int enable_mask);

uint8_t apple525_read_data(device_t *device);
void apple525_write_data(device_t *device, uint8_t data);
int apple525_read_status(device_t *device);
int apple525_get_count(running_machine &machine);

class apple525_floppy_image_device : public legacy_floppy_image_device
{
public:
	// construction/destruction
	apple525_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, const floppy_interface *config, int dividend, int divisor)
		: apple525_floppy_image_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_floppy_config(config);
		set_params(dividend, divisor);
	}
	apple525_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	void set_params(int dividend, int divisor) { m_dividend = dividend; m_divisor = divisor; }

	int get_dividend() { return m_dividend; }
	int get_divisor() { return m_divisor; }

	// these elements should be private, but are not yet
	unsigned int state : 4;     /* bits 0-3 are the phase */
	unsigned int tween_tracks : 1;
	unsigned int track_loaded : 1;
	unsigned int track_dirty : 1;
	int position;
	int spin_count;         /* simulate drive spin to fool RWTS test at $BD34 */
	uint8_t track_data[APPLE2_NIBBLE_SIZE * APPLE2_SECTOR_COUNT];

protected:
	virtual void device_start() override;

private:
	int m_dividend;
	int m_divisor;
};

// device type definition
DECLARE_DEVICE_TYPE(FLOPPY_APPLE, apple525_floppy_image_device)

#endif // MAME_MACHINE_APPLDRIV_H
