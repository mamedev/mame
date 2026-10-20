// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "ataflash.h"

#define LOG_CIS (1U << 1)
//#define VERBOSE (LOG_CIS)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGCIS(...)    LOGMASKED(LOG_CIS, __VA_ARGS__)

ata_flash_pccard_device::ata_flash_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ata_flash_pccard_device(mconfig, ATA_FLASH_PCCARD, tag, owner, clock)
{
}

ata_flash_pccard_device::ata_flash_pccard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: cf_device_base(mconfig, type, tag, owner, clock)
	, device_pccard_interface(mconfig, *this)
{
}

void ata_flash_pccard_device::device_start()
{
	cf_device_base::device_start();

	m_cd1_cb(0);
	m_cd2_cb(0);
}

void ata_flash_pccard_device::device_reset()
{
	cf_device_base::device_reset();

	bool cis = m_image->exists() && !m_image->get_cis_data(m_cis);
	m_cis.resize(512, 0xff);

	if (!cis)
	{
		m_cis[0] = CISTPL_DEVICE;
		m_cis[1] = 0x04; // TPL_LINK
		m_cis[2] = 0xdf; // 4:device type/1:WPS/3:device speed
		m_cis[3] = 0x4a; // extended device speed (1:ext 4:mantissa 3:speed) (if speed==7)
		m_cis[4] = 0x01; // device size byte (5:# of address units -1 3: size code)
		m_cis[5] = 0xff; // end of device info field

		m_cis[6] = CISTPL_MANFID;
		m_cis[7] = 0x04; // TPL_LINK
		m_cis[8] = 0x00; // Manufacture ID
		m_cis[9] = 0x00;
		m_cis[10] = 0x00; // Card ID
		m_cis[11] = 0x00;

		m_cis[12] = CISTPL_FUNCID;
		m_cis[13] = 0x02; // TPL_LINK
		m_cis[14] = 0x04; // TPLFID_FUNCTION
		m_cis[15] = 0x01; // TPLFID_SYSINIT

		m_cis[16] = CISTPL_CONFIG;
		m_cis[17] = 0x05; // TPL_LINK
		m_cis[18] = 0x01; // Size of Fields Byte (2:TPCC_RFSZ 4:TPCC_RMSZ 2:TPCC_RASZ)
		m_cis[19] = 0x03; // TPCC_LAST (2:RFU 6:Last Index) (count of 0x1b?)
		m_cis[20] = 0x00; // TPCC_RADR
		m_cis[21] = 0x02; // TPCC_RADR
		m_cis[22] = 0x0f; // TPCC_RMSK

		m_cis[23] = CISTPL_VERS_1;
		m_cis[24] = 0x14; // TPL_LINK
		m_cis[25] = 0x05; // major version
		m_cis[26] = 0x01; // minor version
		m_cis[27] = 'M';
		m_cis[28] = 'A';
		m_cis[29] = 'M';
		m_cis[30] = 'E';
		m_cis[31] = '\0';
		m_cis[32] = 'A';
		m_cis[33] = 'T';
		m_cis[34] = ' ';
		m_cis[35] = 'F';
		m_cis[36] = 'L';
		m_cis[37] = 'A';
		m_cis[38] = 'S';
		m_cis[39] = 'H';
		m_cis[40] = '\0';
		m_cis[41] = '1';
		m_cis[42] = '.';
		m_cis[43] = '0';
		m_cis[44] = '\0';

		m_cis[45] = CISTPL_END;
		m_cis[46] = 0xff; // TPL_LINK
	}

	if (VERBOSE & LOG_CIS)
	{
		size_t i = 0;
		while (i < m_cis.size() && m_cis[i] != 0x01)
		{
			LOGCIS("CIS header unknown %02x\n", m_cis[i++]);
		}

		while (i < m_cis.size())
		{
			std::string cis;
			if (m_cis[i] == CISTPL_NULL) { LOGCIS("CISTPL_NULL\n"); continue; }
			else if (m_cis[i] == CISTPL_DEVICE) cis = "CISTPL_DEVICE";
			else if (m_cis[i] == CISTPL_LONGLINK_CB) cis = "CISTPL_LONGLINK_CB";
			else if (m_cis[i] == CISTPL_CONFIG_CB) cis = "CISTPL_CONFIG_CB";
			else if (m_cis[i] == CISTPL_CFTABLE_ENTRY_CB) cis = "CISTPL_CFTABLE_ENTRY_CB";
			else if (m_cis[i] == CISTPL_LONGLINK_MFC) cis = "CISTPL_LONGLINK_MFC";
			else if (m_cis[i] == CISTPL_BAR) cis = "CISTPL_BAR";
			else if (m_cis[i] == CISTPL_CHECKSUM) cis = "CISTPL_CHECKSUM";
			else if (m_cis[i] == CISTPL_LONGLINK_A) cis = "CISTPL_LONGLINK_A";
			else if (m_cis[i] == CISTPL_LONGLINK_C) cis = "CISTPL_LONGLINK_C";
			else if (m_cis[i] == CISTPL_LINKTARGET) cis = "CISTPL_LINKTARGET";
			else if (m_cis[i] == CISTPL_NO_LINK) cis = "CISTPL_NO_LINK";
			else if (m_cis[i] == CISTPL_VERS_1) cis = "CISTPL_VERS_1";
			else if (m_cis[i] == CISTPL_ALTSTR) cis = "CISTPL_ALTSTR";
			else if (m_cis[i] == CISTPL_DEVICE_A) cis = "CISTPL_DEVICE_A";
			else if (m_cis[i] == CISTPL_JEDEC_C) cis = "CISTPL_JEDEC_C";
			else if (m_cis[i] == CISTPL_JEDEC_A) cis = "CISTPL_JEDEC_A";
			else if (m_cis[i] == CISTPL_CONFIG) cis = "CISTPL_CONFIG";
			else if (m_cis[i] == CISTPL_CFTABLE_ENTRY) cis = "CISTPL_CFTABLE_ENTRY";
			else if (m_cis[i] == CISTPL_DEVICE_OC) cis = "CISTPL_DEVICE_OC";
			else if (m_cis[i] == CISTPL_DEVICE_OA) cis = "CISTPL_DEVICE_OA";
			else if (m_cis[i] == CISTPL_DEVICE_GEO) cis = "CISTPL_DEVICE_GEO";
			else if (m_cis[i] == CISTPL_DEVICE_GEO_A) cis = "CISTPL_DEVICE_GEO_A";
			else if (m_cis[i] == CISTPL_MANFID) cis = "CISTPL_MANFID";
			else if (m_cis[i] == CISTPL_FUNCID) cis = "CISTPL_FUNCID";
			else if (m_cis[i] == CISTPL_FUNCE) cis = "CISTPL_FUNCE";
			else if (m_cis[i] == CISTPL_SWIL) cis = "CISTPL_SWIL";
			else if (m_cis[i] == CISTPL_END) { LOGCIS("CISTPL_END\n"); break; }
			else cis = util::string_format("UNKNOWN CISTPL %02x", m_cis[i]);
			i++;

			if (i < m_cis.size())
			{
				uint8_t len = m_cis[i++];
				cis += util::string_format(" (len=%d):", len);

				for (size_t j = 0; j < len; j++)
				{
					if (i < m_cis.size())
						cis += util::string_format("  %02x", m_cis[i++]);
					else
					{
						cis += util::string_format(" (truncated data)");
						break;
					}
				}
			}
			else
			{
				cis += (" (truncated len)\n");
				break;
			}

			LOGCIS("%s\n", cis);
		}
	}

	m_configuration_option = 0;
	m_configuration_and_status = 0;
	m_pin_replacement = 0x002e;
}

