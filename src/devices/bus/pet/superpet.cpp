// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore SuperPET emulation

**********************************************************************/

#include "emu.h"
#include "superpet.h"

#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define M6809_TAG       "u4"
#define MOS6551_TAG     "u23"
#define MOS6702_TAG     "u2"
#define RS232_TAG       "rs232"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SUPERPET, superpet_device, "pet_superpet", "Commodore SuperPET")


//-------------------------------------------------
//  ROM( superpet )
//-------------------------------------------------

ROM_START( superpet )
	ROM_REGION( 0x7000, M6809_TAG, 0 )
	ROM_LOAD( "901898-01.u17", 0x1000, 0x1000, CRC(728a998b) SHA1(0414b3ab847c8977eb05c2fcc72efcf2f9d92871) )
	ROM_LOAD( "901898-02.u18", 0x2000, 0x1000, CRC(6beb7c62) SHA1(df154939b934d0aeeb376813ec1ba0d43c2a3378) )
	ROM_LOAD( "901898-03.u19", 0x3000, 0x1000, CRC(5db4983d) SHA1(6c5b0cce97068f8841112ba6d5cd8e568b562fa3) )
	ROM_LOAD( "901898-04.u20", 0x4000, 0x1000, CRC(f55fc559) SHA1(b42a2050a319a1ffca7868a8d8d635fadd37ec37) )
	ROM_LOAD( "901897-01.u21", 0x5000, 0x0800, CRC(b2cee903) SHA1(e8ce8347451a001214a5e71a13081b38b4be23bc) )
	ROM_LOAD( "901898-05.u22", 0x6000, 0x1000, CRC(f42df0cb) SHA1(9b4a5134d20345171e7303445f87c4e0b9addc96) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *superpet_device::device_rom_region() const
{
	return ROM_NAME( superpet );
}


//-------------------------------------------------
//  ADDRESS_MAP( superpet_mem )
//-------------------------------------------------

void superpet_device::superpet_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(superpet_device::read), FUNC(superpet_device::write));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void superpet_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_maincpu, 16_MHz_XTAL / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &superpet_device::superpet_mem);

	MOS6702(config, m_dongle, 16_MHz_XTAL / 16);

	MOS6551(config, m_acia);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->irq_handler().set(FUNC(superpet_device::acia_irq_w));
	m_acia->txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));
}


//-------------------------------------------------
//  INPUT_PORTS( superpet )
//-------------------------------------------------

