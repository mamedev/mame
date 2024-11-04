// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Various 512-byte-block SCSI CD-ROM devices

***************************************************************************/

#ifndef DEVICES_BUS_SCSI_SCSICD512_H
#define DEVICES_BUS_SCSI_SCSICD512_H

#pragma once

#include "scsicd.h"

class scsicd512_device : public scsicd_device
{
public:
	virtual void ExecCommand() override;
	virtual void ReadData(uint8_t *data, int dataLength) override;

protected:
	scsicd512_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	scsicd512_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner,
		const char *mfr, const char *product, const char *rev, uint8_t data)
		: scsicd512_device(mconfig, type, tag, owner, 0)
	{
		strncpy(m_manufacturer, mfr, 8);
		strncpy(m_product, product, 16);
		strncpy(m_revision, rev, 4);
		m_data = data;
	}

	virtual void device_reset() override ATTR_COLD;

	char m_manufacturer[8];
	char m_product[16];
	char m_revision[4];
	uint8_t m_data;
};

class dec_rrd45_device : public scsicd512_device
{
public:
	dec_rrd45_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class toshiba_xm3301_device : public scsicd512_device
{
public:
	toshiba_xm3301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class toshiba_xm5301_sun_device : public scsicd512_device
{
public:
	toshiba_xm5301_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class toshiba_xm5401_sun_device : public scsicd512_device
{
public:
	toshiba_xm5401_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class toshiba_xm5701_device : public scsicd512_device
{
public:
	toshiba_xm5701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class toshiba_xm5701_sun_device : public scsicd512_device
{
public:
	toshiba_xm5701_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(RRD45, dec_rrd45_device)
DECLARE_DEVICE_TYPE(XM3301, toshiba_xm3301_device)
DECLARE_DEVICE_TYPE(XM5301SUN, toshiba_xm5301_sun_device)
DECLARE_DEVICE_TYPE(XM5401SUN, toshiba_xm5401_sun_device)
DECLARE_DEVICE_TYPE(XM5701, toshiba_xm5701_device)
DECLARE_DEVICE_TYPE(XM5701SUN, toshiba_xm5701_sun_device)

#endif // DEVICES_BUS_SCSI_SCSICD512_H
