// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga 500 Keyboard

    TODO: - Move 6500/1 to its own CPU core so that it can be shared with
            other systems

***************************************************************************/

#include "emu.h"
#include "a500.h"
#include "matrix.h"

#include "machine/rescap.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_NS(A500_KBD_US, bus::amiga::keyboard, a500_kbd_us_device, "a500_kbd_us", "Amiga 500 Keyboard (U.S./Canada)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_DE, bus::amiga::keyboard, a500_kbd_de_device, "a500_kbd_de", "Amiga 500 Keyboard (Germany/Austria)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_FR, bus::amiga::keyboard, a500_kbd_fr_device, "a500_kbd_fr", "Amiga 500 Keyboard (France/Belgium)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_IT, bus::amiga::keyboard, a500_kbd_it_device, "a500_kbd_it", "Amiga 500 Keyboard (Italy)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_SE, bus::amiga::keyboard, a500_kbd_se_device, "a500_kbd_se", "Amiga 500 Keyboard (Sweden)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_ES, bus::amiga::keyboard, a500_kbd_es_device, "a500_kbd_es", "Amiga 500 Keyboard (Spain)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_DK, bus::amiga::keyboard, a500_kbd_dk_device, "a500_kbd_dk", "Amiga 500 Keyboard (Denmark)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_CH, bus::amiga::keyboard, a500_kbd_ch_device, "a500_kbd_ch", "Amiga 500 Keyboard (Switzerland)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_NO, bus::amiga::keyboard, a500_kbd_no_device, "a500_kbd_no", "Amiga 500 Keyboard (Norway)")
DEFINE_DEVICE_TYPE_NS(A500_KBD_GB, bus::amiga::keyboard, a500_kbd_gb_device, "a500_kbd_gb", "Amiga 500 Keyboard (UK)")


namespace bus { namespace amiga { namespace keyboard {

ADDRESS_MAP_START(a500_kbd_device::mpu6500_map)
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x000, 0x03f) AM_RAM
	AM_RANGE(0x080, 0x080) AM_READWRITE(port_a_r, port_a_w)
	AM_RANGE(0x081, 0x081) AM_READ_PORT("special") AM_WRITE(port_b_w)
	AM_RANGE(0x082, 0x082) AM_WRITE(port_c_w)
	AM_RANGE(0x083, 0x083) AM_WRITE(port_d_w)
	AM_RANGE(0x084, 0x085) AM_WRITE(latch_w)
	AM_RANGE(0x086, 0x087) AM_READ(counter_r)
	AM_RANGE(0x088, 0x088) AM_WRITE(transfer_latch_w)
	AM_RANGE(0x089, 0x089) AM_WRITE(clear_pa0_detect)
	AM_RANGE(0x08a, 0x08a) AM_WRITE(clear_pa1_detect)
	AM_RANGE(0x08f, 0x08f) AM_READWRITE(control_r, control_w)
	AM_RANGE(0x090, 0x0ff) AM_NOP
	AM_RANGE(0x800, 0xfff) AM_ROM AM_REGION("ic1", 0)
ADDRESS_MAP_END

namespace {

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( kbd_pcb )
	ROM_REGION(0x800, "ic1", 0)
	ROM_LOAD("328191-02.ic1", 0x000, 0x800, CRC(4a3fc332) SHA1(83b21d0c8b93fc9b9b3b287fde4ec8f3badac5a2))
ROM_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_PORTS_START( a500_special )
	PORT_START("special")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LWIN)        PORT_CHAR(UCHAR_MAMEKEY(LWIN))       PORT_NAME("Left Amiga")   PORT_CHANGED_MEMBER(DEVICE_SELF, a500_kbd_device, check_reset, nullptr)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)        PORT_CHAR(UCHAR_MAMEKEY(LALT))       PORT_NAME("Left Alt")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)             PORT_NAME("Left Shift")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))   PORT_NAME("Ctrl")         PORT_CHANGED_MEMBER(DEVICE_SELF, a500_kbd_device, check_reset, nullptr)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RWIN)        PORT_CHAR(UCHAR_MAMEKEY(RWIN))       PORT_NAME("Right Amiga")  PORT_CHANGED_MEMBER(DEVICE_SELF, a500_kbd_device, check_reset, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_SHIFT_2)             PORT_NAME("Right Alt")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))     PORT_NAME("Right Shift")
INPUT_PORTS_END

INPUT_PORTS_START( a500_us_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_us)
INPUT_PORTS_END

INPUT_PORTS_START( a500_de_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_de)
INPUT_PORTS_END

INPUT_PORTS_START( a500_fr_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_fr)
INPUT_PORTS_END

INPUT_PORTS_START( a500_it_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_it)
INPUT_PORTS_END

INPUT_PORTS_START( a500_se_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_se)
INPUT_PORTS_END

INPUT_PORTS_START( a500_es_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_es)
INPUT_PORTS_END

