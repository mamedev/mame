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
	nscsi_cdrom_pc8801_30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	int16_t get_channel_sample(offs_t offset) { return cdda->get_channel_sample(offset); };
	virtual void fader_control_w(u8 data);
	// I/O port $98
	bool is_motor_on() { return image->exists(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void scsi_command() override;
	virtual bool scsi_command_done(u8 command, u8 length) override;
//  virtual attotime scsi_data_command_delay() override;

	virtual TIMER_CALLBACK_MEMBER(cdda_fader_cb);
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

	u32  m_current_frame;
	u32  m_end_frame;
	u32  m_last_frame;
	u8   m_cdda_status;
	u8   m_cdda_play_mode;

	u8   m_end_mark;

	void cdda_end_mark_cb(int state);

	u8   m_fader_ctrl;
	double    m_cdda_volume;

	emu_timer *m_cdda_fader_timer;
};

DECLARE_DEVICE_TYPE(NSCSI_CDROM_PC8801_30, nscsi_cdrom_pc8801_30_device)


#endif // MAME_BUS_NSCSI_PC8801_30_H

