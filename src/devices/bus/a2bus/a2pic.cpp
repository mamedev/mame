// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "a2pic.h"

#include "bus/centronics/ctronics.h"

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

namespace {

class a2bus_pic_device : public device_t, public device_a2bus_card_interface
{
public:
	a2bus_pic_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// DIP switch/jumper handlers
	DECLARE_INPUT_CHANGED_MEMBER(sw1_strobe);
	DECLARE_INPUT_CHANGED_MEMBER(sw1_ack);
	DECLARE_INPUT_CHANGED_MEMBER(sw1_firmware);
	DECLARE_INPUT_CHANGED_MEMBER(sw1_irq);
	DECLARE_INPUT_CHANGED_MEMBER(x_data_out);
	DECLARE_INPUT_CHANGED_MEMBER(x_char_width);

	// device_a2bus_card_interface implementation
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual void write_cnxx(u8 offset, u8 data) override;

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// printer status inputs
	void ack_w(int state);
	void perror_w(int state);
	void select_w(int state);
	void fault_w(int state);

	// timer handlers
	TIMER_CALLBACK_MEMBER(release_strobe);

	// synchronised inputs
	void set_ack_in(s32 param);
	void set_perror_in(s32 param);
	void set_select_in(s32 param);
	void set_fault_in(s32 param);
	void data_write(s32 param);

	// helpers
	void reset_mode();
	void set_ack_latch();
	void clear_ack_latch();
	void enable_irq();
	void disable_irq();

	required_device<centronics_device>      m_printer_conn;
	required_device<output_latch_device>    m_printer_out;
	required_ioport                         m_input_sw1;
	required_ioport                         m_input_x;
	required_region_ptr<u8>                 m_prom;
	emu_timer *                             m_strobe_timer;

	u16 m_firmware_base;        // controlled by SW6
	u8  m_data_latch;           // 9B
	u8  m_autostrobe_disable;   // 2A pin 8
	u8  m_ack_latch;            // 2A pin 6
	u8  m_irq_enable;           // 2B pin 9
	u8  m_ack_in;
	u8  m_perror_in;            // DI5
	u8  m_select_in;            // DI6
	u8  m_fault_in;             // DI3
};



ROM_START(pic)
	ROM_REGION(0x0200, "prom", 0)
	ROM_LOAD( "341-0057.7b", 0x0000, 0x0200, CRC(0a6b084b) SHA1(de8aa285dcac88b1cc80ec9128b092833f5174b6) )
ROM_END


INPUT_PORTS_START(pic)
	PORT_START("SW1")
	PORT_DIPNAME(0x07, 0x07, "Strobe Length")               PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(   0x07, "1 microsecond")
	PORT_DIPSETTING(   0x06, "3 microseconds")
	PORT_DIPSETTING(   0x05, "5 microseconds")
	PORT_DIPSETTING(   0x04, "7 microseconds")
	PORT_DIPSETTING(   0x03, "9 microseconds")
	PORT_DIPSETTING(   0x02, "11 microseconds")
	PORT_DIPSETTING(   0x01, "13 microseconds")
	PORT_DIPSETTING(   0x00, "15 microseconds")
	PORT_DIPNAME(0x08, 0x00, "Strobe Output Polarity")      PORT_DIPLOCATION("SW1:4")       PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_pic_device, sw1_strobe, 0)
	PORT_DIPSETTING(   0x08, "Positive")
	PORT_DIPSETTING(   0x00, "Negative")
	PORT_DIPNAME(0x10, 0x00, "Acknowledge Input Polarity")  PORT_DIPLOCATION("SW1:5")       PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_pic_device, sw1_ack, 0)
	PORT_DIPSETTING(   0x10, "Positive")
	PORT_DIPSETTING(   0x00, "Negative")
	PORT_DIPNAME(0x20, 0x20, "Firmware")                    PORT_DIPLOCATION("SW1:6")       PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_pic_device, sw1_firmware, 0)
	PORT_DIPSETTING(   0x20, "Parallel Printer")    // ROM #341-0005 - auto LF after CR
	PORT_DIPSETTING(   0x00, "Centronics")          // ROM #341-0019 - no auto LF after CR
	PORT_DIPNAME(0x40, 0x40, "Interrupt")                   PORT_DIPLOCATION("SW1:7")       PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_pic_device, sw1_irq, 0)
	PORT_DIPSETTING(   0x40, "Disabled")
	PORT_DIPSETTING(   0x00, "Enabled")

	PORT_START("X")
	PORT_CONFNAME(0x01, 0x01, "PROM Addressing")
	PORT_CONFSETTING(   0x00, "Flat (X1)")
	PORT_CONFSETTING(   0x01, "Standard (X2)")
	PORT_CONFNAME(0x02, 0x02, "Data Output")                PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_pic_device, x_data_out, 0)
	PORT_CONFSETTING(   0x00, "Disabled (X3)")
	PORT_CONFSETTING(   0x02, "Enabled (X4)")
	PORT_CONFNAME(0x04, 0x04, "Character Width")            PORT_CHANGED_MEMBER(DEVICE_SELF, a2bus_pic_device, x_char_width, 0)
	PORT_CONFSETTING(   0x00, "7-bit (X5)")
	PORT_CONFSETTING(   0x04, "8-bit (X6)")
