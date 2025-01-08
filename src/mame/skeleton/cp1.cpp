// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Kosmos CP1

        06/03/2012 Skeleton driver.

        on board there is also 8155
        KEYBOARD Membrane keyboard, 20+10 keys
        6 * 7 seg led display

****************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/i8155.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "video/pwm.h"

#include "speaker.h"
#include "cp1.lh"


namespace {

class cp1_state : public driver_device
{
public:
	cp1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_i8155(*this, "i8155"),
		m_i8155_cp3(*this, "i8155_cp3"),
		m_cassette(*this, "cassette"),
		m_display(*this, "display"),
		m_io_lines(*this, "LINE%u", 0U),
		m_io_config(*this, "CONFIG")
	{ }

	void cp1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void cp1_io(address_map &map) ATTR_COLD;

	uint8_t port1_r();
	uint8_t port2_r();
	void port1_w(uint8_t data);
	void port2_w(uint8_t data);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	uint8_t i8155_read(offs_t offset);
	void i8155_write(offs_t offset, uint8_t data);
	void i8155_porta_w(uint8_t data);
	uint8_t i8155_portb_r();
	void i8155_portb_w(uint8_t data);
	void i8155_portc_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<i8155_device> m_i8155;
	required_device<i8155_device> m_i8155_cp3;
	required_device<cassette_image_device> m_cassette;
	required_device<pwm_display_device> m_display;
	required_ioport_array<5> m_io_lines;
	required_ioport m_io_config;

	uint8_t m_port2 = 0;
	uint8_t m_matrix = 0;
};

uint8_t cp1_state::port1_r()
{
	logerror("Read from expansion port 1\n");

	uint8_t data = 0;

	if (m_io_config->read() & 0x01)
		data |= (m_cassette->input() > 0.03) ? 0x80 : 0x00;

	return data;
}

void cp1_state::port1_w(uint8_t data)
{
	logerror("Write to expansion port 1 %x\n", data);

	if (m_io_config->read() & 0x01)
		m_cassette->output(data & 0x80 ? +1.0 : -1.0);
}

uint8_t cp1_state::port2_r()
{
	// x--- ----   I8155 IO/M
	// -x-- ----   I8155 RESET
	// --x- ----   expansion port CE
	// ---x ----   I8155 CE
	// ---- xxxx   keyboard input

	uint8_t data = 0;

	for (int i = 0; i < 5; i++)
		if (BIT(m_matrix, i))
			data |= m_io_lines[i]->read();

	return (data & 0x0f) | (m_port2 & 0xf0);
}

void cp1_state::port2_w(uint8_t data)
{
	if (data & 0x40)
	{
		m_i8155->reset();

		if (m_io_config->read() & 0x02)
			m_i8155_cp3->reset();
	}

	m_port2 = data;
}

uint8_t cp1_state::i8155_read(offs_t offset)
{
	uint8_t data = 0;

	if (!(m_port2 & 0x10))
	{
		m_i8155->ale_w(BIT(m_port2, 7), offset);
		data |= m_i8155->data_r();
	}
	if ((m_io_config->read() & 0x02) && !(m_port2 & 0x20))
	{
		// CP3 RAM expansion
		m_i8155_cp3->ale_w(BIT(m_port2, 7), offset);
		data |= m_i8155_cp3->data_r();
	}

	return data;
}

void cp1_state::i8155_write(offs_t offset, uint8_t data)
{
	if (!(m_port2 & 0x10))
	{
		m_i8155->ale_w(BIT(m_port2, 7), offset);
		m_i8155->data_w(data);
	}
	if ((m_io_config->read() & 0x02) && !(m_port2 & 0x20))
	{
		// CP3 RAM expansion
		m_i8155_cp3->ale_w(BIT(m_port2, 7), offset);
		m_i8155_cp3->data_w(data);
	}
}

void cp1_state::i8155_porta_w(uint8_t data)
{
	m_display->write_mx(data | 0x80); // PA7 is not connected
}

uint8_t cp1_state::i8155_portb_r()
{
	logerror("read from expansion port 2\n");
	return 0;
}

void cp1_state::i8155_portb_w(uint8_t data)
{
	logerror("Write to expansion port 2 %x\n", data);
}

void cp1_state::i8155_portc_w(uint8_t data)
{
	// --xx xxxx   keyboard matrix, 7seg select
	m_matrix = ~data & 0x3f;
	m_display->write_my(m_matrix);
}


void cp1_state::cp1_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0xff).rw(FUNC(cp1_state::i8155_read), FUNC(cp1_state::i8155_write));
}

