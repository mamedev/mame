// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 UPD7725 / UPD96050 add-on chip emulation (for SNES/SFC)
 used in carts with DSP-1, DSP-1A, DSP-1B, DSP-2, DSP-3, DSP-4, ST-010 & ST-011 add-on chips

 ***********************************************************************************************************/


#include "emu.h"
#include "upd.h"


// helpers
inline UINT32 get_prg(UINT8 *CPU, UINT32 addr)
{
	return ((CPU[addr * 4] << 24) | (CPU[addr * 4 + 1] << 16) | (CPU[addr * 4 + 2] << 8) | 0x00);
}
inline UINT16 get_data(UINT8 *CPU, UINT32 addr)
{
	return ((CPU[addr * 2] << 8) | CPU[addr * 2 + 1]);
}

//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type SNS_LOROM_NECDSP = &device_creator<sns_rom20_necdsp_device>;
const device_type SNS_HIROM_NECDSP = &device_creator<sns_rom21_necdsp_device>;
const device_type SNS_LOROM_SETA10 = &device_creator<sns_rom_seta10dsp_device>;
const device_type SNS_LOROM_SETA11 = &device_creator<sns_rom_seta11dsp_device>;


sns_rom20_necdsp_device::sns_rom20_necdsp_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: sns_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
						m_upd7725(*this, "dsp")
{
}

sns_rom20_necdsp_device::sns_rom20_necdsp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_NECDSP, "SNES Cart (LoROM) + NEC DSP", tag, owner, clock, "sns_rom_necdsp", __FILE__),
						m_upd7725(*this, "dsp")
{
}

sns_rom21_necdsp_device::sns_rom21_necdsp_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: sns_rom21_device(mconfig, type, name, tag, owner, clock, shortname, source),
						m_upd7725(*this, "dsp")
{
}

sns_rom21_necdsp_device::sns_rom21_necdsp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom21_device(mconfig, SNS_HIROM_NECDSP, "SNES Cart (HiROM) + NEC DSP", tag, owner, clock, "sns_rom21_necdsp", __FILE__),
						m_upd7725(*this, "dsp")
{
}

sns_rom_setadsp_device::sns_rom_setadsp_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
					: sns_rom_device(mconfig, type, name, tag, owner, clock, shortname, source),
						m_upd96050(*this, "dsp")
{
}

sns_rom_seta10dsp_device::sns_rom_seta10dsp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA10, "SNES Cart (LoROM) + Seta ST010 DSP", tag, owner, clock, "sns_rom_seta10", __FILE__)
{
}

sns_rom_seta11dsp_device::sns_rom_seta11dsp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA11, "SNES Cart (LoROM) + Seta ST011 DSP", tag, owner, clock, "sns_rom_seta11", __FILE__)
{
}


void sns_rom20_necdsp_device::device_start()
{
	m_dsp_prg.resize(0x2000/sizeof(UINT32));
	m_dsp_data.resize(0x800/sizeof(UINT16));
}

void sns_rom21_necdsp_device::device_start()
{
	m_dsp_prg.resize(0x2000/sizeof(UINT32));
	m_dsp_data.resize(0x800/sizeof(UINT16));
}