INPUT_PORTS_START( a500_dk_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_dk)
INPUT_PORTS_END

INPUT_PORTS_START( a500_ch_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_ch)
INPUT_PORTS_END

INPUT_PORTS_START( a500_no_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_no)
INPUT_PORTS_END

INPUT_PORTS_START( a500_gb_keyboard )
	PORT_INCLUDE(a500_special)
	PORT_INCLUDE(matrix_gb)
INPUT_PORTS_END

} // anonymous namespace


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------


MACHINE_CONFIG_START(a500_kbd_device::device_add_mconfig)
	MCFG_CPU_ADD("ic1", M6502, XTAL(3'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(mpu6500_map)
MACHINE_CONFIG_END

const tiny_rom_entry *a500_kbd_device::device_rom_region() const
{
	return ROM_NAME(kbd_pcb);
}

ioport_constructor a500_kbd_us_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_us_keyboard ); }
ioport_constructor a500_kbd_de_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_de_keyboard ); }
ioport_constructor a500_kbd_fr_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_fr_keyboard ); }
ioport_constructor a500_kbd_it_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_it_keyboard ); }
ioport_constructor a500_kbd_se_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_se_keyboard ); }
ioport_constructor a500_kbd_es_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_es_keyboard ); }
ioport_constructor a500_kbd_dk_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_dk_keyboard ); }
ioport_constructor a500_kbd_ch_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_ch_keyboard ); }
ioport_constructor a500_kbd_no_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_no_keyboard ); }
ioport_constructor a500_kbd_gb_device::device_input_ports() const { return INPUT_PORTS_NAME( a500_gb_keyboard ); }


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a500_kbd_device::a500_kbd_device(
		const machine_config &mconfig,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		device_type type) :
	device_t(mconfig, type, tag, owner, clock),
	device_amiga_keyboard_interface(mconfig, *this),
	m_mpu(*this, "ic1"),
	m_special(*this, "special"),
	m_rows(*this, "ROW%u", 0),
	m_timer(nullptr),
	m_watchdog(nullptr),
	m_reset(nullptr),
	m_host_kdat(1),
	m_mpu_kdat(1),
	m_kclk(1),
	m_port_c(0xff),
	m_port_d(0xff),
	m_latch(0xffff),
	m_counter(0xffff),
	m_control(0x00)
{
}


a500_kbd_us_device::a500_kbd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_US)
{
}

a500_kbd_de_device::a500_kbd_de_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_DE)
{
}

a500_kbd_fr_device::a500_kbd_fr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_FR)
{
}

a500_kbd_it_device::a500_kbd_it_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_IT)
{
}

a500_kbd_se_device::a500_kbd_se_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_SE)
{
}

a500_kbd_es_device::a500_kbd_es_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_ES)
{
}

a500_kbd_dk_device::a500_kbd_dk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_DK)
{
}

a500_kbd_ch_device::a500_kbd_ch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_CH)
{
}

a500_kbd_no_device::a500_kbd_no_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_NO)
{
}