uint16_t ata_flash_pccard_device::read_memory(offs_t offset, uint16_t mem_mask)
{
	PAIR16 data; data.w = 0xffff;
	if (offset < 8)
		command_r(offset, data, mem_mask);
	else if (offset < 16)
		control_r(offset & 7, data, mem_mask);
	return data.w;
}

void ata_flash_pccard_device::write_memory(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(offset < 8)
		command_w(offset, data, mem_mask);
	else if( offset < 16)
		control_w(offset & 7, data, mem_mask);
}

uint16_t ata_flash_pccard_device::read_reg(offs_t offset, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0x100:
		return m_configuration_option;

	case 0x101:
		return m_configuration_and_status;

	case 0x102:
		return m_pin_replacement;

	default:
		if (offset < 0x100)
			return m_cis[offset];
	}

	return device_pccard_interface::read_reg(offset, mem_mask);
}

void ata_flash_pccard_device::write_reg(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/// TODO: get offsets from CIS
	switch (offset)
	{
	case 0x07:
		// TODO: figure out what this is
		/// taito type 1: 0x0e, 0x0a (after unlock)
		/// taito type 2: 0xdf, 0xdf (before unlock)
		/// taito compact 0x0c, 0x0c, 0x0c, 0x0c (before unlock)
		// logerror("unknown reg 0x07 %02x\n", data);
		break;

	case 0x100:
		m_configuration_option = data;
		break;

	case 0x101:
		// TODO: irq ack
		m_configuration_and_status = data;
		break;

	default:
		device_pccard_interface::write_reg(offset, data, mem_mask);
		break;
	}
}