void sns_rom_setadsp_device::device_start()
{
	m_dsp_prg.resize(0x10000/sizeof(UINT32));
	m_dsp_data.resize(0x1000/sizeof(UINT16));
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

//-------------------------------------------------
//    NEC DSP
//-------------------------------------------------

// Lo-ROM

// DSP dump contains prg at offset 0 and data at offset 0x2000
READ32_MEMBER( sns_rom20_necdsp_device::necdsp_prg_r )
{
	return get_prg(&m_bios[0], offset);
}

READ16_MEMBER( sns_rom20_necdsp_device::necdsp_data_r )
{
	return get_data(&m_bios[0], offset + 0x2000/2);
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_prg_map )
//-------------------------------------------------

static ADDRESS_MAP_START( dsp_prg_map_lorom, AS_PROGRAM, 32, sns_rom20_necdsp_device )
	AM_RANGE(0x0000, 0x07ff) AM_READ(necdsp_prg_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( dsp_data_map )
//-------------------------------------------------

static ADDRESS_MAP_START( dsp_data_map_lorom, AS_DATA, 16, sns_rom20_necdsp_device )
	AM_RANGE(0x0000, 0x03ff) AM_READ(necdsp_data_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( snes_dsp )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( snes_dsp_lorom )
	MCFG_CPU_ADD("dsp", UPD7725, 8000000)
	MCFG_CPU_PROGRAM_MAP(dsp_prg_map_lorom)
	MCFG_CPU_DATA_MAP(dsp_data_map_lorom)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sns_rom20_necdsp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_lorom );
}

READ8_MEMBER( sns_rom20_necdsp_device::chip_read )
{
	offset &= 0x7fff;
	return m_upd7725->snesdsp_read(offset < 0x4000);
}


WRITE8_MEMBER( sns_rom20_necdsp_device::chip_write )
{
	offset &= 0x7fff;
	m_upd7725->snesdsp_write(offset < 0x4000, data);
}


// Hi-ROM

// DSP dump contains prg at offset 0 and data at offset 0x2000
READ32_MEMBER( sns_rom21_necdsp_device::necdsp_prg_r )
{
	return get_prg(&m_bios[0], offset);
}

READ16_MEMBER( sns_rom21_necdsp_device::necdsp_data_r )
{
	return get_data(&m_bios[0], offset + 0x2000/2);
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_prg_map )
//-------------------------------------------------

static ADDRESS_MAP_START( dsp_prg_map_hirom, AS_PROGRAM, 32, sns_rom21_necdsp_device )
	AM_RANGE(0x0000, 0x07ff) AM_READ(necdsp_prg_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( dsp_data_map )
//-------------------------------------------------

static ADDRESS_MAP_START( dsp_data_map_hirom, AS_DATA, 16, sns_rom21_necdsp_device )
	AM_RANGE(0x0000, 0x03ff) AM_READ(necdsp_data_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( snes_dsp )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( snes_dsp_hirom )
	MCFG_CPU_ADD("dsp", UPD7725, 8000000)
	MCFG_CPU_PROGRAM_MAP(dsp_prg_map_hirom)
	MCFG_CPU_DATA_MAP(dsp_data_map_hirom)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sns_rom21_necdsp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_hirom );
}

READ8_MEMBER( sns_rom21_necdsp_device::chip_read )
{
	offset &= 0x1fff;
	return m_upd7725->snesdsp_read(offset < 0x1000);
}


WRITE8_MEMBER( sns_rom21_necdsp_device::chip_write )
{
	offset &= 0x1fff;
	m_upd7725->snesdsp_write(offset < 0x1000, data);
}


//-------------------------------------------------
//    Seta DSP
//-------------------------------------------------

// same as above but additional read/write handling for the add-on chip

READ8_MEMBER( sns_rom_setadsp_device::chip_read )
{
	if (offset >= 0x600000 && offset < 0x680000 && (offset & 0xffff) < 0x4000)
		m_upd96050->snesdsp_read((offset & 0x01) ? FALSE : TRUE);

	if (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x8000)
	{
		UINT16 address = offset & 0xffff;
		UINT16 temp = m_upd96050->dataram_r(address/2);
		if (offset & 1)
			return temp >> 8;
		else
			return temp & 0xff;
	}

	return 0xff;
}


WRITE8_MEMBER( sns_rom_setadsp_device::chip_write )
{
	if (offset >= 0x600000 && offset < 0x680000 && (offset & 0xffff) < 0x4000)
	{
		m_upd96050->snesdsp_write((offset & 0x01) ? FALSE : TRUE, data);
		return;
	}

	if (offset >= 0x680000 && offset < 0x700000 && (offset & 0xffff) < 0x8000)
	{
		UINT16 address = offset & 0xffff;
		UINT16 temp = m_upd96050->dataram_r(address/2);

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
READ32_MEMBER( sns_rom_setadsp_device::setadsp_prg_r )
{
	return get_prg(&m_bios[0], offset);
}

READ16_MEMBER( sns_rom_setadsp_device::setadsp_data_r )
{
	return get_data(&m_bios[0], offset + 0x10000/2);
}


//-------------------------------------------------
//  ADDRESS_MAP( st01x_prg_map )
//-------------------------------------------------

static ADDRESS_MAP_START( st01x_prg_map, AS_PROGRAM, 32, sns_rom_setadsp_device )
	AM_RANGE(0x0000, 0x3fff) AM_READ(setadsp_prg_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( st01x_data_map )
//-------------------------------------------------

static ADDRESS_MAP_START( st01x_data_map, AS_DATA, 16, sns_rom_setadsp_device )
	AM_RANGE(0x0000, 0x07ff) AM_READ(setadsp_data_r)
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( snes_st010 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( snes_st010 )
	MCFG_CPU_ADD("dsp", UPD96050, 10000000)
	MCFG_CPU_PROGRAM_MAP(st01x_prg_map)
	MCFG_CPU_DATA_MAP(st01x_data_map)
MACHINE_CONFIG_END

//-------------------------------------------------
//  MACHINE_DRIVER( snes_st011 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( snes_st011 )
	MCFG_CPU_ADD("dsp", UPD96050, 15000000)
	MCFG_CPU_PROGRAM_MAP(st01x_prg_map)
	MCFG_CPU_DATA_MAP(st01x_data_map)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sns_rom_seta10dsp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_st010 );
}

machine_config_constructor sns_rom_seta11dsp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_st011 );
}


// To make faster DSP access to its internal rom, let's install read banks and map m_bios there with correct byte order

void sns_rom20_necdsp_device::speedup_addon_bios_access()
{
	m_upd7725->space(AS_PROGRAM).install_read_bank(0x0000, 0x07ff, "dsp_prg");
	m_upd7725->space(AS_DATA).install_read_bank(0x0000, 0x03ff, "dsp_data");
	membank("dsp_prg")->set_base(&m_dsp_prg[0]);
	membank("dsp_data")->set_base(&m_dsp_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x800; x++)
		m_dsp_prg[x] = (m_bios[x * 4] << 24) | (m_bios[x * 4 + 1] << 16) | (m_bios[x * 4 + 2] << 8) | 0x00;
	for (int x = 0; x < 0x400; x++)
		m_dsp_data[x] = (m_bios[0x2000 + x * 2] << 8) | m_bios[0x2000 + x * 2 + 1];
}

void sns_rom21_necdsp_device::speedup_addon_bios_access()
{
	m_upd7725->space(AS_PROGRAM).install_read_bank(0x0000, 0x07ff, "dsp_prg");
	m_upd7725->space(AS_DATA).install_read_bank(0x0000, 0x03ff, "dsp_data");
	membank("dsp_prg")->set_base(&m_dsp_prg[0]);
	membank("dsp_data")->set_base(&m_dsp_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x800; x++)
		m_dsp_prg[x] = (m_bios[x * 4] << 24) | (m_bios[x * 4 + 1] << 16) | (m_bios[x * 4 + 2] << 8) | 0x00;
	for (int x = 0; x < 0x400; x++)
		m_dsp_data[x] = (m_bios[0x2000 + x * 2] << 8) | m_bios[0x2000 + x * 2 + 1];
}

void sns_rom_setadsp_device::speedup_addon_bios_access()
{
	m_upd96050->space(AS_PROGRAM).install_read_bank(0x0000, 0x3fff, "dsp_prg");
	m_upd96050->space(AS_DATA).install_read_bank(0x0000, 0x07ff, "dsp_data");
	membank("dsp_prg")->set_base(&m_dsp_prg[0]);
	membank("dsp_data")->set_base(&m_dsp_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x3fff; x++)
		m_dsp_prg[x] = (m_bios[x * 4] << 24) | (m_bios[x * 4 + 1] << 16) | (m_bios[x * 4 + 2] << 8) | 0x00;
	for (int x = 0; x < 0x07ff; x++)
		m_dsp_data[x] = (m_bios[0x10000 + x * 2] << 8) | m_bios[0x10000 + x * 2 + 1];
}




// Legacy versions including DSP dump roms, in order to support faulty dumps missing DSP data...

const device_type SNS_LOROM_NECDSP1_LEG = &device_creator<sns_rom20_necdsp1_legacy_device>;
const device_type SNS_LOROM_NECDSP1B_LEG = &device_creator<sns_rom20_necdsp1b_legacy_device>;
const device_type SNS_LOROM_NECDSP2_LEG = &device_creator<sns_rom20_necdsp2_legacy_device>;
const device_type SNS_LOROM_NECDSP3_LEG = &device_creator<sns_rom20_necdsp3_legacy_device>;
const device_type SNS_LOROM_NECDSP4_LEG = &device_creator<sns_rom20_necdsp4_legacy_device>;
const device_type SNS_HIROM_NECDSP1_LEG = &device_creator<sns_rom21_necdsp1_legacy_device>;
const device_type SNS_LOROM_SETA10_LEG = &device_creator<sns_rom_seta10dsp_legacy_device>;
const device_type SNS_LOROM_SETA11_LEG = &device_creator<sns_rom_seta11dsp_legacy_device>;


sns_rom20_necdsp1_legacy_device::sns_rom20_necdsp1_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP1_LEG, "SNES Cart (LoROM) + NEC DSP1 Legacy", tag, owner, clock, "dsp1leg", __FILE__)
{
}

