// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98624 GPIB interface

***************************************************************************/

#include "emu.h"
#include "hp98624.h"

#include "bus/ieee488/ieee488.h"
#include "machine/tms9914.h"

//#define VERBOSE 1
#include "logmacro.h"

namespace {

class dio16_98624_device :
		public device_t,
		public bus::hp_dio::device_dio16_card_interface
{
public:
	dio16_98624_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dio16_98624_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void dmack_w_in(int channel, uint8_t data) override;
	uint8_t dmack_r_in(int channel) override;

	uint16_t io_r(offs_t offset);
	void io_w(offs_t offset, uint16_t data);

	required_device<tms9914_device> m_tms9914;
	required_device<ieee488_device> m_ieee488;
	required_ioport m_switches;

private:
	void update_gpib_irq();
	void update_gpib_dma();
	void gpib_irq(int state);
	void gpib_dreq(int state);

	bool m_gpib_irq_line;
	bool m_gpib_dma_line;

	bool     m_installed_io;
	uint8_t  m_control;
};

void dio16_98624_device::device_add_mconfig(machine_config &config)
{
	tms9914_device &gpib(TMS9914(config, "tms9914", XTAL(5'000'000)));
	gpib.eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	gpib.dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	gpib.nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	gpib.ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	gpib.ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	gpib.srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	gpib.atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	gpib.ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	gpib.dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	gpib.dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	gpib.int_write_cb().set(FUNC(dio16_98624_device::gpib_irq));
	gpib.accrq_write_cb().set(FUNC(dio16_98624_device::gpib_dreq));

	ieee488_device &ieee488(IEEE488(config, IEEE488_TAG, 0));
	ieee488.eoi_callback().set(m_tms9914, FUNC(tms9914_device::eoi_w));
	ieee488.dav_callback().set(m_tms9914, FUNC(tms9914_device::dav_w));
	ieee488.nrfd_callback().set(m_tms9914, FUNC(tms9914_device::nrfd_w));
	ieee488.ndac_callback().set(m_tms9914, FUNC(tms9914_device::ndac_w));
	ieee488.ifc_callback().set(m_tms9914, FUNC(tms9914_device::ifc_w));
	ieee488.srq_callback().set(m_tms9914, FUNC(tms9914_device::srq_w));
	ieee488.atn_callback().set(m_tms9914, FUNC(tms9914_device::atn_w));
	ieee488.ren_callback().set(m_tms9914, FUNC(tms9914_device::ren_w));

	ieee488_slot_device &slot0(IEEE488_SLOT(config, "ieee0", 0));
	hp_ieee488_devices(slot0);
}

dio16_98624_device::dio16_98624_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98624_device(mconfig, HPDIO_98624, tag, owner, clock)
{
}

dio16_98624_device::dio16_98624_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this),
	m_tms9914{*this, "tms9914"},
	m_ieee488{*this, IEEE488_TAG},
	m_switches{*this, "switches"},
	m_installed_io{false},
	m_control{0}
{
}

constexpr unsigned REG_SWITCHES_SC = 0x01;
constexpr unsigned REG_SWITCHES_REMOTE = 0x02;

constexpr unsigned REG_SWITCHES_INT_LEVEL_MASK = 3;
constexpr unsigned REG_SWITCHES_INT_LEVEL_SHIFT = 2;

constexpr unsigned REG_SWITCHES_SELECT_CODE_MASK = 31;
constexpr unsigned REG_SWITCHES_SELECT_CODE_SHIFT = 4;

constexpr unsigned REG_SWITCHES_GPIB_ADDR_MASK = 31;
constexpr unsigned REG_SWITCHES_GPIB_ADDR_SHIFT = 9;

