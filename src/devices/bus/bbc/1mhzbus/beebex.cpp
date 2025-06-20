// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BEEBEX Extender for BBC Micro

    - CUE2701 Beebex with 4 integral sockets for stand alone use
    - CUE2702 Beebex for rack mounting (16 sockets)

**********************************************************************/

#include "emu.h"
#include "beebex.h"

#include "bus/acorn/bus.h"


namespace {

class bbc_beebex_device
	: public device_t
	, public device_memory_interface
	, public device_bbc_1mhzbus_interface
{
public:
	bbc_beebex_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, BBC_BEEBEX, tag, owner, clock)
		, device_memory_interface(mconfig, *this)
		, device_bbc_1mhzbus_interface(mconfig, *this)
		, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0, address_map_constructor(FUNC(bbc_beebex_device::beebex_map), this))
		, m_bus(*this, "bus%u", 0U)
		, m_latch(0)
	{ }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void fred_w(offs_t offset, uint8_t data) override;
	virtual uint8_t jim_r(offs_t offset) override;
	virtual void jim_w(offs_t offset, uint8_t data) override;

private:
	address_space_config m_program_config;
	required_device_array<acorn_bus_device, 4> m_bus;
	uint16_t m_latch;

	address_space *m_program;

	void beebex_map(address_map &map) ATTR_COLD;
};


device_memory_interface::space_config_vector bbc_beebex_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_program_config) };
}


void bbc_beebex_device::beebex_map(address_map &map)
{
	map(0x00000, 0x0ffff).rw(m_bus[0], FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x10000, 0x1ffff).rw(m_bus[1], FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x20000, 0x2ffff).rw(m_bus[2], FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
	map(0x30000, 0x3ffff).rw(m_bus[3], FUNC(acorn_bus_device::read), FUNC(acorn_bus_device::write));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_beebex_device::device_add_mconfig(machine_config &config)
{
	ACORN_BUS(config, m_bus[0], DERIVED_CLOCK(1, 1));
	m_bus[0]->out_irq_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_bus[0]->out_nmi_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
	ACORN_BUS_SLOT(config, "slot1", m_bus[0], eurocube_bus_devices, "cubio_r");

	ACORN_BUS(config, m_bus[1], DERIVED_CLOCK(1, 1));
	m_bus[1]->out_irq_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_bus[1]->out_nmi_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
	ACORN_BUS_SLOT(config, "slot2", m_bus[1], eurocube_bus_devices, nullptr);

	ACORN_BUS(config, m_bus[2], DERIVED_CLOCK(1, 1));
	m_bus[2]->out_irq_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_bus[2]->out_nmi_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
	ACORN_BUS_SLOT(config, "slot3", m_bus[2], eurocube_bus_devices, nullptr);

	ACORN_BUS(config, m_bus[3], DERIVED_CLOCK(1, 1));
	m_bus[3]->out_irq_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::irq_w));
	m_bus[3]->out_nmi_callback().set(DEVICE_SELF_OWNER, FUNC(bbc_1mhzbus_slot_device::nmi_w));
	ACORN_BUS_SLOT(config, "slot4", m_bus[3], eurocube_bus_devices, nullptr);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_beebex_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_latch));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_beebex_device::device_reset()
{
	m_latch = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_beebex_device::fred_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xfe:
		m_latch = (m_latch & 0x00ff) | (data << 8);
		break;
	case 0xff:
		m_latch = (m_latch & 0x0f00) | data;
		break;
	}
}

uint8_t bbc_beebex_device::jim_r(offs_t offset)
{
	return m_program->read_byte((m_latch << 8) | offset);
}

void bbc_beebex_device::jim_w(offs_t offset, uint8_t data)
{
	m_program->write_byte((m_latch << 8) | offset, data);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(BBC_BEEBEX, device_bbc_1mhzbus_interface, bbc_beebex_device, "bbc_beebex", "BEEBEX Extender for BBC Micro")
