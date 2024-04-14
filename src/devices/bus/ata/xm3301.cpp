// license:BSD-3-Clause
// copyright-holders: Angelo Salese, Grull Osgo
/**************************************************************************************************

Toshiba TAISATAP.SYS support / CD_BALLY.SYS (gammagic)

TODO:
- XM-3301 on its own is a SCSI-2 drive, the ATAPI variants must be higher number(s)?

**************************************************************************************************/

#include "emu.h"
#include "xm3301.h"

DEFINE_DEVICE_TYPE(XM3301, toshiba_xm3301_device, "xm3301", "Toshiba XM-3301 CD-ROM Drive")

toshiba_xm3301_device::toshiba_xm3301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atapi_cdrom_device(mconfig, XM3301, tag, owner, clock)
{
}

void toshiba_xm3301_device::device_start()
{
	atapi_cdrom_device::device_start();

	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));

	// From a XM-5401 SCSI dump string $7f08
	// [5401] will be printed as Master Model
	// [3605] as Rev number
	// Both TAISATAP.SYS and CD_BALLY.SYS tests against one of these strings:
	// "TOSHIBA CD-ROM"
	// "TOSHIBA CD-ROM XM-3301"
	// "TOSHIBA CD-ROM DRIVE:XM"
	// "TOSHIBA DVD" (CD_BALLY.SYS only)
	t10mmc::set_model("TOSHIBA CD-ROM XM-5401TA3605");

	// xx20 is the only confirmed part (wants IRQ for command $a0)
	m_identify_buffer[0] = 0x8520;

	// TODO: everything below here is unconfirmed
	m_identify_buffer[23] = ('1' << 8) | '.';
	m_identify_buffer[24] = ('0' << 8) | ' ';
	m_identify_buffer[25] = (' ' << 8) | ' ';
	m_identify_buffer[26] = (' ' << 8) | ' ';

	m_identify_buffer[27] = ('T' << 8) | 'O';
	m_identify_buffer[28] = ('S' << 8) | 'H';
	m_identify_buffer[29] = ('I' << 8) | 'B';
	m_identify_buffer[30] = ('A' << 8) | ' ';
	m_identify_buffer[31] = ('X' << 8) | 'M';
	m_identify_buffer[32] = ('-' << 8) | '3';
	m_identify_buffer[33] = ('3' << 8) | '0';
	m_identify_buffer[34] = ('1' << 8) | ' ';
	m_identify_buffer[35] = (' ' << 8) | ' ';
	m_identify_buffer[36] = (' ' << 8) | ' ';
	m_identify_buffer[37] = (' ' << 8) | ' ';
	m_identify_buffer[38] = (' ' << 8) | ' ';
	m_identify_buffer[39] = (' ' << 8) | ' ';
	m_identify_buffer[40] = (' ' << 8) | ' ';
	m_identify_buffer[41] = (' ' << 8) | ' ';
	m_identify_buffer[42] = (' ' << 8) | ' ';
	m_identify_buffer[43] = (' ' << 8) | ' ';
	m_identify_buffer[44] = (' ' << 8) | ' ';
	m_identify_buffer[45] = (' ' << 8) | ' ';
	m_identify_buffer[46] = (' ' << 8) | ' ';

	m_identify_buffer[49] = 0x0400; // IORDY may be disabled
}

void toshiba_xm3301_device::device_reset()
{
	atapi_cdrom_device::device_reset();

}

void toshiba_xm3301_device::identify_packet_device()
{
	// gammagic CD_BALLY.SYS doesn't care about $a1 contents but wants these two to be high
	// when command is issued.
	m_status |= IDE_STATUS_DSC | IDE_STATUS_DRDY;
}
