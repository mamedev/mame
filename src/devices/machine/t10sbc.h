// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

t10sbc.h

***************************************************************************/

#ifndef _T10SBC_H_
#define _T10SBC_H_

#include "t10spc.h"
#include "imagedev/harddriv.h"

class t10sbc : public virtual t10spc
{
public:
	virtual void SetDevice( void *device );
	virtual void GetDevice( void **device );
	virtual void ExecCommand();
	virtual void WriteData( UINT8 *data, int dataLength );
	virtual void ReadData( UINT8 *data, int dataLength );

protected:
	virtual void t10_start(device_t &device);
	virtual void t10_reset();

	harddisk_image_device *m_image;

	UINT32 m_lba;
	UINT32 m_blocks;

	hard_disk_file *m_disk;
	device_t *m_device;
};

#endif
