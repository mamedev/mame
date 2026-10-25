// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn FileStore SCSI hard disk

    Rodime RO650 and RO3000 series drives as fitted by Acorn. The
    FileStore E01/E01S ROMs only accept a Winchester whose vendor
    maintenance pages 25h and 26h start with "(C)A" and "corn".

**********************************************************************/

#include "emu.h"
#include "e01_hd.h"

#include "multibyte.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(NSCSI_E01, nscsi_e01_device, "scsi_e01_hd", "Acorn FileStore SCSI Hard Disk")

nscsi_e01_device::nscsi_e01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_harddisk_device(mconfig, NSCSI_E01, tag, owner, clock)
{
	set_default_model_name(
			"RODIME  "
			"RO652           "
			"    ");
}

bool nscsi_e01_device::ro650_series() const
{
	// vendor identification, then product identification "RO65x" or "RO30xx"
	std::string_view const ident = m_inquiry_data.empty()
			? std::string_view(m_default_model_name)
			: std::string_view(reinterpret_cast<const char *>(m_inquiry_data.data()), m_inquiry_data.size());

	return (ident.size() >= 12) && (ident.substr(10, 2) == "65");
}

void nscsi_e01_device::scsi_command()
{
	static constexpr char COPYRIGHT[] = "(C)Acorn";

	int const page = m_scsi_cmdbuf[2] & 0x3f;

	if ((m_scsi_cmdbuf[0] != SC_MODE_SENSE_6) || ((page != 0x20) && (page != 0x25) && (page != 0x26)) || get_lun(m_scsi_cmdbuf[1] >> 5))
	{
		nscsi_harddisk_device::scsi_command();
		return;
	}

	int const size = m_scsi_cmdbuf[4];
	int pos = 1;
	m_scsi_cmdbuf[pos++] = 0x00; // medium type
	m_scsi_cmdbuf[pos++] = 0x00; // WP, cache

	const auto &info = image->get_info();
	m_scsi_cmdbuf[pos++] = 0x08; // block descriptor length
	m_scsi_cmdbuf[pos++] = 0x00;
	put_u24be(&m_scsi_cmdbuf[pos], info.cylinders * info.heads * info.sectors - 1);
	pos += 3;
	m_scsi_cmdbuf[pos++] = 0x00;
	put_u24be(&m_scsi_cmdbuf[pos], info.sectorbytes);
	pos += 3;

	m_scsi_cmdbuf[pos++] = page; // page id

	if (page == 0x20)
	{
		// serial number
		m_scsi_cmdbuf[pos++] = 0x08; // page length
		std::fill_n(&m_scsi_cmdbuf[pos], 8, ' ');
		pos += 8;
	}
	else
	{
		// maintenance pages A and B
		int const length = ro650_series() ? 4 : 6;
		m_scsi_cmdbuf[pos++] = length; // page length
		std::fill_n(&m_scsi_cmdbuf[pos], length, 0);
		std::copy_n(&COPYRIGHT[(page - 0x25) * 4], 4, &m_scsi_cmdbuf[pos]);
		pos += length;
	}

	m_scsi_cmdbuf[0] = pos - 1;
	scsi_data_in(0, std::min(pos, size));
	scsi_status_complete(SS_GOOD);
}
