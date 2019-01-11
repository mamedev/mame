// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

t10sbc.h

***************************************************************************/

#ifndef MAME_MACHINE_T10SBC_H
#define MAME_MACHINE_T10SBC_H

#pragma once

#include "t10spc.h"
#include "imagedev/harddriv.h"

class t10sbc : public virtual t10spc
{
public:
	t10sbc()
		: t10spc(), m_image(nullptr), m_lba(0), m_blocks(0), m_disk(nullptr), m_device(nullptr)
	{
	}

	virtual void SetDevice( void *device ) override;
	virtual void GetDevice( void **device ) override;
	virtual void ExecCommand() override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;

protected:
	virtual void t10_start(device_t &device) override;
	virtual void t10_reset() override;

	harddisk_image_device *m_image;

	uint32_t m_lba;
	uint32_t m_blocks;

	hard_disk_file *m_disk;
	device_t *m_device;
};

#endif // MAME_MACHINE_T10SBC_H
