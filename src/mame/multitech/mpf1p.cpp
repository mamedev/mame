// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/************************************************\
* Multitech Micro Professor 1 Plus               *
*                                                *
*     CPU: Z80 @ 1.79 MHz                        *
*     ROM: 8-kilobyte ROM monitor                *
*     RAM: 4 kilobytes                           *
*   Input: 49 key keyboard                       *
* Storage: Cassette tape                         *
*   Video: 20x 16-segment VFD                    *
*   Sound: Speaker                               *
\************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/mpf1/slot.h"
#include "imagedev/cassette.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "mpf1p.lh"


namespace {

class mpf1p_state : public driver_device
{
public:
	mpf1p_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_vfd_pwm(*this, "vfd_pwm")
		, m_cassette(*this, "cassette")
		, m_key(*this, "PC%u", 1U)
		, m_special(*this, "SPECIAL")
		, m_leds(*this, "led%u", 0U)
	{ }

	void mpf1p(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_res );

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pwm_display_device> m_vfd_pwm;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<20> m_key;
	required_ioport m_special;
	output_finder<2> m_leds;

	void mpf1_step(address_map &map) ATTR_COLD;
	void mpf1p_io_map(address_map &map) ATTR_COLD;
	void mpf1p_map(address_map &map) ATTR_COLD;

	uint8_t step_r(offs_t offset);

	uint8_t ppi1_pc_r();
	void ppi1_pa_w(uint8_t data);
	void ppi1_pb_w(uint8_t data);
	void ppi1_pc_w(uint8_t data);

	uint8_t ppi2_pc_r();
	void ppi2_pa_w(uint8_t data);
	void ppi2_pb_w(uint8_t data);
	void ppi2_pc_w(uint8_t data);

	int m_break = 0;
	int m_m1 = 0;

	uint32_t m_select = 0;
	uint16_t m_vfd_data = 0;

	void vfd_refresh();
};


void mpf1p_state::mpf1p_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom().region("rom_u2", 0);
	map(0x2000, 0x3fff).r("rom_u3", FUNC(generic_slot_device::read_rom));
	map(0xf000, 0xffff).ram();
}

void mpf1p_state::mpf1_step(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(mpf1p_state::step_r));
}

void mpf1p_state::mpf1p_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).mirror(0x0c).rw("ppi_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x90, 0x93).mirror(0x0c).rw("ppi_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


INPUT_CHANGED_MEMBER( mpf1p_state::trigger_res )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( mpf1p )
	PORT_START("PC1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)     PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)     PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)     PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)     PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT)  PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)     PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)     PORT_CHAR('D')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)     PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)     PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN)  PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)     PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)     PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP)    PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)     PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)     PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)     PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)     PORT_CHAR('J')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)     PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)     PORT_CHAR('K') PORT_CHAR('^')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)     PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)     PORT_CHAR('L') PORT_CHAR('@')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)     PORT_CHAR('0') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR(';')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC11")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)     PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)     PORT_CHAR('Z')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC12")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)     PORT_CHAR('W')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)     PORT_CHAR('X')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC13")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)     PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)     PORT_CHAR('C')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC14")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)     PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)     PORT_CHAR('V')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC15")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)     PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)     PORT_CHAR('B')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC16")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)     PORT_CHAR('Y')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)     PORT_CHAR('N')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC17")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)     PORT_CHAR('U')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)     PORT_CHAR('M')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC18")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)     PORT_CHAR('I') PORT_CHAR('-')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC19")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)     PORT_CHAR('O') PORT_CHAR('=')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PC20")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)     PORT_CHAR('P') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?') PORT_CHAR('/')
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RESET")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_F3) PORT_CHANGED_MEMBER(DEVICE_SELF, mpf1p_state, trigger_res, 0)
INPUT_PORTS_END


void mpf1p_state::vfd_refresh()
{
	m_vfd_pwm->matrix(~m_select, ~m_vfd_data);
}


uint8_t mpf1p_state::ppi1_pc_r()
{
	uint8_t data = 0xff;

	// bit 4 and 5, shift and control keys
	data &= m_special->read();

	return data;
}

void mpf1p_state::ppi1_pa_w(uint8_t data)
{
	m_select = (m_select & 0xffffff00) | data;
	vfd_refresh();
}

void mpf1p_state::ppi1_pb_w(uint8_t data)
{
	m_select = (m_select & 0xffff00ff) | (data << 8);
	vfd_refresh();
}

void mpf1p_state::ppi1_pc_w(uint8_t data)
{
	m_select = (m_select & 0xff00ffff) | (data << 16);
	vfd_refresh();
}

