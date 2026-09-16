// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_BUS_NSCSI_HD_H
#define MAME_BUS_NSCSI_HD_H

#pragma once

#include "machine/nscsi_hle.h"
#include "imagedev/harddriv.h"

class nscsi_harddisk_device : public nscsi_full_device
{
public:
	nscsi_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	void set_default_model_name(const std::string_view &model);

	// Optional, driver-supplied cylinder-aware seek-timing model, shared by all
	// hard-disk controllers built on this device.  With no call the device keeps
	// its inherited behaviour (the subclass's legacy flat delay, or none for a
	// plain NSCSI_HARDDISK), so existing users are byte-for-byte unchanged.  A
	// driver opts in once the controller is attached at a slot, e.g. from
	// machine_start().  Once set, a command pays a real seek only when the target
	// cylinder changes; consecutive sectors on one cylinder pay only rotational
	// latency scaled by the interleave.
	//
	// Deriving the five parameters -- all four times come from the drive's OEM
	// manual / datasheet; only interleave comes from the host's disk format:
	//
	//   track_us   - track-to-track (single-cylinder) seek time, in microseconds.
	//   average_us - average (random) seek time, in microseconds.  Manufacturers
	//                quote this for a seek over ~1/3 of the stroke; the model
	//                solves its curve's exponent so a 1/3-stroke seek costs
	//                exactly this value (track-to-track and full-stroke anchor
	//                the ends).
	//   full_us    - full-stroke (maximum) seek time, in microseconds.  Use the
	//                "maximum" seek figure; head settling is normally already
	//                included in the quoted seek times.
	//   rpm        - spindle speed.  One revolution = 60e6 / rpm us; a same-
	//                cylinder read costs interleave revolutions per track of
	//                spt sectors, and a seek adds a half-revolution of latency.
	//   interleave - the sector interleave the host CBIOS formatted the media
	//                with (1 = 1:1, 2 = 2:1, ...).  This is a property of the
	//                FORMAT, not the drive, and sets the single-cylinder
	//                sequential transfer rate.
	//
	// The cylinder count is taken from the mounted CHD's geometry, so one call
	// serves differently-sized members of a drive family (e.g. ST-506 153 cyl
	// vs ST-412 306 cyl).  If a datasheet gives only two of the three seek
	// times, pass the missing one equal to a neighbour (e.g. average_us =
	// full_us); the fit then degrades gracefully toward a flat per-seek cost.
	//
	// Example -- Seagate ST-506 / ST-412 (ST412 OEM manual, April 1982): 3600
	// RPM, track-to-track 3 ms, average 85 ms, full-stroke 205 ms, 1:1 format:
	//     if (auto *s = dynamic_cast<nscsi_harddisk_device *>(subdevice("sasi:0:s1410")))
	//         s->set_seek_timing(3000, 85000, 205000, 3600, 1);
	//
	void set_seek_timing(uint32_t track_us, uint32_t average_us, uint32_t full_us, uint32_t rpm, uint8_t interleave);

protected:
	nscsi_harddisk_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, uint8_t data) override;
	virtual attotime scsi_data_command_delay() override;

	// Cylinder-aware seek delay for the block at lba, using the model configured
	// by set_seek_timing().  Updates the tracked head position.  Subclasses with
	// their own command dispatch call this from scsi_data_command_delay() for the
	// commands that move the heads; the base applies it to the generic SCSI
	// READ/WRITE/SEEK (6- and 10-byte) opcodes.
	attotime seek_time(uint32_t lba);

	required_device<harddisk_image_device> image;
	std::string m_default_model_name;
	uint8_t block[512];
	int lba, cur_lba, blocks;
	int bytes_per_sector;

	std::vector<u8> m_inquiry_data;

	// Seek-timing model state (configured by set_seek_timing(); while
	// m_seek_model is false the model is inert and scsi_data_command_delay()
	// returns the base zero delay -- no change for existing users).
	bool     m_seek_model = false;
	int      m_last_cylinder = -1;   // current head cylinder (-1 = unknown)
	uint8_t  m_interleave = 1;       // sector interleave (1 = 1:1)
	uint32_t m_rpm = 3600;           // spindle speed -> rotational latency
	uint32_t m_seek_track_us = 0;    // track-to-track seek time
	uint32_t m_seek_range_us = 0;    // full-stroke minus track-to-track
	double   m_seek_exp = 1.0;       // seek(d) = track + range*(d/ncyl)^exp
};

DECLARE_DEVICE_TYPE(NSCSI_HARDDISK, nscsi_harddisk_device)

#endif // MAME_BUS_NSCSI_HD_H
