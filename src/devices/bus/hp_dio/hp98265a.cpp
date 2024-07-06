// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98265A SCSI interface

***************************************************************************/

#include "emu.h"
#include "hp98265a.h"

#include "machine/nscsi_bus.h"
#include "bus/nscsi/devices.h"
#include "machine/mb87030.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class dio16_98265a_device :
		public device_t,
		public bus::hp_dio::device_dio32_card_interface
{
public:
	// construction/destruction
	dio16_98265a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb87030(device_t *device);

protected:
	dio16_98265a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);


	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	uint16_t io_r(offs_t offset);
	void io_w(offs_t offset, uint16_t data);

	void dmack_w_in(int channel, uint8_t data) override;
	uint8_t dmack_r_in(int channel) override;

	void dmar0_w(int state);

	void irq_w(int state);

	required_device<nscsi_bus_device> m_scsibus;
	required_device<mb87030_device> m_spc;
private:

	static constexpr int REG_CONTROL_DE0 = (1 << 0);
	static constexpr int REG_CONTROL_DE1 = (1 << 1);

	static void mb87030_scsi_adapter(device_t *device);
	required_ioport m_sw1;
	required_ioport m_sw2;
	int get_int_level();
	void update_irq(bool state);
	void update_dma();
	bool     m_installed_io;
	uint8_t  m_control;

	bool m_irq_state;
	bool m_dmar0;
};

void dio16_98265a_device::mb87030_scsi_adapter(device_t *device)
{
	mb87030_device &spc = downcast<mb87030_device &>(*device);
	spc.set_clock(8_MHz_XTAL);

	spc.out_irq_callback().set("^^", FUNC(dio16_98265a_device::irq_w));
	spc.out_dreq_callback().set("^^", FUNC(dio16_98265a_device::dmar0_w));
}

void dio16_98265a_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_scsibus, 0);
	nscsi_connector &scsicon0(NSCSI_CONNECTOR(config, "scsibus:0", 0));
	default_scsi_devices(scsicon0);
	scsicon0.set_default_option("harddisk");

	default_scsi_devices(NSCSI_CONNECTOR(config, "scsibus:1", 0));
	default_scsi_devices(NSCSI_CONNECTOR(config, "scsibus:2", 0));
	default_scsi_devices(NSCSI_CONNECTOR(config, "scsibus:3", 0));
	default_scsi_devices(NSCSI_CONNECTOR(config, "scsibus:4", 0));

	nscsi_connector &scsicon5(NSCSI_CONNECTOR(config, "scsibus:5", 0));
	default_scsi_devices(scsicon5);
	scsicon5.set_default_option("cdrom");

	default_scsi_devices(NSCSI_CONNECTOR(config, "scsibus:6", 0));
	nscsi_connector &scsicon7(NSCSI_CONNECTOR(config, "scsibus:7", 0));
	scsicon7.option_add_internal("mb87030", MB87030);
	scsicon7.set_default_option("mb87030");
	scsicon7.set_fixed(true);
	scsicon7.set_option_machine_config("mb87030", mb87030_scsi_adapter);
}

dio16_98265a_device::dio16_98265a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98265a_device(mconfig, HPDIO_98265A, tag, owner, clock)
{
}

dio16_98265a_device::dio16_98265a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio32_card_interface(mconfig, *this),
	m_scsibus(*this, "scsibus"),
	m_spc(*this, "scsibus:7:mb87030"),
	m_sw1(*this, "SW1"),
	m_sw2(*this, "SW2"),
	m_installed_io(false),
	m_control(0),
	m_irq_state(false),
	m_dmar0(false)
{
}

constexpr int REG_SW1_INT_LEVEL_SHIFT = 0;
constexpr int REG_SW1_INT_LEVEL_MASK = 0x3;

constexpr int REG_SW1_SELECT_CODE_SHIFT = 2;
constexpr int REG_SW1_SELECT_CODE_MASK = 0x1f;

constexpr int REG_SW2_SCSI_ADDR_SHIFT = 0;
constexpr int REG_SW2_SCSI_ADDR_MASK = 0x07;

constexpr int REG_SW2_SCSI_PARITY = 0x08;

