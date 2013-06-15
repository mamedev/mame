#include "ataflash.h"

const device_type ATA_FLASH_PCCARD = &device_creator<ata_flash_pccard_device>;

ata_flash_pccard_device::ata_flash_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	ide_hdd_device(mconfig, ATA_FLASH_PCCARD, "ATA Flash PCCARD", tag, owner, clock, "ataflash", __FILE__)
{
}

void ata_flash_pccard_device::device_reset()
{
	m_locked = 0x1ff;
	m_gnetreadlock = 1;

	ide_hdd_device::device_reset();

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
		return m_locked ? 0x0001 : 0;

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