static INPUT_PORTS_START(hp98624_port)
	PORT_START("switches")
	PORT_DIPNAME(REG_SWITCHES_SC, REG_SWITCHES_SC, "System Controller")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(REG_SWITCHES_SC, DEF_STR(On))

	PORT_DIPNAME(REG_SWITCHES_REMOTE, 0x00, "Remote")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(REG_SWITCHES_REMOTE, DEF_STR(On))

	PORT_DIPNAME(REG_SWITCHES_INT_LEVEL_MASK << REG_SWITCHES_INT_LEVEL_SHIFT, 0x00 << REG_SWITCHES_INT_LEVEL_SHIFT, "Interrupt level")
	PORT_DIPSETTING(0 << REG_SWITCHES_INT_LEVEL_SHIFT, "3")
	PORT_DIPSETTING(1 << REG_SWITCHES_INT_LEVEL_SHIFT, "4")
	PORT_DIPSETTING(2 << REG_SWITCHES_INT_LEVEL_SHIFT, "5")
	PORT_DIPSETTING(3 << REG_SWITCHES_INT_LEVEL_SHIFT, "6")

	PORT_DIPNAME(REG_SWITCHES_SELECT_CODE_MASK << REG_SWITCHES_SELECT_CODE_SHIFT, 0x08 << REG_SWITCHES_SELECT_CODE_SHIFT, "Select code")
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

	PORT_DIPNAME(REG_SWITCHES_GPIB_ADDR_MASK << REG_SWITCHES_GPIB_ADDR_SHIFT, 21 << REG_SWITCHES_GPIB_ADDR_SHIFT, "GPIB Adress")
	PORT_DIPSETTING(0 << REG_SWITCHES_GPIB_ADDR_SHIFT, "0")
	PORT_DIPSETTING(1 << REG_SWITCHES_GPIB_ADDR_SHIFT, "1")
	PORT_DIPSETTING(2 << REG_SWITCHES_GPIB_ADDR_SHIFT, "2")
	PORT_DIPSETTING(3 << REG_SWITCHES_GPIB_ADDR_SHIFT, "3")
	PORT_DIPSETTING(4 << REG_SWITCHES_GPIB_ADDR_SHIFT, "4")
	PORT_DIPSETTING(5 << REG_SWITCHES_GPIB_ADDR_SHIFT, "5")
	PORT_DIPSETTING(6 << REG_SWITCHES_GPIB_ADDR_SHIFT, "6")
	PORT_DIPSETTING(7 << REG_SWITCHES_GPIB_ADDR_SHIFT, "7")
	PORT_DIPSETTING(8 << REG_SWITCHES_GPIB_ADDR_SHIFT, "8")
	PORT_DIPSETTING(9 << REG_SWITCHES_GPIB_ADDR_SHIFT, "9")
	PORT_DIPSETTING(10 << REG_SWITCHES_GPIB_ADDR_SHIFT, "10")
	PORT_DIPSETTING(11 << REG_SWITCHES_GPIB_ADDR_SHIFT, "11")
	PORT_DIPSETTING(12 << REG_SWITCHES_GPIB_ADDR_SHIFT, "12")
	PORT_DIPSETTING(13 << REG_SWITCHES_GPIB_ADDR_SHIFT, "13")
	PORT_DIPSETTING(14 << REG_SWITCHES_GPIB_ADDR_SHIFT, "14")
	PORT_DIPSETTING(15 << REG_SWITCHES_GPIB_ADDR_SHIFT, "15")
	PORT_DIPSETTING(16 << REG_SWITCHES_GPIB_ADDR_SHIFT, "16")
	PORT_DIPSETTING(17 << REG_SWITCHES_GPIB_ADDR_SHIFT, "17")
	PORT_DIPSETTING(18 << REG_SWITCHES_GPIB_ADDR_SHIFT, "18")
	PORT_DIPSETTING(19 << REG_SWITCHES_GPIB_ADDR_SHIFT, "19")
	PORT_DIPSETTING(20 << REG_SWITCHES_GPIB_ADDR_SHIFT, "20")
	PORT_DIPSETTING(21 << REG_SWITCHES_GPIB_ADDR_SHIFT, "21")
	PORT_DIPSETTING(22 << REG_SWITCHES_GPIB_ADDR_SHIFT, "22")
	PORT_DIPSETTING(23 << REG_SWITCHES_GPIB_ADDR_SHIFT, "23")
	PORT_DIPSETTING(24 << REG_SWITCHES_GPIB_ADDR_SHIFT, "24")
	PORT_DIPSETTING(25 << REG_SWITCHES_GPIB_ADDR_SHIFT, "25")
	PORT_DIPSETTING(26 << REG_SWITCHES_GPIB_ADDR_SHIFT, "26")
	PORT_DIPSETTING(27 << REG_SWITCHES_GPIB_ADDR_SHIFT, "27")
	PORT_DIPSETTING(28 << REG_SWITCHES_GPIB_ADDR_SHIFT, "28")
	PORT_DIPSETTING(29 << REG_SWITCHES_GPIB_ADDR_SHIFT, "29")
	PORT_DIPSETTING(30 << REG_SWITCHES_GPIB_ADDR_SHIFT, "30")
	PORT_DIPSETTING(31 << REG_SWITCHES_GPIB_ADDR_SHIFT, "31")
