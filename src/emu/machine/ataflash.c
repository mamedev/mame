#include "ataflash.h"

#define IDE_COMMAND_TAITO_GNET_UNLOCK_1     0xfe
#define IDE_COMMAND_TAITO_GNET_UNLOCK_2     0xfc
#define IDE_COMMAND_TAITO_GNET_UNLOCK_3     0x0f

const device_type ATA_FLASH_PCCARD = &device_creator<ata_flash_pccard_device>;

ata_flash_pccard_device::ata_flash_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	ide_hdd_device(mconfig, ATA_FLASH_PCCARD, "ATA Flash PCCARD", tag, owner, clock, "ataflash", __FILE__)
{
}

void ata_flash_pccard_device::device_start()
{
	ide_hdd_device::device_start();

	save_item(NAME(m_locked));
	save_item(NAME(m_gnetreadlock));
}

void ata_flash_pccard_device::device_reset()
{
	ide_hdd_device::device_reset();

	m_locked = 0x1ff;
	m_gnetreadlock = 1;

	UINT32 metalength;
	memset(m_cis, 0xff, 512);

	if (m_handle != NULL)
		m_handle->read_metadata(PCMCIA_CIS_METADATA_TAG, 0, m_cis, 512, metalength);
}

READ16_MEMBER( ata_flash_pccard_device::read_memory )
{
	if(offset <= 7)
	{
		return read_cs0(space, offset, mem_mask);
	}
	else if(offset <= 15)
	{
		return read_cs1(space, offset & 7, mem_mask);
	}
	else
	{
		return 0xffff;
	}
}

WRITE16_MEMBER( ata_flash_pccard_device::write_memory )
{
	if(offset <= 7)
	{
		write_cs0(space, offset, data, mem_mask);
	}
	else if( offset <= 15)
	{
		write_cs1(space, offset & 7, data, mem_mask);
	}
}

READ16_MEMBER( ata_flash_pccard_device::read_reg )
{
	if(offset < 0x100)
		return m_cis[offset];

	switch(offset)
	{
	case 0x100:
		return 0x0041;

	case 0x101:
		return 0x0080;

	case 0x102:
		return 0x002e;

	case 0x201:
		return m_gnetreadlock;

	default:
		return 0;
	}
}

WRITE16_MEMBER( ata_flash_pccard_device::write_reg )
{
	if(offset >= 0x280 && offset <= 0x288 && m_handle != NULL)
	{
		dynamic_buffer key(m_handle->hunk_bytes());
		m_handle->read_metadata(HARD_DISK_KEY_METADATA_TAG, 0, key);

		UINT8 v = data;
		int pos = offset - 0x280;
		UINT8 k = pos < key.count() ? key[pos] : 0;

		if(v == k)
		{
			m_locked &= ~(1 << pos);
		}
		else
		{
			m_locked |= 1 << pos;
		}

		if (!m_locked)
		{
			m_gnetreadlock = 0;
		}
	}
}

bool ata_flash_pccard_device::is_ready()
{
	return !m_gnetreadlock;
}

bool ata_flash_pccard_device::process_command()
{
	UINT8 key[5];

	switch (m_command)
	{
	case IDE_COMMAND_TAITO_GNET_UNLOCK_1:
		//LOGPRINT(("IDE GNET Unlock 1\n"));

		m_sector_count = 1;
		m_status |= IDE_STATUS_DRDY;

		set_irq(ASSERT_LINE);
		return true;

	case IDE_COMMAND_TAITO_GNET_UNLOCK_2:
		//LOGPRINT(("IDE GNET Unlock 2\n"));

		/* mark the buffer ready */
		m_status |= IDE_STATUS_DRQ;

		set_irq(ASSERT_LINE);
		return true;

	case IDE_COMMAND_TAITO_GNET_UNLOCK_3:
		//LOGPRINT(("IDE GNET Unlock 3\n"));

		/* key check */
		read_key(key);
		if (m_feature == key[0] && m_sector_count == key[1] && m_sector_number == key[2] && m_cylinder_low == key[3] && m_cylinder_high == key[4])
		{
			m_gnetreadlock = 0;
		}
		else
		{
			m_status &= ~IDE_STATUS_DRDY;
		}

		set_irq(ASSERT_LINE);
		return true;

	default:
		if (m_gnetreadlock)
		{
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
			m_status &= ~IDE_STATUS_DRDY;
			return true;
		}

		return ide_hdd_device::process_command();
	}
}

void ata_flash_pccard_device::process_buffer()
{
	if (m_command == IDE_COMMAND_TAITO_GNET_UNLOCK_2)
	{
		UINT8 key[5] = { 0 };
		int i, bad = 0;
		read_key(key);

		for (i=0; !bad && i<512; i++)
			bad = ((i < 2 || i >= 7) && m_buffer[i]) || ((i >= 2 && i < 7) && m_buffer[i] != key[i-2]);

		if (bad)
		{
			m_status |= IDE_STATUS_ERR;
			m_error = IDE_ERROR_NONE;
		}
		else
		{
			m_gnetreadlock= 0;
		}
	}
	else
	{
		ide_hdd_device::process_buffer();
	}
}
