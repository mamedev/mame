// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    SiS 950 SMBus implementation

    TODO:
    - Stub, needs an smbus_interface, bare minimum to make shutms11 happy;

**************************************************************************************************/
#include "emu.h"
#include "sis950_smbus.h"

#define LOG_CMD     (1U << 1) // log commands

#define VERBOSE (LOG_GENERAL | LOG_CMD)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGCMD(...)     LOGMASKED(LOG_CMD,   __VA_ARGS__)

DEFINE_DEVICE_TYPE(SIS950_SMBUS, sis950_smbus_device, "sis950_smbus", "SiS950 SMBus interface")

void sis950_smbus_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(sis950_smbus_device::smb_sts_r), FUNC(sis950_smbus_device::smb_sts_w));

	map(0x03, 0x03).rw(FUNC(sis950_smbus_device::smbhost_cnt_r), FUNC(sis950_smbus_device::smbhost_cnt_w));

#if 0
	map(0x02, 0x02).rw(FUNC(sis950_smbus_device::hst_cnt_r), FUNC(sis950_smbus_device::hst_cnt_w));
	map(0x03, 0x03).rw(FUNC(sis950_smbus_device::hst_cmd_r), FUNC(sis950_smbus_device::hst_cmd_w));
	map(0x04, 0x04).rw(FUNC(sis950_smbus_device::xmit_slva_r), FUNC(sis950_smbus_device::xmit_slva_w));
	map(0x05, 0x05).rw(FUNC(sis950_smbus_device::hst_d0_r), FUNC(sis950_smbus_device::hst_d0_w));
	map(0x06, 0x06).rw(FUNC(sis950_smbus_device::hst_d1_r), FUNC(sis950_smbus_device::hst_d1_w));
	map(0x07, 0x07).rw(FUNC(sis950_smbus_device::host_block_db_r), FUNC(sis950_smbus_device::host_block_db_w));
	map(0x08, 0x08).rw(FUNC(sis950_smbus_device::pec_r), FUNC(sis950_smbus_device::pec_w));
	map(0x09, 0x09).rw(FUNC(sis950_smbus_device::rcv_slva_r), FUNC(sis950_smbus_device::rcv_slva_w));
	map(0x0a, 0x0b).rw(FUNC(sis950_smbus_device::slv_data_r), FUNC(sis950_smbus_device::slv_data_w));
	map(0x0c, 0x0c).rw(FUNC(sis950_smbus_device::aux_sts_r), FUNC(sis950_smbus_device::aux_sts_w));
	map(0x0d, 0x0d).rw(FUNC(sis950_smbus_device::aux_ctl_r), FUNC(sis950_smbus_device::aux_ctl_w));
	map(0x0e, 0x0e).rw(FUNC(sis950_smbus_device::smlink_pin_ctl_r), FUNC(sis950_smbus_device::smlink_pin_ctl_w));
	map(0x0f, 0x0f).rw(FUNC(sis950_smbus_device::smbus_pin_ctl_r), FUNC(sis950_smbus_device::smbus_pin_ctl_w));
	map(0x10, 0x10).rw(FUNC(sis950_smbus_device::slv_sts_r), FUNC(sis950_smbus_device::slv_sts_w));
	map(0x11, 0x11).rw(FUNC(sis950_smbus_device::slv_cmd_r), FUNC(sis950_smbus_device::slv_cmd_w));
	map(0x14, 0x14).r(FUNC(sis950_smbus_device::notify_daddr_r));
	map(0x16, 0x16).r(FUNC(sis950_smbus_device::notify_dlow_r));
	map(0x17, 0x17).r(FUNC(sis950_smbus_device::notify_dhigh_r));
#endif
}

sis950_smbus_device::sis950_smbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SIS950_SMBUS, tag, owner, clock)
{
}

void sis950_smbus_device::device_start()
{

}

void sis950_smbus_device::device_reset()
{

}

u8 sis950_smbus_device::smb_sts_r()
{
	LOGCMD("%s: smb_sts_r (%02x)\n", tag(), m_status);

	return m_status;
}

void sis950_smbus_device::smb_sts_w(u8 data)
{
	m_status &= ~data;
	LOGCMD("%s: smb_sts_w = %02x\n", tag(), data);
}

// reads back bits 0-2 only
u8 sis950_smbus_device::smbhost_cnt_r()
{
	return m_cmd;
}

/*
 * --x- ---- kill (reset state, TODO)
 * ---x ---- start transaction
 * ---- -xxx command protocol
 * ---- -000 quick command
 * ---- -001 send/receive byte
 * ---- -010 read/write byte data
 * ---- -011 read/write word data
 * ---- -100 process call
 * ---- -101 read/write block data
 * ---- -11x <reserved>
 */
void sis950_smbus_device::smbhost_cnt_w(u8 data)
{
	m_cmd = data & 7;
	LOGCMD("%s: smbhost_cnt_w = %02x\n", tag(), data);

	// TODO: process commands properly
	// For now we just reflect shutms11 wanting bit 3 on after each transaction
	if (BIT(data, 4))
		m_status |= 8;
}