sns_rom20_necdsp1b_legacy_device::sns_rom20_necdsp1b_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP1B_LEG, "SNES Cart (LoROM) + NEC DSP1B Legacy", tag, owner, clock, "dsp1bleg", __FILE__)
{
}

sns_rom20_necdsp2_legacy_device::sns_rom20_necdsp2_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP2_LEG, "SNES Cart (LoROM) + NEC DSP2 Legacy", tag, owner, clock, "dsp2leg", __FILE__)
{
}

sns_rom20_necdsp3_legacy_device::sns_rom20_necdsp3_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP3_LEG, "SNES Cart (LoROM) + NEC DSP3 Legacy", tag, owner, clock, "dsp3leg", __FILE__)
{
}

sns_rom20_necdsp4_legacy_device::sns_rom20_necdsp4_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom20_necdsp_device(mconfig, SNS_LOROM_NECDSP4_LEG, "SNES Cart (LoROM) + NEC DSP4 Legacy", tag, owner, clock, "dsp4leg", __FILE__)
{
}

sns_rom21_necdsp1_legacy_device::sns_rom21_necdsp1_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom21_necdsp_device(mconfig, SNS_HIROM_NECDSP1_LEG, "SNES Cart (HiROM) + NEC DSP1 Legacy", tag, owner, clock, "dsp1leg_hi", __FILE__)
{
}