static INPUT_PORTS_START(hp98265a_port)
	PORT_START("SW1")
	PORT_DIPNAME(REG_SW1_INT_LEVEL_MASK << REG_SW1_INT_LEVEL_SHIFT, 0x01, "INT Level")
	PORT_DIPSETTING(0 << REG_SW1_INT_LEVEL_SHIFT, "3")
	PORT_DIPSETTING(1 << REG_SW1_INT_LEVEL_SHIFT, "4")
	PORT_DIPSETTING(2 << REG_SW1_INT_LEVEL_SHIFT, "5")
	PORT_DIPSETTING(3 << REG_SW1_INT_LEVEL_SHIFT, "6")

	PORT_DIPNAME(REG_SW1_SELECT_CODE_MASK << REG_SW1_SELECT_CODE_SHIFT, 0x0e << REG_SW1_SELECT_CODE_SHIFT, "Select code")
	PORT_DIPSETTING(0 << REG_SW1_SELECT_CODE_SHIFT, "0")
	PORT_DIPSETTING(1 << REG_SW1_SELECT_CODE_SHIFT, "1")
	PORT_DIPSETTING(2 << REG_SW1_SELECT_CODE_SHIFT, "2")
	PORT_DIPSETTING(3 << REG_SW1_SELECT_CODE_SHIFT, "3")
	PORT_DIPSETTING(4 << REG_SW1_SELECT_CODE_SHIFT, "4")
	PORT_DIPSETTING(5 << REG_SW1_SELECT_CODE_SHIFT, "5")
	PORT_DIPSETTING(6 << REG_SW1_SELECT_CODE_SHIFT, "6")
	PORT_DIPSETTING(7 << REG_SW1_SELECT_CODE_SHIFT, "7")
	PORT_DIPSETTING(8 << REG_SW1_SELECT_CODE_SHIFT, "8")
	PORT_DIPSETTING(9 << REG_SW1_SELECT_CODE_SHIFT, "9")
	PORT_DIPSETTING(10 << REG_SW1_SELECT_CODE_SHIFT, "10")
	PORT_DIPSETTING(11 << REG_SW1_SELECT_CODE_SHIFT, "11")
	PORT_DIPSETTING(12 << REG_SW1_SELECT_CODE_SHIFT, "12")
	PORT_DIPSETTING(13 << REG_SW1_SELECT_CODE_SHIFT, "13")
	PORT_DIPSETTING(14 << REG_SW1_SELECT_CODE_SHIFT, "14")
	PORT_DIPSETTING(15 << REG_SW1_SELECT_CODE_SHIFT, "15")
	PORT_DIPSETTING(16 << REG_SW1_SELECT_CODE_SHIFT, "16")
	PORT_DIPSETTING(17 << REG_SW1_SELECT_CODE_SHIFT, "17")
	PORT_DIPSETTING(18 << REG_SW1_SELECT_CODE_SHIFT, "18")
	PORT_DIPSETTING(19 << REG_SW1_SELECT_CODE_SHIFT, "19")
	PORT_DIPSETTING(20 << REG_SW1_SELECT_CODE_SHIFT, "20")
	PORT_DIPSETTING(21 << REG_SW1_SELECT_CODE_SHIFT, "21")
	PORT_DIPSETTING(22 << REG_SW1_SELECT_CODE_SHIFT, "22")
	PORT_DIPSETTING(23 << REG_SW1_SELECT_CODE_SHIFT, "23")
	PORT_DIPSETTING(24 << REG_SW1_SELECT_CODE_SHIFT, "24")
	PORT_DIPSETTING(25 << REG_SW1_SELECT_CODE_SHIFT, "25")
	PORT_DIPSETTING(26 << REG_SW1_SELECT_CODE_SHIFT, "26")
	PORT_DIPSETTING(27 << REG_SW1_SELECT_CODE_SHIFT, "27")
	PORT_DIPSETTING(28 << REG_SW1_SELECT_CODE_SHIFT, "28")
	PORT_DIPSETTING(29 << REG_SW1_SELECT_CODE_SHIFT, "29")
	PORT_DIPSETTING(30 << REG_SW1_SELECT_CODE_SHIFT, "30")
	PORT_DIPSETTING(31 << REG_SW1_SELECT_CODE_SHIFT, "31")

	PORT_START("SW2")
	PORT_DIPNAME(REG_SW2_SCSI_ADDR_MASK << REG_SW2_SCSI_ADDR_SHIFT, 0x00, "SCSI Address")
	PORT_DIPSETTING(0 << REG_SW2_SCSI_ADDR_SHIFT, "0")
	PORT_DIPSETTING(1 << REG_SW2_SCSI_ADDR_SHIFT, "1")
	PORT_DIPSETTING(2 << REG_SW2_SCSI_ADDR_SHIFT, "2")
	PORT_DIPSETTING(3 << REG_SW2_SCSI_ADDR_SHIFT, "3")
	PORT_DIPSETTING(4 << REG_SW2_SCSI_ADDR_SHIFT, "4")
	PORT_DIPSETTING(5 << REG_SW2_SCSI_ADDR_SHIFT, "5")
	PORT_DIPSETTING(6 << REG_SW2_SCSI_ADDR_SHIFT, "6")
	PORT_DIPSETTING(7 << REG_SW2_SCSI_ADDR_SHIFT, "7")

	PORT_DIPNAME(REG_SW2_SCSI_PARITY, 0x00, "SCSI Parity")
	PORT_DIPSETTING(0, "Off")
	PORT_DIPSETTING(REG_SW2_SCSI_PARITY, "On")