/* Input ports */
INPUT_PORTS_START( cp1 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)   PORT_NAME("CAS [Cass. speichern]")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CLR [Irrtum]")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C)   PORT_NAME("PC [Programmzahler]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)   PORT_NAME("ACC [Akku]")
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L)   PORT_NAME("CAL [Cass. lesen]")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)   PORT_NAME("STEP [Schritt]")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B)   PORT_NAME("STP [Stopp]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)   PORT_NAME("RUN [Lauf]")
	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)   PORT_NAME("8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9)   PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O)   PORT_NAME("OUT [Auslesen]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)   PORT_NAME("INP [Eingeben]")
	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)   PORT_NAME("4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)   PORT_NAME("5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)   PORT_NAME("6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)   PORT_NAME("7")
	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0)   PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)   PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)   PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)   PORT_NAME("3")

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "CP2 Cassette Interface" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x01, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x02, 0x00, "CP3 RAM Expansion" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x02, DEF_STR( Yes ) )
INPUT_PORTS_END

void cp1_state::machine_start()
{
	save_item(NAME(m_port2));
	save_item(NAME(m_matrix));
}

void cp1_state::machine_reset()
{
	m_cassette->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}

QUICKLOAD_LOAD_MEMBER(cp1_state::quickload_cb)
{
	uint8_t byte = 0;
	unsigned addr = 0;

	char ch = '\0';
	uint32_t actual = 0;
	while ((actual = image.fread(&ch, 1)) != 0 && addr < 0x100)
	{
		// Format: nn.nnn<CR><LF>
		if (ch >= '0' && ch <= '9')
			byte = (byte * 10) + (ch - '0');
		else if (ch == '.' && (addr & 1) == 0)
		{
			m_i8155->memory_w(addr++, byte);
			byte = 0;
		}
		else if (ch == '\r' || ch == '\n')
		{
			if ((addr & 1) != 0)
				m_i8155->memory_w(addr++, byte);
			byte = 0;
		}
		else
		{
			return std::make_pair(image_error::INVALIDIMAGE, std::string());
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

void cp1_state::cp1(machine_config &config)
{
	/* basic machine hardware */
	i8049_device &maincpu(I8049(config, m_maincpu, 6_MHz_XTAL));
	maincpu.set_addrmap(AS_IO, &cp1_state::cp1_io);
	maincpu.p1_in_cb().set(FUNC(cp1_state::port1_r));
	maincpu.p1_out_cb().set(FUNC(cp1_state::port1_w));
	maincpu.p2_in_cb().set(FUNC(cp1_state::port2_r));
	maincpu.p2_out_cb().set(FUNC(cp1_state::port2_w));

	i8155_device &i8155(I8155(config, "i8155", 0));
	i8155.out_pa_callback().set(FUNC(cp1_state::i8155_porta_w));
	i8155.in_pb_callback().set(FUNC(cp1_state::i8155_portb_r));
	i8155.out_pb_callback().set(FUNC(cp1_state::i8155_portb_w));
	i8155.out_pc_callback().set(FUNC(cp1_state::i8155_portc_w));

	I8155(config, "i8155_cp3", 0);

	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0x3f, 0x7f);
	m_display->set_segmask(0x08, 0xff); // 3rd digit DP is always on
	config.set_default_layout(layout_cp1);

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	QUICKLOAD(config, "quickload", "obj", attotime::from_seconds(1)).set_load_callback(FUNC(cp1_state::quickload_cb));
}

/* ROM definition */
/*
  KOSMOS B
  <Mitsubishi Logo> M5L8049-136P-6
  JAPAN 83F301
*/

ROM_START( cp1 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "b", "b" )
	ROMX_LOAD( "cp1-kosmos-b.rom", 0x0000, 0x0800, CRC(fea8a2b2) SHA1(c987b79a7b90fcbd58b66a69e95913f2655a1f0d), ROM_BIOS(0))
	// This is from 2716 eprom that was on board with I8039 instead of I8049
	ROM_SYSTEM_BIOS( 1, "2716", "2716" )
	ROMX_LOAD( "cp1-2716.bin",     0x0000, 0x0800, CRC(3a2caf0e) SHA1(ff4befcf82a664950186d3af1843fdef70d2209f), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY   FULLNAME                 FLAGS
COMP( 1980, cp1,  0,      0,      cp1,     cp1,   cp1_state, empty_init, "Kosmos", "CP1 / Computer-Praxis", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
