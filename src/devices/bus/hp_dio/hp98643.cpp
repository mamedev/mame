// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98643 LANIC Ethernet card

***************************************************************************/

#include "emu.h"
#include "hp98643.h"
#include "machine/am79c90.h"

//#define VERBOSE 1
#include "logmacro.h"


namespace {

static constexpr int REG_SWITCHES_REMOTE = 0x80;

static constexpr int REG_SWITCHES_SELECT_CODE_MASK = 0x1f;
static constexpr int REG_SWITCHES_SELECT_CODE_SHIFT = 0x00;

static constexpr int REG_SWITCHES_INT_LEVEL_MASK = 0x03;
static constexpr int REG_SWITCHES_INT_LEVEL_SHIFT = 0x05;

static constexpr uint16_t REG_ID = 0x15;

static constexpr uint16_t REG_STATUS_ACK = 0x04;

static constexpr uint16_t REG_SC_REV = 0x01;
static constexpr uint16_t REG_SC_LOCK = 0x08;
static constexpr uint16_t REG_SC_IP = 0x40;
static constexpr uint16_t REG_SC_IE = 0x80;

class dio16_98643_device :
		public device_t,
		public bus::hp_dio::device_dio16_card_interface
{
public:
	// construction/destruction
	dio16_98643_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dio16_98643_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);


	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<am7990_device> m_lance;
	required_ioport m_switches;

	void addrmap(address_map &map);
	void update_int();
	int get_irq_line();

	uint16_t sc_r();
	void sc_w(uint16_t data);
	uint16_t id_r();
	void id_w(uint16_t data);
	uint16_t novram_r(offs_t offset);
	void novram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void lance_int_w(int state);
	void lance_dma_out(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t lance_dma_in(offs_t offset);

	uint16_t m_novram[32] = {
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff8, 0xfff0, 0xfff0,
		0xfff0, 0xfff9, 0xfff0, 0xfff6,
		0xfffa, 0xfff1, 0xffff, 0xfff1,
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff0, 0xfff0, 0xfff0,
		0xfff0, 0xfff0, 0xfffa, 0xfff9,
	};

	std::array<uint16_t, 8192> m_ram;

	uint16_t m_sc;
	bool m_installed_io;
};

void dio16_98643_device::device_add_mconfig(machine_config &config)
{
	AM7990(config, m_lance, XTAL(20'000'000));
	m_lance->intr_out().set(FUNC(dio16_98643_device::lance_int_w));
	m_lance->dma_out().set(FUNC(dio16_98643_device::lance_dma_out));
	m_lance->dma_in().set(FUNC(dio16_98643_device::lance_dma_in));
}

dio16_98643_device::dio16_98643_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98643_device(mconfig, HPDIO_98643, tag, owner, clock)
{
}

dio16_98643_device::dio16_98643_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this),
	m_lance(*this, "lance"),
	m_switches{*this, "switches"}
{
}

