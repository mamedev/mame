// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 UPD7725 / UPD96050 add-on chip emulation (for SNES/SFC)
 used in carts with DSP-1, DSP-1A, DSP-1B, DSP-2, DSP-3, DSP-4, ST-010 & ST-011 add-on chips

 ***********************************************************************************************************/


#include "emu.h"
#include "upd.h"


// helpers
inline uint32_t get_prg(uint8_t *CPU, uint32_t addr)
{
	return ((CPU[addr * 4] << 24) | (CPU[addr * 4 + 1] << 16) | (CPU[addr * 4 + 2] << 8) | 0x00);
}
inline uint16_t get_data(uint8_t *CPU, uint32_t addr)
{
	return ((CPU[addr * 2] << 8) | CPU[addr * 2 + 1]);
}

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SNS_LOROM_NECDSP, sns_rom20_necdsp_device,  "sns_rom_necdsp",   "SNES Cart (LoROM) + NEC DSP")
DEFINE_DEVICE_TYPE(SNS_HIROM_NECDSP, sns_rom21_necdsp_device,  "sns_rom21_necdsp", "SNES Cart (HiROM) + NEC DSP")
DEFINE_DEVICE_TYPE(SNS_LOROM_SETA10, sns_rom_seta10dsp_device, "sns_rom_seta10",   "SNES Cart (LoROM) + Seta ST010 DSP")
DEFINE_DEVICE_TYPE(SNS_LOROM_SETA11, sns_rom_seta11dsp_device, "sns_rom_seta11",   "SNES Cart (LoROM) + Seta ST011 DSP")


sns_rom20_necdsp_device::sns_rom20_necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_device(mconfig, type, tag, owner, clock), m_upd7725(*this, "dsp")
{
}

sns_rom20_necdsp_device::sns_rom20_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP, tag, owner, clock)
{
}

sns_rom21_necdsp_device::sns_rom21_necdsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom21_device(mconfig, type, tag, owner, clock), m_upd7725(*this, "dsp")
{
}

sns_rom21_necdsp_device::sns_rom21_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom21_necdsp_device(mconfig, SNS_HIROM_NECDSP, tag, owner, clock)
{
}

sns_rom_setadsp_device::sns_rom_setadsp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_device(mconfig, type, tag, owner, clock), m_upd96050(*this, "dsp")
{
}

sns_rom_seta10dsp_device::sns_rom_seta10dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA10, tag, owner, clock)
{
}

sns_rom_seta11dsp_device::sns_rom_seta11dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA11, tag, owner, clock)
{
}


void sns_rom20_necdsp_device::device_start()
{
	m_dsp_prg.resize(0x2000/sizeof(uint32_t));
	m_dsp_data.resize(0x800/sizeof(uint16_t));
}

void sns_rom21_necdsp_device::device_start()
{
	m_dsp_prg.resize(0x2000/sizeof(uint32_t));
	m_dsp_data.resize(0x800/sizeof(uint16_t));
}

