// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_BUS_NSCSI_CD_H
#define MAME_BUS_NSCSI_CD_H

#pragma once

#include "imagedev/cdromimg.h"
#include "machine/nscsi_bus.h"
#include "sound/cdda.h"

#include "cdrom.h"


class nscsi_cdrom_device : public nscsi_full_device
{
public:
	nscsi_cdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_block_size(u32 block_size);

protected:
	required_device<cdrom_image_device> image;
	required_device<cdda_device> cdda;

	nscsi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	nscsi_cdrom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, const char *mfr, const char *product, const char *rev, uint8_t inq_data, uint8_t compliance)
		: nscsi_cdrom_device(mconfig, type, tag, owner, 0)
	{
		strncpy(manufacturer, mfr, 8);
		strncpy(this->product, product, 16);
		strncpy(revision, rev, 4);
		inquiry_data = inq_data;
		this->compliance = compliance;
	}

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void scsi_command() override;
	virtual uint8_t scsi_get_data(int id, int pos) override;
	virtual void scsi_put_data(int buf, int offset, uint8_t data) override;

	virtual void return_no_cd();
	static int to_msf(int frame);

	bool m_removal_prevented;

private:
	static constexpr uint32_t bytes_per_sector = 2048;

	u32 sequence_counter;
	uint8_t sector_buffer[bytes_per_sector];
	uint32_t bytes_per_block;
	int lba, cur_sector;
	uint8_t mode_data[256];
	uint8_t mode_data_size;

	char manufacturer[8];
	char product[16];
	char revision[4];
	uint8_t inquiry_data;
	uint8_t compliance;

	uint8_t cdda_sotc;
};

class nscsi_cdrom_sgi_device : public nscsi_cdrom_device
{
public:
	nscsi_cdrom_sgi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void scsi_command() override;
	virtual bool scsi_command_done(uint8_t command, uint8_t length) override;
};

class nscsi_cdrom_news_device : public nscsi_cdrom_device
{
public:
	nscsi_cdrom_news_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_dec_rrd45_device : public nscsi_cdrom_device
{
public:
	nscsi_dec_rrd45_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_toshiba_xm3301_device : public nscsi_cdrom_device
{
public:
	nscsi_toshiba_xm3301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_toshiba_xm5301_sun_device : public nscsi_cdrom_device
{
public:
	nscsi_toshiba_xm5301_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_toshiba_xm5401_device : public nscsi_cdrom_device
{
public:
	nscsi_toshiba_xm5401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_toshiba_xm5401_sun_device : public nscsi_cdrom_device
{
public:
	nscsi_toshiba_xm5401_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_toshiba_xm5701_device : public nscsi_cdrom_device
{
public:
	nscsi_toshiba_xm5701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_toshiba_xm5701_sun_device : public nscsi_cdrom_device
{
public:
	nscsi_toshiba_xm5701_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class nscsi_cdrom_apple_device : public nscsi_cdrom_device
{
public:
	nscsi_cdrom_apple_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual void device_start() override ATTR_COLD;

protected:
	virtual void scsi_command() override;
	virtual bool scsi_command_done(uint8_t command, uint8_t length) override;
	virtual void scsi_put_data(int buf, int offset, uint8_t data) override;
	virtual void return_no_cd() override;

private:
	bool m_stopped;
	uint32_t m_stop_position;
};

DECLARE_DEVICE_TYPE(NSCSI_CDROM, nscsi_cdrom_device)
DECLARE_DEVICE_TYPE(NSCSI_CDROM_SGI, nscsi_cdrom_sgi_device)
DECLARE_DEVICE_TYPE(NSCSI_CDROM_NEWS, nscsi_cdrom_news_device)
DECLARE_DEVICE_TYPE(NSCSI_RRD45, nscsi_dec_rrd45_device)
DECLARE_DEVICE_TYPE(NSCSI_XM3301, nscsi_toshiba_xm3301_device)
DECLARE_DEVICE_TYPE(NSCSI_XM5301SUN, nscsi_toshiba_xm5301_sun_device)
DECLARE_DEVICE_TYPE(NSCSI_XM5401, nscsi_toshiba_xm5401_device)
DECLARE_DEVICE_TYPE(NSCSI_XM5401SUN, nscsi_toshiba_xm5401_sun_device)
DECLARE_DEVICE_TYPE(NSCSI_XM5701, nscsi_toshiba_xm5701_device)
DECLARE_DEVICE_TYPE(NSCSI_XM5701SUN, nscsi_toshiba_xm5701_sun_device)
DECLARE_DEVICE_TYPE(NSCSI_CDROM_APPLE, nscsi_cdrom_apple_device)

#endif // MAME_BUS_NSCSI_CD_H
