// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
	Casiotone RC-1

	This is the rhythm component of the "Symphonytron" modular organ system.
	It contains a uPD930 and analog percussion circuits (similar to several
	contemporary keyboard models) that are controlled by a uPD7801.

	In addition to that, it also handles passing messages between the other
	modules via four DIN connectors, and generates the address/data/strobe
	signals for the MB-1's RAM cartridge.

	See ct8000.cpp for the other modules.

	TODO (in no particular order):
	- all sound hardware. the uPD930 is actually a separate CPU, but in this
	  case it may be HLE-able depending on how the main CPU talks to it
	- add MIDI in and out (latter not implemented in the MIDI adapter yet)
	- layout, etc
	- some controls are supposed to behave like positional switches
	- RAM cart - technically part of the MB-1, but it's entirely controlled
	  by the RC-1 via one of the DIN jacks
***************************************************************************/

#include "emu.h"

#include "ct8000_midi.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"

#define LOG_VCO (1<<1)

//#define VERBOSE (LOG_GENERAL | LOG_VCO)

#include "logmacro.h"

namespace {

class ctrc1_state : public driver_device
{
public:
	ctrc1_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_port_upper(*this, "port_upper")
		, m_port_lower(*this, "port_lower")
		, m_port_bass(*this, "port_bass")
		, m_port_mb1(*this, "port_mb1")
		, m_inputs(*this, "KC%u", 0)
	{
	}

	void ctrc1(machine_config &config) ATTR_COLD;

	ioport_value keys_r();

	DECLARE_INPUT_CHANGED_MEMBER(dial_w);
	ioport_value dial_r();

	void rhythm_strobe_w(int state);
	ioport_value rhythm_ack_r() { return m_930_strobe; }

protected:
	void map(address_map &map) ATTR_COLD;

	virtual void driver_start() override ATTR_COLD;

	void keys_w(u8 data) { m_key_select = data; }
	void pll_w(offs_t offset, u8 data);
	void port_ctrl_w(u8 data);
	u8 port_status_r();

	u8 ppi_a_r();
	void ppi_b_w(u8 data);
	void ppi_c_w(u8 data);

	template<int Num> void port_strobe_w(int state);

	void update_int0();

	void rhythm_data_w(u8 data) { m_930_data = data; }

	required_device<upd7801_device> m_maincpu;
	required_device<i8255_device> m_ppi;

	required_device<ct8000_midi_device> m_port_upper;
	required_device<ct8000_midi_device> m_port_lower;
	required_device<ct8000_midi_device> m_port_bass;
	required_device<ct8000_midi_device> m_port_mb1;

	required_ioport_array<8> m_inputs;

	u8 m_port_in_select;
	u8 m_port_in_strobe;
	u8 m_port_irq_disable;
	u8 m_port_out_select;
	u8 m_ppi_pc;

	u8 m_key_select;
	u8 m_dial_pos;
	u8 m_930_data;
	u8 m_930_strobe;

	u16 m_pll_counter;
	u16 m_pll_ref;
};

//**************************************************************************
void ctrc1_state::map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).mirror(0x7000).ram().share("nvram");
	map(0x8000, 0x9fff).rom().region("program", 0);
	map(0x8000, 0x9fff).w(FUNC(ctrc1_state::keys_w));
	map(0xa000, 0xa003).mirror(0x1ffc).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc000, 0xc007).mirror(0x1ff8).w(FUNC(ctrc1_state::pll_w));
	map(0xe000, 0xffff).rw(FUNC(ctrc1_state::port_status_r), FUNC(ctrc1_state::port_ctrl_w));
}

