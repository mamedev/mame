/***************************************************************************

t10mmc.h

***************************************************************************/

#ifndef _T10MMC_H_
#define _T10MMC_H_

#include "t10spc.h"
#include "imagedev/chd_cd.h"
#include "sound/cdda.h"

class t10mmc : public virtual t10spc
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

	void abort_audio();
	int toc_tracks();

	cdrom_image_device *m_image;
	cdda_device *m_cdda;
	cdrom_file *cdrom;

	UINT32 lba;
	UINT32 blocks;
	UINT32 last_lba;
	UINT32 num_subblocks;
	UINT32 cur_subblock;
	int m_audio_sense;
};

#endif
