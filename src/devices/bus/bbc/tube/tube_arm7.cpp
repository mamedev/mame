// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Sprow ARM7TDMI Co-Processor (2005)

    http://www.sprow.co.uk/bbc/armcopro.htm

**********************************************************************/


#include "emu.h"
#include "tube_arm7.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_ARM7, bbc_tube_arm7_device, "bbc_tube_arm7", "Sprow ARM7TDMI Co-Processor")


//-------------------------------------------------
//  ADDRESS_MAP( arm7_map )
//-------------------------------------------------

void bbc_tube_arm7_device::arm7_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).view(m_bank0_view);
	m_bank0_view[0](0x00000000, 0x0007ffff).rom().region("flash", 0).mirror(0x07f80000);
	m_bank0_view[1](0x00000000, 0x07ffffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0x50000000, 0x50007fff).ram();
	map(0x78000000, 0xbfffffff).rw(FUNC(bbc_tube_arm7_device::oki_reg_r), FUNC(bbc_tube_arm7_device::oki_reg_w));
	map(0xc0000000, 0xc7ffffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0xc8000000, 0xc807ffff).rom().region("flash", 0).mirror(0x07f80000);
	map(0xf0000000, 0xf000000f).rw(m_ula, FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w)).umask16(0x00ff);
	map(0xf0000010, 0xf0000010).lrw8(NAME([this]() { return m_tube10; }), NAME([this](u8 data) { m_tube10 = data; }));
}


//-------------------------------------------------
//  ROM( tube_arm7 )
//-------------------------------------------------

ROM_START( tube_arm7 )
	ROM_REGION32_LE(0x80000, "flash", 0)
	ROM_SYSTEM_BIOS(0, "045", "ARM Tube OS 0.45")
	ROMX_LOAD("atos045.rom", 0x00000, 0x80000, CRC(984c594e) SHA1(1909886361c2a143c02a3977f6ce1a529dfc8779), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "040", "ARM Tube OS 0.40")
	ROMX_LOAD("atos040.rom", 0x00000, 0x80000, CRC(b34b5011) SHA1(babfb5bdb8265cf3ac7feff254146cb2d2773da1), ROM_BIOS(1))