sns_rom_seta10dsp_legacy_device::sns_rom_seta10dsp_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA10_LEG, "SNES Cart (LoROM) + Seta ST010 DSP Legacy", tag, owner, clock, "seta10leg", __FILE__)
{
}

sns_rom_seta11dsp_legacy_device::sns_rom_seta11dsp_legacy_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA11_LEG, "SNES Cart (LoROM) + Seta ST011 DSP Legacy", tag, owner, clock, "seta11leg", __FILE__)
{
}


machine_config_constructor sns_rom20_necdsp1_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_lorom );
}

machine_config_constructor sns_rom20_necdsp1b_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_lorom );
}

machine_config_constructor sns_rom20_necdsp2_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_lorom );
}

machine_config_constructor sns_rom20_necdsp3_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_lorom );
}

machine_config_constructor sns_rom20_necdsp4_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_lorom );
}

machine_config_constructor sns_rom21_necdsp1_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_dsp_hirom );
}

machine_config_constructor sns_rom_seta10dsp_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_st010 );
}

machine_config_constructor sns_rom_seta11dsp_legacy_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( snes_st011 );
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

const rom_entry *sns_rom20_necdsp1_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp1 );
}

const rom_entry *sns_rom20_necdsp1b_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp1b );
}

const rom_entry *sns_rom20_necdsp2_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp2 );
}

const rom_entry *sns_rom20_necdsp3_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp3 );
}

const rom_entry *sns_rom20_necdsp4_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp4 );
}

const rom_entry *sns_rom21_necdsp1_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_dsp1 );
}

const rom_entry *sns_rom_seta10dsp_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_st010 );
}

const rom_entry *sns_rom_seta11dsp_legacy_device::device_rom_region() const
{
	return ROM_NAME( snes_st011 );
}