//**************************************************************************
void ctrc1_state::ctrc1(machine_config &config)
{
	UPD7801(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ctrc1_state::map);
	m_maincpu->pa_out_cb().set(FUNC(ctrc1_state::rhythm_data_w));
	m_maincpu->pb_in_cb().set_ioport("PB");
	m_maincpu->pb_out_cb().set_ioport("PB");
	m_maincpu->pc_in_cb().set_ioport("PC");
	m_maincpu->pc_out_cb().set_ioport("PC");

	NVRAM(config, "nvram");

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(ctrc1_state::ppi_a_r));
	m_ppi->out_pb_callback().set(FUNC(ctrc1_state::ppi_b_w));
	m_ppi->out_pc_callback().set(FUNC(ctrc1_state::ppi_c_w));

	INPUT_MERGER_ANY_HIGH(config, "in_ack").output_handler().set(m_ppi, FUNC(i8255_device::pc2_w)).invert();

	CT8000_MIDI(config, m_port_upper);
	m_port_upper->ack_cb().set("in_ack", FUNC(input_merger_device::in_w<3>));
	m_port_upper->int_cb().set(FUNC(ctrc1_state::port_strobe_w<3>));

	CT8000_MIDI(config, m_port_lower);
	m_port_lower->ack_cb().set("in_ack", FUNC(input_merger_device::in_w<2>));
	m_port_lower->int_cb().set(FUNC(ctrc1_state::port_strobe_w<2>));

	CT8000_MIDI(config, m_port_bass);
	m_port_bass->ack_cb().set("in_ack", FUNC(input_merger_device::in_w<1>));
	m_port_bass->int_cb().set(FUNC(ctrc1_state::port_strobe_w<1>));

	CT8000_MIDI(config, m_port_mb1);
	m_port_mb1->ack_cb().set("in_ack", FUNC(input_merger_device::in_w<0>));
	m_port_mb1->int_cb().set(FUNC(ctrc1_state::port_strobe_w<0>));
}

//**************************************************************************
void ctrc1_state::driver_start()
{
	m_port_in_select = 0xf;
	m_port_in_strobe = 0;
	m_port_out_select = 0;
	m_port_irq_disable = 1;
	m_ppi_pc = 0xff;

	m_key_select = 0xff;

	m_930_data = 0xff;
	m_930_strobe = 1;
	
	m_pll_counter = 0;
	m_pll_ref = 0;

	save_item(NAME(m_port_in_select));
	save_item(NAME(m_port_in_strobe));
	save_item(NAME(m_port_out_select));
	save_item(NAME(m_port_irq_disable));
	save_item(NAME(m_ppi_pc));

	save_item(NAME(m_key_select));

	save_item(NAME(m_930_data));
	save_item(NAME(m_930_strobe));

	save_item(NAME(m_pll_counter));
	save_item(NAME(m_pll_ref));
}

//**************************************************************************
ioport_value ctrc1_state::keys_r()
{
	u8 data = 0xff;

	for (int i = 0; i < m_inputs.size(); i++)
		if (!BIT(m_key_select, i))
			data &= m_inputs[i]->read();

	return data;
}

//**************************************************************************
INPUT_CHANGED_MEMBER(ctrc1_state::dial_w)
{
	// convert to 2-bit gray code (one bit goes to INT2, one bit goes to PC2)
	const u8 val = (newval >> 6) ^ (newval >> 7);
	if (BIT(val ^ m_dial_pos, 1))
		m_maincpu->set_input_line(UPD7810_INTF2, BIT(val, 1));

	m_dial_pos = val;
}

//**************************************************************************
ioport_value ctrc1_state::dial_r()
{
	return BIT(m_dial_pos, 0);
}

//**************************************************************************
void ctrc1_state::rhythm_strobe_w(int state)
{
	if (!state && m_930_strobe)
		logerror("rhythm data: %02x\n", m_930_data);

	m_930_strobe = state;
}

//**************************************************************************
void ctrc1_state::pll_w(offs_t offset, u8 data)
{
	data &= 0xf;
	
	switch (offset)
	{
	case 0:
		m_pll_counter &= 0x3ff0;
		m_pll_counter |= data;
		break;

	case 1:
		m_pll_counter &= 0x3f0f;
		m_pll_counter |= (data << 4);
		break;

	case 2:
		m_pll_counter &= 0x30ff;
		m_pll_counter |= (data << 8);
		break;

	case 3:
		m_pll_counter &= 0x0fff;
		m_pll_counter |= ((data & 3) << 12);
		break;

	case 4:
		m_pll_ref &= 0xff0;
		m_pll_ref |= data;
		break;

	case 5:
		m_pll_ref &= 0xf0f;
		m_pll_ref |= (data << 4);
		break;

	case 6:
		m_pll_ref &= 0x0ff;
		m_pll_ref |= (data << 8);
		break;

	default:
		break;
	}

	if (m_pll_counter && m_pll_ref)
	{
		const double clock_scale = (double)m_pll_counter / m_pll_ref;

		// m_930->set_clock_scale(clock_scale);
		LOGMASKED(LOG_VCO, "VCO %.3f MHz\n", clock_scale * 4.946864);
	}
}

//**************************************************************************
void ctrc1_state::port_ctrl_w(u8 data)
{
	m_port_irq_disable = BIT(data, 3);
	m_port_in_select = data >> 4;
	update_int0();
}