void sns_rom_setadsp_device::device_start()
{
	m_dsp_prg.resize(0x10000/sizeof(uint32_t));
	m_dsp_data.resize(0x1000/sizeof(uint16_t));
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

//-------------------------------------------------
//    NEC DSP
//-------------------------------------------------

// Lo-ROM

// DSP dump contains prg at offset 0 and data at offset 0x2000
uint32_t sns_rom20_necdsp_device::necdsp_prg_r(offs_t offset)
{
	return get_prg(&m_bios[0], offset);
}

uint16_t sns_rom20_necdsp_device::necdsp_data_r(offs_t offset)
{
	return get_data(&m_bios[0], offset + 0x2000/2);
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_prg_map )
//-------------------------------------------------

void sns_rom20_necdsp_device::dsp_prg_map_lorom(address_map &map)
{
	map(0x0000, 0x07ff).r(FUNC(sns_rom20_necdsp_device::necdsp_prg_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_data_map )
//-------------------------------------------------

void sns_rom20_necdsp_device::dsp_data_map_lorom(address_map &map)
{
	map(0x0000, 0x03ff).r(FUNC(sns_rom20_necdsp_device::necdsp_data_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sns_rom20_necdsp_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom20_necdsp_device::dsp_prg_map_lorom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom20_necdsp_device::dsp_data_map_lorom);
}

uint8_t sns_rom20_necdsp_device::chip_read(offs_t offset)
{
	offset &= 0x7fff;
	return m_upd7725->snesdsp_read(offset < 0x4000);
}


void sns_rom20_necdsp_device::chip_write(offs_t offset, uint8_t data)
{
	offset &= 0x7fff;
	m_upd7725->snesdsp_write(offset < 0x4000, data);
}


// Hi-ROM

// DSP dump contains prg at offset 0 and data at offset 0x2000
uint32_t sns_rom21_necdsp_device::necdsp_prg_r(offs_t offset)
{
	return get_prg(&m_bios[0], offset);
}

uint16_t sns_rom21_necdsp_device::necdsp_data_r(offs_t offset)
{
	return get_data(&m_bios[0], offset + 0x2000/2);
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_prg_map )
//-------------------------------------------------

void sns_rom21_necdsp_device::dsp_prg_map_hirom(address_map &map)
{
	map(0x0000, 0x07ff).r(FUNC(sns_rom21_necdsp_device::necdsp_prg_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_data_map )
//-------------------------------------------------

void sns_rom21_necdsp_device::dsp_data_map_hirom(address_map &map)
{
	map(0x0000, 0x03ff).r(FUNC(sns_rom21_necdsp_device::necdsp_data_r));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sns_rom21_necdsp_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom21_necdsp_device::dsp_prg_map_hirom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom21_necdsp_device::dsp_data_map_hirom);
}

uint8_t sns_rom21_necdsp_device::chip_read(offs_t offset)
{
	offset &= 0x1fff;
	return m_upd7725->snesdsp_read(offset < 0x1000);
}


void sns_rom21_necdsp_device::chip_write(offs_t offset, uint8_t data)
{
	offset &= 0x1fff;
	m_upd7725->snesdsp_write(offset < 0x1000, data);
}


//-------------------------------------------------
//    Seta DSP
//-------------------------------------------------

// same as above but additional read/write handling for the add-on chip

uint8_t sns_rom_setadsp_device::chip_read(offs_t offset)
{
	if (offset >= 0x600000 && offset < 0x680000 && (offset & 0xffff) < 0x4000)
		m_upd96050->snesdsp_read((offset & 0x01) ? false : true);

	if (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x8000)
	{
		uint16_t address = offset & 0xffff;
		uint16_t temp = m_upd96050->dataram_r(address/2);
		if (offset & 1)
			return temp >> 8;
		else
			return temp & 0xff;
	}

	return 0xff;
}


void sns_rom_setadsp_device::chip_write(offs_t offset, uint8_t data)
{
	if (offset >= 0x600000 && offset < 0x680000 && (offset & 0xffff) < 0x4000)
	{
		m_upd96050->snesdsp_write((offset & 0x01) ? false : true, data);
		return;
	}

	if (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x8000)
	{
		uint16_t address = offset & 0xffff;
		uint16_t temp = m_upd96050->dataram_r(address/2);

		if (offset & 1)
		{
			temp &= 0xff;
			temp |= data << 8;
		}
		else
		{
			temp &= 0xff00;
			temp |= data;
		}

		m_upd96050->dataram_w(address/2, temp);
		return;
	}
}


// DSP dump contains prg at offset 0 and data at offset 0x10000
uint32_t sns_rom_setadsp_device::setadsp_prg_r(offs_t offset)
{
	return get_prg(&m_bios[0], offset);
}

uint16_t sns_rom_setadsp_device::setadsp_data_r(offs_t offset)
{
	return get_data(&m_bios[0], offset + 0x10000/2);
}


//-------------------------------------------------
//  ADDRESS_MAP( st01x_prg_map )
//-------------------------------------------------

void sns_rom_setadsp_device::st01x_prg_map(address_map &map)
{
	map(0x0000, 0x3fff).r(FUNC(sns_rom_setadsp_device::setadsp_prg_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( st01x_data_map )
//-------------------------------------------------

void sns_rom_setadsp_device::st01x_data_map(address_map &map)
{
	map(0x0000, 0x07ff).r(FUNC(sns_rom_setadsp_device::setadsp_data_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sns_rom_seta10dsp_device::device_add_mconfig(machine_config &config)
{
	UPD96050(config, m_upd96050, 10000000);
	m_upd96050->set_addrmap(AS_PROGRAM, &sns_rom_seta10dsp_device::st01x_prg_map);
	m_upd96050->set_addrmap(AS_DATA, &sns_rom_seta10dsp_device::st01x_data_map);
}


void sns_rom_seta11dsp_device::device_add_mconfig(machine_config &config)
{
	UPD96050(config, m_upd96050, 15000000);
	m_upd96050->set_addrmap(AS_PROGRAM, &sns_rom_seta11dsp_device::st01x_prg_map);
	m_upd96050->set_addrmap(AS_DATA, &sns_rom_seta11dsp_device::st01x_data_map);
}


// To make faster DSP access to its internal rom, let's install read banks and map m_bios there with correct byte order

void sns_rom20_necdsp_device::speedup_addon_bios_access()
{
	m_upd7725->space(AS_PROGRAM).install_rom(0x0000, 0x07ff, &m_dsp_prg[0]);
	m_upd7725->space(AS_DATA).install_rom(0x0000, 0x03ff, &m_dsp_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x800; x++)
		m_dsp_prg[x] = (m_bios[x * 4] << 24) | (m_bios[x * 4 + 1] << 16) | (m_bios[x * 4 + 2] << 8) | 0x00;
	for (int x = 0; x < 0x400; x++)
		m_dsp_data[x] = (m_bios[0x2000 + x * 2] << 8) | m_bios[0x2000 + x * 2 + 1];
}

void sns_rom21_necdsp_device::speedup_addon_bios_access()
{
	m_upd7725->space(AS_PROGRAM).install_rom(0x0000, 0x07ff, &m_dsp_prg[0]);
	m_upd7725->space(AS_DATA).install_rom(0x0000, 0x03ff, &m_dsp_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x800; x++)
		m_dsp_prg[x] = (m_bios[x * 4] << 24) | (m_bios[x * 4 + 1] << 16) | (m_bios[x * 4 + 2] << 8) | 0x00;
	for (int x = 0; x < 0x400; x++)
		m_dsp_data[x] = (m_bios[0x2000 + x * 2] << 8) | m_bios[0x2000 + x * 2 + 1];
}

void sns_rom_setadsp_device::speedup_addon_bios_access()
{
	m_upd96050->space(AS_PROGRAM).install_rom(0x0000, 0x3fff, &m_dsp_prg[0]);
	m_upd96050->space(AS_DATA).install_rom(0x0000, 0x07ff, &m_dsp_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x4000; x++)
		m_dsp_prg[x] = (m_bios[x * 4] << 24) | (m_bios[x * 4 + 1] << 16) | (m_bios[x * 4 + 2] << 8) | 0x00;
	for (int x = 0; x < 0x800; x++)
		m_dsp_data[x] = (m_bios[0x10000 + x * 2] << 8) | m_bios[0x10000 + x * 2 + 1];
}




// Legacy versions including DSP dump roms, in order to support faulty dumps missing DSP data...

DEFINE_DEVICE_TYPE(SNS_LOROM_NECDSP1_LEG,  sns_rom20_necdsp1_legacy_device,  "sns_dsp1leg",    "SNES Cart (LoROM) + NEC DSP1 Legacy")
DEFINE_DEVICE_TYPE(SNS_LOROM_NECDSP1B_LEG, sns_rom20_necdsp1b_legacy_device, "sns_dsp1bleg",   "SNES Cart (LoROM) + NEC DSP1B Legacy")
DEFINE_DEVICE_TYPE(SNS_LOROM_NECDSP2_LEG,  sns_rom20_necdsp2_legacy_device,  "sns_dsp2leg",    "SNES Cart (LoROM) + NEC DSP2 Legacy")
DEFINE_DEVICE_TYPE(SNS_LOROM_NECDSP3_LEG,  sns_rom20_necdsp3_legacy_device,  "sns_dsp3leg",    "SNES Cart (LoROM) + NEC DSP3 Legacy")
DEFINE_DEVICE_TYPE(SNS_LOROM_NECDSP4_LEG,  sns_rom20_necdsp4_legacy_device,  "sns_dsp4leg",    "SNES Cart (LoROM) + NEC DSP4 Legacy")
DEFINE_DEVICE_TYPE(SNS_HIROM_NECDSP1_LEG,  sns_rom21_necdsp1_legacy_device,  "sns_dsp1leg_hi", "SNES Cart (HiROM) + NEC DSP1 Legacy")
DEFINE_DEVICE_TYPE(SNS_LOROM_SETA10_LEG,   sns_rom_seta10dsp_legacy_device,  "sns_seta10leg",  "SNES Cart (LoROM) + SETA ST010 DSP Legacy")
DEFINE_DEVICE_TYPE(SNS_LOROM_SETA11_LEG,   sns_rom_seta11dsp_legacy_device,  "sns_seta11leg",  "SNES Cart (LoROM) + SETA ST011 DSP Legacy")


sns_rom20_necdsp1_legacy_device::sns_rom20_necdsp1_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP1_LEG, tag, owner, clock)
{
}

sns_rom20_necdsp1b_legacy_device::sns_rom20_necdsp1b_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP1B_LEG, tag, owner, clock)
{
}

sns_rom20_necdsp2_legacy_device::sns_rom20_necdsp2_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP2_LEG, tag, owner, clock)
{
}

sns_rom20_necdsp3_legacy_device::sns_rom20_necdsp3_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP3_LEG, tag, owner, clock)
{
}

sns_rom20_necdsp4_legacy_device::sns_rom20_necdsp4_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP4_LEG, tag, owner, clock)
{
}

sns_rom21_necdsp1_legacy_device::sns_rom21_necdsp1_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom21_necdsp_device(mconfig, SNS_HIROM_NECDSP1_LEG, tag, owner, clock)
{
}

sns_rom_seta10dsp_legacy_device::sns_rom_seta10dsp_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA10_LEG, tag, owner, clock)
{
}

sns_rom_seta11dsp_legacy_device::sns_rom_seta11dsp_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA11_LEG, tag, owner, clock)
{
}


void sns_rom20_necdsp1_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom20_necdsp1_legacy_device::dsp_prg_map_lorom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom20_necdsp1_legacy_device::dsp_data_map_lorom);
}

void sns_rom20_necdsp1b_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom20_necdsp1b_legacy_device::dsp_prg_map_lorom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom20_necdsp1b_legacy_device::dsp_data_map_lorom);
}

