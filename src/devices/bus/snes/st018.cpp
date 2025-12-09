// license:BSD-3-Clause
// copyright-holders:cam900
/***********************************************************************************************************

 ARMv3 add-on chip emulation (for SNES/SFC)
 used in carts with ST-018 add-on chips

 ***********************************************************************************************************/


#include "emu.h"
#include "st018.h"


// helpers
inline uint32_t get_prg(uint8_t *CPU, uint32_t addr)
{
	return (CPU[addr * 4] | (CPU[addr * 4 + 1] << 8) | (CPU[addr * 4 + 2] << 16) | (CPU[addr * 4 + 3] << 24));
}

//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SNS_LOROM_ST018, sns_rom_st018_device, "sns_rom_st018", "SNES Cart (LoROM) + Seta ST018")


sns_rom_st018_device::sns_rom_st018_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_device(mconfig, SNS_LOROM_ST018, tag, owner, clock)
	, m_copro(*this, "copro")
	, m_cpu2copro(*this, "cpu2copro")
	, m_copro2cpu(*this, "copro2cpu")
	, m_signal(false)
	, m_copro_reset(false)
{
}

void sns_rom_st018_device::device_start()
{
	m_copro_prg.resize(0x20000/sizeof(uint32_t));
	m_copro_data.resize(0x8000/sizeof(uint32_t));

	save_item(NAME(m_signal));
	save_item(NAME(m_copro_reset));
}

void sns_rom_st018_device::device_reset()
{
	m_signal = false;
	m_copro_reset = false;
	m_copro->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

//-------------------------------------------------
//    Seta ST018
//-------------------------------------------------

// ST018 dump contains prg at offset 0 and data at offset 0x20000
uint32_t sns_rom_st018_device::copro_prg_r(offs_t offset)
{
	return get_prg(&m_bios[0], offset);
}

uint32_t sns_rom_st018_device::copro_data_r(offs_t offset)
{
	return get_prg(&m_bios[0], offset + 0x20000/4);
}

uint8_t sns_rom_st018_device::status_r()
{
	return (m_copro2cpu->pending_r() ? 0x01 : 0) |
		(m_signal ? 0x04 : 0) |
		(m_cpu2copro->pending_r() ? 0x08 : 0) |
		(m_copro_reset ? 0 : 0x80);
}

void sns_rom_st018_device::signal_w(uint8_t data)
{
	m_signal = true;
}

//-------------------------------------------------
//  copro_map
//-------------------------------------------------

void sns_rom_st018_device::copro_map(address_map &map)
{
	map(0x0000'0000, 0x0001'ffff).r(FUNC(sns_rom_st018_device::copro_prg_r));
	map(0x4000'0000, 0x4000'0000).w(m_copro2cpu, FUNC(generic_latch_8_device::write));
	map(0x4000'0010, 0x4000'0010).r(m_cpu2copro, FUNC(generic_latch_8_device::read)).w(FUNC(sns_rom_st018_device::signal_w));
	map(0x4000'0020, 0x4000'0020).r(FUNC(sns_rom_st018_device::status_r));
	map(0x4000'0020, 0x4000'002f).nopw(); // Unknown write
	map(0xa000'0000, 0xa000'7fff).r(FUNC(sns_rom_st018_device::copro_data_r));
	map(0xe000'0000, 0xe000'3fff).ram();
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sns_rom_st018_device::device_add_mconfig(machine_config &config)
{
	ARM7(config, m_copro, 21440000); // TODO: ARMv3
	m_copro->set_addrmap(AS_PROGRAM, &sns_rom_st018_device::copro_map);

	GENERIC_LATCH_8(config, m_cpu2copro);
	GENERIC_LATCH_8(config, m_copro2cpu);
}

uint8_t sns_rom_st018_device::chip_read(offs_t offset)
{
	uint8_t ret = 0xff;
	switch (offset & 0x06)
	{
		case 0x00:
			ret = m_copro2cpu->read();
			break;
		case 0x02:
			if (!machine().side_effects_disabled())
				m_signal = false;
			break;
		case 0x04:
			ret = status_r();
			break;
	}
	return ret;
}


void sns_rom_st018_device::chip_write(offs_t offset, uint8_t data)
{
	switch (offset & 0x06)
	{
		case 0x02:
			m_cpu2copro->write(data);
			break;
		case 0x04:
			m_copro_reset = BIT(data, 0);
			m_copro->set_input_line(INPUT_LINE_RESET, m_copro_reset ? ASSERT_LINE : CLEAR_LINE);
			break;
	}
}


// To make faster DSP access to its internal rom, let's install read banks and map m_bios there with correct byte order

void sns_rom_st018_device::speedup_addon_bios_access()
{
	m_copro->space(AS_PROGRAM).install_rom(0x0000'0000, 0x0001'ffff, &m_copro_prg[0]);
	m_copro->space(AS_PROGRAM).install_rom(0xa000'0000, 0xa000'7fff, &m_copro_data[0]);
	// copy data in the correct format
	for (int x = 0; x < 0x8000; x++)
		m_copro_prg[x] = m_bios[x * 4] | (m_bios[x * 4 + 1] << 8) | (m_bios[x * 4 + 2] << 16) | (m_bios[x * 4 + 3] << 24);
	for (int x = 0; x < 0x2000; x++)
		m_copro_data[x] = m_bios[0x20000 + x * 4] | (m_bios[0x20000 + x * 4 + 1] << 8) | (m_bios[0x20000 + x * 4 + 2] << 16) | (m_bios[0x20000 + x * 4 + 3] << 24);
}