INPUT_PORTS_END



a2bus_pic_device::a2bus_pic_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, A2BUS_PIC, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_printer_conn(*this, "prn"),
	m_printer_out(*this, "prn_out"),
	m_input_sw1(*this, "SW1"),
	m_input_x(*this, "X"),
	m_prom(*this, "prom"),
	m_strobe_timer(nullptr),
	m_firmware_base(0x0000U),
	m_data_latch(0x00U),
	m_autostrobe_disable(0U),
	m_ack_latch(0U),
	m_irq_enable(0U),
	m_ack_in(1U),
	m_perror_in(1U),
	m_select_in(1U),
	m_fault_in(1U)
{
}



//----------------------------------------------
//  DIP switch/jumper handlers
//----------------------------------------------

INPUT_CHANGED_MEMBER(a2bus_pic_device::sw1_strobe)
{
	m_printer_conn->write_strobe(BIT(m_input_sw1->read(), 3) ^ (m_strobe_timer->enabled() ? 0U : 1U));
}


INPUT_CHANGED_MEMBER(a2bus_pic_device::sw1_ack)
{
	if (m_ack_in != BIT(m_input_sw1->read(), 4))
		set_ack_latch();
}


INPUT_CHANGED_MEMBER(a2bus_pic_device::sw1_firmware)
{
	m_firmware_base = BIT(m_input_sw1->read(), 5) << 8;
}


INPUT_CHANGED_MEMBER(a2bus_pic_device::sw1_irq)
{
	if (m_ack_latch && m_irq_enable)
	{
		if (BIT(m_input_sw1->read(), 6))
			lower_slot_irq();
		else
			raise_slot_irq();
	}
}


INPUT_CHANGED_MEMBER(a2bus_pic_device::x_data_out)
{
	if (m_data_latch && !BIT(m_input_x->read(), 1))
	{
		m_data_latch = 0x00U;
		m_printer_out->write(0x00U);
	}
}


INPUT_CHANGED_MEMBER(a2bus_pic_device::x_char_width)
{
	if (BIT(m_data_latch, 7))
		m_printer_out->write(m_data_latch & (BIT(m_input_x->read(), 2) ? 0xffU : 0x7fU));
}



//----------------------------------------------
//  device_a2bus_card_interface implementation
//----------------------------------------------

u8 a2bus_pic_device::read_c0nx(u8 offset)
{
	if (!machine().side_effects_disabled())
		LOG("Read C0n%01X\n", offset);

	switch (offset & 0x07U)
	{
	case 3U:
		return 0x97U | (m_perror_in << 5) | (m_select_in << 6) | (m_fault_in << 3);

	case 4U:
		return (m_ack_latch << 7) | (m_ack_in ^ BIT(m_input_sw1->read(), 4));

	case 5U:
		logerror("500ns negative strobe not implemented\n");
		break;

	case 6U:
		if (!machine().side_effects_disabled())
			enable_irq();
		break;

	case 7U:
		if (!machine().side_effects_disabled())
			reset_mode();
		break;
	}

	return 0x00U;
}

