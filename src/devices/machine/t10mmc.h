// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

t10mmc.h

***************************************************************************/

#ifndef MAME_MACHINE_T10MMC_H
#define MAME_MACHINE_T10MMC_H

#pragma once

#include "t10spc.h"
#include "imagedev/cdromimg.h"
#include "sound/cdda.h"

class t10mmc : public virtual t10spc
{
public:
	t10mmc()
		: t10spc(), m_image(nullptr), m_cdda(nullptr), m_model_name("MAME    Virtual CDROM   1.0 "), m_lba(0), m_blocks(0), m_last_lba(0), m_num_subblocks(0), m_cur_subblock(0), m_audio_sense(0), m_device(nullptr)
	{
	}

	virtual void ExecCommand() override;
	virtual void WriteData( uint8_t *data, int dataLength ) override;
	virtual void ReadData( uint8_t *data, int dataLength ) override;

	void set_model(std::string model_name);

protected:
	virtual void t10_start(device_t &device) override;
	virtual void t10_reset() override;

	enum
	{
		T10MMC_CMD_READ_SUB_CHANNEL = 0x42,
		T10MMC_CMD_READ_TOC_PMA_ATIP = 0x43,
		T10MMC_CMD_PLAY_AUDIO_10 = 0x45,
		T10MMC_CMD_PLAY_AUDIO_MSF = 0x47,
		T10MMC_CMD_PLAY_AUDIO_TRACK_INDEX = 0x48,
		T10MMC_CMD_PAUSE_RESUME = 0x4b,
		T10MMC_CMD_STOP_PLAY_SCAN = 0x4e,
		T10MMC_CMD_PLAY_AUDIO_12 = 0xa5,
		T10MMC_CMD_READ_DISC_STRUCTURE = 0xad,
		T10MMC_CMD_SET_CD_SPEED = 0xbb,
		T10MMC_CMD_MECHANISM_STATUS = 0xbd,
		T10MMC_CMD_READ_CD = 0xbe
	};

	enum
	{
		T10MMC_READ_CD_SECTOR_TYPE_ANY = 0,
		T10MMC_READ_CD_SECTOR_TYPE_CDDA = 1,
		T10MMC_READ_CD_SECTOR_TYPE_MODE1 = 2,
		T10MMC_READ_CD_SECTOR_TYPE_MODE2 = 3,
		T10MMC_READ_CD_SECTOR_TYPE_MODE2_FORM1 = 4,
		T10MMC_READ_CD_SECTOR_TYPE_MODE2_FORM2 = 5,

		// These shouldn't be treated as masks, 3 is a reserved value
		T10MMC_READ_CD_C2_ONLY = 1, // C2 error bits (294 bytes)
		T10MMC_READ_CD_C2_BLOCK = 2, // C2 error bits (294 bytes) + block error byte (logical OR of all the C2 error bit bytes) + 1 padding byte

		T10MMC_READ_CD_FIELD_ECC = 0x08, // EDC/ECC
		T10MMC_READ_CD_FIELD_USER_DATA = 0x10,
		T10MMC_READ_CD_FIELD_HEADER = 0x20,
		T10MMC_READ_CD_FIELD_SUBHEADER = 0x40,
		T10MMC_READ_CD_FIELD_SYNC = 0x80,
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

	std::string m_model_name;

	uint32_t m_lba;
	uint32_t m_blocks;
	uint32_t m_last_lba;
	uint32_t m_num_subblocks;
	uint32_t m_cur_subblock;
	int m_audio_sense;
	int m_sotc;
	uint8_t m_read_cd_flags;

	device_t *m_device;
};

#endif // MAME_MACHINE_T10MMC_H
