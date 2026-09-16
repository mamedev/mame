// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    DEC RRD45 CD-ROM

***************************************************************************/

#include "emu.h"
#include "scsicd512.h"

DEFINE_DEVICE_TYPE(RRD45, dec_rrd45_device, "rrd45", "DEC RRD45")
DEFINE_DEVICE_TYPE(XM3301TA, toshiba_xm3301ta_device, "xm3301ta", "Toshiba XM-3301TA CD-ROM")
DEFINE_DEVICE_TYPE(XM5301SUN, toshiba_xm5301_sun_device, "xm5301sun", "Toshiba XM-5301B Sun 4x CD-ROM")
DEFINE_DEVICE_TYPE(XM5401SUN, toshiba_xm5401_sun_device, "xm5401sun", "Toshiba XM-5401B Sun 4x CD-ROM")
DEFINE_DEVICE_TYPE(XM5701, toshiba_xm5701_device, "xm5701", "Toshiba XM-5701B 12x CD-ROM")
DEFINE_DEVICE_TYPE(XM5701SUN, toshiba_xm5701_sun_device, "xm5701sun", "Toshiba XM-5701B Sun 12x CD-ROM")

void scsicd512_device::device_reset()
{
	scsihle_device::device_reset();

	m_sector_bytes = 512;
	m_num_subblocks = 4;
}

void scsicd512_device::ExecCommand()
{
	switch (command[0])
	{
		case 0x12: // INQUIRY
			m_phase = SCSI_PHASE_DATAIN;
			m_status_code = SCSI_STATUS_CODE_GOOD;
			m_transfer_length = SCSILengthFromUINT8(&command[4]);
			break;
		default:
			scsicd_device::ExecCommand();
			break;
	}
}

void scsicd512_device::ReadData(uint8_t *data, int dataLength)
{
	switch (command[0])
	{
		case 0x12: // INQUIRY
		{
			uint8_t buffer[36];
			memset(buffer, 0, dataLength);
			buffer[0] = 0x05; // device is present, device is CD/DVD (MMC-3)
			buffer[1] = 0x80; // media is removable
			buffer[2] = 0x02; // copied from response from actual drive
			buffer[3] = 0x02; // response data format = SPC-3 standard
			buffer[4] = 0x1f; // copied ...
			buffer[7] = 0x98; // copied ...
			memcpy(&buffer[8], m_manufacturer, 8);
			memcpy(&buffer[16], m_product, 16);
			memcpy(&buffer[32], m_revision, 4);
			memcpy(data, buffer, dataLength);
			break;
		}

		default:
			scsicd_device::ReadData(data, dataLength);
			break;
	}
}

scsicd512_device::scsicd512_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	scsicd_device(mconfig, type, tag, owner, clock)
{
}

dec_rrd45_device::dec_rrd45_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	scsicd512_device(mconfig, RRD45, tag, owner, "DEC     ", "RRD45   (C) DEC ", "0436", 0x98)
{
}

toshiba_xm3301ta_device::toshiba_xm3301ta_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	scsicd512_device(mconfig, XM3301TA, tag, owner, "TOSHIBA ", "CD-ROM XM-3301TA", "0272", 0x88)
{
}

toshiba_xm5301_sun_device::toshiba_xm5301_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	scsicd512_device(mconfig, XM5301SUN, tag, owner, "TOSHIBA ", "XM-5301TASUN4XCD", "2915", 0x98)
{
}

toshiba_xm5401_sun_device::toshiba_xm5401_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	scsicd512_device(mconfig, XM5401SUN, tag, owner, "TOSHIBA ", "XM-5401TASUN4XCD", "1036", 0x98)
{
}

toshiba_xm5701_device::toshiba_xm5701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	scsicd512_device(mconfig, XM5701, tag, owner, "TOSHIBA ", "CD-ROM XM-5701TA", "3136", 0x98)
{
}

toshiba_xm5701_sun_device::toshiba_xm5701_sun_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	scsicd512_device(mconfig, XM5701SUN, tag, owner, "TOSHIBA ", "XM5701TASUN12XCD", "0997", 0x98)
{
}