void a2bus_pic_device::write_c0nx(u8 offset, u8 data)
{
	LOG("Write C0n%01X=%02X\n", offset, data);

	switch (offset & 0x07U)
	{
	case 0U:
		if (BIT(m_input_x->read(), 1))
		{
			// latch output data and start strobe if autostrobe is enabled
			LOG("Latch data %02X\n", data);
			machine().scheduler().synchronize(
					timer_expired_delegate(FUNC(a2bus_pic_device::data_write), this),
					unsigned(data) | (1 << 8) | ((m_autostrobe_disable ? 0 : 1) << 9));
		}
		else
		{
			// just start strobe if autostrobe is enabled
			LOG("Output disabled, not latching data\n");
			if (!m_autostrobe_disable)
				machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_pic_device::data_write), this), 1 << 9);
		}
		break;

	case 2U:
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_pic_device::data_write), this), 1 << 9);
		break;

	case 5U:
		logerror("500ns negative strobe not implemented\n");
		break;

	case 6U:
		enable_irq();
		break;

	case 7U:
		reset_mode();
		break;
	}
}


u8 a2bus_pic_device::read_cnxx(u8 offset)
{
	if (BIT(m_input_x->read(), 0))
	{
		if (!BIT(offset, 6) || (BIT(offset, 7) && !m_ack_latch))
			offset |= 0x40U;
		else
			offset &= 0xbfU;
	}

	if (!machine().side_effects_disabled())
	{
		if (m_autostrobe_disable)
			LOG("Enabling autostrobe\n");
		m_autostrobe_disable = 0U;
	}

	return m_prom[m_firmware_base | offset];
}


void a2bus_pic_device::write_cnxx(u8 offset, u8 data)
{
	LOG("Write Cn%02X=%02X (bus conflict)\n", offset, data);

	if (m_autostrobe_disable)
		LOG("Enabling autostrobe\n");
	m_autostrobe_disable = 0U;
}



//----------------------------------------------
//  device_t implementation
//----------------------------------------------

tiny_rom_entry const *a2bus_pic_device::device_rom_region() const
{
	return ROM_NAME(pic);
}


void a2bus_pic_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_printer_conn, centronics_devices, "printer");
	m_printer_conn->ack_handler().set(FUNC(a2bus_pic_device::ack_w));
	m_printer_conn->perror_handler().set(FUNC(a2bus_pic_device::perror_w));
	m_printer_conn->select_handler().set(FUNC(a2bus_pic_device::select_w));
	m_printer_conn->fault_handler().set(FUNC(a2bus_pic_device::fault_w));

	OUTPUT_LATCH(config, m_printer_out);
	m_printer_conn->set_output_latch(*m_printer_out);
}


ioport_constructor a2bus_pic_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pic);
}


void a2bus_pic_device::device_start()
{
	m_strobe_timer = timer_alloc(FUNC(a2bus_pic_device::release_strobe), this);

	m_firmware_base = 0x0100U;
	m_data_latch = 0xffU;

	save_item(NAME(m_data_latch));
	save_item(NAME(m_autostrobe_disable));
	save_item(NAME(m_ack_latch));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_ack_in));
	save_item(NAME(m_perror_in));
	save_item(NAME(m_select_in));
	save_item(NAME(m_fault_in));
}


void a2bus_pic_device::device_reset()
{
	ioport_value const sw1(m_input_sw1->read());
	ioport_value const x(m_input_x->read());

	m_firmware_base = BIT(sw1, 5) << 8;
	m_autostrobe_disable = 1U;

	if (!BIT(x, 1))
	{
		m_data_latch = 0x00U;
		m_printer_out->write(0x00U);
	}

	m_printer_conn->write_strobe(BIT(sw1, 3) ^ (m_strobe_timer->enabled() ? 0U : 1U));

	reset_mode();
}



//----------------------------------------------
//  printer status inputs
//----------------------------------------------

void a2bus_pic_device::ack_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_pic_device::set_ack_in), this), state ? 1 : 0);
}


void a2bus_pic_device::perror_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_pic_device::set_perror_in), this), state ? 1 : 0);
}


void a2bus_pic_device::select_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_pic_device::set_select_in), this), state ? 1 : 0);
}


void a2bus_pic_device::fault_w(int state)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(a2bus_pic_device::set_fault_in), this), state ? 1 : 0);
}



//----------------------------------------------
//  timer handlers
//----------------------------------------------

TIMER_CALLBACK_MEMBER(a2bus_pic_device::release_strobe)
{
	int const state(BIT(~m_input_sw1->read(), 3));
	LOG("Output /STROBE=%d\n", state);
	m_printer_conn->write_strobe(state);
}



//----------------------------------------------
//  synchronised inputs
//----------------------------------------------

