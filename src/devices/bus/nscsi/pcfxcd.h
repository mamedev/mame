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

	// sub-code symbols at 7.35 kHz during audio play: bit 7 = SCOR (block sync, set on S1), bits 6-0 = Q-W
	auto subcode_cb() { return m_subcode_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void scsi_command() override;
	virtual bool scsi_command_done(u8 command, u8 length) override;   // c0h-ffh are 10 bytes
	virtual void scsi_abort() override;                               // stops CD-DA
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, uint8_t data) override;
	virtual attotime scsi_data_byte_period() override;     // wait for the next sector, else bus byte time
	virtual attotime scsi_data_command_delay() override;   // READ: wait for the first sector

private:
	// read sub-channel Q status codes
	enum : u8 { CDDA_PLAYING = 0, CDDA_STILL = 1, CDDA_PAUSED = 2, CDDA_STOPPED = 3 };

	// NEC audio commands
	void nec_set_audio_start_position();   // d8h
	void nec_set_audio_end_position();     // d9h
	void nec_pause();                      // dah
	void nec_read_subq();                  // ddh
	void nec_get_dir_info();               // deh
	void nec_audio_end(int state);         // cdda end callback

	bool nec_address(const u8 *cdb, u32 &lba);     // byte 9 bits 7-6: LBA / BCD MSF / BCD track
	void nec_play(u32 start, u32 end);
	void scsi_play_audio(u32 start, u32 end);   // 47h, 48h

	static u8 to_bcd(u32 v);
	static u8 from_bcd(u8 v);
	static void put_msf(u8 *dest, s32 lba);

	void make_subq(u32 lba, u8 *q);        // 12 bytes of Q (mode 1, current position) with CRC
	void set_cdda_status(u8 status);       // the sub-code stream runs while playing
	TIMER_CALLBACK_MEMBER(subcode_tick);

	void start_read(u32 blocks);
	attotime next_sector_wait();

	u8 m_cdda_status;
	bool m_cdda_loop;
	u32 m_cdda_start;
	u32 m_cdda_end;

	devcb_write8 m_subcode_cb;
	emu_timer *m_subcode_timer;
	u8 m_subcode_symbol;   // 0-97 within the sub-code block, 0 and 1 are the S0/S1 sync symbols
	u8 m_subq[12];

	// m_scsi_cmdbuf[0] as of the last scsi_command() dispatch: scsi_data_command_delay() runs
	// after scsi_command() has returned, by which point a data-in command has overwritten the
	// buffer with its response
	u8 m_command;

	// data reads: sectors still to be read from the disc, sectors waiting in the
	// read-ahead buffer, when the sector being read is done, and the wait to apply
	// before the next sector goes over the bus
	u32 m_read_left;
	u32 m_read_buffered;
	attotime m_read_done;
	attotime m_sector_wait;
};

DECLARE_DEVICE_TYPE(NSCSI_CDROM_PCFX, nscsi_cdrom_pcfx_device)

#endif // MAME_BUS_NSCSI_PCFXCD_H
