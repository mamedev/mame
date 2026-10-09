// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

KMK/JŻ IDE ATA interface a.k.a. IDEa

TODO:
- Error 144 reading sector 1 in Sparta DOS at "D2:MKSDFS D1:" step. PC=D8A9 (PC=D8A8 for nc BIOS)
  reading in $d117 DRQ;
- connected slave drive fails KMKDIAG with a "still busy";
- Sparta DOS X formatting is untested;

**************************************************************************************************/

#include "emu.h"
#include "kmkide.h"

DEFINE_DEVICE_TYPE(KMKIDE_ATA, kmkide_ata_device, "kmkide_ata", "KMK/JŻ IDE ATA interface")

kmkide_ata_device::kmkide_ata_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: atari_pbi_card_device(mconfig, KMKIDE_ATA, tag, owner, clock)
	, m_ata(*this, "ata")
	, m_bios(*this, "bios")
	, m_rambank(*this, "rambank")
	, m_jp(*this, "JP")
{
}

ROM_START( kmkide_ata )
	ROM_REGION(0x600, "bios", 0)
	ROM_SYSTEM_BIOS(0, "std", "Caviar disks v1.11")
	ROMX_LOAD( "hdb111.rom",   0x000, 0x600, CRC(b86e0c6e) SHA1(148c5977698532dd2a5f31ef5c1ab9e6221909e5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "nc", "Non-Caviar disks v1.11")
	ROMX_LOAD( "hdb111nc.rom", 0x000, 0x600, CRC(ddf18682) SHA1(b5f1145f15e8986cff76a2a5d824d7940c53c19e), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *kmkide_ata_device::device_rom_region() const
{
	return ROM_NAME( kmkide_ata );
}

void kmkide_ata_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}

static INPUT_PORTS_START( kmkide_ata )
	// on-board jumper
	PORT_START("JP")
	PORT_CONFNAME(0x07, 0x00, "PBI slot ID")
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x01, "1")
	PORT_CONFSETTING(   0x02, "2")
	PORT_CONFSETTING(   0x03, "3")
	PORT_CONFSETTING(   0x04, "4")
	PORT_CONFSETTING(   0x05, "5")
	PORT_CONFSETTING(   0x06, "6")
	PORT_CONFSETTING(   0x07, "7")
INPUT_PORTS_END

ioport_constructor kmkide_ata_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( kmkide_ata );
}


void kmkide_ata_device::device_start()
{
	const u16 ram_size = 0x200 * 2;
	m_ram.resize(ram_size);
	m_rambank->configure_entries(0, 2, m_ram.data(), ram_size);

	save_item(NAME(m_latch));
	save_item(NAME(m_pbi_id));
}

void kmkide_ata_device::device_reset()
{
	m_rambank->set_entry(0);
	m_pbi_id = m_jp->read() & 7;
}

int kmkide_ata_device::pdvs_slot_w(u8 data)
{
	const int is_active = data == 1 << m_pbi_id;
	address_space &space = m_slot->memspace();

	//printf("%d\n", space != nullptr);

	if (is_active)
	{
		space.install_readwrite_handler(0xd100, 0xd107, read8sm_delegate(*this, FUNC(kmkide_ata_device::ide_cs1_r)), write8sm_delegate(*this, FUNC(kmkide_ata_device::ide_cs1_w)));
		space.install_readwrite_handler(0xd110, 0xd117, read8sm_delegate(*this, FUNC(kmkide_ata_device::ide_cs0_r)), write8sm_delegate(*this, FUNC(kmkide_ata_device::ide_cs0_w)));
		// TODO: $d11e 06 -> 02 writes on reset
		space.install_write_handler(0xd1a0, 0xd1a0, write8sm_delegate(*this, FUNC(kmkide_ata_device::ram_bank_w<1>)));
		space.install_write_handler(0xd1c0, 0xd1c0, write8sm_delegate(*this, FUNC(kmkide_ata_device::ram_bank_w<0>)));
		space.install_rom(0xd800, 0xddff, m_bios->base());
		space.install_readwrite_bank(0xde00, 0xdfff, m_rambank);
	}
	else
		space.unmap_readwrite(0x0000, 0xffff);

	return is_active;
}

u8 kmkide_ata_device::ide_cs0_r(offs_t offset)
{
	if (offset == 0)
	{
		if (machine().side_effects_disabled())
			return 0xff;
		u16 ide_data = m_ata->cs0_r(0);
		m_latch[0] = ide_data >> 8;
		return ide_data & 0xff;
	}

	return m_ata->cs0_r(offset);
}

void kmkide_ata_device::ide_cs0_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		u16 ide_data = (m_latch[1] << 8) | (data);
		m_ata->cs0_w(0, ide_data);
		return;
	}

	m_ata->cs0_w(offset, data);
}

u8 kmkide_ata_device::ide_cs1_r(offs_t offset)
{
	if (offset == 0)
	{
		return m_latch[0];
	}
	return m_ata->cs1_r(offset);
}

void kmkide_ata_device::ide_cs1_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_latch[1] = data;
		//uint16_t ide_data = m_latch[1] | (data);
		//m_ata->cs0_w(0, ide_data);
		return;
	}
	m_ata->cs1_w(offset, data);
}

// TODO: verify this really being strobe instead
template <unsigned N> void kmkide_ata_device::ram_bank_w(offs_t offset, u8 data)
{
	m_rambank->set_entry(N & 1);
}