static INPUT_PORTS_START(hp98643_port)
	PORT_START("switches")
	PORT_DIPNAME(REG_SWITCHES_REMOTE, 0x00, "Remote")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(REG_SWITCHES_REMOTE, DEF_STR(On))

	PORT_DIPNAME(REG_SWITCHES_INT_LEVEL_MASK << REG_SWITCHES_INT_LEVEL_SHIFT, 0x02 << REG_SWITCHES_INT_LEVEL_SHIFT, "Interrupt level")
	PORT_DIPSETTING(0 << REG_SWITCHES_INT_LEVEL_SHIFT, "3")
	PORT_DIPSETTING(1 << REG_SWITCHES_INT_LEVEL_SHIFT, "4")
	PORT_DIPSETTING(2 << REG_SWITCHES_INT_LEVEL_SHIFT, "5")
	PORT_DIPSETTING(3 << REG_SWITCHES_INT_LEVEL_SHIFT, "6")

	PORT_DIPNAME(REG_SWITCHES_SELECT_CODE_MASK << REG_SWITCHES_SELECT_CODE_SHIFT, 21 << REG_SWITCHES_SELECT_CODE_SHIFT, "Select code")
	PORT_DIPSETTING(0 << REG_SWITCHES_SELECT_CODE_SHIFT, "0")
	PORT_DIPSETTING(1 << REG_SWITCHES_SELECT_CODE_SHIFT, "1")
	PORT_DIPSETTING(2 << REG_SWITCHES_SELECT_CODE_SHIFT, "2")
	PORT_DIPSETTING(3 << REG_SWITCHES_SELECT_CODE_SHIFT, "3")
	PORT_DIPSETTING(4 << REG_SWITCHES_SELECT_CODE_SHIFT, "4")
	PORT_DIPSETTING(5 << REG_SWITCHES_SELECT_CODE_SHIFT, "5")
	PORT_DIPSETTING(6 << REG_SWITCHES_SELECT_CODE_SHIFT, "6")
	PORT_DIPSETTING(7 << REG_SWITCHES_SELECT_CODE_SHIFT, "7")
	PORT_DIPSETTING(8 << REG_SWITCHES_SELECT_CODE_SHIFT, "8")
	PORT_DIPSETTING(9 << REG_SWITCHES_SELECT_CODE_SHIFT, "9")
	PORT_DIPSETTING(10 << REG_SWITCHES_SELECT_CODE_SHIFT, "10")
	PORT_DIPSETTING(11 << REG_SWITCHES_SELECT_CODE_SHIFT, "11")
	PORT_DIPSETTING(12 << REG_SWITCHES_SELECT_CODE_SHIFT, "12")
	PORT_DIPSETTING(13 << REG_SWITCHES_SELECT_CODE_SHIFT, "13")
	PORT_DIPSETTING(14 << REG_SWITCHES_SELECT_CODE_SHIFT, "14")
	PORT_DIPSETTING(15 << REG_SWITCHES_SELECT_CODE_SHIFT, "15")
	PORT_DIPSETTING(16 << REG_SWITCHES_SELECT_CODE_SHIFT, "16")
	PORT_DIPSETTING(17 << REG_SWITCHES_SELECT_CODE_SHIFT, "17")
	PORT_DIPSETTING(18 << REG_SWITCHES_SELECT_CODE_SHIFT, "18")
	PORT_DIPSETTING(19 << REG_SWITCHES_SELECT_CODE_SHIFT, "19")
	PORT_DIPSETTING(20 << REG_SWITCHES_SELECT_CODE_SHIFT, "20")
	PORT_DIPSETTING(21 << REG_SWITCHES_SELECT_CODE_SHIFT, "21")
	PORT_DIPSETTING(22 << REG_SWITCHES_SELECT_CODE_SHIFT, "22")
	PORT_DIPSETTING(23 << REG_SWITCHES_SELECT_CODE_SHIFT, "23")
	PORT_DIPSETTING(24 << REG_SWITCHES_SELECT_CODE_SHIFT, "24")
	PORT_DIPSETTING(25 << REG_SWITCHES_SELECT_CODE_SHIFT, "25")
	PORT_DIPSETTING(26 << REG_SWITCHES_SELECT_CODE_SHIFT, "26")
	PORT_DIPSETTING(27 << REG_SWITCHES_SELECT_CODE_SHIFT, "27")
	PORT_DIPSETTING(28 << REG_SWITCHES_SELECT_CODE_SHIFT, "28")
	PORT_DIPSETTING(29 << REG_SWITCHES_SELECT_CODE_SHIFT, "29")
	PORT_DIPSETTING(30 << REG_SWITCHES_SELECT_CODE_SHIFT, "30")
	PORT_DIPSETTING(31 << REG_SWITCHES_SELECT_CODE_SHIFT, "31")
INPUT_PORTS_END

ioport_constructor dio16_98643_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98643_port);
}

void dio16_98643_device::device_start()
{
	save_item(NAME(m_sc));
	save_item(NAME(m_installed_io));
	save_item(NAME(m_ram));
	m_installed_io = false;
}