void ata_flash_pccard_device::ide_build_identify_device()
{
	cf_device_base::ide_build_identify_device();
	m_identify_buffer[0] = 0x848a;                     /*  0: configuration bits */
}


taito_pccard1_device::taito_pccard1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ata_flash_pccard_device(mconfig, TAITO_PCCARD1, tag, owner, clock),
	m_locked(0)
{
}

void taito_pccard1_device::device_start()
{
	ata_flash_pccard_device::device_start();

	save_item(NAME(m_locked));
}

void taito_pccard1_device::device_reset()
{
	ata_flash_pccard_device::device_reset();

	if (m_image->exists() && !m_image->get_disk_key_data(m_key) && m_key.size() == 5)
	{
		m_locked = 0x1ff;
	}
	m_key.resize(5, 0);
}

uint16_t taito_pccard1_device::read_reg(offs_t offset, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0x201:
		return m_locked != 0;

	default:
		return ata_flash_pccard_device::read_reg(offset, mem_mask);
	}
}

void taito_pccard1_device::write_reg(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset >= 0x280 && offset <= 0x288)
	{
		uint8_t v = data;
		int pos = offset - 0x280;
		uint8_t k = pos < m_key.size() ? m_key[pos] : 0;

		// TODO: find out if unlocking the key then using an incorrect key will re-lock the card.
		if (v == k)
		{
			m_locked &= ~(1 << pos);
		}
		else
		{
			m_locked |= 1 << pos;
		}

		// logerror("unlock %d %02x %04x\n", pos, data, m_locked);
	}
	else
	{
		ata_flash_pccard_device::write_reg(offset, data, mem_mask);
	}
}

void taito_pccard1_device::process_command()
{
	m_buffer_size = IDE_DISK_SECTOR_SIZE;

	switch (m_command)
	{
	default:
		if (m_locked != 0)
		{
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
			m_status &= ~IDE_STATUS_DRDY;
			return;
		}
		break;
	}

	ata_flash_pccard_device::process_command();
}

bool taito_pccard1_device::is_ready()
{
	return m_locked == 0;
}

taito_pccard2_device::taito_pccard2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ata_flash_pccard_device(mconfig, TAITO_PCCARD2, tag, owner, clock),
	m_locked(false)
{
}

void taito_pccard2_device::device_start()
{
	ata_flash_pccard_device::device_start();

	save_item(NAME(m_locked));
}

void taito_pccard2_device::device_reset()
{
	ata_flash_pccard_device::device_reset();

	if (m_image->exists() && !m_image->get_disk_key_data(m_key) && m_key.size() == 5)
	{
		m_locked = true;
	}
	m_key.resize(5, 0);
}