void sns_rom20_necdsp2_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom20_necdsp2_legacy_device::dsp_prg_map_lorom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom20_necdsp2_legacy_device::dsp_data_map_lorom);
}

void sns_rom20_necdsp3_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom20_necdsp3_legacy_device::dsp_prg_map_lorom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom20_necdsp3_legacy_device::dsp_data_map_lorom);
}

void sns_rom20_necdsp4_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom20_necdsp4_legacy_device::dsp_prg_map_lorom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom20_necdsp4_legacy_device::dsp_data_map_lorom);
}

void sns_rom21_necdsp1_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_rom21_necdsp1_legacy_device::dsp_prg_map_hirom);
	m_upd7725->set_addrmap(AS_DATA, &sns_rom21_necdsp1_legacy_device::dsp_data_map_hirom);
}

void sns_rom_seta10dsp_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD96050(config, m_upd96050, 10000000);
	m_upd96050->set_addrmap(AS_PROGRAM, &sns_rom_seta10dsp_legacy_device::st01x_prg_map);
	m_upd96050->set_addrmap(AS_DATA, &sns_rom_seta10dsp_legacy_device::st01x_data_map);
}

void sns_rom_seta11dsp_legacy_device::device_add_mconfig(machine_config &config)
{
	UPD96050(config, m_upd96050, 15000000);
	m_upd96050->set_addrmap(AS_PROGRAM, &sns_rom_seta11dsp_legacy_device::st01x_prg_map);
	m_upd96050->set_addrmap(AS_DATA, &sns_rom_seta11dsp_legacy_device::st01x_data_map);
}


