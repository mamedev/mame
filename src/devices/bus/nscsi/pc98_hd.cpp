// license:BSD-3-Clause
// copyright-holders:Angelo Salese

// Generic option for NEC-based hard disk drives

#include "emu.h"
#include "bus/nscsi/pc98_hd.h"

#define LOG_COMMAND     (1U << 1)
#define LOG_DATA        (1U << 2)
#define LOG_UNSUPPORTED (1U << 3)

#define VERBOSE 0

#include "logmacro.h"


DEFINE_DEVICE_TYPE(NSCSI_PC98_HD, nscsi_pc98_hd_device, "scsi_pc98_hd", "NEC PC-98 SCSI Hard Disk")

nscsi_pc98_hd_device::nscsi_pc98_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_PC98_HD, tag, owner, clock)
{
}

void nscsi_pc98_hd_device::scsi_command()
{
	if(m_scsi_cmdbuf[0] != SC_READ_6) {
		LOGMASKED(LOG_COMMAND, "%02x %02x %02x %02x %02x %02x\n",
			m_scsi_cmdbuf[0], m_scsi_cmdbuf[1], m_scsi_cmdbuf[2],
			m_scsi_cmdbuf[3], m_scsi_cmdbuf[4], m_scsi_cmdbuf[5]);
	}

	switch(m_scsi_cmdbuf[0]) {
	case SC_INQUIRY: {
		int lun = get_lun(m_scsi_cmdbuf[1] >> 5);
		LOG("command INQUIRY lun=%d EVPD=%d page=%d alloc=%02x link=%02x\n",
			lun, m_scsi_cmdbuf[1] & 1, m_scsi_cmdbuf[2], m_scsi_cmdbuf[4], m_scsi_cmdbuf[5]);

		int page = m_scsi_cmdbuf[2];
		int size = m_scsi_cmdbuf[4];
		switch(page) {
		case 0:
			std::fill_n(m_scsi_cmdbuf, 148, 0);

			// vendor and product information must be padded with spaces
			std::fill_n(&m_scsi_cmdbuf[8], 28, 0x20);

			if (lun != 0)
				m_scsi_cmdbuf[0] = 0x7f;
			else
				m_scsi_cmdbuf[0] = 0x00; // device is direct-access (e.g. hard disk)
			m_scsi_cmdbuf[1] = 0x00; // media is not removable
			m_scsi_cmdbuf[2] = 0x05; // device complies with SPC-3 standard
			m_scsi_cmdbuf[3] = 0x01; // response data format = CCS
			m_scsi_cmdbuf[4] = 52;   // additional length
			if(m_inquiry_data.empty()) {
				LOG("IDNT tag not found in chd metadata, using default inquiry data\n");

				// from np2kai, pc9801_55 wants to see "NEC" as part of their protection scheme
				strcpy((char *)&m_scsi_cmdbuf[8], "NECITSU ");
				// TODO: copied over base HD, to be filled properly
				strcpy((char *)&m_scsi_cmdbuf[16], "          ST225N");
				strcpy((char *)&m_scsi_cmdbuf[32], "1.00");
				m_scsi_cmdbuf[36] = 0x00; // # of extents high
				m_scsi_cmdbuf[37] = 0x08; // # of extents low
				m_scsi_cmdbuf[38] = 0x00; // group 0 commands 0-1f
				m_scsi_cmdbuf[39] = 0x99; // commands 0,3,4,7
				m_scsi_cmdbuf[40] = 0xa0; // commands 8, a
				m_scsi_cmdbuf[41] = 0x27; // commands 12,15,16,17
				m_scsi_cmdbuf[42] = 0x34; // commands 1a,1b,1d
				m_scsi_cmdbuf[43] = 0x01; // group 1 commands 20-3f
				m_scsi_cmdbuf[44] = 0x04;
				m_scsi_cmdbuf[45] = 0xa0;
				m_scsi_cmdbuf[46] = 0x01;
				m_scsi_cmdbuf[47] = 0x18;
				m_scsi_cmdbuf[48] = 0x07; // group 7 commands e0-ff
				m_scsi_cmdbuf[49] = 0x00;
				m_scsi_cmdbuf[50] = 0xa0; // commands 8, a
				m_scsi_cmdbuf[51] = 0x00;
				m_scsi_cmdbuf[52] = 0x00;
				m_scsi_cmdbuf[53] = 0xff; // end of list
			}
			else
				std::copy_n(m_inquiry_data.begin(), std::min(m_inquiry_data.size(), size_t(48)), &m_scsi_cmdbuf[8]);

			if(size > 56)
				size = 56;
			scsi_data_in(0, size);
			break;
		}
		scsi_status_complete(SS_GOOD);
		break;
	}


	default:
		// Parent may handle this
		nscsi_harddisk_device::scsi_command();
		break;
	}
}


attotime nscsi_pc98_hd_device::scsi_data_byte_period()
{
	// throttle a bit so that wd33c9x can catch up with DRQ with lha201 controller
	// playing safe, minimum threshold is ~307
	return attotime::from_nsec(500);
}

