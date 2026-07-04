// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2applicard.cpp

    Implementation of the Appli-Card Z-80 card from Personal Computer
    Products, Inc. (PCPI)

    Unlike the SoftCard and clones, this has its own 64k of RAM on board
    and the Z80 runs completely independently of the host's 6502. It
    also has a socket for a Z80 CTC, but this is unpopulated.

*********************************************************************/

#include "emu.h"
#include "a2applicard.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

ROM_START( a2applicard )
	ROM_REGION(0x800, "z80_rom", 0)
	ROM_LOAD( "applicard-v9.bin", 0x000000, 0x000800, CRC(1d461000) SHA1(71d633be864b6084362e85108a4e600cbe6e44fe) )
ROM_END

ROM_START( ace80 )
	ROM_REGION(0x800, "z80_rom", 0)
	ROM_LOAD( "1331414.u12", 0x000, 0x800, CRC(7ecb5c6d) SHA1(8e9e9f12b78d545ccd90f5e30b6804ceeb6c939f) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_applicard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_applicard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t z80_io_r(offs_t offset);
	void z80_io_w(offs_t offset, uint8_t data);

protected:
	a2bus_applicard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual void reset_from_bus() override;

private:
	required_device<cpu_device> m_z80;
	memory_view m_z80rom;

	bool m_z80stat, m_6502stat;
	uint8_t m_toz80, m_to6502;

	void z80_io(address_map &map) ATTR_COLD;
	void z80_mem(address_map &map) ATTR_COLD;
};

class a2bus_ace80_device : public a2bus_applicard_device
{
public:
	a2bus_ace80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void a2bus_applicard_device::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).ram();
	map(0x0000, 0x7fff).view(m_z80rom);
	m_z80rom[0](0x0000, 0x07ff).mirror(0x7800).rom().region("z80_rom", 0).nopw(); // // writing only works if ROM not mapped from 0-7fff
}

void a2bus_applicard_device::z80_io(address_map &map)
{
	map(0x00, 0x60).mirror(0xff00).rw(FUNC(a2bus_applicard_device::z80_io_r), FUNC(a2bus_applicard_device::z80_io_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_applicard_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 12_MHz_XTAL / 2); // Z80B CPU runs at 6 MHz
	m_z80->set_addrmap(AS_PROGRAM, &a2bus_applicard_device::z80_mem);
	m_z80->set_addrmap(AS_IO, &a2bus_applicard_device::z80_io);

	config.set_maximum_quantum(attotime::from_usec(40)); // CP/M boot takes unbearably long without this
}

//-------------------------------------------------
//  device_rom_region - device-specific ROMs
//-------------------------------------------------

const tiny_rom_entry *a2bus_applicard_device::device_rom_region() const
{
	return ROM_NAME( a2applicard );
}

const tiny_rom_entry *a2bus_ace80_device::device_rom_region() const
{
	return ROM_NAME( ace80 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_applicard_device::a2bus_applicard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_z80(*this, "z80"),
	m_z80rom(*this, "z80_rom"),
	m_z80stat(false), m_6502stat(false), m_toz80(0), m_to6502(0)
{
}

a2bus_applicard_device::a2bus_applicard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_applicard_device(mconfig, A2BUS_APPLICARD, tag, owner, clock)
{
}

a2bus_ace80_device::a2bus_ace80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_applicard_device(mconfig, A2BUS_ACE80, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_applicard_device::device_start()
{
	save_item(NAME(m_z80stat));
	save_item(NAME(m_6502stat));
	save_item(NAME(m_toz80));
	save_item(NAME(m_to6502));
}

void a2bus_applicard_device::device_reset()
{
	reset_from_bus();
}

void a2bus_applicard_device::reset_from_bus()
{
	m_z80rom.select(0);
	m_z80stat = false;
	m_z80->reset();
}

uint8_t a2bus_applicard_device::read_c0nx(uint8_t offset)
{
	switch (offset & 0xf)
	{
		case 0:
			m_6502stat = false;
			return m_to6502;

		case 1:
			return m_toz80;

		case 2:
			if (m_z80stat)
			{
				return 0x80;
			}
			return false;

		case 3:
			if (m_6502stat)
			{
				return 0x80;
			}
			return false;

		case 5:
			m_z80rom.select(0);
			m_toz80 = false;
			m_to6502 = false;
			m_z80->reset();
			break;

		case 6: // IRQ on Z80 via CTC channel 3 (CP/M doesn't use the CTC or IRQs)
			fatalerror("Applicard: Z80 IRQ not supported yet\n");

		case 7: // NMI on Z80 (direct)
			m_z80->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			break;

	}
	return 0xff;
}

void a2bus_applicard_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset & 0xf)
	{
		case 0: // are these legal to write?
		case 2:
		case 3:
			break;

		case 1:
			m_z80stat = true;
			m_toz80 = data;
			break;

		case 5:
		case 6:
		case 7:
			read_c0nx(offset);   // let the read handler take care of these
			break;
	}
}

uint8_t a2bus_applicard_device::z80_io_r(offs_t offset)
{
	uint8_t tmp = 0;

	switch (offset)
	{
		case 0:
			return m_to6502;

		case 0x20:
			m_z80stat = false;
			return m_toz80;

		case 0x40:
			if (m_z80stat)
			{
				tmp |= 0x80;
			}
			if (m_6502stat)
			{
				tmp |= 1;
			}
			return tmp;

		case 0x60:
			break;
	}
	return 0xff;
}

void a2bus_applicard_device::z80_io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_to6502 = data;
			m_6502stat = true;
			break;

		case 0x60:
			if (data & 1)
			{
				m_z80rom.select(0);
			}
			else
			{
				m_z80rom.disable();
			}
			break;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_APPLICARD, device_a2bus_card_interface, a2bus_applicard_device, "a2aplcrd", "PCPI Appli-Card")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ACE80, device_a2bus_card_interface, a2bus_ace80_device, "ace80", "Franklin ACE 80 CPU Card")
