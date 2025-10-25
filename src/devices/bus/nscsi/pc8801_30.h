// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_NSCSI_PC8801_30_H
#define MAME_BUS_NSCSI_PC8801_30_H

#pragma once

#include "bus/nscsi/cd.h"

#include "imagedev/cdromimg.h"
#include "machine/nscsi_bus.h"
#include "sound/cdda.h"

#include "cdrom.h"

class nscsi_cdrom_pc8801_30_device : public nscsi_cdrom_device
{
public:
	nscsi_cdrom_pc8801_30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_reset() override ATTR_COLD;

	virtual void scsi_command() override;
	virtual bool scsi_command_done(uint8_t command, uint8_t length) override;
private:
	void nec_set_audio_start_position();
	void nec_set_audio_stop_position();
	void nec_pause();
	void nec_get_subq();
	void nec_get_dir_info();

	enum {
		PCE_CD_CDDA_OFF = 0,
		PCE_CD_CDDA_PLAYING,
		PCE_CD_CDDA_PAUSED
	};

	uint32_t  m_current_frame = 0;
	uint32_t  m_end_frame = 0;
	uint32_t  m_last_frame = 0;
	uint8_t   m_cdda_status = 0;
	uint8_t   m_cdda_play_mode = 0;

	uint8_t   m_end_mark = 0;

};

DECLARE_DEVICE_TYPE(NSCSI_CDROM_PC8801_30, nscsi_cdrom_pc8801_30_device)


#endif // MAME_BUS_NSCSI_PC8801_30_H

