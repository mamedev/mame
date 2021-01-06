// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Super NES/Famicom Event cartridges emulation (for SNES/SFC)

 TODO: figure out how the Test Mode switch works...

 ***********************************************************************************************************/


#include "emu.h"
#include "event.h"


//-------------------------------------------------
//  sns_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SNS_PFEST94, sns_pfest94_device, "sns_pfest94", "SNES Powerfest '94")


sns_pfest94_device::sns_pfest94_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNS_PFEST94, tag, owner, clock)
	, device_sns_cart_interface(mconfig, *this)
	, m_upd7725(*this, "dsp")
	, m_dsw(*this, "DIPSW")
	, m_base_bank(0)
	, m_mask(0)
	, m_status(0)
	, m_count(0)
	, pfest94_timer(nullptr)
{
}


void sns_pfest94_device::device_start()
{
	m_dsp_prg.resize(0x2000/sizeof(uint32_t));
	m_dsp_data.resize(0x800/sizeof(uint16_t));
	pfest94_timer = timer_alloc(TIMER_EVENT);
	pfest94_timer->reset();

	save_item(NAME(m_base_bank));
	save_item(NAME(m_mask));
	save_item(NAME(m_status));
	save_item(NAME(m_count));
}

void sns_pfest94_device::device_reset()
{
	m_base_bank = 0;
	m_mask = 0x07;
	m_status = 0;
	m_count = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t sns_pfest94_device::read_l(offs_t offset)
{
	// menu
	if ((offset & 0x208000) == 0x208000)
	{
		int bank = ((offset - 0x200000) / 0x10000) & 7;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else
	{
		// never called beyond 0x400000!
		offset &= 0x1fffff;
		int bank = (m_base_bank == 0x18) ? offset / 0x8000 : offset / 0x10000;
		return m_rom[rom_bank_map[m_base_bank + (bank & m_mask)] * 0x8000 + (offset & 0x7fff)];
	}
}

uint8_t sns_pfest94_device::read_h(offs_t offset)
{
	// menu
	if ((offset & 0x208000) == 0x208000)
	{
		int bank = ((offset - 0x200000) / 0x8000) & 7;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}

	// called beyond 0x400000!
	if (offset < 0x400000)
	{
		offset &= 0x1fffff;
		int bank = (m_base_bank == 0x18) ? offset / 0x8000 : offset / 0x10000;
		return m_rom[rom_bank_map[m_base_bank + (bank & m_mask)] * 0x8000 + (offset & 0x7fff)];
	}
	else
	{
		offset &= 0x3fffff;
		int bank = offset / 0x8000;
		return m_rom[rom_bank_map[m_base_bank + (bank & m_mask)] * 0x8000 + (offset & 0x7fff)];
	}
}


// these are used for two diff effects: both to select game from menu and to access the DSP when running SMK!
uint8_t sns_pfest94_device::chip_read(offs_t offset)
{
	if (offset & 0x8000)
	{
		// menu access
		return m_status;
	}
	else
	{
		// DSP access
		offset &= 0x1fff;
		return m_upd7725->snesdsp_read(offset < 0x1000);
	}
}


void sns_pfest94_device::chip_write(offs_t offset, uint8_t data)
{
	if (offset & 0x8000)
	{
		// menu access
		if (data == 0x00)
		{
			m_base_bank = 0;
			m_mask = 0x07;
		}
		if (data == 0x09)
		{
			m_base_bank = 0x08;
			m_mask = 0x0f;
			// start timer
			m_count = (3 + ((m_dsw->read() & 0xf0) >> 4)) * 60;
			pfest94_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
		}
		if (data == 0x0c)
		{
			m_base_bank = 0x18;
			m_mask = 0x0f;
		}
		if (data == 0x0a)
		{
			m_base_bank = 0x28;
			m_mask = 0x1f;
		}
	}
	else
	{
		// DSP access
		offset &= 0x1fff;
		m_upd7725->snesdsp_write(offset < 0x1000, data);
	}
}


//-------------------------------------------------
//    NEC DSP
//-------------------------------------------------

// helpers
inline uint32_t get_prg(uint8_t *CPU, uint32_t addr)
{
	return ((CPU[addr * 4] << 24) | (CPU[addr * 4 + 1] << 16) | (CPU[addr * 4 + 2] << 8) | 0x00);
}
inline uint16_t get_data(uint8_t *CPU, uint32_t addr)
{
	return ((CPU[addr * 2] << 8) | CPU[addr * 2 + 1]);
}

void sns_pfest94_device::speedup_addon_bios_access()
{
	m_upd7725->space(AS_PROGRAM).install_rom(0x0000, 0x07ff, &m_dsp_prg[0]);
	m_upd7725->space(AS_DATA).install_rom(0x0000, 0x03ff, &m_dsp_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x800; x++)
		m_dsp_prg[x] = (m_bios[x * 4] << 24) | (m_bios[x * 4 + 1] << 16) | (m_bios[x * 4 + 2] << 8) | 0x00;
	for (int x = 0; x < 0x400; x++)
		m_dsp_data[x] = (m_bios[0x2000 + x * 2] << 8) | m_bios[0x2000 + x * 2 + 1];
}


// DSP dump contains prg at offset 0 and data at offset 0x2000
uint32_t sns_pfest94_device::necdsp_prg_r(offs_t offset)
{
	return get_prg(&m_bios[0], offset);
}

uint16_t sns_pfest94_device::necdsp_data_r(offs_t offset)
{
	return get_data(&m_bios[0], offset + 0x2000/2);
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_prg_map )
//-------------------------------------------------

void sns_pfest94_device::dsp_prg_map_lorom(address_map &map)
{
	map(0x0000, 0x07ff).r(FUNC(sns_pfest94_device::necdsp_prg_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( dsp_data_map )
//-------------------------------------------------

void sns_pfest94_device::dsp_data_map_lorom(address_map &map)
{
	map(0x0000, 0x03ff).r(FUNC(sns_pfest94_device::necdsp_data_r));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sns_pfest94_device::device_add_mconfig(machine_config &config)
{
	UPD7725(config, m_upd7725, 8000000);
	m_upd7725->set_addrmap(AS_PROGRAM, &sns_pfest94_device::dsp_prg_map_lorom);
	m_upd7725->set_addrmap(AS_DATA, &sns_pfest94_device::dsp_data_map_lorom);
}

//-------------------------------------------------
//  Dipswitch
//-------------------------------------------------

static INPUT_PORTS_START( pfest94_dsw )
	PORT_START("DIPSW")
	PORT_DIPUNUSED(0x03, 0x00)
	PORT_DIPNAME( 0x04, 0x00, "Test Mode" )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x04, DEF_STR( On ) )
	PORT_DIPUNUSED(0x08, 0x08)
	PORT_DIPNAME( 0xf0, 0x30, "Timer" )
	PORT_DIPSETTING( 0x00, "3 Minutes" )
	PORT_DIPSETTING( 0x10, "4 Minutes" )
	PORT_DIPSETTING( 0x20, "5 Minutes" )
	PORT_DIPSETTING( 0x30, "6 Minutes" )
	PORT_DIPSETTING( 0x40, "7 Minutes" )
	PORT_DIPSETTING( 0x50, "8 Minutes" )
	PORT_DIPSETTING( 0x60, "9 Minutes" )
	PORT_DIPSETTING( 0x70, "10 Minutes" )
	PORT_DIPSETTING( 0x80, "11 Minutes" )
	PORT_DIPSETTING( 0x90, "12 Minutes" )
	PORT_DIPSETTING( 0xa0, "13 Minutes" )
	PORT_DIPSETTING( 0xb0, "14 Minutes" )
	PORT_DIPSETTING( 0xc0, "15 Minutes" )
	PORT_DIPSETTING( 0xd0, "16 Minutes" )
	PORT_DIPSETTING( 0xe0, "17 Minutes" )
	PORT_DIPSETTING( 0xf0, "18 Minutes" )
INPUT_PORTS_END


ioport_constructor sns_pfest94_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pfest94_dsw );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void sns_pfest94_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_EVENT)
	{
		if (!m_count)
		{
			m_status |= 2;
			pfest94_timer->reset();
		}
		m_count--;
	}
}