INPUT_PORTS_END

ioport_constructor dio16_98624_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98624_port);
}

void dio16_98624_device::device_start()
{
	save_item(NAME(m_installed_io));
	save_item(NAME(m_control));
	m_installed_io = false;
}

void dio16_98624_device::device_reset()
{
	uint8_t code = m_switches->read() >> REG_SWITCHES_SELECT_CODE_SHIFT;
	code &= REG_SWITCHES_SELECT_CODE_MASK;

	if (!m_installed_io)
	{
		dio().install_memory(
				0x600000 + (code * 0x10000),
				0x6007ff + (code * 0x10000),
				read16sm_delegate(*this, FUNC(dio16_98624_device::io_r)),
				write16sm_delegate(*this, FUNC(dio16_98624_device::io_w)));
		m_installed_io = true;
	}
	m_control = 0;
}

void dio16_98624_device::update_gpib_irq()
{
	bool enable = m_control & 0x80;

	switch((m_switches->read() >> REG_SWITCHES_INT_LEVEL_SHIFT) & REG_SWITCHES_INT_LEVEL_MASK) {
	case 0:
		irq3_out(enable && m_gpib_irq_line);
		break;
	case 1:
		irq4_out(enable && m_gpib_irq_line);
		break;
	case 2:
		irq5_out(enable && m_gpib_irq_line);
		break;
	case 3:
		irq6_out(enable && m_gpib_irq_line);
		break;
	}
}

void dio16_98624_device::gpib_irq(int state)
{
	m_gpib_irq_line = state;
	update_gpib_irq();
}

void dio16_98624_device::update_gpib_dma()
{
	dmar0_out((m_control & 1) && m_gpib_dma_line);
	dmar1_out((m_control & 2) && m_gpib_dma_line);
}

void dio16_98624_device::gpib_dreq(int state)
{
	m_gpib_dma_line = state;
	update_gpib_dma();
}

uint16_t dio16_98624_device::io_r(offs_t offset)
{
	uint16_t ret = 0xffff;

	if (offset & 8)
		return m_tms9914->read(offset & 0x07);

	switch(offset) {
	case 0: /* ID */
		ret = 0x01;
		if (!(m_switches->read() & REG_SWITCHES_REMOTE))
			ret |= 0x80;
		break;

	case 1:
		ret = m_control;
		ret |= m_gpib_irq_line ? 0x40 : 0;
		ret |= ((m_switches->read() >> REG_SWITCHES_INT_LEVEL_SHIFT) & REG_SWITCHES_INT_LEVEL_MASK) << 4;
		break;
	case 2:
		ret = (m_switches->read() & REG_SWITCHES_SC) ? 0x80 : 0;
		ret |= (m_tms9914->cont_r() ? 0x0 : 0x40) | 0x80;
		ret |= (m_switches->read() >> REG_SWITCHES_GPIB_ADDR_SHIFT) & REG_SWITCHES_GPIB_ADDR_MASK;
		break;
	default:
		break;
	}
	LOG("%s: %02x = %02x\n", __func__, offset, ret);
	return ret;
}

void dio16_98624_device::io_w(offs_t offset, uint16_t data)
{
	LOG("%s: %02x = %02x\n", __func__, offset, data);
	if (offset & 0x08) {
		m_tms9914->write(offset & 0x07, data);
		return;
	}

	switch(offset) {
	case 1:
		m_control = data & 0x8b;
		update_gpib_dma();
		update_gpib_irq();
		break;
	default:
		break;
	}
}

void dio16_98624_device::dmack_w_in(int channel, uint8_t data)
{
	if (channel == 0 && (m_control & 1))
		m_tms9914->write(7, data);
	if (channel == 1 && (m_control & 2))
		m_tms9914->write(7, data);
}

uint8_t dio16_98624_device::dmack_r_in(int channel)
{
	uint8_t ret = 0xff;

	if (channel == 0 && (m_control & 1))
		ret = m_tms9914->read(7);
	if (channel == 1 && (m_control & 2))
		ret = m_tms9914->read(7);
	return ret;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98624, bus::hp_dio::device_dio16_card_interface, dio16_98624_device, "dio98624", "HP98624 GPIB Interface")
