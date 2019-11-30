// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC04 Z80 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC04_Z802ndproc.html

**********************************************************************/


#include "emu.h"
#include "tube_z80.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_Z80, bbc_tube_z80_device, "bbc_tube_z80", "Acorn Z80 2nd Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_z80_mem )
//-------------------------------------------------

void bbc_tube_z80_device::tube_z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(bbc_tube_z80_device::mem_r), FUNC(bbc_tube_z80_device::mem_w));
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_z80_fetch )
//-------------------------------------------------

void bbc_tube_z80_device::tube_z80_fetch(address_map &map)
{
	map(0x000, 0xffff).r(FUNC(bbc_tube_z80_device::opcode_r));
}

//-------------------------------------------------
//  ADDRESS_MAP( tube_z80_io )
//-------------------------------------------------

void bbc_tube_z80_device::tube_z80_io(address_map &map)
{
	map(0x00, 0x07).mirror(0xff00).rw("ula", FUNC(tube_device::parasite_r), FUNC(tube_device::parasite_w));
}

//-------------------------------------------------
//  ROM( tube_z80 )
//-------------------------------------------------

ROM_START( tube_z80 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("z80_120.rom", 0x0000, 0x1000, CRC(315bfc20) SHA1(069077df498599a9c880d4ec9f4bc53fcc602d82))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_z80_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 12_MHz_XTAL / 2);
	m_z80->set_addrmap(AS_PROGRAM, &bbc_tube_z80_device::tube_z80_mem);
	m_z80->set_addrmap(AS_OPCODES, &bbc_tube_z80_device::tube_z80_fetch);
	m_z80->set_addrmap(AS_IO, &bbc_tube_z80_device::tube_z80_io);
	m_z80->set_irq_acknowledge_callback(FUNC(bbc_tube_z80_device::irq_callback));

	TUBE(config, m_ula);
	m_ula->hirq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_tube_slot_device::irq_w));
	m_ula->pnmi_handler().set(FUNC(bbc_tube_z80_device::nmi_w));
	m_ula->pirq_handler().set_inputline(m_z80, INPUT_LINE_IRQ0);

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K").set_default_value(0);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_z80").set_original("bbc_flop_z80");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_z80_device::device_rom_region() const
{
	return ROM_NAME( tube_z80 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_z80_device - constructor
//-------------------------------------------------

bbc_tube_z80_device::bbc_tube_z80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_TUBE_Z80, tag, owner, clock),
		device_bbc_tube_interface(mconfig, *this),
		m_z80(*this, "z80"),
		m_ula(*this, "ula"),
		m_ram(*this, "ram"),
		m_rom(*this, "rom"),
		m_rom_enabled(true)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_z80_device::device_start()
{
	m_slot = dynamic_cast<bbc_tube_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_z80_device::device_reset()
{
	m_ula->reset();

	m_rom_enabled = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_z80_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_z80_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}


uint8_t bbc_tube_z80_device::opcode_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset == 0x0066 && m_z80->input_state(INPUT_LINE_NMI))
			m_rom_enabled = true;
		else if (offset >= 0x8000)
			m_rom_enabled = false;
	}
	return m_z80->space(AS_PROGRAM).read_byte(offset);
}


uint8_t bbc_tube_z80_device::mem_r(offs_t offset)
{
	uint8_t data;

	if (m_rom_enabled && (offset < 0x1000))
		data = m_rom->base()[offset & 0xfff];
	else
		data = m_ram->pointer()[offset];

	return data;
}

void bbc_tube_z80_device::mem_w(offs_t offset, uint8_t data)
{
	m_ram->pointer()[offset] = data;
}

WRITE_LINE_MEMBER(bbc_tube_z80_device::nmi_w)
{
	m_z80->set_input_line(INPUT_LINE_NMI, state);
}


//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER(bbc_tube_z80_device::irq_callback)
{
	return 0xfe;
}
