// license:BSD-3-Clause
// copyright-holders: R. Belmont
/*********************************************************************

    a9m0331.cpp
    Apple Desktop Bus Mouse (model A9M0331)
    by R. Belmont

    Logitech-designed, built around an MC68705P3-class MCU (marked ZC84506P).
    The firmware identifies itself as
    "3.2 870514 R. Sommer, Switzerland Copyright LOGITECH 1987".

    Port A: bit 7:   Button 1, active low.  Reported in R0 bit 15.
            bit 6:   Button 2, active low.  Reported in R0 bit 7.  Not fitted.
            bit 5:   Unused, reads as 1.
            bit 4:   Handler ID strap.  High selects handler 2 (200 cpi), low
                     selects handler 1 (100 cpi): $04AB is BRCLR, so it is a set
                     bit that falls through into the INC $4B at $04AE.
            bits 3-0: Outputs once DDRA is set to $0F, otherwise unused.  These
                     must read back as 1 before that: the reset entropy loop at
                     $048C shifts PA0 into $51 28 times, and if bit 7 of $51 ends
                     up clear the firmware hangs forever in the factory test loop
                     at $045B.

    Port B: bit 0:   ADB data, open collector.  The latch bit is always 0, so
                     DDRB selects between pulling the bus low ($FF) and releasing
                     it ($F0).
            bits 7-4: Encoder illumination / scan drive.  Driven low at the head
                     of every Port C sample and high again between the two halves
                     of the sample.

    Port C: bits 1-0: X quadrature channel.
            bits 3-2: Y quadrature channel.

    /IRQ:   The ADB line, asserted when the bus is low.  The falling edge drives
            the receive state machine and BIL/BIH read the live level.

    TIMER:  Also the ADB line.  The timer only counts while the ADB line is high,
	        which gets cheap pulse-width measurement without polling/cycle counting.

*********************************************************************/

#include "emu.h"
#include "a9m0331.h"

#include "cpu/m6805/m68705.h"

#define LOG_LINE (1U << 1)  // every bus transition with the time since the last one

#define VERBOSE (0)

#include "logmacro.h"

namespace {

class a9m0331_device : public adb_device_interface, public adb_slot_card_interface
{
public:
	a9m0331_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void adb_w(int state) override;

	required_device<m68705p3_device> m_mcu;
	required_ioport m_buttons, m_mouse_x, m_mouse_y, m_handler;

private:
	u8 porta_r();
	u8 portc_r();
	void portb_w(offs_t offset, u8 data, u8 mem_mask);

	TIMER_CALLBACK_MEMBER(sample_inputs);
	void step_encoder();

	emu_timer *m_sample_timer;
	attotime m_last_edge;

	int m_adb_drive;
	u8 m_portb_last;
	u8 m_x_phase, m_y_phase;
	s16 m_x_pending, m_y_pending;
	u8 m_last_x, m_last_y;
};

ROM_START(a9m0331)
	ROM_REGION(0x800, "mcu", 0)
	// "3.2 870514 R. Sommer, Switzerland Copyright LOGITECH 1987"
	ROM_LOAD( "zc84506p_1987_ea_3.2_trim.bin", 0x000000, 0x000800, CRC(85f858d9) SHA1(61ebad9953f4f88b6fcabf498f3875918493d138) )
ROM_END

static INPUT_PORTS_START(a9m0331)
	PORT_START("button")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("mousex")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("mousey")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("handler")
	PORT_CONFNAME(0x10, 0x00, "Resolution")     // PA4 strap
	PORT_CONFSETTING(0x00, "100 cpi (handler ID 1)")
	PORT_CONFSETTING(0x10, "200 cpi (handler ID 2)")
INPUT_PORTS_END

ioport_constructor a9m0331_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(a9m0331);
}

void a9m0331_device::device_add_mconfig(machine_config &config)
{
	// Crystal value is unknown, but 3 MHz gives a spec-perfect 100 us ADB bit cell
	M68705P3(config, m_mcu, 3_MHz_XTAL);
	m_mcu->porta_r().set(FUNC(a9m0331_device::porta_r));
	m_mcu->portb_w().set(FUNC(a9m0331_device::portb_w));
	m_mcu->portc_r().set(FUNC(a9m0331_device::portc_r));
}