uint8_t mpf1p_state::ppi2_pc_r()
{
	uint8_t data = 0xf7;

	// bit 0 to 2, keyboard rows 0 to 20
	for (int row = 0; row < 20; row++)
		if (!BIT(m_select, row))
			data &= m_key[row]->read();

	// bit 3, tape input
	data |= (m_cassette->input() > 0) ? 0x08 : 0x00;

	return data;
}

void mpf1p_state::ppi2_pa_w(uint8_t data)
{
	m_vfd_data = (m_vfd_data & 0xff00) | data;
	vfd_refresh();
}

void mpf1p_state::ppi2_pb_w(uint8_t data)
{
	// swap bits around for the 14-segment emulation
	data = bitswap<8>(data, 7, 6, 4, 2, 3, 5, 1, 0);

	m_vfd_data = (m_vfd_data & 0x00ff) | (data << 8);
	vfd_refresh();
}

void mpf1p_state::ppi2_pc_w(uint8_t data)
{
	// bit 4, monitor break control
	m_break = BIT(data, 4);

	if (m_break)
	{
		m_m1 = 0;
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}

	// bit 5, tape output, tone and led
	m_leds[0] = !BIT(data, 5);
	m_speaker->level_w(BIT(data, 5));
	m_cassette->output(BIT(data, 5) ? 1.0 : -1.0);
}


uint8_t mpf1p_state::step_r(offs_t offset)
{
	if (!m_break)
	{
		m_m1++;

		if (m_m1 == 5)
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}


void mpf1p_state::machine_start()
{
	m_leds.resolve();

	// register for state saving */
	save_item(NAME(m_break));
	save_item(NAME(m_m1));
	save_item(NAME(m_select));
}


void mpf1p_state::mpf1p(machine_config &config)
{
	Z80(config, m_maincpu, 3.579545_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpf1p_state::mpf1p_map);
	m_maincpu->set_addrmap(AS_OPCODES, &mpf1p_state::mpf1_step);
	m_maincpu->set_addrmap(AS_IO, &mpf1p_state::mpf1p_io_map);
	m_maincpu->halt_cb().set_output("led1");

	config.set_default_layout(layout_mpf1p);

	i8255_device &ppi_1(I8255A(config, "ppi_1"));
	ppi_1.out_pa_callback().set(FUNC(mpf1p_state::ppi1_pa_w));
	ppi_1.out_pb_callback().set(FUNC(mpf1p_state::ppi1_pb_w));
	ppi_1.out_pc_callback().set(FUNC(mpf1p_state::ppi1_pc_w));
	ppi_1.in_pc_callback().set(FUNC(mpf1p_state::ppi1_pc_r));

	i8255_device &ppi_2(I8255A(config, "ppi_2"));
	ppi_2.out_pa_callback().set(FUNC(mpf1p_state::ppi2_pa_w));
	ppi_2.out_pb_callback().set(FUNC(mpf1p_state::ppi2_pb_w));
	ppi_2.out_pc_callback().set(FUNC(mpf1p_state::ppi2_pc_w));
	ppi_2.in_pc_callback().set(FUNC(mpf1p_state::ppi2_pc_r));

	PWM_DISPLAY(config, m_vfd_pwm).set_size(20, 16);
	m_vfd_pwm->set_segmask(0xfffff, 0xffff);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	mpf1_exp_device &exp(MPF1_EXP(config, "exp", 3.579545_MHz_XTAL/2, mpf1p_exp_devices, nullptr));
	exp.set_program_space(m_maincpu, AS_PROGRAM);
	exp.set_io_space(m_maincpu, AS_IO);
	exp.int_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	exp.nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	exp.wait_handler().set_inputline(m_maincpu, Z80_INPUT_LINE_WAIT);

	GENERIC_SOCKET(config, "rom_u3", generic_linear_slot, "mpf1_rom", "bin,rom");

	SOFTWARE_LIST(config, "rom_ls").set_original("mpf1_rom").set_filter("IP");
}

} // anonymous namespace


ROM_START( mpf1p )
	ROM_REGION(0x2000, "rom_u2", 0)
	ROM_LOAD("mpf-1p_v1.1.u2", 0x0000, 0x2000, CRC(8bb241d3) SHA1(a05cf397452fe6a03e1ea9320985654f5910b20f))
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME                  FLAGS
COMP( 1983, mpf1p,  0,      0,      mpf1p,   mpf1p, mpf1p_state, empty_init, "Multitech", "Micro-Professor 1 Plus", 0 )
