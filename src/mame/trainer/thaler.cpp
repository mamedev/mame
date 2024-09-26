// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Thaler CT-65 / MPS-65

    TODO:
    - CT-65 cassette interface (add-on board for MPS-65)

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"

#include "mps65.lh"


namespace {

class mps65_state : public driver_device
{
public:
	mps65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keypad(*this, "COL%u", 1U)
		, m_digit(*this, "digit%u", 1U)
		, m_keycol(0)
	{ }

	void mps65(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(reset_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<6> m_keypad;
	output_finder<6> m_digit;

	void mps65_map(address_map &map) ATTR_COLD;

	uint8_t pa_r();
	void pa_w(uint8_t data);
	void pb_w(uint8_t data);

	uint8_t m_keycol;
};


void mps65_state::machine_start()
{
	m_digit.resolve();

	save_item(NAME(m_keycol));
}


uint8_t mps65_state::pa_r()
{
	uint8_t data = 0xff;

	if (m_keycol > 0 && m_keycol < 7)
		data = m_keypad[m_keycol - 1]->read() & 0x0f;

	return data;
}

void mps65_state::pa_w(uint8_t data)
{
	m_keycol = (data >> 4) & 7;
}

void mps65_state::pb_w(uint8_t data)
{
	if (m_keycol > 0 && m_keycol < 7)
	{
		m_digit[m_keycol - 1] = bitswap<8>(data, 7, 3, 5, 0, 1, 2, 4, 6) & 0x7f;
	}
}


void mps65_state::mps65_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0xa000, 0xa00f).mirror(0xf0).m("via", FUNC(via6522_device::map));
	map(0xf000, 0xffff).rom().region("rom", 0);
}


static INPUT_PORTS_START( mps65 )
	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_NAME("Save")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_NAME("Load")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_NAME("Address")

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_NAME("Index")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_NAME("Backstep")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13) PORT_NAME("Enter")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, mps65_state, reset_changed, 0)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(mps65_state::reset_changed)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	if (newval)
	{
		machine().schedule_soft_reset();
	}
}


void mps65_state::mps65(machine_config &config)
{
	M6502(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mps65_state::mps65_map);

	config.set_default_layout(layout_mps65);

	via6522_device &via(MOS6522(config, "via", 1_MHz_XTAL));
	via.readpa_handler().set(FUNC(mps65_state::pa_r));
	via.writepa_handler().set(FUNC(mps65_state::pa_w));
	via.writepb_handler().set(FUNC(mps65_state::pb_w));
	via.irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);
}


ROM_START( ct65 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("ct6502.bin", 0x0000, 0x1000, CRC(832e0d2a) SHA1(f5f7d00360747a194fca01fb95f31c092505b523))
ROM_END

ROM_START( mps65 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_LOAD("mps65.bin", 0x0800, 0x0800, CRC(512c8cde) SHA1(b6a43ad26d30bcf088b72f3c29e9bcbbf371f4cf))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT    CLASS         INIT         COMPANY    FULLNAME   FLAGS
COMP( 1982, ct65,   mps65,  0,      mps65,    mps65,   mps65_state,  empty_init,  "Thaler",  "CT-65",   MACHINE_NO_SOUND_HW )
COMP( 1984, mps65,  0,      0,      mps65,    mps65,   mps65_state,  empty_init,  "Thaler",  "MPS-65",  MACHINE_NO_SOUND_HW )