const tiny_rom_entry *a9m0331_device::device_rom_region() const
{
	return ROM_NAME(a9m0331);
}

a9m0331_device::a9m0331_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	adb_device_interface(mconfig, ADB_A9M0331, tag, owner, clock),
	adb_slot_card_interface(mconfig, *this, DEVICE_SELF),
	m_mcu(*this, "mcu"),
	m_buttons(*this, "button"),
	m_mouse_x(*this, "mousex"),
	m_mouse_y(*this, "mousey"),
	m_handler(*this, "handler"),
	m_sample_timer(nullptr),
	m_adb_drive(ASSERT_LINE),
	m_portb_last(0xff),
	m_x_phase(0), m_y_phase(0),
	m_x_pending(0), m_y_pending(0),
	m_last_x(0), m_last_y(0)
{
}

void a9m0331_device::device_start()
{
	adb_device_interface::device_start();

	m_sample_timer = timer_alloc(FUNC(a9m0331_device::sample_inputs), this);

	save_item(NAME(m_adb_drive));
	save_item(NAME(m_portb_last));
	save_item(NAME(m_x_phase));
	save_item(NAME(m_y_phase));
	save_item(NAME(m_x_pending));
	save_item(NAME(m_y_pending));
	save_item(NAME(m_last_x));
	save_item(NAME(m_last_y));
}

void a9m0331_device::device_reset()
{
	adb_device_interface::device_reset();

	m_adb_drive = ASSERT_LINE;
	m_portb_last = 0xff;
	m_x_phase = m_y_phase = 0;
	m_x_pending = m_y_pending = 0;
	m_last_x = m_mouse_x->read();
	m_last_y = m_mouse_y->read();

	m_sample_timer->adjust(attotime::from_hz(1000), 0, attotime::from_hz(1000));
}

void a9m0331_device::adb_w(int state)
{
	if (VERBOSE & LOG_LINE)
	{
		attotime const now = machine().time();
		LOGMASKED(LOG_LINE, "ADB %d after %6d us\n", state, int((now - m_last_edge).as_double() * 1000000.0));
		m_last_edge = now;
	}

	m_mcu->set_input_line(M6805_IRQ_LINE, state ? CLEAR_LINE : ASSERT_LINE);
	
	m_mcu->timer_w(state);
}

u8 a9m0331_device::porta_r()
{
	u8 const buttons = m_buttons->read();

	return 0x2f
		| m_handler->read()
		| (BIT(~buttons, 0) << 7)
		| (BIT(~buttons, 1) << 6);
}

u8 a9m0331_device::portc_r()
{
	static constexpr u8 gray_code[4] = { 0, 1, 3, 2 };

	return (gray_code[m_y_phase] << 2) | gray_code[m_x_phase];
}

void a9m0331_device::portb_w(offs_t offset, u8 data, u8 mem_mask)
{
	data |= ~mem_mask;

	int const drive = BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE;
	if (drive != m_adb_drive)
	{
		m_adb_drive = drive;
		adb_drive_w(drive);
		m_mcu->yield();
	}

	if ((m_portb_last & 0xf0) && !(data & 0xf0))
	{
		step_encoder();
	}
	m_portb_last = data;
}

TIMER_CALLBACK_MEMBER(a9m0331_device::sample_inputs)
{
	u8 const x = m_mouse_x->read();
	u8 const y = m_mouse_y->read();

	m_x_pending = std::clamp<s32>(m_x_pending + 2 * s8(x - m_last_x), -255, 255);
	m_y_pending = std::clamp<s32>(m_y_pending + 2 * s8(y - m_last_y), -255, 255);

	m_last_x = x;
	m_last_y = y;
}

void a9m0331_device::step_encoder()
{
	if (m_x_pending)
	{
		int const dir = (m_x_pending > 0) ? 1 : -1;
		m_x_phase = (m_x_phase + dir) & 3;
		m_x_pending -= dir;
	}

	if (m_y_pending)
	{
		int const dir = (m_y_pending > 0) ? 1 : -1;
		m_y_phase = (m_y_phase - dir) & 3;
		m_y_pending -= dir;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ADB_A9M0331, adb_slot_card_interface, a9m0331_device, "a9m0331", "Apple ADB Mouse (A9M0331)");