ROM_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_arm7_device::device_add_mconfig(machine_config &config)
{
	ARM7(config, m_maincpu, 64_MHz_XTAL); // Oki ML67Q5003
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_arm7_device::arm7_map);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set(FUNC(bbc_tube_arm7_device::efiq_w));
	m_ula->pirq_handler().set(FUNC(bbc_tube_arm7_device::exint3_w));
	m_ula->prst_handler().set(FUNC(bbc_tube_arm7_device::prst_w));

	RAM(config, m_ram).set_default_size("32M").set_extra_options("16M,64M");
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_arm7_device::device_rom_region() const
{
	return ROM_NAME( tube_arm7 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_arm7_device - constructor
//-------------------------------------------------

bbc_tube_arm7_device::bbc_tube_arm7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_ARM7, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_flash(*this, "flash")
	, m_ula(*this, "ula")
	, m_ram(*this, "ram")
	, m_bank0_view(*this, "bank0")
	, m_tube10(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_arm7_device::device_start()
{
	save_item(NAME(m_registers));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_arm7_device::device_reset()
{
	// interrupt states
	m_efiq_state = 0;
	m_exint3_state = 0;

	// reset registers
	memset(m_registers, 0, sizeof(m_registers));
	update_interrupts();

	// enable rom
	m_registers[CHIP_CONFIG][3] = 1; // ROMSEL
	update_bank0();
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_tube_arm7_device::prst_w(int state)
{
	device_reset();

	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}

uint8_t bbc_tube_arm7_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_arm7_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}


void bbc_tube_arm7_device::update_interrupts()
{
	int firq = BIT(m_registers[INTERRUPT][4], 0) && m_efiq_state;
	int irq = BIT(m_registers[EXP_INTERRUPT][6], 28, 4) && m_exint3_state;

	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, firq ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
}

void bbc_tube_arm7_device::efiq_w(int state)
{
	m_efiq_state = state;
	update_interrupts();
}

void bbc_tube_arm7_device::exint3_w(int state)
{
	m_exint3_state = state;
	if (state)
	{
		m_registers[INTERRUPT][5] = 31; // IRN
	}
	update_interrupts();
}


void bbc_tube_arm7_device::update_bank0()
{
	if (BIT(m_registers[CHIP_CONFIG][3], 0) && !BIT(m_registers[SYSTEM][4], 3))
	{
		m_bank0_view.select(0);
	}
	else
	{
		m_bank0_view.select(1);
	}
}


uint32_t bbc_tube_arm7_device::oki_reg_r(offs_t offset)
{
	uint32_t data = 0x00;
	uint32_t reg_addr = 0x78000000 + (offset * 4);

	switch (reg_addr >> 4)
	{
	case 0x7800000: case 0x7800001: case 0x7800002:
		data = m_registers[INTERRUPT][offset & 0x0f];
		if (reg_addr == 0x78000014) // IRN
		{
			m_registers[INTERRUPT][5] = 0; // IRN
			m_registers[INTERRUPT][6] = 0; // CIL
		}
		update_interrupts();
		break;

	case 0x7810000:
		data = m_registers[EXT_MEMORY][offset & 0x03];
		break;

	case 0x7820000: case 0x7820001:
		data = m_registers[CACHE_MEMORY][offset & 0x07];
		break;

	case 0x7818000: case 0x7818001:
		data = m_registers[DRAM_CONTROL][offset & 0x07];
		break;

	case 0x7bf0000: case 0x7bf0001:
		data = m_registers[EXP_INTERRUPT][offset & 0x07];
		break;

	case 0xb700000:
		data = m_registers[CHIP_CONFIG][offset & 0x03];
		break;

	case 0xb7a0100: case 0xb7a0101: case 0xb7a0102: case 0xb7a0103: case 0xb7a0104: case 0xb7a0105:
		data = m_registers[PORT_CONTROL][offset & 0x1f];
		break;

	case 0xb800000: case 0xb800001:
		data = m_registers[SYSTEM][offset & 0x07];
		break;

	case 0xb800100: case 0xb800101:
		data = m_registers[SYSTEM_TIMER][offset & 0x07];
		break;

	default:
		logerror("oki_reg_r: Unhandled register %08x\n", reg_addr);
		break;
	}

	return data;
}

void bbc_tube_arm7_device::oki_reg_w(offs_t offset, uint32_t data)
{
	uint32_t reg_addr = 0x78000000 + (offset * 4);

	switch (reg_addr >> 4)
	{
	case 0x7800000: case 0x7800001: case 0x7800002:
		m_registers[INTERRUPT][offset & 0x0f] = data;
		if (reg_addr == 0x78000028) // CILCL
		{
			m_registers[INTERRUPT][6] = 0; // CIL
		}
		update_interrupts();
		break;

	case 0x7810000:
		m_registers[EXT_MEMORY][offset & 0x03] = data;
		break;

	case 0x7820000: case 0x7820001:
		m_registers[CACHE_MEMORY][offset & 0x07] = data;
		break;

	case 0x7818000: case 0x7818001:
		m_registers[DRAM_CONTROL][offset & 0x07] = data;
		break;

	case 0x7bf0000: case 0x7bf0001:
		m_registers[EXP_INTERRUPT][offset & 0x07] = data;
		update_interrupts();
		break;

	case 0xb700000:
		m_registers[CHIP_CONFIG][offset & 0x03] = data;
		update_bank0();
		break;

	case 0xb7a0100: case 0xb7a0101: case 0xb7a0102: case 0xb7a0103: case 0xb7a0104: case 0xb7a0105:
		m_registers[PORT_CONTROL][offset & 0x1f] = data;
		break;

	case 0xb800000: case 0xb800001:
		m_registers[SYSTEM][offset & 0x07] = data;
		update_bank0();
		break;

	case 0xb800100: case 0xb800101:
		m_registers[SYSTEM_TIMER][offset & 0x07] = data;
		break;

	default:
		logerror("oki_reg_w: Unhandled register %08x = %04x\n", reg_addr, data);
		break;
	}
}