void a2bus_pic_device::set_ack_in(s32 param)
{
	if (u32(param) != m_ack_in)
	{
		m_ack_in = u8(u32(param));
		LOG("/ACK=%u\n", m_ack_in);
		if (started() && (m_ack_in != BIT(m_input_sw1->read(), 4)))
		{
			LOG("Active /ACK edge\n");
			set_ack_latch();
		}
	}
}


void a2bus_pic_device::set_perror_in(s32 param)
{
	if (u32(param) != m_perror_in)
	{
		m_perror_in = u8(u32(param));
		LOG("PAPER EMPTY=%u\n", m_perror_in);
	}
}


void a2bus_pic_device::set_select_in(s32 param)
{
	if (u32(param) != m_select_in)
	{
		m_select_in = u8(u32(param));
		LOG("SELECT=%u\n", m_select_in);
	}
}


void a2bus_pic_device::set_fault_in(s32 param)
{
	if (u32(param) != m_fault_in)
	{
		m_fault_in = u8(u32(param));
		LOG("/FAULT=%u\n", m_fault_in);
	}
}


void a2bus_pic_device::data_write(s32 param)
{
	// latch output data - remember MSB can be forced low by jumper
	if (BIT(param, 8))
	{
		m_data_latch = u8(u32(param));
		m_printer_out->write(m_data_latch & (BIT(m_input_x->read(), 2) ? 0xffU : 0x7fU));
	}

	// start/extend strobe output
	if (BIT(param, 9))
	{
		ioport_value const sw1(m_input_sw1->read());
		unsigned const cycles(15U - ((sw1 & 0x07U) << 1));
		int const state(BIT(sw1, 3));
		if (!m_strobe_timer->enabled())
		{
			LOG("Output /STROBE=%d for %u cycles\n", state, cycles);
			clear_ack_latch();
			m_printer_conn->write_strobe(state);
		}
		else
		{
			LOG("Adjust /STROBE=%d remaining to %u cycles\n", state, cycles);
		}
		m_strobe_timer->adjust(attotime::from_ticks(cycles * 7, clock()));
	}
}



//----------------------------------------------
//  helpers
//----------------------------------------------

void a2bus_pic_device::reset_mode()
{
	if (!m_autostrobe_disable)
		LOG("Disabling autostrobe\n");
	else
		LOG("Autostrobe already disabled\n");
	m_autostrobe_disable = 1U;
	disable_irq();
	set_ack_latch();
}


void a2bus_pic_device::set_ack_latch()
{
	if (m_strobe_timer->enabled())
	{
		LOG("Active strobe prevents acknowledge latch from being set\n");
	}
	else if (!m_ack_latch)
	{
		LOG("Setting acknowledge latch\n");
		m_ack_latch = 1U;
		if (m_irq_enable && !BIT(m_input_sw1->read(), 6))
		{
			LOG("Asserting slot IRQ\n");
			raise_slot_irq();
		}
	}
	else
	{
		LOG("Acknowledge latch already set\n");
	}
}


void a2bus_pic_device::clear_ack_latch()
{
	if (m_ack_latch)
	{
		LOG("Clearing acknowledge latch\n");
		m_ack_latch = 0U;
		if (m_irq_enable && !BIT(m_input_sw1->read(), 6))
		{
			LOG("Releasing slot IRQ\n");
			lower_slot_irq();
		}
	}
	else
	{
		LOG("Acknowledge latch already clear\n");
	}
}


void a2bus_pic_device::enable_irq()
{
	if (!m_irq_enable)
	{
		LOG("Enabling IRQ\n");
		m_irq_enable = 1U;
		if (m_ack_latch && !BIT(m_input_sw1->read(), 6))
		{
			LOG("Asserting slot IRQ\n");
			raise_slot_irq();
		}
	}
	else
	{
		LOG("IRQ already enabled\n");
	}
}


void a2bus_pic_device::disable_irq()
{
	if (m_irq_enable)
	{
		LOG("Disabling IRQ\n");
		m_irq_enable = 0U;
		if (m_ack_latch && !BIT(m_input_sw1->read(), 6))
		{
			LOG("Releasing slot IRQ\n");
			lower_slot_irq();
		}
	}
	else
	{
		LOG("IRQ already disabled\n");
	}
}

} // anonymous namespace



DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_PIC, device_a2bus_card_interface, a2bus_pic_device, "a2pic", "Apple II Parallel Interface Card")
