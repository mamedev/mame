// license:BSD-3-Clause
// copyright-holders:F. Ulivi

/*********************************************************************

    HP98259 1 Mbit bubble memory

    Example usage of bubble memory from BASIC.
    - Boot BASIC from HPIB disk unit 0
    - Load driver disk in unit 1
    - Load bubble memory driver: LOAD BIN "BUBBLE:,700,1"
    - Format memory:
      INITIALIZE °:BUBBLE,30"
      (Takes about 30 seconds)
    - Display directory:
      CAT ":BUBBLE,30"
    - Load/save BASIC files:
      GET/SAVE "FILE:BUBBLE,30"

    Reference docs:
    - HP98259 schematic diagram by Tony Duell

*********************************************************************/

#include "emu.h"
#include "hp98259.h"

#include "machine/i7110.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

namespace {

// Clock
constexpr auto CLOCK = 4_MHz_XTAL;

// Bit manipulation
template<typename T> constexpr T BIT_MASK(unsigned n)
{
	return (T)1U << n;
}

template<typename T> void BIT_CLR(T& w , unsigned n)
{
	w &= ~BIT_MASK<T>(n);
}

template<typename T> void BIT_SET(T& w , unsigned n)
{
	w |= BIT_MASK<T>(n);
}

// +--------------------+
// | dio16_98259_device |
// +--------------------+
class dio16_98259_device : public device_t,
						   public bus::hp_dio::device_dio16_card_interface
{
public:
	dio16_98259_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	dio16_98259_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void reset_in(int state) override;
	void reset(int state);
	void update_irq();

	uint8_t reg_r(offs_t addr);
	void reg_w(offs_t addr, uint8_t data);

	void bmc_irq_w(int state);
	void bmc_drq_w(int state);

	required_device<i7220_1_device> m_bmc;
	required_device<ibubble_device> m_mbm;
	required_ioport m_sw1;
	required_ioport m_sw2;

	bool m_installed_io;
	bool m_reset;
	bool m_irq;
	bool m_irq_enabled;
	bool m_bmc_irq;
	bool m_bmc_drq;
};

dio16_98259_device::dio16_98259_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dio16_98259_device(mconfig, HPDIO_98259, tag, owner, clock)
{
}

dio16_98259_device::dio16_98259_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HPDIO_98259, tag, owner, clock)
	, bus::hp_dio::device_dio16_card_interface(mconfig, *this)
	, m_bmc(*this, "bmc")
	, m_mbm(*this, "mbm")
	, m_sw1(*this, "sw1")
	, m_sw2(*this, "sw2")
	, m_installed_io{false}
{
}

void dio16_98259_device::device_start()
{
	save_item(NAME(m_installed_io));
	save_item(NAME(m_reset));
	save_item(NAME(m_irq));
	save_item(NAME(m_irq_enabled));
	save_item(NAME(m_bmc_irq));
	save_item(NAME(m_bmc_drq));
}

constexpr unsigned SW2_SC_SHIFT = 0;
constexpr ioport_value SW2_SC_MASK = 0x1f;
constexpr ioport_value SW2_SC_DEF = 30 << SW2_SC_SHIFT;

void dio16_98259_device::device_reset()
{
	if (!m_installed_io) {
		unsigned sc = (m_sw2->read() >> SW2_SC_SHIFT) & SW2_SC_MASK;
		offs_t base_addr = 0x600000 + sc * 0x10000;
		dio().install_memory(
							 0x0000 + base_addr,
							 0x000b + base_addr,
							 read16sm_delegate(*this, NAME([this] (offs_t addr) { return reg_r(addr); })),
							 write16sm_delegate(*this, NAME([this] (offs_t addr, uint16_t data) { reg_w(addr, uint8_t(data)); })));
		m_installed_io = true;
	}

	m_reset = false;
	reset(1);
}

