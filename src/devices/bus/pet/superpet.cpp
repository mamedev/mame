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
	map(0x0000, 0xffff).rw(this, FUNC(superpet_device::read), FUNC(superpet_device::write));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(superpet_device::device_add_mconfig)
	MCFG_DEVICE_ADD(M6809_TAG, M6809, XTAL(16'000'000)/16)
	MCFG_DEVICE_PROGRAM_MAP(superpet_mem)

	MCFG_MOS6702_ADD(MOS6702_TAG, XTAL(16'000'000)/16)

	MCFG_DEVICE_ADD(MOS6551_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(XTAL(1'843'200))
	MCFG_MOS6551_IRQ_HANDLER(WRITELINE(*this, superpet_device, acia_irq_w))
	MCFG_MOS6551_TXD_HANDLER(WRITELINE(RS232_TAG, rs232_port_device, write_txd))

	MCFG_DEVICE_ADD(RS232_TAG, RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(MOS6551_TAG, mos6551_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE(MOS6551_TAG, mos6551_device, write_dcd))
	MCFG_RS232_DSR_HANDLER(WRITELINE(MOS6551_TAG, mos6551_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(MOS6551_TAG, mos6551_device, write_cts))
MACHINE_CONFIG_END


//-------------------------------------------------
//  INPUT_PORTS( superpet )
//-------------------------------------------------

static INPUT_PORTS_START( superpet )
	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x02, "RAM" )
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
	int cpu = (m_sw2 == 2) ? BIT(m_system, 0) : m_sw2;

	if (cpu)
	{
		// 6502 active
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
	else
	{
		// 6809 active
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
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
	m_ram(*this, "ram"),
	m_io_sw1(*this, "SW1"),
	m_io_sw2(*this, "SW2"),
	m_system(0),
	m_bank(0), m_sw1(0), m_sw2(0),
	m_sel9_rom(0),
	m_pet_irq(CLEAR_LINE),
	m_acia_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void superpet_device::device_start()
{
	// allocate memory
	m_ram.allocate(0x10000);

	// state saving
	save_item(NAME(m_system));
	save_item(NAME(m_bank));
	save_item(NAME(m_sel9_rom));
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

	update_cpu();
}


//-------------------------------------------------
//  pet_norom_r - NO ROM read
//-------------------------------------------------

int superpet_device::pet_norom_r(address_space &space, offs_t offset, int sel)
{
	return BIT(m_system, 0);
}


//-------------------------------------------------
//  pet_bd_r - buffered data read
//-------------------------------------------------

uint8_t superpet_device::pet_bd_r(address_space &space, offs_t offset, uint8_t data, int &sel)
{
	int norom = pet_norom_r(space, offset, sel);

	switch (sel)
	{
	case pet_expansion_slot_device::SEL9:
		if (m_sel9_rom)
		{
			data = m_rom->base()[offset - 0x9000];
		}
		else
		{
			data = m_ram[((m_bank & 0x0f) << 12) | (offset & 0xfff)];
		}
		break;

	case pet_expansion_slot_device::SELA:
	case pet_expansion_slot_device::SELB:
	case pet_expansion_slot_device::SELC:
	case pet_expansion_slot_device::SELD:
	case pet_expansion_slot_device::SELF:
		if (!norom)
		{
			data = m_rom->base()[offset - 0x9000];
		}
		break;

	case pet_expansion_slot_device::SELE:
		if (!norom && !BIT(offset, 11))
		{
			data = m_rom->base()[offset - 0x9000];
		}
		break;
	}

	switch (offset)
	{
	case 0xefe0:
	case 0xefe1:
	case 0xefe2:
	case 0xefe3:
		data = m_dongle->read(space, offset & 0x03);
		break;

	case 0xeff0:
	case 0xeff1:
	case 0xeff2:
	case 0xeff3:
		data = m_acia->read(space, offset & 0x03);
		break;
	}

	return data;
}


//-------------------------------------------------
//  pet_bd_w - buffered data write
//-------------------------------------------------

void superpet_device::pet_bd_w(address_space &space, offs_t offset, uint8_t data, int &sel)
{
	switch (sel)
	{
	case pet_expansion_slot_device::SEL9:
		if (!m_sel9_rom && is_ram_writable())
		{
			m_ram[((m_bank & 0x0f) << 12) | (offset & 0xfff)] = data;
		}
		break;
	}

	switch (offset)
	{
	case 0xefe0:
	case 0xefe1:
	case 0xefe2:
	case 0xefe3:
		m_dongle->write(space, offset & 0x03, data);
		printf("6702 %u %02x\n", offset & 0x03, data);
		break;

	case 0xeff0:
	case 0xeff1:
	case 0xeff2:
	case 0xeff3:
		m_acia->write(space, offset & 0x03, data);
		break;

	case 0xeff8:
	case 0xeff9:
		if (BIT(m_bank, 7))
		{
			/*

			    bit     description

			    0       SW2 CPU (0=6809, 1=6502)
			    1       SW1 RAM (0=read only, 1=read/write)
			    2
			    3       DIAG
			    4
			    5
			    6
			    7

			*/

			m_system = data;
			update_cpu();
			printf("SYSTEM %02x\n", data);
		}
		break;

	case 0xeffc:
	case 0xeffd:
		/*

		    bit     description

		    0       A0
		    1       A1
		    2       A2
		    3       SEL A
		    4       J1 pin 40
		    5       SEL B
		    6       J1 pin 39
		    7       BIT 7

		*/

		m_bank = data;
		printf("BANK %02x\n", data);
		break;
	}
}


//-------------------------------------------------
//  pet_diag_r - DIAG read
//-------------------------------------------------

int superpet_device::pet_diag_r()
{
	return BIT(m_system, 3);
}


//-------------------------------------------------
//  pet_irq_w - IRQ write
//-------------------------------------------------

void superpet_device::pet_irq_w(int state)
{
	m_pet_irq = state;

	//m_maincpu->set_input_line(M6809_IRQ_LINE, m_pet_irq || m_acia_irq);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( superpet_device::read )
{
	return m_slot->dma_bd_r(offset);
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( superpet_device::write )
{
	m_slot->dma_bd_w(offset, data);
}


//-------------------------------------------------
//  acia_irq_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( superpet_device::acia_irq_w )
{
	m_acia_irq = state;

	//m_maincpu->set_input_line(M6809_IRQ_LINE, m_pet_irq || m_acia_irq);
}