INPUT_PORTS_END

ioport_constructor dio16_98265a_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98265a_port);
}

void dio16_98265a_device::device_start()
{
	save_item(NAME(m_irq_state));
	save_item(NAME(m_installed_io));
	save_item(NAME(m_control));
	save_item(NAME(m_dmar0));

	m_installed_io = false;
}

void dio16_98265a_device::device_reset()
{
	uint8_t code = m_sw1->read() >> REG_SW1_SELECT_CODE_SHIFT;
	code &= REG_SW1_SELECT_CODE_MASK;

	if (!m_installed_io) {
		program_space().install_readwrite_handler(
				0x600000 + (code * 0x10000),
				0x6007ff + (code * 0x10000),
				read16sm_delegate(*this, FUNC(dio16_98265a_device::io_r)),
				write16sm_delegate(*this, FUNC(dio16_98265a_device::io_w)));
		program_space().install_device(0x600020 + (code * 0x10000), 0x60003f + (code * 0x10000), *m_spc, &mb87030_device::map, 0x00ff00ff);
		m_installed_io = true;
	}
	m_control = 0;
	m_irq_state = false;
	m_spc->reset_w(true);
	m_spc->reset_w(false);
}

int dio16_98265a_device::get_int_level()
{
	return (m_sw1->read() >> REG_SW1_INT_LEVEL_SHIFT) &
			REG_SW1_INT_LEVEL_MASK;

}
uint16_t dio16_98265a_device::io_r(offs_t offset)
{

	uint16_t ret = 0xffff;

	switch (offset) {
	case 0: /* ID */
		ret = 0x07;
		break;
	case 1:
		ret = m_control | (m_irq_state ? 0x40 : 0) | (get_int_level() << 4);
		break;
	case 2:
		ret = m_spc->data_read();
		break;
	case 3:
		ret = 0x88; /* Flush/Configuration register */
		break;
	}
	LOG("io_r: offset=%02X ret=%02X\n",offset, ret);
	return ret;
}

void dio16_98265a_device::io_w(offs_t offset, uint16_t data)
{
	LOG("io_w: offset=%02X, data=%02X\n", offset, data);

	switch (offset) {
	case 0:
		device_reset();
		break;
	case 1:
		if (data & 0x80)
			update_irq(m_irq_state);
		else
			update_irq(false);
		m_control = data & 0x8f;
		update_dma();
		break;
	case 2:
		uint8_t val = 0;
		if (data & 0x80)
			val |= nscsi_device::S_REQ;
		if (data & 0x40)
			val |= nscsi_device::S_ACK;
		if (data & 0x08)
			val |= nscsi_device::S_BSY;
		if (data & 0x04)
			val |= nscsi_device::S_MSG;
		if (data & 0x02)
			val |= nscsi_device::S_CTL;
		if (data & 0x01)
			val |= nscsi_device::S_INP;
		m_spc->ctrl_write(val, nscsi_device::S_ALL);
		break;
	}
}

void dio16_98265a_device::update_irq(bool state)
{
	int irq_level = get_int_level();
	LOG("irq_w: %sassert %d\n", state ? "" : "de", irq_level+3);

	irq3_out(state && irq_level == 0);
	irq4_out(state && irq_level == 1);
	irq5_out(state && irq_level == 2);
	irq6_out(state && irq_level == 3);
}

void dio16_98265a_device::irq_w(int state)
{
	LOG("%s: %s\n", __FUNCTION__, state ? "true" : "false");

	if ((m_control & 0x80) && m_irq_state != state)
		update_irq(state);

	m_irq_state = state;
}

void dio16_98265a_device::dmack_w_in(int channel, uint8_t data)
{
	if(channel == 0 && !(m_control & REG_CONTROL_DE0))
		return;
	if(channel == 1 && !(m_control & REG_CONTROL_DE1))
		return;

	m_spc->dma_w(data);
}

uint8_t dio16_98265a_device::dmack_r_in(int channel)
{
	if(channel == 0 && !(m_control & REG_CONTROL_DE0))
		return 0xff;
	if(channel == 1 && !(m_control & REG_CONTROL_DE1))
		return 0xff;

	return m_spc->dma_r();
}

void dio16_98265a_device::update_dma()
{
	dmar0_out((m_control & REG_CONTROL_DE0) && m_dmar0);
	dmar1_out((m_control & REG_CONTROL_DE1) && m_dmar0);
}
void dio16_98265a_device::dmar0_w(int state)
{
	m_dmar0 = state;
	update_dma();

}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HPDIO_98265A, bus::hp_dio::device_dio16_card_interface, dio16_98265a_device, "hp98265a", "HP98265A SCSI S16 Interface")