void dio16_98259_device::device_add_mconfig(machine_config &config)
{
	I7220_1(config, m_bmc, CLOCK);
	IBUBBLE(config, m_mbm, CLOCK);

	m_bmc->field_rotate_callback().set(m_mbm, FUNC(ibubble_device::field_rotate));
	m_bmc->select_w_callback().set(m_mbm, FUNC(ibubble_device::select_w));
	m_bmc->cmd_w_callback().set(m_mbm, FUNC(ibubble_device::cmd_w));
	m_bmc->dio_w_callback().set(m_mbm, FUNC(ibubble_device::dio_w));
	m_bmc->dio_r_callback().set(m_mbm, FUNC(ibubble_device::dio_r));
	m_bmc->status_r_callback().set(m_mbm, FUNC(ibubble_device::status_r));
	m_bmc->shiftclk_callback().set(m_mbm, FUNC(ibubble_device::shiftclk));
	m_bmc->bubble_replicate_callback().set(m_mbm, FUNC(ibubble_device::bubble_replicate));
	m_bmc->bootloop_replicate_callback().set(m_mbm, FUNC(ibubble_device::bootloop_replicate));
	m_bmc->bubble_swap_callback().set(m_mbm, FUNC(ibubble_device::bubble_swap));
	m_bmc->bootloop_swap_callback().set(m_mbm, FUNC(ibubble_device::bootloop_swap));
	m_bmc->errflg_r_callback().set(m_mbm, FUNC(ibubble_device::errflg_r));

	m_bmc->irq_callback().set(FUNC(dio16_98259_device::bmc_irq_w));
	m_bmc->drq_callback().set(FUNC(dio16_98259_device::bmc_drq_w));
}

constexpr unsigned SW1_IRQ_LEVEL_SHIFT = 0;
constexpr ioport_value SW1_IRQ_LEVEL_MASK = 3;
constexpr ioport_value SW1_IRQ_LEVEL_IRQ3 = 0;
constexpr ioport_value SW1_IRQ_LEVEL_IRQ4 = 1;
constexpr ioport_value SW1_IRQ_LEVEL_IRQ5 = 2;
constexpr ioport_value SW1_IRQ_LEVEL_IRQ6 = 3;
constexpr ioport_value SW1_IRQ_LEVEL_DEF = SW1_IRQ_LEVEL_IRQ4;

static INPUT_PORTS_START(dio98259_port)
	PORT_START("sw1")
	PORT_DIPNAME(SW1_IRQ_LEVEL_MASK << SW1_IRQ_LEVEL_SHIFT, SW1_IRQ_LEVEL_DEF << SW1_IRQ_LEVEL_SHIFT, "IRQ level")
	PORT_DIPSETTING(SW1_IRQ_LEVEL_IRQ3 << SW1_IRQ_LEVEL_SHIFT, "3")
	PORT_DIPSETTING(SW1_IRQ_LEVEL_IRQ4 << SW1_IRQ_LEVEL_SHIFT, "4")
	PORT_DIPSETTING(SW1_IRQ_LEVEL_IRQ5 << SW1_IRQ_LEVEL_SHIFT, "5")
	PORT_DIPSETTING(SW1_IRQ_LEVEL_IRQ6 << SW1_IRQ_LEVEL_SHIFT, "6")

	PORT_START("sw2")
	PORT_DIPNAME(SW2_SC_MASK << SW2_SC_SHIFT, SW2_SC_DEF, "Select code")
	PORT_DIPSETTING(0 << SW2_SC_SHIFT, "0")
	PORT_DIPSETTING(1 << SW2_SC_SHIFT, "1")
	PORT_DIPSETTING(2 << SW2_SC_SHIFT, "2")
	PORT_DIPSETTING(3 << SW2_SC_SHIFT, "3")
	PORT_DIPSETTING(4 << SW2_SC_SHIFT, "4")
	PORT_DIPSETTING(5 << SW2_SC_SHIFT, "5")
	PORT_DIPSETTING(6 << SW2_SC_SHIFT, "6")
	PORT_DIPSETTING(7 << SW2_SC_SHIFT, "7")
	PORT_DIPSETTING(8 << SW2_SC_SHIFT, "8")
	PORT_DIPSETTING(9 << SW2_SC_SHIFT, "9")
	PORT_DIPSETTING(10 << SW2_SC_SHIFT, "10")
	PORT_DIPSETTING(11 << SW2_SC_SHIFT, "11")
	PORT_DIPSETTING(12 << SW2_SC_SHIFT, "12")
	PORT_DIPSETTING(13 << SW2_SC_SHIFT, "13")
	PORT_DIPSETTING(14 << SW2_SC_SHIFT, "14")
	PORT_DIPSETTING(15 << SW2_SC_SHIFT, "15")
	PORT_DIPSETTING(16 << SW2_SC_SHIFT, "16")
	PORT_DIPSETTING(17 << SW2_SC_SHIFT, "17")
	PORT_DIPSETTING(18 << SW2_SC_SHIFT, "18")
	PORT_DIPSETTING(19 << SW2_SC_SHIFT, "19")
	PORT_DIPSETTING(20 << SW2_SC_SHIFT, "20")
	PORT_DIPSETTING(21 << SW2_SC_SHIFT, "21")
	PORT_DIPSETTING(22 << SW2_SC_SHIFT, "22")
	PORT_DIPSETTING(23 << SW2_SC_SHIFT, "23")
	PORT_DIPSETTING(24 << SW2_SC_SHIFT, "24")
	PORT_DIPSETTING(25 << SW2_SC_SHIFT, "25")
	PORT_DIPSETTING(26 << SW2_SC_SHIFT, "26")
	PORT_DIPSETTING(27 << SW2_SC_SHIFT, "27")
	PORT_DIPSETTING(28 << SW2_SC_SHIFT, "28")
	PORT_DIPSETTING(29 << SW2_SC_SHIFT, "29")
	PORT_DIPSETTING(30 << SW2_SC_SHIFT, "30")
	PORT_DIPSETTING(31 << SW2_SC_SHIFT, "31")