static INPUT_PORTS_START( superpet )
	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x01, "RAM" )
	PORT_DIPSETTING(    0x00, "Read Only" )
	PORT_DIPSETTING(    0x01, "Read/Write" )
	PORT_DIPSETTING(    0x02, "System Port" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x03, 0x02, "CPU" )
	PORT_DIPSETTING(    0x00, "6809" )
	PORT_DIPSETTING(    0x01, "6502" )
	PORT_DIPSETTING(    0x02, "System Port" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor superpet_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( superpet );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_cpu -
//-------------------------------------------------

inline void superpet_device::update_cpu()
{
	bool active = is_6809_active();
	update_window();

	m_slot->halt_w(active ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(INPUT_LINE_HALT, active ? CLEAR_LINE : ASSERT_LINE);

	if (active != m_6809_active)
	{
		m_6809_active = active;

		if (active)
		{
			m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		}
		else
		{
			m_slot->reset_w(ASSERT_LINE);
			m_slot->reset_w(CLEAR_LINE);
		}
	}
}


//-------------------------------------------------
//  is_6809_active -
//-------------------------------------------------

inline bool superpet_device::is_6809_active()
{
	return !((m_sw2 == 2) ? BIT(m_system, 0) : m_sw2);
}


//-------------------------------------------------
//  is_ram_writable -
//-------------------------------------------------

inline bool superpet_device::is_ram_writable()
{
	return (m_sw1 == 2) ? BIT(m_system, 1) : m_sw1;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  superpet_device - constructor
//-------------------------------------------------

superpet_device::superpet_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SUPERPET, tag, owner, clock),
	device_pet_expansion_card_interface(mconfig, *this),
	m_maincpu(*this, M6809_TAG),
	m_acia(*this, MOS6551_TAG),
	m_dongle(*this, MOS6702_TAG),
	m_rom(*this, M6809_TAG),
	m_ram(*this, "ram", 0x10000, ENDIANNESS_LITTLE),
	m_io_sw1(*this, "SW1"),
	m_io_sw2(*this, "SW2"),
	m_system(0),
	m_bank(0), m_sw1(0), m_sw2(0),
	m_sel9_rom(0),
	m_6809_active(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void superpet_device::device_start()
{
	// state saving
	save_item(NAME(m_system));
	save_item(NAME(m_bank));
	save_item(NAME(m_sw1));
	save_item(NAME(m_sw2));
	save_item(NAME(m_sel9_rom));
	save_item(NAME(m_6809_active));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void superpet_device::device_reset()
{
	m_maincpu->reset();
	m_acia->reset();
	m_dongle->reset();

	m_system = 0;
	m_bank = 0;
	m_sel9_rom = 0;
	m_sw1 = m_io_sw1->read();
	m_sw2 = m_io_sw2->read();

	m_6809_active = is_6809_active();
	update_cpu();
}


void superpet_device::device_post_load()
{
	update_window();
}

void superpet_device::update_window()
{
	memory_view &window = m_slot->window();
	for (int slot = 0; slot < 2; slot++)
	{
		auto &view = window[slot];
		uint8_t *const bank = &m_ram[0] + ((m_bank & 0x0f) << 12);

		if (m_sel9_rom)
		{
			view.install_rom(0x9000, 0x9fff, m_rom->base());
			view.nop_write(0x9000, 0x9fff);
		}
		else if (is_ram_writable())
			view.install_ram(0x9000, 0x9fff, bank);
		else
		{
			view.install_rom(0x9000, 0x9fff, bank);
			view.nop_write(0x9000, 0x9fff);
		}

		view.nop_readwrite(0xef00, 0xefff);
		view.install_readwrite_handler(0xefe0, 0xefe3,
			read8sm_delegate(*m_dongle, FUNC(mos6702_device::read)),
			write8sm_delegate(*m_dongle, FUNC(mos6702_device::write)));
		view.install_readwrite_handler(0xeff0, 0xeff3,
			read8sm_delegate(*m_acia, FUNC(mos6551_device::read)),
			write8sm_delegate(*m_acia, FUNC(mos6551_device::write)));
		view.install_write_handler(0xeff8, 0xeffb, write8smo_delegate(*this, FUNC(superpet_device::system_w)));
		view.install_write_handler(0xeffc, 0xefff, write8smo_delegate(*this, FUNC(superpet_device::bank_w)));
	}

	auto &rom = window[1];
	rom.install_rom(0xa000, 0xe7ff, m_rom->base() + 0x1000);
	rom.install_rom(0xf000, 0xffff, m_rom->base() + 0x6000);
	window.select(is_6809_active() ? 1 : 0);
}

void superpet_device::system_w(uint8_t data)
{
	if (BIT(m_bank, 7))
	{
		m_system = data;
		update_cpu();
		logerror("SYSTEM %02x\n", data);
	}
}

void superpet_device::bank_w(uint8_t data)
{
	m_bank = data;
	update_window();
	logerror("BANK %02x\n", data);
}


//-------------------------------------------------
//  pet_diag_r - DIAG read
//-------------------------------------------------

int superpet_device::pet_diag_r()
{
	return !BIT(m_system, 3);
}


//-------------------------------------------------
//  pet_irq_w - IRQ write
//-------------------------------------------------

void superpet_device::pet_irq_w(int state)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, state);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t superpet_device::read(offs_t offset)
{
	return m_slot->dma_bd_r(offset);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void superpet_device::write(offs_t offset, uint8_t data)
{
	m_slot->dma_bd_w(offset, data);
}


//-------------------------------------------------
//  acia_irq_w -
//-------------------------------------------------

void superpet_device::acia_irq_w(int state)
{
	m_slot->card_irq_w(state);
}