void taito_pccard2_device::process_command()
{
	m_buffer_size = IDE_DISK_SECTOR_SIZE;

	switch (m_command)
	{
	case IDE_COMMAND_TAITO_GNET_UNLOCK_1:
		//LOGPRINT(("IDE GNET Unlock 1\n"));

		m_sector_count = 1;
		m_status |= IDE_STATUS_DRDY;

		set_irq(ASSERT_LINE);
		return;

	case IDE_COMMAND_TAITO_GNET_UNLOCK_2:
		//LOGPRINT(("IDE GNET Unlock 2\n"));

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;

		set_irq(ASSERT_LINE);
		return;

	default:
		if (m_locked)
		{
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
			m_status &= ~IDE_STATUS_DRDY;
			return;
		}
		break;
	}

	ata_flash_pccard_device::process_command();
}

void taito_pccard2_device::process_buffer()
{
	if (m_command == IDE_COMMAND_TAITO_GNET_UNLOCK_2)
	{
		int i, bad = 0;

		for (i = 0; !bad && i<512; i++)
			bad = ((i < 2 || i >= 7) && m_buffer[i]) || ((i >= 2 && i < 7) && m_buffer[i] != m_key[i - 2]);

		if (bad)
		{
			// TODO: find out if unlocking the key then using an incorrect key will re-lock the card.
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
		}
		else
		{
			m_locked = false;
		}

		// logerror("unlock %02x %02x %02x %02x %02x %d\n", m_buffer[2], m_buffer[3], m_buffer[4], m_buffer[5], m_buffer[6], m_locked);
	}
	else
	{
		ata_flash_pccard_device::process_buffer();
	}
}

bool taito_pccard2_device::is_ready()
{
	return !m_locked;
}


taito_compact_flash_device::taito_compact_flash_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	ata_flash_pccard_device(mconfig, TAITO_COMPACT_FLASH, tag, owner, clock),
	m_locked(false)
{
}

void taito_compact_flash_device::device_start()
{
	ata_flash_pccard_device::device_start();

	save_item(NAME(m_locked));
}

void taito_compact_flash_device::device_reset()
{
	ata_flash_pccard_device::device_reset();

	if (m_image->exists() && !m_image->get_disk_key_data(m_key) && m_key.size() == 5)
	{
		m_locked = true;
	}
	m_key.resize(5, 0);
}

void taito_compact_flash_device::process_command()
{
	m_buffer_size = IDE_DISK_SECTOR_SIZE;

	switch (m_command)
	{
	case IDE_COMMAND_TAITO_COMPACT_FLASH_UNLOCK:
		/* key check */
		if (m_feature != m_key[0] || m_sector_count != m_key[1] || m_sector_number != m_key[2] || m_cylinder_low != m_key[3] || m_cylinder_high != m_key[4])
		{
			// TODO: find out if unlocking the key then using an incorrect key will lock the re-card.
			m_status &= ~IDE_STATUS_DRDY; // TODO: check if this is used as a flag to the unlock code, would it already be set this early?
		}
		else
		{
			m_locked = false;
		}

		// logerror("unlock %02x %02x %02x %02x %02x %d\n", m_feature, m_sector_count, m_sector_number, m_cylinder_low, m_cylinder_high, m_locked);

		set_irq(ASSERT_LINE);
		return;

	default:
		if (m_locked)
		{
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
			m_status &= ~IDE_STATUS_DRDY;
			return;
		}
		break;
	}

	ata_flash_pccard_device::process_command();
}

bool taito_compact_flash_device::is_ready()
{
	return !m_locked;
}

DEFINE_DEVICE_TYPE(ATA_FLASH_PCCARD, ata_flash_pccard_device, "ataflash", "ATA Flash PC Card")
DEFINE_DEVICE_TYPE(TAITO_PCCARD1, taito_pccard1_device, "taito_pccard1", "Taito PC Card (Type 1)")
DEFINE_DEVICE_TYPE(TAITO_PCCARD2, taito_pccard2_device, "taito_pccard2", "Taito PC Card (Type 2)")
DEFINE_DEVICE_TYPE(TAITO_COMPACT_FLASH, taito_compact_flash_device, "taito_cf", "Taito Compact Flash")