ROM_START( snes_dsp1 )
	ROM_REGION(0x2800, "addon", 0)
	ROM_LOAD( "dsp1.bin",       0,  0x02800, CRC(2838f9f5) SHA1(0a03ccb1fd2bea91151c745a4d1f217ae784f889) )
ROM_END

ROM_START( snes_dsp1b )
	ROM_REGION(0x2800, "addon", 0)
	ROM_LOAD( "dsp1b.bin",      0,  0x02800, CRC(453557e0) SHA1(3a218b0e4572a8eba6d0121b17fdac9529609220) )
ROM_END

ROM_START( snes_dsp2 )
	ROM_REGION(0x2800, "addon", 0)
	ROM_LOAD( "dsp2.bin",       0,  0x02800, CRC(8e9fbd9b) SHA1(06dd9fcb118d18f6bbe234e013cb8780e06d6e63) )
ROM_END

ROM_START( snes_dsp3 )
	ROM_REGION(0x2800, "addon", 0)
	ROM_LOAD( "dsp3.bin",       0,  0x02800, CRC(6b86728a) SHA1(1b133741fad810eb7320c21ecfdd427d25a46da1) )
ROM_END

ROM_START( snes_dsp4 )
	ROM_REGION(0x2800, "addon", 0)
	ROM_LOAD( "dsp4.bin",       0,  0x02800, CRC(ce0c7783) SHA1(76fd25f7dc26c3b3f7868a3aa78c7684068713e5) )
ROM_END

ROM_START( snes_st010 )
	ROM_REGION(0x11000, "addon", 0)
	ROM_LOAD( "st010.bin",      0,  0x11000, CRC(aa11ee2d) SHA1(cc1984e989cb94e3dcbb5f99e085b5414e18a017) )
ROM_END

ROM_START( snes_st011 )
	ROM_REGION(0x11000, "addon", 0)
	ROM_LOAD( "st011.bin",      0,  0x11000, CRC(34d2952c) SHA1(1375b8c1efc8cae4962b57dfe22f6b78e1ddacc8) )
ROM_END

const tiny_rom_entry *sns_rom20_necdsp1_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp1 );
}

const tiny_rom_entry *sns_rom20_necdsp1b_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp1b );
}

const tiny_rom_entry *sns_rom20_necdsp2_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp2 );
}

const tiny_rom_entry *sns_rom20_necdsp3_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp3 );
}

const tiny_rom_entry *sns_rom20_necdsp4_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp4 );
}

const tiny_rom_entry *sns_rom21_necdsp1_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp1 );
}

const tiny_rom_entry *sns_rom_seta10dsp_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_st010 );
}

const tiny_rom_entry *sns_rom_seta11dsp_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_st011 );
}