a500_kbd_gb_device::a500_kbd_gb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a500_kbd_device(mconfig, tag, owner, clock, A500_KBD_GB)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a500_kbd_device::device_start()
{
	// allocate timers
	m_timer = timer_alloc(0, nullptr);
	m_watchdog = timer_alloc(1, nullptr);
	m_reset = timer_alloc(2, nullptr);

	// register for save states
	save_item(NAME(m_host_kdat));
	save_item(NAME(m_mpu_kdat));
	save_item(NAME(m_kclk));
	save_item(NAME(m_port_c));
	save_item(NAME(m_port_d));
	save_item(NAME(m_latch));
	save_item(NAME(m_counter));
	save_item(NAME(m_control));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a500_kbd_device::device_reset()
{
	// stack starts 0
	m_mpu->set_state_int(M6502_S, 0);

	m_host_kdat = 1;
	m_mpu_kdat = 1;
	m_kclk = 1;
	m_port_c = 0xff;
	m_port_d = 0xff;
	m_latch = 0xffff;   // not initialized by hardware
	m_counter = 0xffff; // not initialized by hardware
	m_control = 0x00;

	m_timer->adjust(attotime::zero, 0, attotime::from_hz(XTAL(3'000'000) / 2));
	m_watchdog->adjust(attotime::from_msec(54));
}

void a500_kbd_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	switch (tid)
	{
	// 6500/1 internal timer
	case 0:
		switch (m_control & 0x03)
		{
		// interval timer, pulse width measurement (connected to gnd here)
		case 0:
		case 3:
			if (m_counter-- == 0)
			{
				// counter overflow
				m_control |= COUNTER_OVERFLOW;
				m_counter = m_latch;

				// generate interrupt?
				update_irqs();
			}
			break;

		// pulse generator
		case 1:
			break;

		// event counter
		case 2:
			break;
		}
		break;

	// watchdog
	case 1:
		m_mpu->reset();
		m_watchdog->adjust(attotime::from_msec(54));
		break;

	// keyboard reset timer
	case 2:
		m_host->krst_w(1);
		break;
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

INPUT_CHANGED_MEMBER( a500_kbd_device::check_reset )
{
	uint8_t keys = m_special->read();

	// ctrl-amiga-amiga pressed?
	if (!BIT(keys, 6) && !BIT(keys, 3) && !BIT(keys, 2))
	{
		m_host->krst_w(0);
		m_reset->adjust(PERIOD_OF_555_MONOSTABLE(RES_K(47), CAP_U(10)));
	}
}

void a500_kbd_device::update_irqs()
{
	if ((m_control & PA1_INT_ENABLED) && (m_control & PA1_NEGATIVE_EDGE))
		m_mpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

	else if ((m_control & PA0_INT_ENABLED) && (m_control & PA0_POSITIVE_EDGE))
		m_mpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

	else if ((m_control & COUNTER_INT_ENABLED) && (m_control & COUNTER_OVERFLOW))
		m_mpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);

	else
		m_mpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
}

READ8_MEMBER( a500_kbd_device::port_a_r )
{
	uint8_t data = 0xfc;

	// kdat & kclk
	data |= (m_host_kdat & m_mpu_kdat) << 0;
	data |= m_kclk << 1;

	// scan port d and c rows
	uint16_t const row_drive = (uint16_t(m_port_d | 0x80) << 8) | uint16_t(m_port_c);
	for (unsigned i = 0; i < m_rows.size(); i++)
		if (!BIT(row_drive, m_rows.size() - i - 1)) data &= m_rows[i]->read();

	return data;
}

WRITE8_MEMBER( a500_kbd_device::port_a_w )
{
	// look for pa0 edge
	if (m_host_kdat && !m_mpu_kdat && BIT(data, 0))
	{
		m_control |= PA0_POSITIVE_EDGE;
		update_irqs();
	}

	// and pa1 edge
	if (m_kclk && !BIT(data, 1))
	{
		m_control |= PA1_NEGATIVE_EDGE;
		update_irqs();
	}

	// update with new values and output
	if (m_mpu_kdat != BIT(data, 0))
	{
		m_mpu_kdat = BIT(data, 0);
		m_host->kdat_w(m_mpu_kdat);
	}

	if (m_kclk != BIT(data, 1))
	{
		m_kclk = BIT(data, 1);
		m_host->kclk_w(m_kclk);
	}
}

WRITE8_MEMBER( a500_kbd_device::port_b_w )
{
	// caps lock led
	machine().output().set_value("led_kbd_caps", BIT(data, 7));
}

WRITE8_MEMBER( a500_kbd_device::port_c_w )
{
	m_port_c = data;
}

WRITE8_MEMBER( a500_kbd_device::port_d_w )
{
	// reset watchdog on 0 -> 1 transition
	if (!BIT(m_port_d, 7) && BIT(data, 7))
		m_watchdog->adjust(attotime::from_msec(54));

	m_port_d = data;
}

WRITE8_MEMBER( a500_kbd_device::latch_w )
{
	if (offset == 0)
	{
		m_latch &= 0x00ff;
		m_latch |= data << 8;
	}
	else
	{
		m_latch &= 0xff00;
		m_latch |= data << 0;
	}
}

READ8_MEMBER( a500_kbd_device::counter_r )
{
	if (!machine().side_effect_disabled())
	{
		m_control &= ~COUNTER_OVERFLOW;
		update_irqs();
	}

	if (offset == 0)
		return m_counter >> 8;
	else
		return m_counter >> 0;
}

WRITE8_MEMBER( a500_kbd_device::transfer_latch_w )
{
	m_control &= ~COUNTER_OVERFLOW;
	update_irqs();

	m_latch &= 0x00ff;
	m_latch |= data << 8;

	m_counter = m_latch;
}

WRITE8_MEMBER( a500_kbd_device::clear_pa0_detect )
{
	m_control &= ~PA0_POSITIVE_EDGE;
	update_irqs();
}

WRITE8_MEMBER( a500_kbd_device::clear_pa1_detect )
{
	m_control &= ~PA1_NEGATIVE_EDGE;
	update_irqs();
}

READ8_MEMBER( a500_kbd_device::control_r )
{
	return m_control;
}

WRITE8_MEMBER( a500_kbd_device::control_w )
{
	m_control = data;
	update_irqs();
}

WRITE_LINE_MEMBER( a500_kbd_device::kdat_w )
{
	// detect positive edge
	if (!m_host_kdat && m_mpu_kdat && state)
	{
		m_control |= PA0_POSITIVE_EDGE;
		update_irqs();
	}

	m_host_kdat = state;
}

} } } // namespace bus::amiga::keyboard
