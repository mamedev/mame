// license:BSD-3-Clause
// copyright-holders:smf
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

	enum
	{
		T10MMC_CMD_READ_SUB_CHANNEL = 0x42,
		T10MMC_CMD_READ_TOC_PMA_ATIP = 0x43,
		T10MMC_CMD_PLAY_AUDIO_10 = 0x45,
		T10MMC_CMD_PLAY_AUDIO_TRACK_INDEX = 0x48,
		T10MMC_CMD_PAUSE_RESUME = 0x4b,
		T10MMC_CMD_STOP_PLAY_SCAN = 0x4e,
		T10MMC_CMD_PLAY_AUDIO_12 = 0xa5,
		T10MMC_CMD_SET_CD_SPEED = 0xbb
	};

	enum toc_format_t
	{
		TOC_FORMAT_TRACKS = 0,
		TOC_FORMAT_SESSIONS = 1
	};

	void abort_audio();
	toc_format_t toc_format();
	int toc_tracks();

	cdrom_image_device *m_image;
	cdda_device *m_cdda;
	cdrom_file *m_cdrom;

	UINT32 m_lba;
	UINT32 m_blocks;
	UINT32 m_last_lba;
	UINT32 m_num_subblocks;
	UINT32 m_cur_subblock;
	int m_audio_sense;

	device_t *m_device;
};

#endif
