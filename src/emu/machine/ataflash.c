#include "ataflash.h"

const device_type ATA_FLASH_PCCARD = &device_creator<ata_flash_pccard_device>;

ata_flash_pccard_device::ata_flash_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ATA_FLASH_PCCARD, "ATA Flash PCCARD", tag, owner, clock, "ataflash", __FILE__),
	device_slot_card_interface(mconfig, *this),
	m_card(*this,"card")
{
}

static MACHINE_CONFIG_FRAGMENT( ata_flash_pccard_device )
	MCFG_IDE_CONTROLLER_ADD( "card", ide_devices, "hdd", NULL, true)
MACHINE_CONFIG_END

machine_config_constructor ata_flash_pccard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ata_flash_pccard_device );
}

void ata_flash_pccard_device::device_start()
{
	UINT32 metalength;
	memset(m_cis, 0xff, 512);

	astring drive_tag;
	subtag(drive_tag, "drive_0");

	m_chd_file = get_disk_handle(machine(), drive_tag);
	if(m_chd_file != NULL)
	{
		m_chd_file->read_metadata(PCMCIA_CIS_METADATA_TAG, 0, m_cis, 512, metalength);
	}
}

void ata_flash_pccard_device::device_reset_after_children()
{
	m_locked = 0x1ff;
	m_card->ide_set_gnet_readlock(1);
}

READ16_MEMBER( ata_flash_pccard_device::read_memory )
{
	if(offset <= 7)
	{
		return m_card->read_cs0(space, offset, mem_mask);
	}
	else if(offset <= 15)
	{
		return m_card->read_cs1(space, offset & 7, mem_mask);
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
		m_card->write_cs0(space, offset, data, mem_mask);
	}
	else if( offset <= 15)
	{
		m_card->write_cs1(space, offset & 7, data, mem_mask);
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
	if(offset >= 0x280 && offset <= 0x288 && m_chd_file != NULL)
	{
		dynamic_buffer key(m_chd_file->hunk_bytes());
		m_chd_file->read_metadata(HARD_DISK_KEY_METADATA_TAG, 0, key);

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
			m_card->ide_set_gnet_readlock(0);
		}
	}
}
