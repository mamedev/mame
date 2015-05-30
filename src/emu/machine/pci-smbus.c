// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "pci-smbus.h"

const device_type SMBUS = &device_creator<smbus_device>;

DEVICE_ADDRESS_MAP_START(map, 32, smbus_device)
	AM_RANGE(0x00, 0x03) AM_READWRITE8 (hst_sts_r,        hst_sts_w,        0x000000ff)
	AM_RANGE(0x00, 0x03) AM_READWRITE8 (hst_cnt_r,        hst_cnt_w,        0x00ff0000)
	AM_RANGE(0x00, 0x03) AM_READWRITE8 (hst_cmd_r,        hst_cmd_w,        0xff000000)
	AM_RANGE(0x04, 0x07) AM_READWRITE8 (xmit_slva_r,      xmit_slva_w,      0x000000ff)
	AM_RANGE(0x04, 0x07) AM_READWRITE8 (hst_d0_r,         hst_d0_w,         0x0000ff00)
	AM_RANGE(0x04, 0x07) AM_READWRITE8 (hst_d1_r,         hst_d1_w,         0x00ff0000)
	AM_RANGE(0x04, 0x07) AM_READWRITE8 (host_block_db_r,  host_block_db_w,  0xff000000)
	AM_RANGE(0x08, 0x0b) AM_READWRITE8 (pec_r,            pec_w,            0x000000ff)
	AM_RANGE(0x08, 0x0b) AM_READWRITE8 (rcv_slva_r,       rcv_slva_w,       0x0000ff00)
	AM_RANGE(0x08, 0x0b) AM_READWRITE16(slv_data_r,       slv_data_w,       0xffff0000)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE8 (aux_sts_r,        aux_sts_w,        0x000000ff)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE8 (aux_ctl_r,        aux_ctl_w,        0x0000ff00)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE8 (smlink_pin_ctl_r, smlink_pin_ctl_w, 0x00ff0000)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE8 (smbus_pin_ctl_r,  smbus_pin_ctl_w,  0xff000000)
	AM_RANGE(0x10, 0x13) AM_READWRITE8 (slv_sts_r,        slv_sts_w,        0x000000ff)
	AM_RANGE(0x10, 0x13) AM_READWRITE8 (slv_cmd_r,        slv_cmd_w,        0x0000ff00)
	AM_RANGE(0x14, 0x17) AM_READ8      (notify_daddr_r,                     0x000000ff)
	AM_RANGE(0x14, 0x17) AM_READ8      (notify_dlow_r,                      0x00ff0000)
	AM_RANGE(0x14, 0x17) AM_READ8      (notify_dhigh_r,                     0xff000000)
ADDRESS_MAP_END

smbus_device::smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, SMBUS, "SMBUS interface", tag, owner, clock, "smbus", __FILE__)
{
}

void smbus_device::device_start()
{
	pci_device::device_start();
	skip_map_regs(4);
	add_map(32, M_IO, FUNC(smbus_device::map));
}

void smbus_device::device_reset()
{
	pci_device::device_reset();
	hst_sts = 0x00;
	hst_cnt = 0x00;
	hst_cmd = 0x00;
	xmit_slva = 0x00;
	hst_d0 = 0x00;
	hst_d1 = 0x00;
	host_block_db = 0x00;
	pec = 0x00;
	rcv_slva = 0x44;
	slv_data = 0x0000;
	aux_sts = 0x00;
	aux_ctl = 0x00;
	smlink_pin_ctl = 0x00;
	smbus_pin_ctl = 0x00;
	slv_sts = 0x00;
	slv_cmd = 0x00;
	notify_daddr = 0x00;
	notify_dlow = 0x00;
	notify_dhigh = 0x00;
}

READ8_MEMBER  (smbus_device::hst_sts_r)
{
	return hst_sts;
}

WRITE8_MEMBER (smbus_device::hst_sts_w)
{
	hst_sts &= ~data;
	logerror("%s: hst_sts = %02x\n", tag(), hst_sts);
}

READ8_MEMBER  (smbus_device::hst_cnt_r)
{
	return hst_cnt;
}

WRITE8_MEMBER (smbus_device::hst_cnt_w)
{
	hst_cnt = data;
	logerror("%s: hst_cnt = %02x\n", tag(), hst_cnt);

	if(xmit_slva != 0xa1)
		hst_sts = 4;
	else {
		const UINT8 eeprom[256] = {
			0x80, 0x08, 0x07, 0x0D, 0x0A, 0x02, 0x40, 0x00, 0x04, 0x50, 0x60, 0x00, 0x82, 0x08, 0x00, 0x01,
			0x0E, 0x04, 0x08, 0x01, 0x02, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x28, 0x3C, 0x28, 0x40,
			0x60, 0x60, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x46, 0x28, 0x28, 0x55, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xCE,
			0x7F, 0x7F, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x56, 0x53, 0x35, 0x31, 0x32, 0x4D, 0x42,
			0x34, 0x30, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};

		hst_sts = 2;
		hst_d0 = eeprom[hst_cmd];
	}
}

