/***********************************************************************************************************

 UPD7725 / UPD96050 add-on chip emulation (for SNES/SFC)
 used in carts with DSP-1, DSP-1A, DSP-1B, DSP-2, DSP-3, DSP-4, ST-010 & ST-011 add-on chips

 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/sns_upd.h"

//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type SNS_LOROM_NECDSP = &device_creator<sns_rom20_necdsp_device>;
const device_type SNS_HIROM_NECDSP = &device_creator<sns_rom21_necdsp_device>;
const device_type SNS_LOROM_SETA10 = &device_creator<sns_rom_seta10dsp_device>;
const device_type SNS_LOROM_SETA11 = &device_creator<sns_rom_seta11dsp_device>;


sns_rom20_necdsp_device::sns_rom20_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_NECDSP, "SNES Cart (LoROM) + NEC DSP", tag, owner, clock),
						m_upd7725(*this, "dsp")
{
}

sns_rom21_necdsp_device::sns_rom21_necdsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom21_device(mconfig, SNS_HIROM_NECDSP, "SNES Cart (HiROM) + NEC DSP", tag, owner, clock),
						m_upd7725(*this, "dsp")
{
}

sns_rom_setadsp_device::sns_rom_setadsp_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, type, name, tag, owner, clock),
						m_upd96050(*this, "dsp")
{
}

sns_rom_seta10dsp_device::sns_rom_seta10dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA10, "SNES Cart (LoROM) + Seta ST010 DSP", tag, owner, clock)
{
}

sns_rom_seta11dsp_device::sns_rom_seta11dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_setadsp_device(mconfig, SNS_LOROM_SETA11, "SNES Cart (LoROM) + Seta ST011 DSP", tag, owner, clock)
{
}


void sns_rom20_necdsp_device::device_start()
{
}

void sns_rom21_necdsp_device::device_start()
{
}

void sns_rom_setadsp_device::device_start()
{
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
	return (m_bios[offset * 4] << 24) | (m_bios[offset * 4 + 1] << 16) |
				(m_bios[offset * 4 + 2] << 8) | 0x00;
}

READ16_MEMBER( sns_rom20_necdsp_device::necdsp_data_r )
{
	return (m_bios[0x2000 + offset * 2] << 8) | m_bios[0x2000 + offset * 2 + 1];
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
	return (m_bios[offset * 4] << 24) | (m_bios[offset * 4 + 1] << 16) |
	(m_bios[offset * 4 + 2] << 8) | 0x00;
}

READ16_MEMBER( sns_rom21_necdsp_device::necdsp_data_r )
{
	return (m_bios[0x2000 + offset * 2] << 8) | m_bios[0x2000 + offset * 2 + 1];
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
	return (m_bios[offset * 4] << 24) | (m_bios[offset * 4 + 1] << 16) |
				(m_bios[offset * 4 + 2] << 8) | 0x00;
}

READ16_MEMBER( sns_rom_setadsp_device::setadsp_data_r )
{
	return (m_bios[0x10000 + offset * 2] << 8) | m_bios[0x10000 + offset * 2 + 1];
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
