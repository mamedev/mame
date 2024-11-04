// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    romcard.cpp

    Implemention of the Apple II ROM card.  This is like a language
    card, but with 12K instead of 16K, and ROM instead of RAM.

    Apple at various points called it both "ROM Card" and
    "Firmware Card",

*********************************************************************/

#include "emu.h"
#include "a2bus.h"
#include "romcard.h"
#include <errno.h>

namespace {

class a2bus_romcard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	a2bus_romcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void bus_reset() override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_inh_rom(uint16_t offset) override;
	virtual void write_inh_rom(uint16_t offset, uint8_t data) override;
	virtual uint16_t inh_start() override { return 0xd000; }
	virtual uint16_t inh_end() override { return 0xffff; }
	virtual int inh_type() override;

protected:
	u8 *m_rom;
	required_ioport m_config;

private:
	void do_io(int offset);

	int m_inh_state;
};

class a2bus_romcardfp_device: public a2bus_romcard_device
{
public:
	a2bus_romcardfp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class a2bus_romcardint_device: public a2bus_romcard_device
{
public:
	a2bus_romcardint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class a2bus_romcarduser_device: public a2bus_romcard_device, public device_image_interface
{
public:
	a2bus_romcarduser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface overrides
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }
	virtual const char *image_type_name() const noexcept override { return "romimage"; }
	virtual const char *image_brief_type_name() const noexcept override { return "rom"; }

protected:
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

ROM_START( romcardfp )
	ROM_REGION(0x3000, "romcard", 0)
	ROM_LOAD ( "341-0011.d0", 0x0000, 0x0800, CRC(6f05f949) SHA1(0287ebcef2c1ce11dc71be15a99d2d7e0e128b1e))
	ROM_LOAD ( "341-0012.d8", 0x0800, 0x0800, CRC(1f08087c) SHA1(a75ce5aab6401355bf1ab01b04e4946a424879b5))
	ROM_LOAD ( "341-0013.e0", 0x1000, 0x0800, CRC(2b8d9a89) SHA1(8d82a1da63224859bd619005fab62c4714b25dd7))
	ROM_LOAD ( "341-0014.e8", 0x1800, 0x0800, CRC(5719871a) SHA1(37501be96d36d041667c15d63e0c1eff2f7dd4e9))
	ROM_LOAD ( "341-0015.f0", 0x2000, 0x0800, CRC(9a04eecf) SHA1(e6bf91ed28464f42b807f798fc6422e5948bf581))
	ROM_LOAD ( "341-0020-00.f8", 0x2800, 0x0800, CRC(079589c4) SHA1(a28852ff997b4790e53d8d0352112c4b1a395098))
ROM_END

ROM_START( romcardint )
	/* Integer ROM card: Integer BASIC, the old Monitor, and Programmer's Aid #1 */
	ROM_REGION(0x3000, "romcard", 0)
	ROM_LOAD ( "341-0016-00.d0", 0x0000, 0x0800, CRC(4234e88a) SHA1(c9a81d704dc2f0c3416c20f9c4ab71fedda937ed))
	ROM_LOAD ( "341-0001-00.e0", 0x1000, 0x0800, CRC(c0a4ad3b) SHA1(bf32195efcb34b694c893c2d342321ec3a24b98f))
	ROM_LOAD ( "341-0002-00.e8", 0x1800, 0x0800, CRC(a99c2cf6) SHA1(9767d92d04fc65c626223f25564cca31f5248980))
	ROM_LOAD ( "341-0003-00.f0", 0x2000, 0x0800, CRC(62230d38) SHA1(f268022da555e4c809ca1ae9e5d2f00b388ff61c))
	ROM_LOAD ( "341-0004-00.f8", 0x2800, 0x0800, CRC(020a86d0) SHA1(52a18bd578a4694420009cad7a7a5779a8c00226))
ROM_END

static INPUT_PORTS_START( romcard )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Enable at boot")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
INPUT_PORTS_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_romcard_device::a2bus_romcard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_rom(nullptr),
	m_config(*this, "CONFIG"),
	m_inh_state(0)
{
}

a2bus_romcardfp_device::a2bus_romcardfp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_romcard_device(mconfig, A2BUS_ROMCARDFP, tag, owner, clock)
{
}

a2bus_romcardint_device::a2bus_romcardint_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_romcard_device(mconfig, A2BUS_ROMCARDINT, tag, owner, clock)
{
}

a2bus_romcarduser_device::a2bus_romcarduser_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_romcard_device(mconfig, A2BUS_ROMCARDUSER, tag, owner, clock),
	device_image_interface(mconfig, *this)
{
}

const tiny_rom_entry *a2bus_romcardfp_device::device_rom_region() const
{
	return ROM_NAME( romcardfp );
}

const tiny_rom_entry *a2bus_romcardint_device::device_rom_region() const
{
	return ROM_NAME( romcardint );
}

ioport_constructor a2bus_romcard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( romcard );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_romcard_device::device_start()
{
	save_item(NAME(m_inh_state));
}

void a2bus_romcardfp_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag("romcard"))->base();
	a2bus_romcard_device::device_start();
}

void a2bus_romcardint_device::device_start()
{
	m_rom = device().machine().root_device().memregion(this->subtag("romcard"))->base();
	a2bus_romcard_device::device_start();
}

void a2bus_romcard_device::bus_reset()
{
	if ((m_config->read() == 1) && (m_rom != nullptr))
	{
		m_inh_state = INH_READ;
	}
	else
	{
		m_inh_state = 0;
	}
	recalc_slot_inh();
}

void a2bus_romcard_device::do_io(int offset)
{
	int old_inh_state = m_inh_state;

	// any even access enables ROM reading
	if (((offset & 1) == 1) && (m_rom != nullptr))
	{
		m_inh_state |= INH_READ;
	}
	else
	{
		m_inh_state &= ~INH_READ;
	}

	if (m_inh_state != old_inh_state)
	{
		recalc_slot_inh();
	}
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_romcard_device::read_c0nx(uint8_t offset)
{
	do_io(offset & 0xf);
	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_romcard_device::write_c0nx(uint8_t offset, uint8_t data)
{
	do_io(offset & 0xf);
}

uint8_t a2bus_romcard_device::read_inh_rom(uint16_t offset)
{
	assert(m_inh_state & INH_READ); // this should never happen
	return m_rom[offset-0xd000];
}

void a2bus_romcard_device::write_inh_rom(uint16_t offset, uint8_t data)
{
}

int a2bus_romcard_device::inh_type()
{
	return m_inh_state;
}

std::pair<std::error_condition, std::string> a2bus_romcarduser_device::call_load()
{
	if (is_open())
	{
		m_rom = (u8 *)malloc(12*1024*1024);
		fread(m_rom, 12*1024*1024);
	}
	else
	{
		return std::make_pair(std::error_condition(ENOENT, std::generic_category()), std::string());
	}
	return std::make_pair(std::error_condition(), std::string());
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ROMCARDUSER, device_a2bus_card_interface, a2bus_romcarduser_device, "a2romusr", "Apple II ROM Card (Custom)")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ROMCARDFP, device_a2bus_card_interface, a2bus_romcardfp_device, "a2romfp", "Apple II ROM Card (Applesoft BASIC)")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ROMCARDINT, device_a2bus_card_interface, a2bus_romcardint_device, "a2romint", "Apple II ROM Card (Integer BASIC)")