READ8_MEMBER  (smbus_device::hst_cmd_r)
{
	return hst_cmd;
}

WRITE8_MEMBER (smbus_device::hst_cmd_w)
{
	hst_cmd = data;
	logerror("%s: hst_cmd = %02x\n", tag(), hst_cmd);
}

READ8_MEMBER  (smbus_device::xmit_slva_r)
{
	return xmit_slva;
}

WRITE8_MEMBER (smbus_device::xmit_slva_w)
{
	xmit_slva = data;
	logerror("%s: xmit_slva = %02x\n", tag(), xmit_slva);
}

READ8_MEMBER  (smbus_device::hst_d0_r)
{
	return hst_d0;
}

WRITE8_MEMBER (smbus_device::hst_d0_w)
{
	hst_d0 = data;
	logerror("%s: hst_d0 = %02x\n", tag(), hst_d0);
}

READ8_MEMBER  (smbus_device::hst_d1_r)
{
	return hst_d1;
}

WRITE8_MEMBER (smbus_device::hst_d1_w)
{
	hst_d1 = data;
	logerror("%s: hst_d1 = %02x\n", tag(), hst_d1);
}

READ8_MEMBER  (smbus_device::host_block_db_r)
{
	return host_block_db;
}

WRITE8_MEMBER (smbus_device::host_block_db_w)
{
	host_block_db = data;
	logerror("%s: host_block_db = %02x\n", tag(), host_block_db);
}

READ8_MEMBER  (smbus_device::pec_r)
{
	return pec;
}

WRITE8_MEMBER (smbus_device::pec_w)
{
	pec = data;
	logerror("%s: pec = %02x\n", tag(), pec);
}

READ8_MEMBER  (smbus_device::rcv_slva_r)
{
	return rcv_slva;
}

WRITE8_MEMBER (smbus_device::rcv_slva_w)
{
	rcv_slva = data;
	logerror("%s: rcv_slva = %02x\n", tag(), rcv_slva);
}

READ16_MEMBER (smbus_device::slv_data_r)
{
	return slv_data;
}

WRITE16_MEMBER(smbus_device::slv_data_w)
{
	slv_data = data;
	logerror("%s: slv_data = %02x\n", tag(), slv_data);
}

READ8_MEMBER  (smbus_device::aux_sts_r)
{
	return aux_sts;
}

WRITE8_MEMBER (smbus_device::aux_sts_w)
{
	aux_sts = data;
	logerror("%s: aux_sts = %02x\n", tag(), aux_sts);
}

READ8_MEMBER  (smbus_device::aux_ctl_r)
{
	return aux_ctl;
}

WRITE8_MEMBER (smbus_device::aux_ctl_w)
{
	aux_ctl = data;
	logerror("%s: aux_ctl = %02x\n", tag(), aux_ctl);
}

READ8_MEMBER  (smbus_device::smlink_pin_ctl_r)
{
	return smlink_pin_ctl;
}

WRITE8_MEMBER (smbus_device::smlink_pin_ctl_w)
{
	smlink_pin_ctl = data;
	logerror("%s: smlink_pin_ctl = %02x\n", tag(), smlink_pin_ctl);
}

READ8_MEMBER  (smbus_device::smbus_pin_ctl_r)
{
	return smbus_pin_ctl;
}

WRITE8_MEMBER (smbus_device::smbus_pin_ctl_w)
{
	smbus_pin_ctl = data;
	logerror("%s: smbus_pin_ctl = %02x\n", tag(), smbus_pin_ctl);
}

READ8_MEMBER  (smbus_device::slv_sts_r)
{
	return slv_sts;
}

WRITE8_MEMBER (smbus_device::slv_sts_w)
{
	slv_sts = data;
	logerror("%s: slv_sts = %02x\n", tag(), slv_sts);
}

READ8_MEMBER  (smbus_device::slv_cmd_r)
{
	return slv_cmd;
}

WRITE8_MEMBER (smbus_device::slv_cmd_w)
{
	slv_cmd = data;
	logerror("%s: slv_cmd = %02x\n", tag(), slv_cmd);
}

READ8_MEMBER  (smbus_device::notify_daddr_r)
{
	return notify_daddr;
}

READ8_MEMBER  (smbus_device::notify_dlow_r)
{
	return notify_dlow;
}

READ8_MEMBER  (smbus_device::notify_dhigh_r)
{
	return notify_dhigh;
}