//**************************************************************************
u8 ctrc1_state::port_status_r()
{
	u8 data = 0x09 | (m_port_in_strobe << 4);
	if (BIT(m_ppi_pc, 0))
		data |= 0x02;
	if (BIT(m_ppi_pc, 3))
		data |= 0x04;

	return data;
}

//**************************************************************************
u8 ctrc1_state::ppi_a_r()
{
	u8 data = 0x0f;

	if (BIT(~m_port_in_select, 0))
		data &= m_port_mb1->data_r();
	if (BIT(~m_port_in_select, 1))
		data &= m_port_bass->data_r();
	if (BIT(~m_port_in_select, 2))
		data &= m_port_lower->data_r();
	if (BIT(~m_port_in_select, 3))
		data &= m_port_upper->data_r();

	return data | 0xf0;
}

//**************************************************************************
void ctrc1_state::ppi_b_w(u8 data)
{
	m_port_out_select = data >> 4;

	data &= 0xf;
	m_port_mb1->data_w(data);
	m_port_bass->data_w(data);
	m_port_lower->data_w(data);
	m_port_upper->data_w(data);
}

//**************************************************************************
void ctrc1_state::ppi_c_w(u8 data)
{
	m_ppi_pc = data;

	const u8 obfb = BIT(data, 1);
	m_port_mb1->strobe_w(obfb | BIT(~m_port_out_select, 0));
	m_port_bass->strobe_w(obfb | BIT(~m_port_out_select, 1));
	m_port_lower->strobe_w(obfb | BIT(~m_port_out_select, 2));
	m_port_upper->strobe_w(obfb | BIT(~m_port_out_select, 3));

	const u8 ibfa = BIT(data, 5);
	m_port_mb1->ack_w(ibfa & BIT(~m_port_in_select, 0));
	m_port_bass->ack_w(ibfa & BIT(~m_port_in_select, 1));
	m_port_lower->ack_w(ibfa & BIT(~m_port_in_select, 2));
	m_port_upper->ack_w(ibfa & BIT(~m_port_in_select, 3));

	update_int0();
}

//**************************************************************************
template<int Num>
void ctrc1_state::port_strobe_w(int state)
{
	if (state)
		m_port_in_strobe |= (1 << Num);
	else
		m_port_in_strobe &= ~(1 << Num);

	update_int0();
}

//**************************************************************************
void ctrc1_state::update_int0()
{
	if (BIT(m_ppi_pc, 0) || BIT(m_ppi_pc, 3) || (!m_port_irq_disable && m_port_in_strobe))
		m_maincpu->set_input_line(UPD7810_INTF0, ASSERT_LINE);
	else
		m_maincpu->set_input_line(UPD7810_INTF0, CLEAR_LINE);

	if (~m_port_in_select & m_port_in_strobe)
		m_ppi->pc4_w(0);
	else
		m_ppi->pc4_w(1);
}


INPUT_PORTS_START(ctrc1)
	PORT_START("PB")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ctrc1_state::keys_r))
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) // uPD930 ready (also connected to INT1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ctrc1_state::rhythm_ack_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(ctrc1_state::rhythm_strobe_w))

	PORT_START("PC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) // upd930 reset
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) // chord volume
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ctrc1_state::dial_r))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) // tempo LED (red)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) // tempo LED (green)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) // ram cart read/write
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) // ram cart enable
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("TEMPO")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Tempo") PORT_SENSITIVITY(50) PORT_KEYDELTA(75) PORT_REVERSE PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ctrc1_state::dial_w), 0)

	PORT_START("KC0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm Select")

	PORT_START("KC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythm 8")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Casio Chord (Off)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Casio Chord (Fingered)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Casio Chord (On)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Casio Chord (Free Bass)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Memory")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rhythmic")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Continuous")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bass Variation")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Start/Stop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Synchro")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Arpeggio 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Arpeggio 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Auto Harmonize")

	PORT_START("KC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Fill In")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Ending")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Tune Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Tune Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Tone Mix")

	PORT_START("KC6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Registration 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Registration 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Registration Record")
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KC7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Registration 3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Registration 4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Registration Cancel")
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


ROM_START(ctrc1)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7801g-134.p6", 0x0000, 0x1000, CRC(5abb054f) SHA1(c8266e9506ff53e40d2a223fb3c305aa31995e8c))

	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("rc1_2764.d4", 0x0000, 0x2000, CRC(101946eb) SHA1(e1a4f7f4699a7148c678992ffa8a49116a6848a3))
ROM_END

} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY  FULLNAME          FLAGS
SYST( 1983, ctrc1,   0,      0,      ctrc1,   ctrc1,   ctrc1_state,  empty_init, "Casio", "Casiotone RC-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
