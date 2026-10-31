// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Kres DSM-240 Dual Speed Module

  Port 0x74
    bit 2 - CPU at 4 MHz
    bit 3 - bus clock at the CPU speed, otherwise 2 MHz

  Jumpers
    JJ1 - wait states added to each I/O operation
    JJ2 - bus speed at power up and reset
    JJ3 - CPU speed at power up and reset

  The device clock is the CPU board's 2 MHz clock.

  TODO
    - JJ1 wait states.

****************************************************************************/

#include "emu.h"

#include "dsm240.h"

#define LOG_FUNC (1U << 1) // Function calls

//#define VERBOSE (LOG_GENERAL | LOG_FUNC)

#include "logmacro.h"

#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

namespace {

class dsm240_device : public device_t, public device_h89bus_cpu_board_mod_interface
{
public:
	dsm240_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

	void update_speeds();

	bool m_installed;

	bool m_add_wait_state;
	bool m_cpu_fast;
	bool m_bus_cpu;

	required_ioport m_jj1;
	required_ioport m_jj2;
	required_ioport m_jj3;

	static constexpr u8 DSM240_ADDR_LO = 0x74;
	static constexpr u8 DSM240_ADDR_HI = DSM240_ADDR_LO;

	// 4 MHz is twice the CPU board's 2 MHz clock
	static constexpr u32 MULTIPLIER_4MHZ = 2;
};

static INPUT_PORTS_START( dsm240 )

	PORT_START("JJ1")
	PORT_DIPNAME(0x01, 0x00, "I/O Wait States")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "2")

	PORT_START("JJ2")
	PORT_DIPNAME(0x01, 0x00, "Bus Speed")
	PORT_DIPSETTING(0x00, "2 MHz")
	PORT_DIPSETTING(0x01, "Same as Processor")

	PORT_START("JJ3")
	PORT_DIPNAME(0x01, 0x00, "Processor Speed")
	PORT_DIPSETTING(0x00, "2 MHz")
	PORT_DIPSETTING(0x01, "4 MHz")

INPUT_PORTS_END

dsm240_device::dsm240_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H89BUS_DSM240, tag, owner, clock)
	, device_h89bus_cpu_board_mod_interface(mconfig, *this)
	, m_jj1(*this, "JJ1")
	, m_jj2(*this, "JJ2")
	, m_jj3(*this, "JJ3")
{
}

void dsm240_device::write(offs_t offset, u8 data)
{
	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, data);

	if (offset == 0)
	{
		m_cpu_fast = BIT(data, 2);
		m_bus_cpu = BIT(data, 3);
	}
	update_speeds();
}

u8 dsm240_device::read(offs_t offset)
{
	u8 value = 0xff;

	if (offset == 0)
	{
		value = (m_cpu_fast ? 0x04 : 0x00) | (m_bus_cpu ? 0x08 : 0x00);
	}

	LOGFUNC("%s: reg: %d val: 0x%02x\n", FUNCNAME, offset, value);

	return value;
}

void dsm240_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
	save_item(NAME(m_add_wait_state));
	save_item(NAME(m_cpu_fast));
	save_item(NAME(m_bus_cpu));
}

void dsm240_device::device_reset()
{
	if (!m_installed)
	{
		h89bus().install_io_device(DSM240_ADDR_LO, DSM240_ADDR_HI,
			read8sm_delegate(*this, FUNC(dsm240_device::read)),
			write8sm_delegate(*this, FUNC(dsm240_device::write)));

		m_installed = true;
	}

	// jumpers select the power-up configuration
	m_add_wait_state = bool(m_jj1->read());
	m_bus_cpu = bool(m_jj2->read());
	m_cpu_fast = bool(m_jj3->read());

	update_speeds();
}

ioport_constructor dsm240_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dsm240);
}

void dsm240_device::update_speeds()
{
	u32 const cpu_clock = m_cpu_fast ? clock() * MULTIPLIER_4MHZ : clock();

	set_cpu_clock(cpu_clock);

	// the bus clock is either fixed at 2 MHz or follows the CPU clock
	set_bus_clock(m_bus_cpu ? cpu_clock : clock());
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_DSM240, device_h89bus_cpu_board_mod_interface, dsm240_device, "dsm240", "Kres DSM-240 Dual Speed Module");