INPUT_PORTS_END

ioport_constructor dio16_98259_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(dio98259_port);
}

void dio16_98259_device::reset_in(int state)
{
	if (state) {
		reset(1);
	}
}

void dio16_98259_device::reset(int state)
{
	bool new_reset = bool(state);
	if (!m_reset && new_reset) {
		m_irq = false;
		m_irq_enabled = false;
		m_bmc->reset();
		m_mbm->reset();
		update_irq();
	}

}

void dio16_98259_device::update_irq()
{
	m_irq = m_bmc_irq || m_bmc_drq;
	auto level = (m_sw1->read() >> SW1_IRQ_LEVEL_SHIFT) & SW1_IRQ_LEVEL_MASK;
	bool irq = m_irq && m_irq_enabled;
	irq3_out(irq && level == SW1_IRQ_LEVEL_IRQ3);
	irq4_out(irq && level == SW1_IRQ_LEVEL_IRQ4);
	irq5_out(irq && level == SW1_IRQ_LEVEL_IRQ5);
	irq6_out(irq && level == SW1_IRQ_LEVEL_IRQ6);
}

uint8_t dio16_98259_device::reg_r(offs_t addr)
{
	uint8_t res = 0;

	switch (addr & 7) {
	case 0:
		// ID register
		res = 0x1e;
		break;
	case 1:
		// IRQ register
		//
		// |  Bit | Content     |
		// |------+-------------|
		// |    7 | IRQ enabled |
		// |    6 | IRQ pending |
		// | 5..4 | IRQ level   |
		// | 3..0 | 0           |
		if (m_irq_enabled) {
			BIT_SET(res, 7);
		}
		if (m_irq) {
			BIT_SET(res, 6);
		}
		res |= ((m_sw1->read() >> SW1_IRQ_LEVEL_SHIFT) & SW1_IRQ_LEVEL_MASK) << 4;
		break;
	case 2:
		// Read 0
		res = 0;
		break;
	case 4:
	case 5:
		// BMC
		res = m_bmc->read(addr & 1);
		break;
	}

	LOG("REG R %u=%02x\n", addr, res);
	return res;
}

void dio16_98259_device::reg_w(offs_t addr, uint8_t data)
{
	LOG("REG W %u=%02x\n", addr, data);
	switch (addr & 7) {
	case 0:
		// Reset control
		reset(1);
		break;
	case 1:
		// IRQ enable
		if (!m_reset) {
			m_irq_enabled = BIT(data, 7);
			update_irq();
		}
		break;
	case 4:
	case 5:
		// BMC
		m_bmc->write(addr & 1, data);
		break;
	}
}

void dio16_98259_device::bmc_irq_w(int state)
{
	m_bmc_irq = bool(state);
	update_irq();
}

void dio16_98259_device::bmc_drq_w(int state)
{
	m_bmc_drq = bool(state);
	update_irq();
}

}  // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98259, bus::hp_dio::device_dio16_card_interface, dio16_98259_device, "dio98259", "HP98259 1 Mbit bubble memory")
