// license:BSD-3-Clause
// copyright-holders:Damian Rogers
/***************************************************************************

    NEC CD-ROM DRIVE:FX (PC-FX internal CD-ROM drive)

    A SCSI-2 CD-ROM drive with the NEC vendor audio commands.

***************************************************************************/

#ifndef MAME_BUS_NSCSI_PCFXCD_H
#define MAME_BUS_NSCSI_PCFXCD_H

#pragma once

#include "cd.h"

class nscsi_cdrom_pcfx_device : public nscsi_cdrom_device
{
public:
	nscsi_cdrom_pcfx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void scsi_command() override;
	virtual bool scsi_command_done(u8 command, u8 length) override;   // c0h-ffh are 10 bytes
	virtual void scsi_abort() override;                               // stops CD-DA
	virtual attotime scsi_data_byte_period() override;     // 2x: 307,200 bytes/s
	virtual attotime scsi_data_command_delay() override;   // command processing time

private:
	// NEC audio commands
	void nec_set_audio_start_position();   // d8h
	void nec_set_audio_end_position();     // d9h
	void nec_pause();                      // dah
	void nec_read_subq();                  // ddh
	void nec_get_dir_info();               // deh
	void nec_audio_end(int state);         // cdda end callback

	bool nec_address(const u8 *cdb, u32 &lba);     // byte 9 bits 7-6: LBA / BCD MSF / BCD track
	void nec_play(u32 start, u32 end);

	static u8 to_bcd(u32 v);
	static u8 from_bcd(u8 v);
	static void put_msf(u8 *dest, s32 lba);

	// read sub-channel Q status codes
	enum : u8 { CDDA_PLAYING = 0, CDDA_STILL = 1, CDDA_PAUSED = 2, CDDA_STOPPED = 3 };

	u8 m_cdda_status;
	bool m_cdda_loop;
	u32 m_cdda_start;
	u32 m_cdda_end;

	// m_scsi_cmdbuf[0] as of the last scsi_command() dispatch: scsi_data_command_delay() runs
	// after scsi_command() has returned, by which point a data-in command has overwritten the
	// buffer with its response
	u8 m_command;
};

DECLARE_DEVICE_TYPE(NSCSI_CDROM_PCFX, nscsi_cdrom_pcfx_device)

#endif // MAME_BUS_NSCSI_PCFXCD_H