void dio16_98643_device::device_reset()
{
	if (!m_installed_io) {
		uint8_t code = (m_switches->read() >> REG_SWITCHES_SELECT_CODE_SHIFT)
					& REG_SWITCHES_SELECT_CODE_MASK;

		uint32_t baseaddr = 0x600000 + (code << 16);

		program_space().install_device(baseaddr, baseaddr + 0xffff,
			*this, &dio16_98643_device::addrmap);
		m_installed_io = true;
	}
	m_sc = REG_SC_REV;
	m_sc |= get_irq_line() << 4;
}

void dio16_98643_device::lance_int_w(int state)
{
	if (state)
		m_sc &= ~REG_SC_IP;
	else
		m_sc |= REG_SC_IP;
	update_int();
}

void dio16_98643_device::sc_w(uint16_t data)
{
	LOG("%s: %02x\n", __func__, data);
	data &= (REG_SC_LOCK|REG_SC_IE);
	m_sc &= ~(REG_SC_LOCK|REG_SC_IE);

	if (data & REG_SC_LOCK)
		data |= REG_STATUS_ACK;

	m_sc |= data;
	update_int();
}

uint16_t dio16_98643_device::sc_r()
{
	LOG("%s: %02x\n", __func__, m_sc);
	return m_sc;
}

uint16_t dio16_98643_device::id_r()
{
	return (REG_ID | (m_switches->read() & REG_SWITCHES_REMOTE));
}

void dio16_98643_device::id_w(uint16_t data)
{
	reset();
}
uint16_t dio16_98643_device::novram_r(offs_t offset)
{
	return m_novram[offset];
}

void dio16_98643_device::novram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_novram[offset & 0x3f]);
}

void dio16_98643_device::lance_dma_out(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s: offset=%04x, data=%d\n", __func__, offset, data);
	COMBINE_DATA(&m_ram[(offset >> 1) & 0x1fff]);
}

uint16_t dio16_98643_device::lance_dma_in(offs_t offset)
{
	uint16_t ret = m_ram[(offset >> 1) & 0x1fff];

	LOG("%s: offset=%04x data %04x\n", __func__, offset, ret);
	return ret;
}

int dio16_98643_device::get_irq_line()
{
	return (m_switches->read() >> REG_SWITCHES_INT_LEVEL_SHIFT) & REG_SWITCHES_INT_LEVEL_MASK;
}

void dio16_98643_device::update_int()
{
	const int line = get_irq_line() + 3;
	const bool state = (m_sc & (REG_SC_IE|REG_SC_IP)) == (REG_SC_IE|REG_SC_IP);
	LOG("%s: line %d, state %d\n", __func__, line, state);
	irq3_out(state && line == 3);
	irq4_out(state && line == 4);
	irq5_out(state && line == 5);
	irq6_out(state && line == 6);
}

void dio16_98643_device::addrmap(address_map &map)
{
	map(0x0000, 0x0001).rw(FUNC(dio16_98643_device::id_r), FUNC(dio16_98643_device::id_w));
	map(0x0002, 0x0003).rw(FUNC(dio16_98643_device::sc_r), FUNC(dio16_98643_device::sc_w));
	map(0x4000, 0x4003).lrw16(
			[this] (address_space &space, offs_t offset) -> u16 {
				m_sc |= REG_STATUS_ACK;
				return m_lance->regs_r(space, offset);
			}, "lance_r",
			[this] (offs_t offset, u16 data) {
				m_sc |= REG_STATUS_ACK;
				return m_lance->regs_w(offset, data);
			}, "lance_w");

	map(0x8000, 0xbfff).lrw16(
			NAME([this] (offs_t offset) -> u16 { return m_ram[offset]; }),
			NAME([this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_ram[offset]); }));

	map(0xc000, 0xffff).rw(FUNC(dio16_98643_device::novram_r), FUNC(dio16_98643_device::novram_w));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98643, bus::hp_dio::device_dio16_card_interface, dio16_98643_device, "dio98643", "HP98643A LANIC Ethernet card")
